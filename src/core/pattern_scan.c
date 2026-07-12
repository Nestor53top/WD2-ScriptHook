#include "pattern_scan.h"
#include <psapi.h>

DWORD64 wd2_pattern_scan(HMODULE hModule, const char *pattern, const char *mask) {
    if (!hModule || !pattern || !mask) return 0;

    MODULEINFO mod_info = {0};
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &mod_info, sizeof(mod_info))) {
        return 0;
    }

    return wd2_pattern_scan_range(
        (DWORD64)mod_info.lpBaseOfDll,
        (DWORD64)mod_info.lpBaseOfDll + mod_info.SizeOfImage,
        pattern, mask);
}

DWORD64 wd2_pattern_scan_range(DWORD64 start, DWORD64 end, const char *pattern, const char *mask) {
    if (!pattern || !mask || start >= end) return 0;

    size_t pattern_len = strlen(mask);
    if (pattern_len == 0) return 0;

    BYTE *scan_start = (BYTE *)start;
    BYTE *scan_end = (BYTE *)end - pattern_len;

    for (BYTE *current = scan_start; current <= scan_end; current++) {
        BOOL found = TRUE;
        for (size_t i = 0; i < pattern_len; i++) {
            if (mask[i] == 'x' && current[i] != (BYTE)pattern[i]) {
                found = FALSE;
                break;
            }
        }
        if (found) {
            return (DWORD64)current;
        }
    }
    return 0;
}

DWORD64 wd2_find_string_in_module(HMODULE hModule, const char *str) {
    if (!hModule || !str) return 0;

    MODULEINFO mod_info = {0};
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &mod_info, sizeof(mod_info))) {
        return 0;
    }

    size_t str_len = strlen(str);
    if (str_len == 0) return 0;

    BYTE *base = (BYTE *)mod_info.lpBaseOfDll;
    BYTE *end = base + mod_info.SizeOfImage - str_len;

    for (BYTE *current = base; current <= end; current++) {
        if (memcmp(current, str, str_len) == 0) {
            return (DWORD64)current;
        }
    }
    return 0;
}

DWORD64 wd2_find_relative_call(DWORD64 addr) {
    if (!addr) return 0;

    BYTE *code = (BYTE *)addr;
    if (code[0] == 0xE8) {
        INT32 rel_offset = *(INT32 *)(code + 1);
        return addr + 5 + rel_offset;
    }
    return 0;
}

DWORD64 wd2_resolve_address(DWORD64 base, DWORD offset) {
    return base + offset;
}

DWORD64 wd2_scan_export(HMODULE hModule, const char *name) {
    if (!hModule || !name) return 0;

    BYTE *base = (BYTE *)hModule;
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return 0;

    IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return 0;

    IMAGE_DATA_DIRECTORY *export_dir = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (export_dir->Size == 0) return 0;

    IMAGE_EXPORT_DIRECTORY *exports = (IMAGE_EXPORT_DIRECTORY *)(base + export_dir->VirtualAddress);
    DWORD *names = (DWORD *)(base + exports->AddressOfNames);
    WORD *ordinals = (WORD *)(base + exports->AddressOfNameOrdinals);
    DWORD *functions = (DWORD *)(base + exports->AddressOfFunctions);

    for (DWORD i = 0; i < exports->NumberOfNames; i++) {
        const char *export_name = (const char *)(base + names[i]);
        if (strcmp(export_name, name) == 0) {
            return (DWORD64)(base + functions[ordinals[i]]);
        }
    }
    return 0;
}

DWORD64 wd2_pattern_scan_aob(HMODULE hModule, const BYTE *aob, const char *mask, size_t len) {
    if (!hModule || !aob || !mask || len == 0) return 0;

    MODULEINFO mod_info = {0};
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &mod_info, sizeof(mod_info))) {
        return 0;
    }

    return wd2_pattern_scan_range(
        (DWORD64)mod_info.lpBaseOfDll,
        (DWORD64)mod_info.lpBaseOfDll + mod_info.SizeOfImage,
        (const char *)aob, mask);
}

DWORD64 wd2_scan_for_call(DWORD64 start, DWORD64 end) {
    BYTE *scan_start = (BYTE *)start;
    BYTE *scan_end = (BYTE *)end;

    for (BYTE *current = scan_start; current < scan_end; current++) {
        if (current[0] == 0xE8) {
            INT32 rel = *(INT32 *)(current + 1);
            DWORD64 target = (DWORD64)(current + 5 + rel);
            if (target >= start && target <= end) {
                return (DWORD64)current;
            }
        }
    }
    return 0;
}

DWORD64 wd2_scan_for_mov_reg(HMODULE hModule, BYTE reg_code, DWORD value) {
    BYTE pattern[7];
    char mask[8];

    pattern[0] = 0x48;
    pattern[1] = 0xC7;
    pattern[2] = 0xC0 | (reg_code & 0x07);
    memcpy(pattern + 3, &value, 4);
    strcpy(mask, "xxxxxxx");

    return wd2_pattern_scan(hModule, (const char *)pattern, mask);
}
