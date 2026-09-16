#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __SWITCH__
void Port_Diagnostics_Init(void);
void Port_Diagnostics_Frame(void);
void Port_Diagnostics_Pause(int paused);
void Port_Diagnostics_Entity(uint32_t area, uint32_t room, uint32_t actor, uintptr_t address);
void Port_Diagnostics_Script(uintptr_t instruction, uint32_t command);
void Port_Diagnostics_Event(uint32_t kind, uint32_t area, uint32_t room,
                            uint32_t actor, uintptr_t value, uintptr_t target);
#else
static inline void Port_Diagnostics_Init(void) {}
static inline void Port_Diagnostics_Frame(void) {}
static inline void Port_Diagnostics_Pause(int p) { (void)p; }
static inline void Port_Diagnostics_Entity(uint32_t a, uint32_t r, uint32_t e, uintptr_t p)
    { (void)a; (void)r; (void)e; (void)p; }
static inline void Port_Diagnostics_Script(uintptr_t p, uint32_t c) { (void)p; (void)c; }
static inline void Port_Diagnostics_Event(uint32_t k, uint32_t a, uint32_t r,
                                         uint32_t e, uintptr_t v, uintptr_t t)
    { (void)k; (void)a; (void)r; (void)e; (void)v; (void)t; }
#endif
#ifdef __cplusplus
}
#endif
