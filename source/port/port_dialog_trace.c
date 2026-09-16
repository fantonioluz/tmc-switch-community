#include "port_dialog_trace.h"
#include "port_diagnostics.h"
#include "room.h"

static u32 Actor(const Entity* e) {
    return e ? ((u32)e->kind << 24) | ((u32)e->id << 16) |
                   ((u32)e->type << 8) | e->action : 0;
}

void Port_DialogTrace_Call(const Entity* e, const ScriptExecutionContext* context,
                           u32 gba_addr, void* native_func) {
    (void)context;
    Port_Diagnostics_Event(1, gRoomControls.area, gRoomControls.room, Actor(e),
                           gba_addr, (uintptr_t)native_func);
}

void Port_DialogTrace_Result(const Entity* e, u32 gba_addr, void* native_func) {
    Port_Diagnostics_Event(2, gRoomControls.area, gRoomControls.room, Actor(e),
                           gba_addr, (uintptr_t)native_func);
}

void Port_DialogTrace_Show(const Entity* e, u32 dialog_type, uintptr_t func_value) {
    Port_Diagnostics_Event(3, gRoomControls.area, gRoomControls.room, Actor(e),
                           dialog_type, func_value);
}
