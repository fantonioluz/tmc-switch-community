/* Bounded RAM breadcrumbs; write only at startup or on a fault/stall.
 * No stdio, malloc, game pointer dereferences, or app mutexes in the handler.
 * Separate pre-opened FS sessions/buffers allow a crash during a stall report.
 */
#include <switch.h>
#include <stdatomic.h>
#include "port_diagnostics.h"

#ifndef TMC_COMMUNITY_VERSION
#define TMC_COMMUNITY_VERSION "development"
#endif
#define TRACE_COUNT 64
#define REPORT_SIZE 16384

typedef struct {
    atomic_uint sequence, kind, area, room, actor;
    atomic_uintptr_t value, target;
} Trace;
static Trace sTrace[TRACE_COUNT];
static atomic_uint sSequence, sFrame, sPaused, sArea, sRoom, sActor, sCommand;
static atomic_uintptr_t sEntity, sInstruction;
static atomic_bool sStopping;
static atomic_flag sInCrash = ATOMIC_FLAG_INIT;
static FsFileSystem sFs;
static FsFile sCrashFile, sStallFile;
static bool sFsReady, sCrashReady, sStallReady, sThreadReady;
static Thread sThread;
static Result sThreadResult;
static char sCrashBuffer[REPORT_SIZE], sStallBuffer[REPORT_SIZE];
__attribute__((aligned(16))) u8 __nx_exception_stack[16384];
u64 __nx_exception_stack_size = sizeof(__nx_exception_stack);
extern void _start(void);

typedef struct { char* data; unsigned size; } Text;
static void Add(Text* out, const char* text) {
    while (*text && out->size < REPORT_SIZE - 1) out->data[out->size++] = *text++;
    out->data[out->size] = 0;
}
static void Hex(Text* out, const char* name, uint64_t value) {
    static const char digits[] = "0123456789abcdef";
    char number[19] = "0x0000000000000000";
    for (unsigned i = 0; i < 16; i++) number[17-i] = digits[(value >> (i*4)) & 15];
    Add(out, name); Add(out, number); Add(out, "\n");
}

void Port_Diagnostics_Frame(void) {
    atomic_fetch_add_explicit(&sFrame, 1, memory_order_relaxed);
}
void Port_Diagnostics_Pause(int paused) {
    atomic_store_explicit(&sPaused, paused != 0, memory_order_relaxed);
}
void Port_Diagnostics_Entity(uint32_t area, uint32_t room, uint32_t actor, uintptr_t address) {
    atomic_store_explicit(&sArea, area, memory_order_relaxed);
    atomic_store_explicit(&sRoom, room, memory_order_relaxed);
    atomic_store_explicit(&sActor, actor, memory_order_relaxed);
    atomic_store_explicit(&sEntity, address, memory_order_relaxed);
    atomic_store_explicit(&sInstruction, 0, memory_order_relaxed);
    atomic_store_explicit(&sCommand, 0, memory_order_relaxed);
}
void Port_Diagnostics_Script(uintptr_t instruction, uint32_t command) {
    atomic_store_explicit(&sInstruction, instruction, memory_order_relaxed);
    atomic_store_explicit(&sCommand, command, memory_order_relaxed);
}
void Port_Diagnostics_Event(uint32_t kind, uint32_t area, uint32_t room,
                            uint32_t actor, uintptr_t value, uintptr_t target) {
    /* Single writer: game thread. All fields atomic for watchdog readers. */
    unsigned seq = atomic_load_explicit(&sSequence, memory_order_relaxed) + 1;
    Trace* t = &sTrace[seq % TRACE_COUNT];
    atomic_store(&t->sequence, 0);
    atomic_store(&t->kind, kind); atomic_store(&t->area, area);
    atomic_store(&t->room, room); atomic_store(&t->actor, actor);
    atomic_store(&t->value, value); atomic_store(&t->target, target);
    atomic_store(&t->sequence, seq);
    atomic_store(&sSequence, seq);
}

static void Snapshot(Text* out, const char* reason) {
    Add(out, "TMC Switch Community " TMC_COMMUNITY_VERSION "\n");
    Add(out, reason); Add(out, "\nAll numeric values below are hexadecimal.\n");
    Hex(out, "module_base=", (uintptr_t)&_start);
    Hex(out, "heartbeat=", atomic_load(&sFrame));
    Hex(out, "area=", atomic_load(&sArea)); Hex(out, "room=", atomic_load(&sRoom));
    Add(out, "actor bytes: kind/id/type/action; last observed, not necessarily faulting\n");
    Hex(out, "actor=", atomic_load(&sActor)); Hex(out, "entity=", atomic_load(&sEntity));
    Hex(out, "script_instruction=", atomic_load(&sInstruction));
    Hex(out, "script_command=", atomic_load(&sCommand));
    Hex(out, "watchdog_start_result=", sThreadResult);
    Add(out, "Recent dialog events: 1=call, 2=callback entered, 3=show\n");
    unsigned end = atomic_load(&sSequence);
    unsigned first = end >= TRACE_COUNT ? end - TRACE_COUNT + 1 : 1;
    for (unsigned seq = first; seq && seq <= end; seq++) {
        Trace* t = &sTrace[seq % TRACE_COUNT];
        if (atomic_load(&t->sequence) != seq) continue;
        unsigned kind = atomic_load(&t->kind), area = atomic_load(&t->area);
        unsigned room = atomic_load(&t->room), actor = atomic_load(&t->actor);
        uintptr_t value = atomic_load(&t->value), target = atomic_load(&t->target);
        if (atomic_load(&t->sequence) != seq) continue;
        Hex(out, "event=", seq); Hex(out, "kind=", kind);
        Hex(out, "area_room=", (area << 16) | room); Hex(out, "actor=", actor);
        Hex(out, "value=", value); Hex(out, "target=", target);
    }
}

