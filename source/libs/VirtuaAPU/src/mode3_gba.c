#include "cpu/mode3_gba.h"

#include <string.h>

enum {
    MODE3_FIFO_CAPACITY = 32
};

typedef struct DirectSoundFifo {
    int8_t samples[MODE3_FIFO_CAPACITY];
    uint8_t read_index;
    uint8_t write_index;
    uint8_t count;
    float phase;
    float current_sample;
} DirectSoundFifo;

typedef struct VirtuaAPUMode3State {
    DirectSoundFifo fifo_a;
    DirectSoundFifo fifo_b;
} VirtuaAPUMode3State;

static VirtuaAPUMode3State g_mode3_audio;

_Static_assert(sizeof(GBAAPULayout) <= VIRTUAAPU_AMEM_SIZE, "GBAAPULayout exceeds audio memory");

static GBAAPULayout *mode3_get_layout_mut(void)
{
    return (GBAAPULayout *)virtuapu_amem;
}

const GBAAPULayout *virtuapu_mode3_get_layout(void)
{
    return (const GBAAPULayout *)virtuapu_amem;
}

static void fifo_reset(DirectSoundFifo *fifo)
{
    memset(fifo, 0, sizeof(*fifo));
    fifo->phase = 1.0f;
}

static void fifo_push_sample(DirectSoundFifo *fifo, int8_t sample)
{
    if (fifo->count >= MODE3_FIFO_CAPACITY) {
        return;
    }

    fifo->samples[fifo->write_index] = sample;
    fifo->write_index = (uint8_t)((fifo->write_index + 1u) % MODE3_FIFO_CAPACITY);
    fifo->count++;
}

static void fifo_push_word(DirectSoundFifo *fifo, uint32_t value)
{
    uint32_t i;

    for (i = 0; i < 4; ++i) {
        fifo_push_sample(fifo, (int8_t)(value >> (i * 8u)));
    }
}

static float fifo_pop_resampled(DirectSoundFifo *fifo, float phase_step)
{
    if (phase_step <= 0.0f) {
        return fifo->current_sample;
    }

    fifo->phase += phase_step;
    while (fifo->phase >= 1.0f) {
        fifo->phase -= 1.0f;
        if (fifo->count != 0u) {
            fifo->current_sample = (float)fifo->samples[fifo->read_index] / 128.0f;
            fifo->read_index = (uint8_t)((fifo->read_index + 1u) % MODE3_FIFO_CAPACITY);
            fifo->count--;
        } else {
            fifo->current_sample = 0.0f;
        }
    }

    return fifo->current_sample;
}

static float clamp_unit(float value)
{
    if (value > 1.0f) {
        return 1.0f;
    }
    if (value < -1.0f) {
        return -1.0f;
    }
    return value;
}

void virtuapu_mode3_reset(void)
{
    memset(&g_mode3_audio, 0, sizeof(g_mode3_audio));
    fifo_reset(&g_mode3_audio.fifo_a);
    fifo_reset(&g_mode3_audio.fifo_b);
    virtuapu_mode1_reset();
}

void virtuapu_mode3_sync_audio_registers(const VirtuaAPUMemory *regs)
{
    GBAAPULayout *layout = mode3_get_layout_mut();
    uint16_t soundcnt_h;

    virtuapu_mode1_sync_audio_registers(regs);

    soundcnt_h = layout->soundcnt_h;
    if ((soundcnt_h & (1u << 11)) != 0u) {
        fifo_reset(&g_mode3_audio.fifo_a);
    }
    if ((soundcnt_h & (1u << 15)) != 0u) {
        fifo_reset(&g_mode3_audio.fifo_b);
    }

    layout->soundcnt_h &= (uint16_t)~((1u << 11) | (1u << 15));
}

void virtuapu_mode3_step_frame_sequencer(void)
{
    virtuapu_mode1_step_frame_sequencer();
}

void virtuapu_mode3_render_audio(const VirtuaAPUMemory *regs, uint32_t sample_count)
{
    GBAAPULayout *layout;
    uint32_t sample_rate;
    uint32_t fifo_rate;
    float phase_step;
    uint16_t soundcnt_h;
    float fifo_a_volume;
    float fifo_b_volume;
    uint32_t i;

    if (sample_count == 0u || sample_count > VIRTUAAPU_MAX_RENDER_SAMPLES || regs == NULL) {
        return;
    }

    virtuapu_mode1_render_audio(regs, sample_count);

    layout = mode3_get_layout_mut();
    if ((layout->psg.nr52 & 0x80u) == 0u) {
        return;
    }

    soundcnt_h = layout->soundcnt_h;
    sample_rate = (regs->sample_rate != 0u) ? regs->sample_rate : 48000u;
    fifo_rate = (regs->fifo_sample_rate != 0u) ? regs->fifo_sample_rate : 32768u;
    phase_step = (float)fifo_rate / (float)sample_rate;
    fifo_a_volume = ((soundcnt_h & (1u << 2)) != 0u) ? 1.0f : 0.5f;
    fifo_b_volume = ((soundcnt_h & (1u << 3)) != 0u) ? 1.0f : 0.5f;

    for (i = 0; i < sample_count; ++i) {
        float base_l = (float)virtuapu_audio_buffer[i * 2u + 0u] / 32760.0f;
        float base_r = (float)virtuapu_audio_buffer[i * 2u + 1u] / 32760.0f;
        float ds_a = fifo_pop_resampled(&g_mode3_audio.fifo_a, phase_step) * fifo_a_volume * 0.35f;
        float ds_b = fifo_pop_resampled(&g_mode3_audio.fifo_b, phase_step) * fifo_b_volume * 0.35f;
        float mix_l = base_l;
        float mix_r = base_r;

        if ((soundcnt_h & (1u << 9)) != 0u) {
            mix_l += ds_a;
        }
        if ((soundcnt_h & (1u << 8)) != 0u) {
            mix_r += ds_a;
        }
        if ((soundcnt_h & (1u << 13)) != 0u) {
            mix_l += ds_b;
        }
        if ((soundcnt_h & (1u << 12)) != 0u) {
            mix_r += ds_b;
        }

        virtuapu_audio_buffer[i * 2u + 0u] = (int16_t)(clamp_unit(mix_l) * 32760.0f);
        virtuapu_audio_buffer[i * 2u + 1u] = (int16_t)(clamp_unit(mix_r) * 32760.0f);
    }
}

void virtuapu_mode3_fifo_write_a(uint32_t value)
{
    fifo_push_word(&g_mode3_audio.fifo_a, value);
}

void virtuapu_mode3_fifo_write_b(uint32_t value)
{
    fifo_push_word(&g_mode3_audio.fifo_b, value);
}

uint32_t virtuapu_mode3_fifo_level_a(void)
{
    return g_mode3_audio.fifo_a.count;
}

uint32_t virtuapu_mode3_fifo_level_b(void)
{
    return g_mode3_audio.fifo_b.count;
}
