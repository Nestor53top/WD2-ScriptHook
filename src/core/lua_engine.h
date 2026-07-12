#pragma once

#include "core.h"

typedef struct _WD2_SCRIPT_ENTRY {
    char szPath[MAX_PATH];
    char szName[128];
    FILETIME ftLastWrite;
    BOOL bLoaded;
    int  iRef;
} WD2_SCRIPT_ENTRY;

BOOL         wd2_lua_init(void);
void         wd2_lua_shutdown(void);
BOOL         wd2_lua_load_script(const char *path);
BOOL         wd2_lua_load_scripts_dir(const char *dir);
int          wd2_lua_pcall(int nargs, int nresults);
lua_State   *wd2_lua_get_state(void);
int          wd2_lua_on_render(void);
int          wd2_lua_on_update(void);
int          wd2_lua_on_event(const char *event, ...);
void         wd2_lua_register_api(void);
void         wd2_lua_hot_reload_check(void);
