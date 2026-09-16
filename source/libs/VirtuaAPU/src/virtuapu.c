#include "virtuapu.h"

#include <string.h>

#include "cpu/mode1_dmg.h"
#include "cpu/mode2_cgb.h"
#include "cpu/mode3_gba.h"

int16_t virtuapu_audio_buffer[VIRTUAAPU_MAX_RENDER_SAMPLES * 2];
uint8_t virtuapu_amem[VIRTUAAPU_AMEM_SIZE];
VirtuaAPUMemory virtuapu_registers;

void virtuapu_reset(void)
{
    VirtuaAPUMemory saved = virtuapu_registers;

    memset(virtuapu_audio_buffer, 0, sizeof(virtuapu_audio_buffer));
    memset(virtuapu_amem, 0, sizeof(virtuapu_amem));
    virtuapu_registers = saved;

    if (virtuapu_registers.mode == VIRTUAAPU_MODE_CGB) {
        virtuapu_mode2_reset();
    } else if (virtuapu_registers.mode == VIRTUAAPU_MODE_GBA) {
        virtuapu_mode3_reset();
    } else {
        virtuapu_mode1_reset();
    }
}

void virtuapu_render_audio(uint32_t sample_count)
{
    if (virtuapu_registers.mode == VIRTUAAPU_MODE_CGB) {
        virtuapu_mode2_render_audio(&virtuapu_registers, sample_count);
    } else if (virtuapu_registers.mode == VIRTUAAPU_MODE_GBA) {
        virtuapu_mode3_render_audio(&virtuapu_registers, sample_count);
    } else {
        virtuapu_mode1_render_audio(&virtuapu_registers, sample_count);
    }
}

void virtuapu_sync_audio(void)
{
    if (virtuapu_registers.mode == VIRTUAAPU_MODE_CGB) {
        virtuapu_mode2_sync_audio_registers(&virtuapu_registers);
    } else if (virtuapu_registers.mode == VIRTUAAPU_MODE_GBA) {
        virtuapu_mode3_sync_audio_registers(&virtuapu_registers);
    } else {
        virtuapu_mode1_sync_audio_registers(&virtuapu_registers);
    }
}

void virtuapu_step_frame_sequencer(void)
{
    if (virtuapu_registers.mode == VIRTUAAPU_MODE_CGB) {
        virtuapu_mode2_step_frame_sequencer();
    } else if (virtuapu_registers.mode == VIRTUAAPU_MODE_GBA) {
        virtuapu_mode3_step_frame_sequencer();
    } else {
        virtuapu_mode1_step_frame_sequencer();
    }
}

void virtuapu_fifo_write_a(uint32_t value)
{
    if (virtuapu_registers.mode == VIRTUAAPU_MODE_GBA) {
        virtuapu_mode3_fifo_write_a(value);
    }
}

void virtuapu_fifo_write_b(uint32_t value)
{
    if (virtuapu_registers.mode == VIRTUAAPU_MODE_GBA) {
        virtuapu_mode3_fifo_write_b(value);
    }
}

uint32_t virtuapu_fifo_level_a(void)
{
    if (virtuapu_registers.mode == VIRTUAAPU_MODE_GBA) {
        return virtuapu_mode3_fifo_level_a();
    }
    return 0u;
}

uint32_t virtuapu_fifo_level_b(void)
{
    if (virtuapu_registers.mode == VIRTUAAPU_MODE_GBA) {
        return virtuapu_mode3_fifo_level_b();
    }
    return 0u;
}

int16_t *virtuapu_get_audio_buffer(void)
{
    return virtuapu_audio_buffer;
}

uint8_t *virtuapu_get_amem(void)
{
    return virtuapu_amem;
}

VirtuaAPUMemory *virtuapu_get_registers(void)
{
    return &virtuapu_registers;
}
