#include "dinput8.h"
#include <shlwapi.h>
#include <time.h>

#pragma comment(lib, "shlwapi.lib")

// Real DInput8 function pointers
DirectInput8Create_t Real_DirectInput8Create = NULL;
DllCanUnloadNow_t Real_DllCanUnloadNow = NULL;
DllGetClassObject_t Real_DllGetClassObject = NULL;
DllRegisterServer_t Real_DllRegisterServer = NULL;
DllUnregisterServer_t Real_DllUnregisterServer = NULL;

static HMODULE g_hModule = NULL;
static HMODULE g_hCoreDll = NULL;
static HANDLE g_hInitThread = NULL;
static BOOL g_bDllAttached = FALSE;

// core.dll export types
typedef int  (*CoreInit_t)(void);
typedef int  (*CorePostInit_t)(void);
typedef int  (*CorePreInit_DInput_t)(void);

static CoreInit_t CoreInit = NULL;
static CorePostInit_t CorePostInit = NULL;
static CorePreInit_DInput_t CorePreInit_DInput = NULL;

// ---------------------------------------------------------------------------
// Logging
// ---------------------------------------------------------------------------
void DInput8Log(const char* fmt, ...)
{
    char logPath[MAX_PATH];
    GetModuleFileNameA(g_hModule, logPath, MAX_PATH);
    PathRemoveFileSpecA(logPath);
    PathAppendA(logPath, DINPUT8_LOG_FILE);

    FILE* fp = fopen(logPath, "a");
    if (!fp) return;

    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] ",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec);

    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);

    fprintf(fp, "\n");
    fclose(fp);
}

// ---------------------------------------------------------------------------
// Version detection — scan WatchDogs2.exe for version string
// ---------------------------------------------------------------------------
static void DInput8DetectVersion(void)
{
    HMODULE hGame = GetModuleHandleA("WatchDogs2.exe");
    if (!hGame) {
        DInput8Log("WatchDogs2.exe not found in memory yet.");
        return;
    }

    MODULEINFO mi = {0};
    if (!GetModuleInformation(GetCurrentProcess(), hGame, &mi, sizeof(mi))) {
        DInput8Log("Failed to get WatchDogs2.exe module info.");
        return;
    }

    BYTE* base = (BYTE*)mi.lpBaseOfDll;
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)base;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
    SIZE_T imageSize = nt->OptionalHeader.SizeOfImage;

    const char* versionPatterns[] = {
        "2.0.",
        "1.17.",
        "1.16.",
        "1.15.",
        NULL
    };

    for (int p = 0; versionPatterns[p] != NULL; p++) {
        for (SIZE_T i = 0; i < imageSize - 32; i++) {
            if (memcmp(base + i, versionPatterns[p], strlen(versionPatterns[p])) == 0) {
                char verBuf[64] = {0};
                memcpy(verBuf, base + i, sizeof(verBuf) - 1);
                // Trim to printable
                for (int c = 0; c < sizeof(verBuf) - 1; c++) {
                    if (verBuf[c] < 0x20 || verBuf[c] > 0x7E) {
                        verBuf[c] = '\0';
                        break;
                    }
                }
                DInput8Log("Detected version string: %s", verBuf);
                return;
            }
        }
    }

    DInput8Log("No version string found in game module.");
}

// ---------------------------------------------------------------------------
// Load real dinput8.dll from System32
// ---------------------------------------------------------------------------
BOOL DInput8LoadRealDll(void)
{
    char sysDir[MAX_PATH];
    if (!GetSystemDirectoryA(sysDir, MAX_PATH)) {
        DInput8Log("GetSystemDirectoryA failed (error %lu)", GetLastError());
        return FALSE;
    }

    char dllPath[MAX_PATH];
    PathCombineA(dllPath, sysDir, "dinput8.dll");

    HMODULE hReal = LoadLibraryA(dllPath);
    if (!hReal) {
        DInput8Log("Failed to load real dinput8.dll from %s (error %lu)", dllPath, GetLastError());
        return FALSE;
    }

    Real_DirectInput8Create = (DirectInput8Create_t)GetProcAddress(hReal, "DirectInput8CreateA");
    Real_DllCanUnloadNow = (DllCanUnloadNow_t)GetProcAddress(hReal, "DllCanUnloadNow");
    Real_DllGetClassObject = (DllGetClassObject_t)GetProcAddress(hReal, "DllGetClassObject");
    Real_DllRegisterServer = (DllRegisterServer_t)GetProcAddress(hReal, "DllRegisterServer");
    Real_DllUnregisterServer = (DllUnregisterServer_t)GetProcAddress(hReal, "DllUnregisterServer");

    if (!Real_DirectInput8Create) {
        DInput8Log("Failed to get DirectInput8CreateA proc address.");
        return FALSE;
    }

    DInput8Log("Loaded real dinput8.dll from %s", dllPath);
    return TRUE;
}

// ---------------------------------------------------------------------------
// core.dll path resolution
// ---------------------------------------------------------------------------
static BOOL DInput8GetCoreDllPath(char* outPath, DWORD outSize)
{
    if (!GetModuleFileNameA(g_hModule, outPath, outSize))
        return FALSE;
    PathRemoveFileSpecA(outPath);
    return PathAppendA(outPath, "core.dll");
}

