#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <shlwapi.h>
#include <psapi.h>

#include "../lua/lua.h"
#include "../lua/lualib.h"
#include "../lua/lauxlib.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "psapi.lib")

#define WD2_VERSION_MAJOR 1
#define WD2_VERSION_MINOR 0
#define WD2_VERSION_PATCH 0
#define WD2_VERSION_STRING "1.0.0"

#define WD2_MAX_HOOKS 256
#define WD2_MAX_SCRIPTS 64
#define WD2_MAX_PATH_LEN 260
#define WD2_LOG_BUFFER_SIZE 4096

typedef struct _WD2_SCRIPTHOOK_CTX {
    HMODULE         hGameModule;
    lua_State      *L;
    HWND            hWindow;
    ID3D11Device   *pDevice;
    ID3D11DeviceContext *pContext;
    IDXGISwapChain *pSwapChain;
    HMODULE         hCoreDll;
    HMODULE         hSkiDll;
    HMODULE         hCrashDll;
    char            szBasePath[MAX_PATH];
    char            szScriptPath[MAX_PATH];
    BOOL            bInitialized;
    BOOL            bGameReady;
    DWORD           dwGameVersion;
    DWORD64         qwBaseAddress;
    DWORD           dwBaseSize;
    FARPROC         pfnOriginalPresent;
    FARPROC         pfnOriginalResizeBuffers;
    FARPROC         pfnOriginalCreateSwapChain;
    DWORD           dwMainThreadId;
    CRITICAL_SECTION csLock;
} WD2_SCRIPTHOOK_CTX;

extern WD2_SCRIPTHOOK_CTX g_ctx;

void wd2_log(const char *fmt, ...);
void wd2_log_error(const char *fmt, ...);
void wd2_log_warn(const char *fmt, ...);
void wd2_log_info(const char *fmt, ...);

DWORD64 wd2_pattern_scan(HMODULE hModule, const char *pattern, const char *mask);
DWORD64 wd2_pattern_scan_range(DWORD64 start, DWORD64 end, const char *pattern, const char *mask);
DWORD64 wd2_find_string_in_module(HMODULE hModule, const char *str);
DWORD64 wd2_find_relative_call(DWORD64 addr);

int  wd2_hook_create(void *target, void *detour, void **original, const char *name);
void wd2_hook_remove(int index);
void wd2_hook_enable_all(void);
void wd2_hook_disable_all(void);
int  wd2_hook_init(void);
void wd2_hook_shutdown(void);

BOOL wd2_lua_init(void);
void wd2_lua_shutdown(void);
BOOL wd2_lua_load_script(const char *path);
BOOL wd2_lua_load_scripts_dir(const char *dir);
int  wd2_lua_pcall(int nargs, int nresults);
lua_State *wd2_lua_get_state(void);
int  wd2_lua_on_render(void);
int  wd2_lua_on_update(void);
int  wd2_lua_on_event(const char *event, ...);
void wd2_lua_register_api(void);

BOOL wd2_render_init(IDXGISwapChain *pSwapChain);
void wd2_render_shutdown(void);
void wd2_render_begin_frame(void);
void wd2_render_end_frame(void);
void wd2_render_draw_text(float x, float y, const char *text, float r, float g, float b, float a, float size);
void wd2_render_draw_rect(float x, float y, float w, float h, float r, float g, float b, float a);
void wd2_render_draw_line(float x1, float y1, float x2, float y2, float r, float g, float b, float a, float thickness);
void wd2_render_draw_circle(float cx, float cy, float radius, float r, float g, float b, float a);

void wd2_scripts_load_all(void);
void wd2_scripts_reload(void);
void wd2_scripts_unload(void);
void wd2_scripts_get_list(char *buf, int bufsize);

void wd2_crash_init(void);
void wd2_crash_shutdown(void);

__declspec(dllexport) void __cdecl Init(void);
__declspec(dllexport) void __cdecl PostInit(void);
__declspec(dllexport) void __cdecl PreInit_DInput(void);
void ProperShutdown(void);

void wd2_game_api_register_all(void);
void wd2_game_api_init(void);
void wd2_game_api_shutdown(void);
