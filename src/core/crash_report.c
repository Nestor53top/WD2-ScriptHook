#include "crash_report.h"

#include <dbghelp.h>
#include <time.h>

#pragma comment(lib, "dbghelp.lib")

static LPTOP_LEVEL_EXCEPTION_FILTER g_prevFilter = NULL;
static BOOL g_crashInitialized = FALSE;

static void wd2_write_minidump(EXCEPTION_POINTERS *ep) {
    SYSTEMTIME st;
    GetLocalTime(&st);

    char dump_path[MAX_PATH];
    snprintf(dump_path, sizeof(dump_path), "%scrash_%04d%02d%02d_%02d%02d%02d.dmp",
             g_ctx.szBasePath,
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond);

    HANDLE hFile = CreateFileA(dump_path, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        wd2_log_error("Cannot create minidump: %s", dump_path);
        return;
    }

    MINIDUMP_EXCEPTION_INFORMATION mei;
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = ep;
    mei.ClientPointers = FALSE;

    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                      hFile, MiniDumpWithDataSegs, &mei, NULL, NULL);

    CloseHandle(hFile);
    wd2_log_info("Minidump written: %s", dump_path);
}

static void wd2_write_crash_log(EXCEPTION_POINTERS *ep) {
    SYSTEMTIME st;
    GetLocalTime(&st);

    char log_path[MAX_PATH];
    snprintf(log_path, sizeof(log_path), "%scrash.log", g_ctx.szBasePath);

    FILE *f = fopen(log_path, "a");
    if (!f) return;

    fprintf(f, "\n=== CRASH REPORT ===\n");
    fprintf(f, "Date: %04d-%02d-%02d %02d:%02d:%02d\n",
            st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond);
    fprintf(f, "WD2 ScriptHook v%s\n", WD2_VERSION_STRING);
    fprintf(f, "Game version: %u\n", g_ctx.dwGameVersion);
    fprintf(f, "Game base: 0x%llX (size: 0x%X)\n", g_ctx.qwBaseAddress, g_ctx.dwBaseSize);

    if (ep && ep->ExceptionRecord) {
        EXCEPTION_RECORD *er = ep->ExceptionRecord;
        fprintf(f, "Exception: 0x%08lX at 0x%llX\n", er->ExceptionCode, (DWORD64)er->ExceptionAddress);
        fprintf(f, "Flags: 0x%08lX\n", er->ExceptionFlags);

        CONTEXT *ctx = ep->ContextRecord;
        if (ctx) {
            fprintf(f, "RIP: 0x%llX\n", ctx->Rip);
            fprintf(f, "RSP: 0x%llX\n", ctx->Rsp);
            fprintf(f, "RBP: 0x%llX\n", ctx->Rbp);
            fprintf(f, "RAX: 0x%llX\n", ctx->Rax);
            fprintf(f, "RBX: 0x%llX\n", ctx->Rbx);
            fprintf(f, "RCX: 0x%llX\n", ctx->Rcx);
            fprintf(f, "RDX: 0x%llX\n", ctx->Rdx);
            fprintf(f, "RSI: 0x%llX\n", ctx->Rsi);
            fprintf(f, "RDI: 0x%llX\n", ctx->Rdi);
            fprintf(f, "R8:  0x%llX\n", ctx->R8);
            fprintf(f, "R9:  0x%llX\n", ctx->R9);
            fprintf(f, "R10: 0x%llX\n", ctx->R10);
            fprintf(f, "R11: 0x%llX\n", ctx->R11);
            fprintf(f, "R12: 0x%llX\n", ctx->R12);
            fprintf(f, "R13: 0x%llX\n", ctx->R13);
            fprintf(f, "R14: 0x%llX\n", ctx->R14);
            fprintf(f, "R15: 0x%llX\n", ctx->R15);
        }
    }

    fprintf(f, "===================\n\n");
    fclose(f);
}

static LONG WINAPI wd2_exception_handler(EXCEPTION_POINTERS *ep) {
    if (!ep) {
        wd2_log_error("Null exception pointers");
        return EXCEPTION_CONTINUE_SEARCH;
    }

    wd2_log_error("=== CRASH ===");
    wd2_log_error("Exception: 0x%08lX at 0x%llX",
                  ep->ExceptionRecord->ExceptionCode,
                  (DWORD64)ep->ExceptionRecord->ExceptionAddress);

    wd2_write_crash_log(ep);
    wd2_write_minidump(ep);

    return EXCEPTION_CONTINUE_SEARCH;
}

void wd2_crash_init(void) {
    if (g_crashInitialized) return;

    SetUnhandledExceptionFilter(wd2_exception_handler);
    g_crashInitialized = TRUE;
    wd2_log_info("Crash handler installed");
}

void wd2_crash_shutdown(void) {
    if (!g_crashInitialized) return;

    SetUnhandledExceptionFilter(g_prevFilter);
    g_crashInitialized = FALSE;
    wd2_log_info("Crash handler removed");
}
