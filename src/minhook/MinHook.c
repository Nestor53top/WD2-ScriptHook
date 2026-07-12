#include "MinHook.h"
#include <windows.h>

typedef struct _MH_HOOK_ENTRY {
    LPVOID pTarget;
    LPVOID pDetour;
    LPVOID pOriginal;
    BOOL   bEnabled;
    BYTE   oldBytes[32];
    DWORD  oldProtect;
} MH_HOOK_ENTRY;

static MH_HOOK_ENTRY g_hooks[256];
static int g_hookCount = 0;
static BOOL g_initialized = FALSE;

MH_STATUS MH_Initialize(void) {
    if (g_initialized) return MH_ERROR_ALREADY_INITIALIZED;
    g_hookCount = 0;
    g_initialized = TRUE;
    return MH_OK;
}

MH_STATUS MH_Uninitialize(void) {
    if (!g_initialized) return MH_ERROR_NOT_INITIALIZED;
    for (int i = 0; i < g_hookCount; i++) {
        if (g_hooks[i].bEnabled) {
            MH_DisableHook(g_hooks[i].pTarget);
        }
    }
    g_hookCount = 0;
    g_initialized = FALSE;
    return MH_OK;
}

MH_STATUS MH_CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal) {
    if (!g_initialized) return MH_ERROR_NOT_INITIALIZED;
    if (!pTarget || !pDetour) return MH_ERROR_UNSUPPORTED_FUNCTION;

    for (int i = 0; i < g_hookCount; i++) {
        if (g_hooks[i].pTarget == pTarget) return MH_ERROR_ALREADY_CREATED;
    }

    if (g_hookCount >= 256) return MH_ERROR_MEMORY_ALLOC;

    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery(pTarget, &mbi, sizeof(mbi))) return MH_ERROR_MEMORY_PROTECT;

    DWORD oldProtect;
    if (!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return MH_ERROR_MEMORY_PROTECT;
    }

    MH_HOOK_ENTRY *entry = &g_hooks[g_hookCount];
    entry->pTarget = pTarget;
    entry->pDetour = pDetour;
    entry->pOriginal = NULL;
    entry->bEnabled = FALSE;
    entry->oldProtect = oldProtect;

    memcpy(entry->oldBytes, pTarget, 32);

    if (ppOriginal) {
        BYTE *trampoline = (BYTE *)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!trampoline) {
            VirtualProtect(mbi.BaseAddress, mbi.RegionSize, oldProtect, &oldProtect);
            return MH_ERROR_MEMORY_ALLOC;
        }
        memcpy(trampoline, entry->oldBytes, 32);
        trampoline[32] = 0xFF;
        trampoline[33] = 0x25;
        *(UINT_PTR *)(trampoline + 34) = (UINT_PTR)((BYTE *)pTarget + 32);
        entry->pOriginal = trampoline;
        *ppOriginal = trampoline;
    }

    g_hookCount++;
    return MH_OK;
}

MH_STATUS MH_RemoveHook(LPVOID pTarget) {
    if (!g_initialized) return MH_ERROR_NOT_INITIALIZED;

    for (int i = 0; i < g_hookCount; i++) {
        if (g_hooks[i].pTarget == pTarget) {
            if (g_hooks[i].bEnabled) {
                MH_DisableHook(pTarget);
            }
            MEMORY_BASIC_INFORMATION mbi;
            VirtualQuery(g_hooks[i].pTarget, &mbi, sizeof(mbi));
            VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &g_hooks[i].oldProtect);
            memcpy(g_hooks[i].pTarget, g_hooks[i].oldBytes, 32);
            VirtualProtect(mbi.BaseAddress, mbi.RegionSize, g_hooks[i].oldProtect, &g_hooks[i].oldProtect);

            if (g_hooks[i].pOriginal) {
                VirtualFree(g_hooks[i].pOriginal, 0, MEM_RELEASE);
            }

            memmove(&g_hooks[i], &g_hooks[i + 1], (g_hookCount - i - 1) * sizeof(MH_HOOK_ENTRY));
            g_hookCount--;
            return MH_OK;
        }
    }
    return MH_ERROR_NOT_CREATED;
}

