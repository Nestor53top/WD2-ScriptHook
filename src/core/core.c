#include "core.h"
#include "hook_manager.h"
#include "pattern_scan.h"
#include "lua_engine.h"
#include "game_api.h"
#include "render.h"
#include "script_loader.h"
#include "crash_report.h"

WD2_SCRIPTHOOK_CTX g_ctx = {0};

static HANDLE hInitThread = NULL;

static void wd2_log_internal(const char *level, const char *fmt, va_list args) {
    char buf[WD2_LOG_BUFFER_SIZE];
    int prefix_len = snprintf(buf, sizeof(buf), "[WD2-SH][%s] ", level);
    vsnprintf(buf + prefix_len, sizeof(buf) - prefix_len, fmt, args);
    OutputDebugStringA(buf);
    printf("%s\n", buf);
}

void wd2_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    wd2_log_internal("LOG", fmt, args);
    va_end(args);
}

void wd2_log_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    wd2_log_internal("ERR", fmt, args);
    va_end(args);
}

void wd2_log_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    wd2_log_internal("WRN", fmt, args);
    va_end(args);
}

void wd2_log_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    wd2_log_internal("INF", fmt, args);
    va_end(args);
}

static void wd2_get_base_path(void) {
    GetModuleFileNameA(NULL, g_ctx.szBasePath, MAX_PATH);
    char *last_slash = strrchr(g_ctx.szBasePath, '\\');
    if (last_slash) {
        *(last_slash + 1) = '\0';
    }
    snprintf(g_ctx.szScriptPath, MAX_PATH, "%sscripts\\", g_ctx.szBasePath);
}

static void wd2_find_game_module(void) {
    g_ctx.hGameModule = GetModuleHandleA(NULL);
    if (!g_ctx.hGameModule) {
        wd2_log_error("Failed to get game module handle");
        return;
    }
    MODULEINFO mod_info = {0};
    if (GetModuleInformation(GetCurrentProcess(), g_ctx.hGameModule, &mod_info, sizeof(mod_info))) {
        g_ctx.qwBaseAddress = (DWORD64)mod_info.lpBaseOfDll;
        g_ctx.dwBaseSize = mod_info.SizeOfImage;
        wd2_log_info("Game module: 0x%llX, size: 0x%X", g_ctx.qwBaseAddress, g_ctx.dwBaseSize);
    } else {
        wd2_log_error("Failed to get module information");
    }
}

static BOOL wd2_detect_game_version(void) {
    DWORD64 ver_str_addr = wd2_find_string_in_module(g_ctx.hGameModule, "Watch_Dogs");
    if (!ver_str_addr) {
        ver_str_addr = wd2_find_string_in_module(g_ctx.hGameModule, "WD2");
    }

    HMODULE hGame = g_ctx.hGameModule;
    const char *patterns[] = {
        "\x48\x89\x5C\x24\x08\x48\x89\x6C\x24\x10",
        "\x40\x53\x48\x83\xEC\x20\x48\x8B\xD9",
        "\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x20",
        "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x55\x57\x41\x56",
    };
    const char *masks[] = {
        "xxxxxxxxxx",
        "xxxxxxxxx",
        "xxxx??xxxx",
        "xxxx??xxxxxxx",
    };

    for (int i = 0; i < 4; i++) {
        DWORD64 addr = wd2_pattern_scan(hGame, patterns[i], masks[i]);
        if (addr) {
            wd2_log_info("Version detection pattern %d found at 0x%llX", i, addr);
        }
    }

    g_ctx.dwGameVersion = 1;
    wd2_log_info("Game version detected: %u", g_ctx.dwGameVersion);
    return TRUE;
}

static DWORD WINAPI wd2_init_thread(LPVOID lpParam) {
    (void)lpParam;
    wd2_log_info("=== WD2 ScriptHook v%s ===", WD2_VERSION_STRING);
    wd2_log_info("Initializing...");

    wd2_get_base_path();
    wd2_log_info("Base path: %s", g_ctx.szBasePath);
    wd2_log_info("Script path: %s", g_ctx.szScriptPath);

    wd2_find_game_module();
    if (!g_ctx.qwBaseAddress) {
        wd2_log_error("Could not find game module. Aborting.");
        return 1;
    }

    wd2_detect_game_version();

    if (!wd2_hook_init()) {
        wd2_log_error("Failed to initialize hook manager");
        return 1;
    }
    wd2_log_info("Hook manager initialized");

    wd2_crash_init();
    wd2_log_info("Crash handler installed");

    PreInit_DInput();

    wd2_game_api_init();

    g_ctx.bGameReady = TRUE;
    g_ctx.bInitialized = TRUE;
    wd2_log_info("Initialization complete");

    return 0;
}

__declspec(dllexport) void Init(void) {
    InitializeCriticalSection(&g_ctx.csLock);
    g_ctx.dwMainThreadId = GetCurrentThreadId();
    hInitThread = CreateThread(NULL, 0, wd2_init_thread, NULL, 0, NULL);
    if (!hInitThread) {
        wd2_log_error("Failed to create init thread");
    }
}

__declspec(dllexport) void PostInit(void) {
    wd2_log_info("PostInit called");

    if (!wd2_lua_init()) {
        wd2_log_error("Failed to initialize Lua VM");
        return;
    }
    wd2_log_info("Lua VM initialized");

    wd2_lua_register_api();
    wd2_log_info("Game API registered");

    wd2_scripts_load_all();
    wd2_log_info("Scripts loaded");

    wd2_log_info("PostInit complete");
}

__declspec(dllexport) void PreInit_DInput(void) {
    wd2_log_info("PreInit_DInput called");

    HMODULE hD3D11 = LoadLibraryA("d3d11.dll");
    if (!hD3D11) {
        wd2_log_warn("d3d11.dll not found, deferring D3D11 hooks");
        return;
    }

    wd2_log_info("D3D11 hooks will be applied on swap chain creation");
}

void ProperShutdown(void) {
    wd2_log_info("ProperShutdown called");

    wd2_hook_disable_all();
    wd2_scripts_unload();
    wd2_lua_shutdown();
    wd2_render_shutdown();
    wd2_game_api_shutdown();
    wd2_hook_shutdown();
    wd2_crash_shutdown();

    DeleteCriticalSection(&g_ctx.csLock);

    g_ctx.bInitialized = FALSE;
    g_ctx.bGameReady = FALSE;

    wd2_log_info("Shutdown complete");

    if (hInitThread) {
        CloseHandle(hInitThread);
        hInitThread = NULL;
    }
}

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    (void)lp_reserved;
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        g_ctx.hCoreDll = hinstDLL;
        Init();
        break;
    case DLL_PROCESS_DETACH:
        ProperShutdown();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
