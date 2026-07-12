#include "hook_manager.h"

#include "../minhook/MinHook.h"

WD2_HOOK_ENTRY g_hooks[WD2_MAX_HOOKS] = {0};
int g_hookCount = 0;
static BOOL g_mhInitialized = FALSE;

int wd2_hook_init(void) {
    if (g_mhInitialized) return 1;
    MH_STATUS st = MH_Initialize();
    if (st != MH_OK) {
        wd2_log_error("MH_Initialize failed: %d", st);
        return 0;
    }
    g_mhInitialized = TRUE;
    wd2_log_info("MinHook initialized");
    return 1;
}

void wd2_hook_shutdown(void) {
    if (!g_mhInitialized) return;
    wd2_hook_disable_all();
    for (int i = 0; i < g_hookCount; i++) {
        if (g_hooks[i].pTarget) {
            MH_RemoveHook(g_hooks[i].pTarget);
            g_hooks[i].pTarget = NULL;
            g_hooks[i].pDetour = NULL;
            g_hooks[i].pOriginal = NULL;
            g_hooks[i].bEnabled = FALSE;
        }
    }
    g_hookCount = 0;
    MH_Uninitialize();
    g_mhInitialized = FALSE;
    wd2_log_info("Hook manager shut down");
}

int wd2_hook_create(void *target, void *detour, void **original, const char *name) {
    if (!g_mhInitialized) {
        wd2_log_error("Hook manager not initialized");
        return -1;
    }
    if (g_hookCount >= WD2_MAX_HOOKS) {
        wd2_log_error("Hook table full");
        return -1;
    }
    for (int i = 0; i < g_hookCount; i++) {
        if (g_hooks[i].pTarget == target) {
            wd2_log_warn("Hook already exists for %s", name ? name : "unknown");
            return i;
        }
    }

    MH_STATUS st = MH_CreateHook(target, detour, original);
    if (st != MH_OK) {
        wd2_log_error("MH_CreateHook failed for %s: %d", name ? name : "unknown", st);
        return -1;
    }

    int idx = g_hookCount;
    g_hooks[idx].pTarget = target;
    g_hooks[idx].pDetour = detour;
    g_hooks[idx].pOriginal = original ? *original : NULL;
    g_hooks[idx].bEnabled = FALSE;
    g_hooks[idx].iIndex = idx;
    if (name) {
        strncpy(g_hooks[idx].szName, name, sizeof(g_hooks[idx].szName) - 1);
    } else {
        snprintf(g_hooks[idx].szName, sizeof(g_hooks[idx].szName), "hook_%d", idx);
    }
    g_hookCount++;

    st = MH_EnableHook(target);
    if (st != MH_OK) {
        wd2_log_warn("MH_EnableHook failed for %s: %d", g_hooks[idx].szName, st);
    } else {
        g_hooks[idx].bEnabled = TRUE;
    }

    wd2_log_info("Hook created: %s at 0x%p", g_hooks[idx].szName, target);
    return idx;
}

void wd2_hook_remove(int index) {
    if (index < 0 || index >= g_hookCount) return;
    if (!g_hooks[index].pTarget) return;

    if (g_hooks[index].bEnabled) {
        MH_DisableHook(g_hooks[index].pTarget);
    }
    MH_RemoveHook(g_hooks[index].pTarget);

    wd2_log_info("Hook removed: %s", g_hooks[index].szName);

    memmove(&g_hooks[index], &g_hooks[index + 1],
            (g_hookCount - index - 1) * sizeof(WD2_HOOK_ENTRY));
    g_hookCount--;
}

void wd2_hook_enable_all(void) {
    for (int i = 0; i < g_hookCount; i++) {
        if (g_hooks[i].pTarget && !g_hooks[i].bEnabled) {
            MH_STATUS st = MH_EnableHook(g_hooks[i].pTarget);
            if (st == MH_OK) {
                g_hooks[i].bEnabled = TRUE;
            } else {
                wd2_log_warn("Failed to enable hook: %s", g_hooks[i].szName);
            }
        }
    }
}

void wd2_hook_disable_all(void) {
    for (int i = 0; i < g_hookCount; i++) {
        if (g_hooks[i].pTarget && g_hooks[i].bEnabled) {
            MH_STATUS st = MH_DisableHook(g_hooks[i].pTarget);
            if (st == MH_OK) {
                g_hooks[i].bEnabled = FALSE;
            } else {
                wd2_log_warn("Failed to disable hook: %s", g_hooks[i].szName);
            }
        }
    }
}

BOOL wd2_hook_enable(int index) {
    if (index < 0 || index >= g_hookCount) return FALSE;
    if (!g_hooks[index].pTarget) return FALSE;
    MH_STATUS st = MH_EnableHook(g_hooks[index].pTarget);
    if (st == MH_OK) {
        g_hooks[index].bEnabled = TRUE;
        return TRUE;
    }
    return FALSE;
}

BOOL wd2_hook_disable(int index) {
    if (index < 0 || index >= g_hookCount) return FALSE;
    if (!g_hooks[index].pTarget) return FALSE;
    MH_STATUS st = MH_DisableHook(g_hooks[index].pTarget);
    if (st == MH_OK) {
        g_hooks[index].bEnabled = FALSE;
        return TRUE;
    }
    return FALSE;
}

void *wd2_hook_get_original(int index) {
    if (index < 0 || index >= g_hookCount) return NULL;
    return g_hooks[index].pOriginal;
}