// ---------------------------------------------------------------------------
// Init thread: loads core.dll and calls its exports
// ---------------------------------------------------------------------------
DWORD WINAPI DInput8InitThread(LPVOID param)
{
    (void)param;
    char corePath[MAX_PATH];

    // Wait for game initialization
    DInput8Log("Init thread: waiting 2 seconds for game init...");
    Sleep(2000);

    DInput8DetectVersion();

    // Resolve core.dll relative to this DLL
    if (!DInput8GetCoreDllPath(corePath, MAX_PATH)) {
        DInput8Log("Init thread: failed to resolve core.dll path.");
        return 1;
    }

    DInput8Log("Init thread: loading %s", corePath);
    g_hCoreDll = LoadLibraryA(corePath);
    if (!g_hCoreDll) {
        DInput8Log("Init thread: LoadLibraryA failed for core.dll (error %lu)", GetLastError());
        return 1;
    }

    CoreInit = (CoreInit_t)GetProcAddress(g_hCoreDll, "Init");
    CorePostInit = (CorePostInit_t)GetProcAddress(g_hCoreDll, "PostInit");
    CorePreInit_DInput = (CorePreInit_DInput_t)GetProcAddress(g_hCoreDll, "PreInit_DInput");

    DInput8Log("Init thread: core.dll loaded (Init=%p, PostInit=%p, PreInit_DInput=%p)",
        (void*)CoreInit, (void*)CorePostInit, (void*)CorePreInit_DInput);

    // Call PreInit_DInput first
    if (CorePreInit_DInput) {
        DInput8Log("Init thread: calling PreInit_DInput()...");
        int result = CorePreInit_DInput();
        DInput8Log("Init thread: PreInit_DInput returned %d", result);
    } else {
        DInput8Log("Init thread: PreInit_DInput export not found, skipping.");
    }

    // Call Init if available
    if (CoreInit) {
        DInput8Log("Init thread: calling Init()...");
        int result = CoreInit();
        DInput8Log("Init thread: Init returned %d", result);
    }

    // Poll for game readiness, then call PostInit
    DInput8Log("Init thread: waiting for game to become ready...");
    for (int i = 0; i < 60; i++) {
        Sleep(1000);
        HWND hWnd = FindWindowA("LaunchUnrealUWindowsClient", NULL);
        if (!hWnd)
            hWnd = FindWindowA(NULL, "Watch Dogs 2");
        if (hWnd) {
            DInput8Log("Init thread: game window found at attempt %d", i + 1);
            break;
        }
    }

    // Additional settle time
    Sleep(2000);

    if (CorePostInit) {
        DInput8Log("Init thread: calling PostInit()...");
        int result = CorePostInit();
        DInput8Log("Init thread: PostInit returned %d", result);
    } else {
        DInput8Log("Init thread: PostInit export not found, skipping.");
    }

    DInput8Log("Init thread: complete.");
    return 0;
}

// ---------------------------------------------------------------------------
// Proxy exports
// ---------------------------------------------------------------------------
HRESULT WINAPI Proxy_DirectInput8CreateA(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
    DInput8Log("DirectInput8CreateA called");
    if (!Real_DirectInput8Create) {
        DInput8Log("DirectInput8CreateA: real function not loaded!");
        return E_FAIL;
    }
    return Real_DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

HRESULT WINAPI Proxy_DllCanUnloadNow(void)
{
    if (!Real_DllCanUnloadNow) return S_FALSE;
    return Real_DllCanUnloadNow();
}

HRESULT WINAPI Proxy_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    if (!Real_DllGetClassObject) return CLASS_E_CLASSNOTAVAILABLE;
    return Real_DllGetClassObject(rclsid, riid, ppv);
}

HRESULT WINAPI Proxy_DllRegisterServer(void)
{
    DInput8Log("DllRegisterServer called (no-op)");
    return S_OK;
}

HRESULT WINAPI Proxy_DllUnregisterServer(void)
{
    DInput8Log("DllUnregisterServer called (no-op)");
    return S_OK;
}

// ---------------------------------------------------------------------------
// DllMain
// ---------------------------------------------------------------------------
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    (void)lpReserved;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        g_hModule = hModule;

        DInput8Log("=== dinput8.dll proxy loaded ===");
        DInput8Log("PID: %lu", GetCurrentProcessId());

        if (!DInput8LoadRealDll()) {
            DInput8Log("FATAL: Could not load real dinput8.dll!");
            return FALSE;
        }

        g_bDllAttached = TRUE;

        // Start initialization thread
        g_hInitThread = CreateThread(NULL, 0, DInput8InitThread, NULL, 0, NULL);
        if (!g_hInitThread) {
            DInput8Log("Failed to create init thread (error %lu)", GetLastError());
            return FALSE;
        }

        DInput8Log("Init thread started.");
        break;

    case DLL_PROCESS_DETACH:
        DInput8Log("=== dinput8.dll proxy detaching ===");

        if (g_hInitThread) {
            WaitForSingleObject(g_hInitThread, 5000);
            CloseHandle(g_hInitThread);
            g_hInitThread = NULL;
        }

        if (g_hCoreDll) {
            FreeLibrary(g_hCoreDll);
            g_hCoreDll = NULL;
        }

        g_bDllAttached = FALSE;
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}
