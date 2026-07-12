#include "lua_engine.h"
#include "script_loader.h"
#include "game_api.h"

static int lua_print_redirect(lua_State *L);
static lua_State *g_luaState = NULL;
static WD2_SCRIPT_ENTRY g_scripts[WD2_MAX_SCRIPTS];
static int g_scriptCount = 0;

static int wd2_lua_panic(lua_State *L) {
    const char *msg = lua_tostring(L, -1);
    wd2_log_error("Lua panic: %s", msg ? msg : "(no message)");
    return 0;
}

static int wd2_lua_error_handler(lua_State *L) {
    const char *msg = lua_tostring(L, 1);
    if (!msg) msg = "unknown error";
    lua_Debug ar;
    if (lua_getstack(L, 0, &ar)) {
        lua_getinfo(L, "nSl", &ar);
        wd2_log_error("Lua error: %s in %s:%d", msg,
                      ar.short_src ? ar.short_src : "?",
                      ar.currentline);
    } else {
        wd2_log_error("Lua error: %s", msg);
    }
    return 1;
}

BOOL wd2_lua_init(void) {
    if (g_luaState) {
        wd2_log_warn("Lua VM already initialized");
        return TRUE;
    }

    g_luaState = luaL_newstate();
    if (!g_luaState) {
        wd2_log_error("Failed to create Lua state");
        return FALSE;
    }

    lua_atpanic(g_luaState, wd2_lua_panic);
    luaL_openlibs(g_luaState);

    lua_pushstring(g_luaState, "WD2_SCRIPTHOOK_VERSION");
    lua_pushstring(g_luaState, WD2_VERSION_STRING);
    lua_rawset(g_luaState, LUA_GLOBALSINDEX);

    lua_pushstring(g_luaState, "print");
    lua_pushcfunction(g_luaState, lua_print_redirect);
    lua_rawset(g_luaState, LUA_GLOBALSINDEX);

    wd2_log_info("Lua VM created, version: %s", LUA_VERSION);
    return TRUE;
}

void wd2_lua_shutdown(void) {
    if (!g_luaState) return;

    wd2_log_info("Shutting down Lua VM");

    for (int i = 0; i < g_scriptCount; i++) {
        if (g_scripts[i].bLoaded && g_scripts[i].iRef != LUA_NOREF) {
            luaL_unref(g_luaState, LUA_REGISTRYINDEX, g_scripts[i].iRef);
        }
    }
    g_scriptCount = 0;

    lua_close(g_luaState);
    g_luaState = NULL;
    wd2_log_info("Lua VM destroyed");
}

lua_State *wd2_lua_get_state(void) {
    return g_luaState;
}

int wd2_lua_pcall(int nargs, int nresults) {
    if (!g_luaState) return -1;

    int errfunc = 0;
    lua_pushcfunction(g_luaState, wd2_lua_error_handler);
    if (nargs > 0) {
        lua_insert(g_luaState, -(nargs + 2));
    }
    errfunc = lua_gettop(g_luaState) - nargs - 1;

    int status = lua_pcall(g_luaState, nargs, nresults, errfunc);
    if (status != 0) {
        const char *err = lua_tostring(g_luaState, -1);
        wd2_log_error("Lua pcall error [%d]: %s", status, err ? err : "(no msg)");
        lua_pop(g_luaState, 1);
    }
    return status;
}

BOOL wd2_lua_load_script(const char *path) {
    if (!g_luaState || !path) return FALSE;

    FILE *f = fopen(path, "rb");
    if (!f) {
        wd2_log_error("Cannot open script: %s", path);
        return FALSE;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        wd2_log_warn("Empty script: %s", path);
        return FALSE;
    }

    char *source = (char *)malloc(size + 1);
    if (!source) {
        fclose(f);
        return FALSE;
    }

    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);

    char chunk_name[260];
    snprintf(chunk_name, sizeof(chunk_name), "@%s", path);

    int status = luaL_loadbuffer(g_luaState, source, size, chunk_name);
    free(source);

    if (status != 0) {
        const char *err = lua_tostring(g_luaState, -1);
        wd2_log_error("Load error in %s: %s", path, err ? err : "unknown");
        lua_pop(g_luaState, 1);
        return FALSE;
    }

    lua_pushcfunction(g_luaState, wd2_lua_error_handler);
    lua_insert(g_luaState, -2);

    status = lua_pcall(g_luaState, 0, 0, -2);
    if (status != 0) {
        const char *err = lua_tostring(g_luaState, -1);
        wd2_log_error("Exec error in %s: %s", path, err ? err : "unknown");
        lua_pop(g_luaState, 1);
        return FALSE;
    }
    lua_pop(g_luaState, 1);

    if (g_scriptCount < WD2_MAX_SCRIPTS) {
        WD2_SCRIPT_ENTRY *entry = &g_scripts[g_scriptCount];
        strncpy(entry->szPath, path, MAX_PATH - 1);
        const char *fname = strrchr(path, '\\');
        if (!fname) fname = strrchr(path, '/');
        strncpy(entry->szName, fname ? fname + 1 : path, sizeof(entry->szName) - 1);
        GetFileTimeA(path, &entry->ftLastWrite);
        entry->bLoaded = TRUE;
        entry->iRef = LUA_NOREF;
        g_scriptCount++;
    }

    wd2_log_info("Loaded script: %s", path);
    return TRUE;
}

