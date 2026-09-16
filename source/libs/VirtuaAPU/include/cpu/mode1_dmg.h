#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "../apu_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DMGLayout {
    uint8_t nr10;
    uint8_t nr11;
    uint8_t nr12;
    uint8_t nr13;
    uint8_t nr14;
    uint8_t pad15;
    uint8_t nr21;
    uint8_t nr22;
    uint8_t nr23;
    uint8_t nr24;
    uint8_t nr30;
    uint8_t nr31;
    uint8_t nr32;
    uint8_t nr33;
    uint8_t nr34;
    uint8_t pad1f;
    uint8_t nr41;
    uint8_t nr42;
    uint8_t nr43;
    uint8_t nr44;
    uint8_t nr50;
    uint8_t nr51;
    uint8_t nr52;
    uint8_t pad27_2f[9];
    uint8_t wave_ram[16];
} DMGLayout;

typedef struct FrameSequencer {
    uint32_t step_counter;
    int32_t length[4];
    uint32_t volume[4];
    uint32_t frequency[4];
    int32_t env_direction[4];
    uint32_t env_period[4];
    uint32_t env_period_timer[4];
    bool env_overflow[4];
    uint32_t sweep_period;
    uint32_t sweep_timer;
    uint32_t sweep_shadow_frequency;
    int32_t sweep_direction;
    uint32_t sweep_shift;
    bool sweep_enable;
    bool sweep_subtracted;
    bool use_length[4];
    bool active[4];
    bool powered[4];
    float chan_t[4];
    uint16_t lfsr4;
} FrameSequencer;

void virtuapu_mode1_reset(void);
void virtuapu_mode1_sync_audio_registers(const VirtuaAPUMemory *regs);
void virtuapu_mode1_step_frame_sequencer(void);
void virtuapu_mode1_render_audio(const VirtuaAPUMemory *regs, uint32_t sample_count);
const FrameSequencer *virtuapu_mode1_get_frame_sequencer(void);
const DMGLayout *virtuapu_mode1_get_layout(void);

#ifdef __cplusplus
}
#endif
