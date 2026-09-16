#include "port_dialog_trace.h"
#include "room.h"

#ifdef PC_PORT

#include <stdio.h>

static FILE* sDialogLog;

static FILE* DialogTraceFile(void) {
    if (sDialogLog == NULL) {
        sDialogLog = fopen("dialog_trace.log", "a");
        if (sDialogLog != NULL) {
            setvbuf(sDialogLog, NULL, _IONBF, 0);
        }
    }
    return sDialogLog;
}

void Port_DialogTrace_Call(const Entity* entity, const ScriptExecutionContext* context, u32 gba_addr,
                           void* native_func) {
    FILE* log = DialogTraceFile();
    if (log == NULL) {
        return;
    }
    fprintf(log,
            "[DIALOG] call area=%u room=%u entity=%u:%u:%u addr=0x%08X native=%p context=%p\n",
            gRoomControls.area, gRoomControls.room, entity->kind, entity->id, entity->type, gba_addr, native_func,
            (const void*)context);
}

void Port_DialogTrace_Result(const Entity* entity, u32 gba_addr, void* native_func) {
    FILE* log = DialogTraceFile();
    if (log == NULL) {
        return;
    }
    fprintf(log, "[DIALOG] callback entered area=%u room=%u entity=%u:%u:%u addr=0x%08X native=%p\n",
            gRoomControls.area, gRoomControls.room, entity->kind, entity->id, entity->type, gba_addr, native_func);
}

void Port_DialogTrace_Show(const Entity* entity, u32 dialog_type, uintptr_t func_value) {
    FILE* log = DialogTraceFile();
    if (log == NULL) {
        return;
    }
    fprintf(log, "[DIALOG] show area=%u room=%u entity=%u:%u:%u type=%u func=0x%lX\n",
            gRoomControls.area, gRoomControls.room, entity->kind, entity->id, entity->type, dialog_type,
            (unsigned long)func_value);
}

#else

void Port_DialogTrace_Call(const Entity* entity, const ScriptExecutionContext* context, u32 gba_addr,
                           void* native_func) {
    (void)entity;
    (void)context;
    (void)gba_addr;
    (void)native_func;
}

void Port_DialogTrace_Result(const Entity* entity, u32 gba_addr, void* native_func) {
    (void)entity;
    (void)gba_addr;
    (void)native_func;
}

void Port_DialogTrace_Show(const Entity* entity, u32 dialog_type, uintptr_t func_value) {
    (void)entity;
    (void)dialog_type;
    (void)func_value;
}

#endif
