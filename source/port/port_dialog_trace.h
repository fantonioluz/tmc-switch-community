#ifndef PORT_DIALOG_TRACE_H
#define PORT_DIALOG_TRACE_H

#include "global.h"
#include "entity.h"
#include "script.h"

#ifdef __cplusplus
extern "C" {
#endif

void Port_DialogTrace_Call(const Entity* entity, const ScriptExecutionContext* context, u32 gba_addr, void* native_func);
void Port_DialogTrace_Result(const Entity* entity, u32 gba_addr, void* native_func);
void Port_DialogTrace_Show(const Entity* entity, u32 dialog_type, uintptr_t func_value);

#ifdef __cplusplus
}
#endif

#endif
