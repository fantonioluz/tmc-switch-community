#pragma once

#include <stdint.h>

#include "mode1_dmg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef DMGLayout CGBLayout;

typedef struct VirtuaAPUPCMReadback {
    uint8_t pcm12;
    uint8_t pcm34;
} VirtuaAPUPCMReadback;

void virtuapu_mode2_reset(void);
void virtuapu_mode2_sync_audio_registers(const VirtuaAPUMemory *regs);
void virtuapu_mode2_step_frame_sequencer(void);
void virtuapu_mode2_render_audio(const VirtuaAPUMemory *regs, uint32_t sample_count);
VirtuaAPUPCMReadback virtuapu_mode2_get_pcm_readback(void);

#ifdef __cplusplus
}
#endif
