#pragma once

#include <stdint.h>

#include "apu_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    VIRTUAAPU_MODE_DMG = 1,
    VIRTUAAPU_MODE_CGB = 2,
    VIRTUAAPU_MODE_GBA = 3
};

void virtuapu_reset(void);
void virtuapu_render_audio(uint32_t sample_count);
void virtuapu_sync_audio(void);
void virtuapu_step_frame_sequencer(void);
void virtuapu_fifo_write_a(uint32_t value);
void virtuapu_fifo_write_b(uint32_t value);
uint32_t virtuapu_fifo_level_a(void);
uint32_t virtuapu_fifo_level_b(void);
int16_t *virtuapu_get_audio_buffer(void);
uint8_t *virtuapu_get_amem(void);
VirtuaAPUMemory *virtuapu_get_registers(void);

#ifdef __cplusplus
}
#endif
