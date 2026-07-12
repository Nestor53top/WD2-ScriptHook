#pragma once

#include "core.h"

DWORD64 wd2_pattern_scan(HMODULE hModule, const char *pattern, const char *mask);
DWORD64 wd2_pattern_scan_range(DWORD64 start, DWORD64 end, const char *pattern, const char *mask);
DWORD64 wd2_find_string_in_module(HMODULE hModule, const char *str);
DWORD64 wd2_find_relative_call(DWORD64 addr);
DWORD64 wd2_resolve_address(DWORD64 base, DWORD offset);
DWORD64 wd2_scan_export(HMODULE hModule, const char *name);
