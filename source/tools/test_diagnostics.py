"""Host fault-injection tests for the actual Switch diagnostic module.

The mock replaces only libnx services; snapshot, watchdog, ring and exception
handler are compiled directly from production. No hardware crash is triggered.
"""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HEADER = r'''
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef uint8_t u8;
typedef uint64_t u64;
typedef unsigned Result;
typedef struct { int id; } FsFileSystem;
typedef struct { int id; } FsFile;
typedef struct { int id; } Thread;
typedef union { uint64_t x; } Register;
typedef struct {
    unsigned error_desc;
    Register cpu_gprs[29], pc, lr, sp, fp, far;
    unsigned esr, pstate;
} ThreadExceptionDump;
#define R_SUCCEEDED(x) ((x) == 0)
#define FsOpenMode_Write 2
#define FsWriteOption_Flush 1
Result fsOpenSdCardFileSystem(FsFileSystem*);
Result fsFsCreateFile(FsFileSystem*, const char*, int, int);
Result fsFsOpenFile(FsFileSystem*, const char*, int, FsFile*);
Result fsFileSetSize(FsFile*, uint64_t);
Result fsFileWrite(FsFile*, int64_t, const void*, uint64_t, unsigned);
void fsFileClose(FsFile*);
void fsFsClose(FsFileSystem*);
Result threadCreate(Thread*, void(*)(void*), void*, void*, size_t, int, int);
Result threadStart(Thread*);
void threadClose(Thread*);
void threadWaitForExit(Thread*);
void svcSleepThread(int64_t);
void svcExitProcess(void);
'''
TEST = r'''
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PRODUCTION"
static char files[3][REPORT_SIZE];
static unsigned writes[3], truncates[3], steps, mode;
static void (*watchdog)(void*);
static jmp_buf jump;
Result fsOpenSdCardFileSystem(FsFileSystem* f) { f->id = 1; return 0; }
Result fsFsCreateFile(FsFileSystem* f, const char* p, int a, int b) { return 1; }
Result fsFsOpenFile(FsFileSystem* f, const char* p, int m, FsFile* out) {
    out->id = strstr(p, "crash-last") ? 0 : strstr(p, "freeze-last") ? 1 : 2;
    return 0;
}
Result fsFileSetSize(FsFile* f, uint64_t s) {
    assert(s < REPORT_SIZE); truncates[f->id]++; files[f->id][s] = 0; return 0;
}
Result fsFileWrite(FsFile* f, int64_t off, const void* p, uint64_t n, unsigned opt) {
    assert(off == 0 && opt == FsWriteOption_Flush);
    assert(n < REPORT_SIZE); memcpy(files[f->id], p, n); files[f->id][n] = 0;
    writes[f->id]++; return 0;
}
void fsFileClose(FsFile* f) {}
void fsFsClose(FsFileSystem* f) {}
Result threadCreate(Thread* t, void(*fn)(void*), void* a, void* b, size_t z, int p, int c) {
    assert(z >= 16384 && c == -2); watchdog = fn; return 0;
}
Result threadStart(Thread* t) { return 0; }
void threadClose(Thread* t) {}
void threadWaitForExit(Thread* t) {}
void svcExitProcess(void) { longjmp(jump, 2); }
void svcSleepThread(int64_t ns) {
    assert(ns == 1000000000LL);
    if (++steps > 20) longjmp(jump, 1);
    if (mode == 1) Port_Diagnostics_Frame();
    if (mode == 2) Port_Diagnostics_Pause(1);
}
static void RunWatchdog(unsigned testMode) {
    mode = testMode; steps = 0;
    if (setjmp(jump) == 0) watchdog(NULL);
}
int main(void) {
    strcpy(files[0], "previous crash"); strcpy(files[1], "previous freeze");
    Port_Diagnostics_Init();
    assert(writes[2] == 1 && writes[0] == 0 && writes[1] == 0);
    assert(!truncates[0] && !truncates[1]);
    assert(!strcmp(files[0], "previous crash"));
    RunWatchdog(0); assert(!writes[1]); /* no frame yet: startup is not a stall */
    RunWatchdog(1); assert(!writes[1]); /* ordinary running game */
    RunWatchdog(2); assert(!writes[1]); /* HOME/pause */
    Port_Diagnostics_Pause(0);
    Port_Diagnostics_Entity(2, 3, 0x071f0104, 0x1234);
    Port_Diagnostics_Script(0x8001234, 0x444);
    for (unsigned i = 0; i < 10000; i++)
        Port_Diagnostics_Event(1, 2, 3, 0x071f0104, 0x08060000 + i, 0xabc);
    assert(!writes[0] && !writes[1]); /* no SD traffic on hot paths */
    RunWatchdog(0); assert(writes[1] == 1); /* one report despite 20 missed ticks */
    assert(strstr(files[1], "STALL") && strstr(files[1], "0x0000000008001234"));
    assert(strstr(files[1], "0x00000000071f0104"));
    unsigned events = 0;
    for (char* p = files[1]; (p = strstr(p, "event=")); p++) events++;
    assert(events == TRACE_COUNT);
    ThreadExceptionDump ctx = {0};
    ctx.pc.x = 0x123456; ctx.lr.x = 0xabcdef; ctx.far.x = 0x0811111c08111114ULL;
    if (setjmp(jump) == 0) __libnx_exception_handler(&ctx);
    assert(writes[0] == 1 && strstr(files[0], "CRASH"));
    assert(strstr(files[0], "0x0811111c08111114"));
    assert(strstr(files[0], "0x0000000000123456"));
    if (setjmp(jump) == 0) __libnx_exception_handler(&ctx);
    assert(writes[0] == 1); /* recursive fault does not retry damaged IO */
    userAppExit();
    puts("Diagnostics passed: no hot-path IO, preserved logs, ring wrap, pause, stall and CPU dump.");
}
'''

def main():
    with tempfile.TemporaryDirectory(prefix='tmc-diag-') as name:
        tmp = Path(name)
        (tmp / 'switch.h').write_text(HEADER)
        production = ROOT / 'platforms/switch/switch_diagnostics.c'
        (tmp / 'test.c').write_text(TEST.replace('PRODUCTION', production.as_posix()))
        output = tmp / 'test'
        subprocess.run(['gcc', '-std=gnu11', '-D__SWITCH__', '-O1', '-g',
                        '-fsanitize=address,undefined', '-I' + str(tmp),
                        '-I' + str(ROOT / 'port'), str(tmp / 'test.c'), '-o', str(output)], check=True)
        subprocess.run([str(output)], check=True)

if __name__ == '__main__':
    main()