BOOL wd2_lua_load_scripts_dir(const char *dir) {
    if (!dir) return FALSE;

    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s*.lua", dir);

    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        wd2_log_info("No scripts found in: %s", dir);
        return FALSE;
    }

    int loaded = 0;
    do {
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (find_data.cFileName[0] == '.') continue;

        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s%s", dir, find_data.cFileName);

        if (wd2_lua_load_script(full_path)) {
            loaded++;
        }
    } while (FindNextFileA(hFind, &find_data));

    FindClose(hFind);
    wd2_log_info("Loaded %d scripts from %s", loaded, dir);
    return loaded > 0;
}

int wd2_lua_on_render(void) {
    if (!g_luaState) return 0;

    lua_getglobal(g_luaState, "OnRender");
    if (!lua_isfunction(g_luaState, -1)) {
        lua_pop(g_luaState, 1);
        return 0;
    }

    int status = lua_pcall(g_luaState, 0, 0, 0);
    if (status != 0) {
        const char *err = lua_tostring(g_luaState, -1);
        wd2_log_error("OnRender error: %s", err ? err : "unknown");
        lua_pop(g_luaState, 1);
        return 0;
    }
    return 1;
}

int wd2_lua_on_update(void) {
    if (!g_luaState) return 0;

    lua_getglobal(g_luaState, "OnUpdate");
    if (!lua_isfunction(g_luaState, -1)) {
        lua_pop(g_luaState, 1);
        return 0;
    }

    int status = lua_pcall(g_luaState, 0, 0, 0);
    if (status != 0) {
        const char *err = lua_tostring(g_luaState, -1);
        wd2_log_error("OnUpdate error: %s", err ? err : "unknown");
        lua_pop(g_luaState, 1);
        return 0;
    }
    return 1;
}

int wd2_lua_on_event(const char *event, ...) {
    if (!g_luaState || !event) return 0;

    lua_getglobal(g_luaState, "OnEvent");
    if (!lua_isfunction(g_luaState, -1)) {
        lua_pop(g_luaState, 1);
        return 0;
    }

    lua_pushstring(g_luaState, event);

    va_list args;
    va_start(args, event);
    int nargs = 1;
    const char *arg;
    while ((arg = va_arg(args, const char *)) != NULL && nargs < 10) {
        lua_pushstring(g_luaState, arg);
        nargs++;
    }
    va_end(args);

    int status = lua_pcall(g_luaState, nargs, 0, 0);
    if (status != 0) {
        const char *err = lua_tostring(g_luaState, -1);
        wd2_log_error("OnEvent error: %s", err ? err : "unknown");
        lua_pop(g_luaState, 1);
        return 0;
    }
    return 1;
}

void wd2_lua_hot_reload_check(void) {
    if (!g_luaState) return;

    for (int i = 0; i < g_scriptCount; i++) {
        if (!g_scripts[i].bLoaded) continue;

        FILETIME ftNew;
        if (GetFileTimeA(g_scripts[i].szPath, &ftNew)) {
            if (CompareFileTime(&ftNew, &g_scripts[i].ftLastWrite) > 0) {
                wd2_log_info("Script changed, reloading: %s", g_scripts[i].szName);
                if (g_scripts[i].iRef != LUA_NOREF) {
                    luaL_unref(g_luaState, LUA_REGISTRYINDEX, g_scripts[i].iRef);
                }
                g_scripts[i].bLoaded = FALSE;
                g_scripts[i].iRef = LUA_NOREF;
                wd2_lua_load_script(g_scripts[i].szPath);
            }
        }
    }
}

static int lua_print_redirect(lua_State *L) {
    int n = lua_gettop(L);
    const char *sep = "";
    for (int i = 1; i <= n; i++) {
        const char *s = lua_tostring(L, i);
        if (!s) {
            s = lua_typename(L, lua_type(L, i));
        }
        printf("%s%s", sep, s);
        sep = "\t";
    }
    printf("\n");
    return 0;
}

void wd2_lua_register_api(void) {
    if (!g_luaState) return;
    wd2_game_api_register_all();
}
