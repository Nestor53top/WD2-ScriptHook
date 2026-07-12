#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#pragma comment(lib, "dbghelp.lib")

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------
#define CRASH_LOG_FILE      "crash.log"
#define CRASH_DUMP_FILE     "crash.dmp"
#define CRASH_EVENT_NAME    "Global\\WD2CrashpadEvent"
#define CRASH_PIPE_NAME     "\\\\.\\pipe\\WD2Crashpad"

// ---------------------------------------------------------------------------
// Minidump type (use full memory if available)
// ---------------------------------------------------------------------------
typedef BOOL (WINAPI* MiniDumpWriteDump_t)(
    HANDLE hProcess, DWORD ProcessId, HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    PVOID ExceptionParam,
    PVOID UserStreamParam,
    PVOID CallbackParam
);

static MiniDumpWriteDump_t pMiniDumpWriteDump = NULL;

// ---------------------------------------------------------------------------
// Stack walk helpers (ARM64 / x64)
// ---------------------------------------------------------------------------
#ifdef _M_ARM64
static void WalkStackArm64(void* fp, FILE* log, DWORD64* frames, int* count)
{
    // Minimal ARM64 frame walk using __debugbreak as fallback
    (void)fp;
    (void)log;
    *count = 0;
}
#endif

static void WalkStack(void* fp, FILE* log, DWORD64* frames, int* count)
{
    *count = 0;
#ifdef _M_X64
    CONTEXT ctx = {0};
    ctx.ContextFlags = CONTEXT_CONTROL;
    RtlCaptureContext(&ctx);

    DWORD64 imageBase = 0;
    ULONG64 functionEntry = 0;

    for (int i = 0; i < 128 && *count < 128; i++) {
        if (!RtlLookupFunctionEntry(ctx.Rip, &imageBase, NULL))
            break;

        frames[*count] = ctx.Rip;
        (*count)++;

        // Simple frame walk — just capture RIP values
        // Real stack walk uses RtlVirtualUnwind but that requires full UNWIND_INFO
        break; // One-shot capture for safety
    }
#elif defined(_M_ARM64)
    WalkStackArm64(fp, log, frames, count);
#endif
    (void)fp;
    (void)log;
}

// ---------------------------------------------------------------------------
// Module info lookup
// ---------------------------------------------------------------------------
static void GetModuleInfo(void* addr, char* buf, DWORD bufSize)
{
    HMODULE hMod = NULL;
    if (!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCSTR)addr, &hMod))
    {
        snprintf(buf, bufSize, "Unknown module at %p", addr);
        return;
    }

    char modPath[MAX_PATH] = {0};
    GetModuleFileNameA(hMod, modPath, MAX_PATH);

    // Extract just filename
    char* fname = strrchr(modPath, '\\');
    if (fname) fname++; else fname = modPath;

    MODULEINFO mi = {0};
    if (GetModuleInformation(GetCurrentProcess(), hMod, &mi, sizeof(mi))) {
        snprintf(buf, bufSize, "%s (base=0x%p, size=0x%lX)",
            fname, mi.lpBaseOfDll, mi.SizeOfImage);
    } else {
        snprintf(buf, bufSize, "%s", fname);
    }
}

