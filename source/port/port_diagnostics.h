#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    PORT_DIAG_STARTUP,
    PORT_DIAG_TASK,
    PORT_DIAG_TASK_DONE,
    PORT_DIAG_MESSAGE,
    PORT_DIAG_FADE,
    PORT_DIAG_AUDIO,
    PORT_DIAG_FRAME_WAIT,
    PORT_DIAG_ENTITY_WALK,
    PORT_DIAG_ENTITY_UPDATE,
    PORT_DIAG_ENTITY_COLLISION,
    PORT_DIAG_ENTITY_DONE,
    PORT_DIAG_DRAW_UI,
    PORT_DIAG_DRAW_UI_DONE,
    PORT_DIAG_CARRIED_OBJECT,
    PORT_DIAG_CARRIED_OBJECT_DONE,
    PORT_DIAG_DRAW_SPRITES,
    PORT_DIAG_DRAW_SPRITES_DONE,
    PORT_DIAG_DELETE_SLEEPING,
    PORT_DIAG_DELETE_SLEEPING_DONE,
    PORT_DIAG_VSYNC_SETUP,
    PORT_DIAG_PENDING_LOAD,
    PORT_DIAG_FRAME_PACING,
    PORT_DIAG_INPUT,
    PORT_DIAG_VBLANK,
    PORT_DIAG_VBLANK_DONE,
    PORT_DIAG_PPU_ENTRY,
    PORT_DIAG_ACHIEVEMENTS,
    PORT_DIAG_APPLET,
    PORT_DIAG_PPU_RENDER,
    PORT_DIAG_PPU_SCALE,
    PORT_DIAG_TEXTURE_UPLOAD,
    PORT_DIAG_RENDER_OVERLAY,
    PORT_DIAG_PRESENT,
    PORT_DIAG_PRESENT_DONE,
    PORT_DIAG_WAIT_INTERRUPT,
    PORT_DIAG_FRAME_RESOURCES,
    PORT_DIAG_FRAME_DONE,
} PortDiagnosticStage;

#ifdef __SWITCH__
void Port_Diagnostics_Stage(uint32_t stage);
void Port_Diagnostics_Init(void);
void Port_Diagnostics_Frame(void);
void Port_Diagnostics_Pause(int paused);
void Port_Diagnostics_Entity(uint32_t area, uint32_t room, uint32_t actor, uintptr_t address);
void Port_Diagnostics_Script(uintptr_t instruction, uint32_t command);
void Port_Diagnostics_Event(uint32_t kind, uint32_t area, uint32_t room,
                            uint32_t actor, uintptr_t value, uintptr_t target);
#else
static inline void Port_Diagnostics_Stage(uint32_t s) { (void)s; }
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
