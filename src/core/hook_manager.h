#pragma once

#include "core.h"

typedef struct _WD2_HOOK_ENTRY {
    void *pTarget;
    void *pDetour;
    void *pOriginal;
    BOOL  bEnabled;
    int   iIndex;
    char  szName[64];
} WD2_HOOK_ENTRY;

extern WD2_HOOK_ENTRY g_hooks[WD2_MAX_HOOKS];
extern int g_hookCount;

int  wd2_hook_init(void);
void wd2_hook_shutdown(void);
int  wd2_hook_create(void *target, void *detour, void **original, const char *name);
void wd2_hook_remove(int index);
void wd2_hook_enable_all(void);
void wd2_hook_disable_all(void);
BOOL wd2_hook_enable(int index);
BOOL wd2_hook_disable(int index);
void *wd2_hook_get_original(int index);