// ---------------------------------------------------------------------------
// Write crash log
// ---------------------------------------------------------------------------
static void WriteCrashLog(EXCEPTION_POINTERS* ep)
{
    FILE* fp = fopen(CRASH_LOG_FILE, "w");
    if (!fp) return;

    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    fprintf(fp, "=====================================\n");
    fprintf(fp, "  Watch Dogs 2 ScriptHook Crash Log\n");
    fprintf(fp, "=====================================\n");
    fprintf(fp, "Timestamp:  %04d-%02d-%02d %02d:%02d:%02d\n",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec);
    fprintf(fp, "PID:        %lu\n", GetCurrentProcessId());
    fprintf(fp, "\n");

    if (ep && ep->ExceptionRecord) {
        EXCEPTION_RECORD* er = ep->ExceptionRecord;

        fprintf(fp, "--- Exception Info ---\n");
        fprintf(fp, "Code:       0x%08lX", er->ExceptionCode);
        switch (er->ExceptionCode) {
            case EXCEPTION_ACCESS_VIOLATION:     fprintf(fp, " (ACCESS_VIOLATION)"); break;
            case EXCEPTION_STACK_OVERFLOW:       fprintf(fp, " (STACK_OVERFLOW)"); break;
            case EXCEPTION_INT_DIVIDE_BY_ZERO:   fprintf(fp, " (INT_DIVIDE_BY_ZERO)"); break;
            case EXCEPTION_FLT_DIVIDE_BY_ZERO:   fprintf(fp, " (FLT_DIVIDE_BY_ZERO)"); break;
            case EXCEPTION_ILLEGAL_INSTRUCTION:  fprintf(fp, " (ILLEGAL_INSTRUCTION)"); break;
            case EXCEPTION_IN_PAGE_ERROR:        fprintf(fp, " (IN_PAGE_ERROR)"); break;
            case EXCEPTION_DATATYPE_MISALIGNMENT:fprintf(fp, " (DATATYPE_MISALIGNMENT)"); break;
            case 0xE06D7363:                     fprintf(fp, " (C++ EXCEPTION)"); break;
            case 0xC0000374:                     fprintf(fp, " (HEAP_CORRUPTION)"); break;
            case 0xC0000139:                     fprintf(fp, " (DLL_NOT_FOUND)"); break;
            case 0xC0000142:                     fprintf(fp, " (DLL_INIT_FAILED)"); break;
            default: break;
        }
        fprintf(fp, "\n");

        fprintf(fp, "Address:    0x%p\n", er->ExceptionAddress);
        fprintf(fp, "Flags:      0x%08lX\n", er->ExceptionFlags);

        if (er->ExceptionCode == EXCEPTION_ACCESS_VIOLATION ||
            er->ExceptionCode == EXCEPTION_IN_PAGE_ERROR) {
            fprintf(fp, "Access:     %s at 0x%p\n",
                er->ExceptionInformation[0] == 0 ? "READ" :
                er->ExceptionInformation[0] == 1 ? "WRITE" :
                er->ExceptionInformation[0] == 8 ? "DEP VIOLATION" : "UNKNOWN",
                (void*)er->ExceptionInformation[1]);
        }

        fprintf(fp, "\n");

        // Module info
        char modInfo[512];
        GetModuleInfo(er->ExceptionAddress, modInfo, sizeof(modInfo));
        fprintf(fp, "--- Module ---\n");
        fprintf(fp, "Crash in:   %s\n", modInfo);
        fprintf(fp, "\n");
    }

    // Stack trace
    fprintf(fp, "--- Stack Trace ---\n");
    if (ep && ep->ContextRecord) {
        CONTEXT* ctx = ep->ContextRecord;
        DWORD64 frames[128] = {0};
        int frameCount = 0;

        WalkStack(NULL, fp, frames, &frameCount);

        if (frameCount == 0) {
            // Fallback: just print the instruction pointer
            fprintf(fp, "  [0x%p] ", (void*)ctx->Rip);
            char modInfo[512];
            GetModuleInfo((void*)ctx->Rip, modInfo, sizeof(modInfo));
            fprintf(fp, "%s\n", modInfo);
        } else {
            for (int i = 0; i < frameCount; i++) {
                fprintf(fp, "  [%d] 0x", i);
                fprintf(fp, "%llX ", (unsigned long long)frames[i]);
                char modInfo[512];
                GetModuleInfo((void*)frames[i], modInfo, sizeof(modInfo));
                fprintf(fp, "%s\n", modInfo);
            }
        }
    } else {
        fprintf(fp, "  (no context available)\n");
    }

    fprintf(fp, "\n");
    fprintf(fp, "--- End of Log ---\n");
    fclose(fp);
}

// ---------------------------------------------------------------------------
// Write minidump
// ---------------------------------------------------------------------------
static void WriteMiniDump(EXCEPTION_POINTERS* ep)
{
    if (!pMiniDumpWriteDump) return;

    HANDLE hFile = CreateFileA(
        CRASH_DUMP_FILE, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) return;

    MINIDUMP_EXCEPTION_INFORMATION exInfo;
    exInfo.ThreadId = GetCurrentThreadId();
    exInfo.ExceptionPointers = ep;
    exInfo.ClientPointers = FALSE;

    pMiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        (MINIDUMP_TYPE)(MiniDumpWithDataSegs | MiniDumpWithHandleData),
        &exInfo, NULL, NULL);

    CloseHandle(hFile);
}