MH_STATUS MH_EnableHook(LPVOID pTarget) {
    if (!g_initialized) return MH_ERROR_NOT_INITIALIZED;

    for (int i = 0; i < g_hookCount; i++) {
        if (g_hooks[i].pTarget == pTarget || pTarget == MH_ALL_HOOKS) {
            if (g_hooks[i].bEnabled) continue;

            MEMORY_BASIC_INFORMATION mbi;
            VirtualQuery(g_hooks[i].pTarget, &mbi, sizeof(mbi));
            DWORD oldProtect;
            VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &oldProtect);

            BYTE *target = (BYTE *)g_hooks[i].pTarget;
            target[0] = 0xFF;
            target[1] = 0x25;
            *(UINT_PTR *)(target + 2) = (UINT_PTR)&target[6];
            *(UINT_PTR *)(target + 6) = (UINT_PTR)g_hooks[i].pDetour;

            VirtualProtect(mbi.BaseAddress, mbi.RegionSize, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), g_hooks[i].pTarget, 14);
            g_hooks[i].bEnabled = TRUE;

            if (pTarget != MH_ALL_HOOKS) return MH_OK;
        }
    }
    return MH_OK;
}

MH_STATUS MH_DisableHook(LPVOID pTarget) {
    if (!g_initialized) return MH_ERROR_NOT_INITIALIZED;

    for (int i = 0; i < g_hookCount; i++) {
        if (g_hooks[i].pTarget == pTarget || pTarget == MH_ALL_HOOKS) {
            if (!g_hooks[i].bEnabled) continue;

            MEMORY_BASIC_INFORMATION mbi;
            VirtualQuery(g_hooks[i].pTarget, &mbi, sizeof(mbi));
            DWORD oldProtect;
            VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &oldProtect);
            memcpy(g_hooks[i].pTarget, g_hooks[i].oldBytes, 32);
            VirtualProtect(mbi.BaseAddress, mbi.RegionSize, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), g_hooks[i].pTarget, 32);
            g_hooks[i].bEnabled = FALSE;

            if (pTarget != MH_ALL_HOOKS) return MH_OK;
        }
    }
    return MH_OK;
}

MH_STATUS MH_QueueEnableHook(LPVOID pTarget) {
    return MH_EnableHook(pTarget);
}

MH_STATUS MH_QueueDisableHook(LPVOID pTarget) {
    return MH_DisableHook(pTarget);
}

MH_STATUS MH_ApplyQueued(void) {
    return MH_OK;
}

const char *MH_StatusToString(MH_STATUS status) {
    switch (status) {
    case MH_OK: return "MH_OK";
    case MH_ERROR_ALREADY_INITIALIZED: return "MH_ERROR_ALREADY_INITIALIZED";
    case MH_ERROR_NOT_INITIALIZED: return "MH_ERROR_NOT_INITIALIZED";
    case MH_ERROR_ALREADY_CREATED: return "MH_ERROR_ALREADY_CREATED";
    case MH_ERROR_NOT_CREATED: return "MH_ERROR_NOT_CREATED";
    case MH_ERROR_ENABLED: return "MH_ERROR_ENABLED";
    case MH_ERROR_DISABLED: return "MH_ERROR_DISABLED";
    case MH_ERROR_NOT_EXECUTABLE: return "MH_ERROR_NOT_EXECUTABLE";
    case MH_ERROR_UNSUPPORTED_FUNCTION: return "MH_ERROR_UNSUPPORTED_FUNCTION";
    case MH_ERROR_MEMORY_ALLOC: return "MH_ERROR_MEMORY_ALLOC";
    case MH_ERROR_MEMORY_PROTECT: return "MH_ERROR_MEMORY_PROTECT";
    case MH_ERROR_MODULE_NOT_FOUND: return "MH_ERROR_MODULE_NOT_FOUND";
    case MH_ERROR_FUNCTION_NOT_FOUND: return "MH_ERROR_FUNCTION_NOT_FOUND";
    default: return "MH_UNKNOWN";
    }
}
