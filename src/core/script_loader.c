#include "script_loader.h"
#include "lua_engine.h"

typedef struct _LOADED_SCRIPT {
    char szPath[MAX_PATH];
    char szName[128];
    BOOL bLoaded;
    FILETIME ftLastWrite;
} LOADED_SCRIPT;

static LOADED_SCRIPT g_loadedScripts[WD2_MAX_SCRIPTS];
static int g_loadedCount = 0;

static void wd2_scripts_scan_dir(const char *dir, int depth) {
    if (depth > 5) return;

    char search[MAX_PATH];
    snprintf(search, sizeof(search), "%s*", dir);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (fd.cFileName[0] == '.') continue;

        char full[MAX_PATH];
        snprintf(full, sizeof(full), "%s%s", dir, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            wd2_scripts_scan_dir(full, depth + 1);
        } else {
            const char *ext = strrchr(fd.cFileName, '.');
            if (ext && _stricmp(ext, ".lua") == 0) {
                if (g_loadedCount >= WD2_MAX_SCRIPTS) {
                    wd2_log_warn("Script limit reached, skipping: %s", full);
                    continue;
                }

                BOOL already_loaded = FALSE;
                for (int i = 0; i < g_loadedCount; i++) {
                    if (_stricmp(g_loadedScripts[i].szPath, full) == 0) {
                        already_loaded = TRUE;
                        break;
                    }
                }
                if (already_loaded) continue;

                LOADED_SCRIPT *s = &g_loadedScripts[g_loadedCount];
                strncpy(s->szPath, full, MAX_PATH - 1);
                strncpy(s->szName, fd.cFileName, sizeof(s->szName) - 1);
                GetFileTimeA(full, &s->ftLastWrite);
                s->bLoaded = FALSE;
                g_loadedCount++;
            }
        }
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
}

void wd2_scripts_load_all(void) {
    wd2_log_info("Loading scripts...");

    g_loadedCount = 0;
    wd2_scripts_scan_dir(g_ctx.szScriptPath, 0);

    wd2_log_info("Found %d scripts", g_loadedCount);

    for (int i = 0; i < g_loadedCount; i++) {
        if (wd2_lua_load_script(g_loadedScripts[i].szPath)) {
            g_loadedScripts[i].bLoaded = TRUE;
        } else {
            wd2_log_error("Failed to load: %s", g_loadedScripts[i].szName);
        }
    }

    int loaded = 0;
    for (int i = 0; i < g_loadedCount; i++) {
        if (g_loadedScripts[i].bLoaded) loaded++;
    }
    wd2_log_info("Loaded %d/%d scripts", loaded, g_loadedCount);
}

void wd2_scripts_reload(void) {
    wd2_log_info("Reloading scripts...");
    wd2_scripts_unload();
    wd2_scripts_load_all();
}

void wd2_scripts_unload(void) {
    wd2_log_info("Unloading %d scripts", g_loadedCount);

    lua_State *L = wd2_lua_get_state();
    if (L) {
        lua_getglobal(L, "__wd2_commands");
        if (lua_istable(L, -1)) {
            lua_pushnil(L);
            while (lua_next(L, -2) != 0) {
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);
    }

    for (int i = 0; i < g_loadedCount; i++) {
        g_loadedScripts[i].bLoaded = FALSE;
    }
    g_loadedCount = 0;
}

void wd2_scripts_get_list(char *buf, int bufsize) {
    if (!buf || bufsize <= 0) return;

    buf[0] = '\0';
    int offset = 0;

    for (int i = 0; i < g_loadedCount; i++) {
        int written = snprintf(buf + offset, bufsize - offset, "%s%s [%s]\n",
                               i > 0 ? ", " : "",
                               g_loadedScripts[i].szName,
                               g_loadedScripts[i].bLoaded ? "OK" : "FAIL");
        if (written < 0 || offset + written >= bufsize) break;
        offset += written;
    }
}