static bool OpenReport(FsFile* file, const char* path) {
    /* Create may return AlreadyExists. Opening preserves the prior report. */
    fsFsCreateFile(&sFs, path, 0, 0);
    return R_SUCCEEDED(fsFsOpenFile(&sFs, path, FsOpenMode_Write, file));
}
static void WriteReport(FsFile* file, Text* text) {
    /* No truncation during ordinary startup. Replace only on a new incident. */
    if (R_SUCCEEDED(fsFileSetSize(file, text->size)))
        fsFileWrite(file, 0, text->data, text->size, FsWriteOption_Flush);
}

static void Watchdog(void* unused) {
    (void)unused;
    unsigned last = 0, missed = 0;
    bool reported = false;
    while (!atomic_load(&sStopping)) {
        svcSleepThread(1000000000LL);
        unsigned frame = atomic_load(&sFrame);
        if (!frame || frame != last || atomic_load(&sPaused)) missed = 0;
        else if (missed < 8) missed++;
        last = frame;
        if (missed == 8 && !reported) {
            reported = true; /* one report per launch, never repeated SD spam */
            if (sStallReady) {
                Text out = {sStallBuffer, 0};
                Snapshot(&out, "STALL: no frame progress for eight watchdog samples; game left running");
                WriteReport(&sStallFile, &out);
            }
        }
    }
}

void Port_Diagnostics_Init(void) {
    if (sFsReady) return;
    sFsReady = R_SUCCEEDED(fsOpenSdCardFileSystem(&sFs));
    if (!sFsReady) return;
    sCrashReady = OpenReport(&sCrashFile, "/switch/tmc/crash-last.log");
    sStallReady = OpenReport(&sStallFile, "/switch/tmc/freeze-last.log");
    /* Same priority as the game, default allowed CPU; sleeps between samples. */
    sThreadResult = threadCreate(&sThread, Watchdog, NULL, NULL, 16384, 0x2c, -2);
    if (R_SUCCEEDED(sThreadResult)) {
        sThreadResult = threadStart(&sThread);
        sThreadReady = R_SUCCEEDED(sThreadResult);
        if (!sThreadReady) threadClose(&sThread);
    }
    FsFile status;
    if (OpenReport(&status, "/switch/tmc/diagnostics.log")) {
        Text out = {sStallBuffer, 0};
        Add(&out, "TMC Switch Community " TMC_COMMUNITY_VERSION "\n");
        Hex(&out, "crash_file_ready=", sCrashReady);
        Hex(&out, "freeze_file_ready=", sStallReady);
        Hex(&out, "watchdog_ready=", sThreadReady);
        Hex(&out, "watchdog_start_result=", sThreadResult);
        WriteReport(&status, &out); fsFileClose(&status);
    }
}

void userAppExit(void) {
    atomic_store(&sStopping, true);
    if (sThreadReady) { threadWaitForExit(&sThread); threadClose(&sThread); }
    if (sCrashReady) fsFileClose(&sCrashFile);
    if (sStallReady) fsFileClose(&sStallFile);
    if (sFsReady) fsFsClose(&sFs);
}

void __libnx_exception_handler(ThreadExceptionDump* ctx) {
    if (!atomic_flag_test_and_set(&sInCrash) && sCrashReady) {
        Text out = {sCrashBuffer, 0};
        Snapshot(&out, "CRASH: native CPU exception");
        Hex(&out, "error_desc=", ctx->error_desc);
        Hex(&out, "pc=", ctx->pc.x); Hex(&out, "lr=", ctx->lr.x);
        Hex(&out, "sp=", ctx->sp.x); Hex(&out, "fp=", ctx->fp.x);
        Hex(&out, "far=", ctx->far.x); Hex(&out, "esr=", ctx->esr);
        Hex(&out, "pstate=", ctx->pstate);
        for (unsigned i = 0; i < 29; i++) {
            Hex(&out, "register_index=", i); Hex(&out, "x=", ctx->cpu_gprs[i].x);
        }
        WriteReport(&sCrashFile, &out);
    }
    /* No stdio/SDL exit cleanup on a damaged process and no recursive svcBreak.
     * The own report is the artifact; an Atmosphere report is not guaranteed. */
    svcExitProcess();
}
