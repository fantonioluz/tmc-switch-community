#pragma once

#include <stdint.h>

#include "mode1_dmg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GBAAPULayout {
    DMGLayout psg;
    uint16_t soundcnt_h;
    uint16_t soundbias;
} GBAAPULayout;

void virtuapu_mode3_reset(void);
void virtuapu_mode3_sync_audio_registers(const VirtuaAPUMemory *regs);
void virtuapu_mode3_step_frame_sequencer(void);
void virtuapu_mode3_render_audio(const VirtuaAPUMemory *regs, uint32_t sample_count);
void virtuapu_mode3_fifo_write_a(uint32_t value);
void virtuapu_mode3_fifo_write_b(uint32_t value);
uint32_t virtuapu_mode3_fifo_level_a(void);
uint32_t virtuapu_mode3_fifo_level_b(void);
const GBAAPULayout *virtuapu_mode3_get_layout(void);

#ifdef __cplusplus
}
#endif