// ---------------------------------------------------------------------------
// Signal crash event
// ---------------------------------------------------------------------------
static void SignalCrashEvent(void)
{
    HANDLE hEvent = CreateEventA(NULL, TRUE, TRUE, CRASH_EVENT_NAME);
    if (hEvent) {
        SetEvent(hEvent);
        // Don't close immediately — let other processes see it
        Sleep(500);
        CloseHandle(hEvent);
    }
}

// ---------------------------------------------------------------------------
// Unhandled exception filter
// ---------------------------------------------------------------------------
static LONG WINAPI UnhandledExceptionFilterFunc(EXCEPTION_POINTERS* ep)
{
    WriteCrashLog(ep);
    WriteMiniDump(ep);
    SignalCrashEvent();
    return EXCEPTION_EXECUTE_HANDLER;
}

// ---------------------------------------------------------------------------
// Named pipe server — receive commands from core.dll
// ---------------------------------------------------------------------------
static DWORD WINAPI PipeServerThread(LPVOID param)
{
    (void)param;

    while (TRUE) {
        HANDLE hPipe = CreateNamedPipeA(
            CRASH_PIPE_NAME,
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            1, 256, 256, 0, NULL);

        if (hPipe == INVALID_HANDLE_VALUE) continue;

        if (!ConnectNamedPipe(hPipe, NULL)) {
            if (GetLastError() != ERROR_PIPE_CONNECTED) {
                CloseHandle(hPipe);
                continue;
            }
        }

        char buf[256] = {0};
        DWORD bytesRead = 0;
        ReadFile(hPipe, buf, sizeof(buf) - 1, &bytesRead, NULL);

        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);

        // Minimal command handling — "ping" responds with "pong"
        if (strncmp(buf, "ping", 4) == 0) {
            HANDLE hReply = CreateFileA(
                "\\\\.\\pipe\\WD2CrashpadReply",
                GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
            if (hReply != INVALID_HANDLE_VALUE) {
                DWORD written = 0;
                WriteFile(hReply, "pong", 4, &written, NULL);
                CloseHandle(hReply);
            }
        } else if (strncmp(buf, "quit", 4) == 0) {
            break;
        }
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Parse command line for --pipe <name>
// ---------------------------------------------------------------------------
static BOOL ParseCommandLine(int argc, char* argv[], char* pipeBuf, DWORD pipeBufSize)
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--pipe") == 0 && i + 1 < argc) {
            strncpy(pipeBuf, argv[i + 1], pipeBufSize - 1);
            return TRUE;
        }
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// entry point
// ---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    // Load DbgHelp for minidump
    HMODULE hDbgHelp = LoadLibraryA("dbghelp.dll");
    if (hDbgHelp) {
        pMiniDumpWriteDump = (MiniDumpWriteDump_t)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
    }

    // Parse --pipe argument
    char pipeName[256] = {0};
    ParseCommandLine(argc, argv, pipeName, sizeof(pipeName));

    // Set up exception handler
    SetUnhandledExceptionFilter(UnhandledExceptionFilterFunc);

    // Start pipe server thread
    HANDLE hPipeThread = CreateThread(NULL, 0, PipeServerThread, NULL, 0, NULL);

    // Main loop — keep process alive
    // The crashpad_handler.exe is long-lived; the game injects into it
    // or it sits waiting for crash events.
    HANDLE hEvent = CreateEventA(NULL, TRUE, FALSE, CRASH_EVENT_NAME);

    printf("crashpad_handler.exe running (PID %lu)\n", GetCurrentProcessId());
    printf("Waiting for crash events or 'quit' on pipe...\n");

    // Wait indefinitely (or until crash/event)
    WaitForSingleObject(hEvent, INFINITE);

    if (hPipeThread) {
        // Signal pipe thread to stop
        HANDLE hQuitPipe = CreateFileA(
            "\\\\.\\pipe\\WD2Crashpad",
            GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hQuitPipe != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            WriteFile(hQuitPipe, "quit", 4, &written, NULL);
            CloseHandle(hQuitPipe);
        }
        WaitForSingleObject(hPipeThread, 3000);
        CloseHandle(hPipeThread);
    }

    if (hEvent) CloseHandle(hEvent);

    return 0;
}
