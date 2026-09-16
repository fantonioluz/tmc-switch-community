#include "cpu/mode1_dmg.h"

#include <math.h>
#include <string.h>

enum {
    MODE1_CPU_CLOCK = 4194304
};

_Static_assert(sizeof(DMGLayout) == 48, "DMGLayout must be 48 bytes");

typedef struct VirtuaAPUMode1State {
    float capacitor_l;
    float capacitor_r;
    FrameSequencer seq;
    float frame_seq_accum;
    uint8_t prev_trig;
    uint8_t prev_regs[48];
} VirtuaAPUMode1State;

static VirtuaAPUMode1State g_mode1_audio;

static DMGLayout *mode1_get_layout_mut(void)
{
    return (DMGLayout *)virtuapu_amem;
}

const DMGLayout *virtuapu_mode1_get_layout(void)
{
    return (const DMGLayout *)virtuapu_amem;
}

const FrameSequencer *virtuapu_mode1_get_frame_sequencer(void)
{
    return &g_mode1_audio.seq;
}

static uint8_t read_wave_ram(const DMGLayout *layout, int index)
{
    return layout->wave_ram[index];
}

static int32_t length_from_reg(int chan, uint8_t len_reg)
{
    if (chan == 2) {
        return 256 - (int32_t)len_reg;
    }
    return 64 - (int32_t)(len_reg & 0x3F);
}

static float polyblep(float t, float dt)
{
    if (t <= dt) {
        t = t / dt;
        return t + t - t * t - 1.0f;
    }
    if (t >= 1.0f - dt) {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

static float bandlimited_square(float t, float duty_cycle, float dt)
{
    float t2 = t - duty_cycle;
    float y;

    if (t2 < 0.0f) {
        t2 += 1.0f;
    }

    y = (t < duty_cycle) ? -1.0f : 1.0f;
    y -= polyblep(t, dt);
    y += polyblep(t2, dt);
    return y;
}

static uint32_t compute_next_sweep_freq(FrameSequencer *seq, bool *overflow)
{
    uint32_t shadow = seq->sweep_shadow_frequency & 0x7FFu;
    uint32_t delta = shadow >> seq->sweep_shift;

    if (seq->sweep_direction == -1) {
        if (overflow != NULL) {
            *overflow = false;
        }
        return (shadow - delta) & 0x7FFu;
    }

    {
        uint32_t sum = shadow + delta;
        if (overflow != NULL) {
            *overflow = sum > 0x7FFu;
        }
        return sum & 0x7FFu;
    }
}

static void write_ch1_freq(DMGLayout *layout, uint32_t freq)
{
    layout->nr13 = (uint8_t)(freq & 0xFFu);
    layout->nr14 = (uint8_t)((layout->nr14 & 0x78u) | ((freq >> 8) & 0x07u));
}

static void tick_frame_seq(FrameSequencer *seq, DMGLayout *layout)
{
    int step = (int)(seq->step_counter++ % 8u);
    int i;

    if (step == 2 || step == 6) {
        if (seq->active[0] && seq->sweep_enable) {
            if (seq->sweep_timer > 0) {
                seq->sweep_timer--;
            }
            if (seq->sweep_timer == 0) {
                if (seq->sweep_period > 0) {
                    bool overflow = false;
                    uint32_t nf;

                    seq->sweep_timer = seq->sweep_period;
                    nf = compute_next_sweep_freq(seq, &overflow);
                    if (overflow) {
                        seq->active[0] = false;
                    } else if (seq->sweep_direction == -1) {
                        seq->sweep_subtracted = true;
                    }

                    if (!overflow && seq->sweep_shift != 0) {
                        seq->sweep_shadow_frequency = nf;
                        seq->frequency[0] = nf;
                        write_ch1_freq(layout, nf);
                        nf = compute_next_sweep_freq(seq, &overflow);
                        if (overflow) {
                            seq->active[0] = false;
                        }
                    }
                } else {
                    seq->sweep_timer = 8;
                }
            }
        }
    }

    if (step == 7) {
        for (i = 0; i < 4; ++i) {
            int v;

            if (i == 2) {
                continue;
            }
            if (seq->env_period[i] == 0) {
                continue;
            }
            if (seq->env_period_timer[i] > 0) {
                seq->env_period_timer[i]--;
            }
            if (seq->env_period_timer[i] != 0) {
                continue;
            }

            seq->env_period_timer[i] = seq->env_period[i];
            v = (int)seq->volume[i] + seq->env_direction[i];
            if (v <= 0) {
                v = 0;
                seq->env_overflow[i] = true;
            }
            if (v > 0xF) {
                v = 0xF;
                seq->env_overflow[i] = true;
            }
            seq->volume[i] = (uint32_t)v;
        }
    }

    if ((step % 2) == 0) {
        for (i = 0; i < 4; ++i) {
            if (!seq->use_length[i]) {
                continue;
            }
            if (seq->length[i] > 0) {
                seq->length[i]--;
            }
            if (seq->length[i] == 0) {
                seq->active[i] = false;
            }
        }
    }
}

static void process_audio_writes(DMGLayout *layout)
{
    FrameSequencer *seq = &g_mode1_audio.seq;
    uint8_t last_write = layout->pad27_2f[0];
    uint8_t nr52 = layout->nr52 & 0xF0u;
    bool master_enable = ((nr52 >> 7) & 1u) != 0;
    bool prev_master_enable = (g_mode1_audio.prev_regs[0x16] & 0x80u) != 0;
    int i;

    if (!prev_master_enable && master_enable) {
        seq->step_counter = 0;
        seq->chan_t[0] = 0.0f;
        seq->chan_t[1] = 0.0f;
        seq->chan_t[2] = 0.0f;
    }

    for (i = 0; i < 4; ++i) {
        uint8_t len_reg;
        uint8_t prev_len_reg;
        uint8_t len_write_index;

        switch (i) {
        case 0:
            len_reg = layout->nr11;
            prev_len_reg = g_mode1_audio.prev_regs[1];
            len_write_index = 0x01u;
            break;
        case 1:
            len_reg = layout->nr21;
            prev_len_reg = g_mode1_audio.prev_regs[6];
            len_write_index = 0x06u;
            break;
        case 2:
            len_reg = layout->nr31;
            prev_len_reg = g_mode1_audio.prev_regs[11];
            len_write_index = 0x0Bu;
            break;
        default:
            len_reg = layout->nr41;
            prev_len_reg = g_mode1_audio.prev_regs[16];
            len_write_index = 0x10u;
            break;
        }

        if (len_reg != prev_len_reg || last_write == len_write_index) {
            seq->length[i] = length_from_reg(i, len_reg);
        }
    }

    if (!master_enable) {
        for (i = 0; i < 4; ++i) {
            seq->active[i] = false;
            seq->powered[i] = false;
        }
    } else {
        uint8_t sweep = layout->nr10;

        seq->sweep_period = (sweep >> 4) & 7u;
        seq->sweep_shift = sweep & 7u;
        seq->sweep_direction = (sweep & 0x08u) ? -1 : 1;

        for (i = 0; i < 4; ++i) {
            uint8_t freq_hi;
            uint8_t freq_hi_write_index;
            bool wrote_freq_hi;
            bool prev_length_en;
            uint8_t len_reg;
            uint8_t vol_env;
            bool next_step_no_length_clock;
            bool trig_bit;
            bool triggered;

            prev_length_en = seq->use_length[i];

            switch (i) {
            case 0:
                freq_hi = layout->nr14;
                freq_hi_write_index = 0x04u;
                len_reg = layout->nr11;
                vol_env = layout->nr12;
                break;
            case 1:
                freq_hi = layout->nr24;
                freq_hi_write_index = 0x09u;
                len_reg = layout->nr21;
                vol_env = layout->nr22;
                break;
            case 2:
                freq_hi = layout->nr34;
                freq_hi_write_index = 0x0Eu;
                len_reg = layout->nr31;
                vol_env = 0;
                break;
            default:
                freq_hi = layout->nr44;
                freq_hi_write_index = 0x13u;
                len_reg = layout->nr41;
                vol_env = layout->nr42;
                break;
            }

            wrote_freq_hi = last_write == freq_hi_write_index;
            seq->use_length[i] = ((freq_hi >> 6) & 1u) != 0;

            if (i == 2) {
                seq->powered[i] = ((layout->nr30 >> 7) & 1u) != 0;
            } else {
                bool power = ((vol_env >> 3) & 0x1Fu) != 0;
                seq->powered[i] = power;
                seq->active[i] = seq->active[i] && power;
            }
            seq->active[i] = seq->active[i] && seq->powered[i];

            if (i == 2) {
                seq->env_direction[i] = 0;
                seq->env_period[i] = 0;
            } else {
                seq->env_direction[i] = ((vol_env >> 3) & 1u) ? 1 : -1;
                seq->env_period[i] = vol_env & 7u;
            }

            if (i < 3) {
                uint8_t freq_lo;

                switch (i) {
                case 0:
                    freq_lo = layout->nr13;
                    break;
                case 1:
                    freq_lo = layout->nr23;
                    break;
                default:
                    freq_lo = layout->nr33;
                    break;
                }
                seq->frequency[i] = (uint32_t)freq_lo | (((uint32_t)(freq_hi & 7u)) << 8);
            }

            next_step_no_length_clock = (seq->step_counter & 1u) != 0;
            trig_bit = ((freq_hi >> 7) & 1u) != 0;
            triggered = wrote_freq_hi && trig_bit;

            if (wrote_freq_hi && seq->use_length[i] && !prev_length_en && next_step_no_length_clock) {
                if (seq->length[i] != 0) {
                    seq->length[i]--;
                    if (seq->length[i] == 0 && !triggered) {
                        seq->active[i] = false;
                    }
                }
            }

            if (triggered) {
                uint8_t freq_lo = 0;

                switch (i) {
                case 0:
                    freq_lo = layout->nr13;
                    break;
                case 1:
                    freq_lo = layout->nr23;
                    break;
                case 2:
                    freq_lo = layout->nr33;
                    break;
                default:
                    break;
                }

                if (i < 3) {
                    seq->frequency[i] = (uint32_t)freq_lo | (((uint32_t)(freq_hi & 7u)) << 8);
                }

                seq->volume[i] = (uint32_t)((vol_env >> 4) & 0xFu);

                if (seq->length[i] == 0) {
                    seq->length[i] = (i == 2) ? 256 : 64;
                    if (seq->use_length[i] && next_step_no_length_clock) {
                        seq->length[i]--;
                    }
                }

                if (i == 3) {
                    seq->lfsr4 = 0x7FFFu;
                }

                seq->env_period_timer[i] = seq->env_period[i] ? seq->env_period[i] : 8u;
                seq->env_overflow[i] = false;
                seq->chan_t[i] = 0.0f;
                seq->active[i] = true;

                if (i == 0) {
                    bool overflow = false;

                    seq->sweep_shadow_frequency = seq->frequency[0];
                    seq->sweep_subtracted = false;
                    seq->sweep_enable = seq->sweep_period != 0 || seq->sweep_shift != 0;
                    seq->sweep_timer = seq->sweep_period;
                    if (seq->sweep_timer == 0) {
                        seq->sweep_timer = 8;
                    }
                    (void)compute_next_sweep_freq(seq, &overflow);
                    if (seq->sweep_shift != 0 && seq->sweep_direction == -1 && !overflow) {
                        seq->sweep_subtracted = true;
                    }
                    if (seq->sweep_shift != 0 && overflow) {
                        seq->active[0] = false;
                    }
                    seq->sweep_enable = seq->sweep_period > 0 || seq->sweep_shift > 0;
                }
            }

            if (i == 0 &&
                last_write == 0x00u &&
                (g_mode1_audio.prev_regs[0] & 0x08u) != 0 &&
                (layout->nr10 & 0x08u) == 0 &&
                seq->sweep_subtracted) {
                seq->active[0] = false;
                seq->sweep_enable = false;
            }

            switch (i) {
            case 0:
                layout->nr14 &= 0x7Fu;
                break;
            case 1:
                layout->nr24 &= 0x7Fu;
                break;
            case 2:
                layout->nr34 &= 0x7Fu;
                break;
            default:
                layout->nr44 &= 0x7Fu;
                break;
            }
        }
    }

    nr52 = layout->nr52 & 0xF0u;
    for (i = 0; i < 4; ++i) {
        seq->active[i] = seq->active[i] && seq->powered[i];
        if (seq->active[i]) {
            nr52 |= (uint8_t)(1u << i);
        }
    }
    layout->nr52 = nr52;
    memcpy(g_mode1_audio.prev_regs, layout, sizeof(g_mode1_audio.prev_regs));
}

void virtuapu_mode1_reset(void)
{
    memset(&g_mode1_audio, 0, sizeof(g_mode1_audio));
    g_mode1_audio.seq.lfsr4 = 0x7FFFu;
    g_mode1_audio.frame_seq_accum = 0.0f;
}

void virtuapu_mode1_sync_audio_registers(const VirtuaAPUMemory *regs)
{
    (void)regs;
    process_audio_writes(mode1_get_layout_mut());
}

void virtuapu_mode1_step_frame_sequencer(void)
{
    DMGLayout *layout = mode1_get_layout_mut();
    uint8_t nr52;
    int i;

    if ((layout->nr52 & 0x80u) == 0) {
        return;
    }

    tick_frame_seq(&g_mode1_audio.seq, layout);

    nr52 = layout->nr52 & 0xF0u;
    for (i = 0; i < 4; ++i) {
        g_mode1_audio.seq.active[i] = g_mode1_audio.seq.active[i] && g_mode1_audio.seq.powered[i];
        if (g_mode1_audio.seq.active[i]) {
            nr52 |= (uint8_t)(1u << i);
        }
    }
    layout->nr52 = nr52;
}

void virtuapu_mode1_render_audio(const VirtuaAPUMemory *regs, uint32_t sample_count)
{
    static const float duty_lookup[4] = {0.125f, 0.25f, 0.5f, 0.75f};
    DMGLayout *layout;
    FrameSequencer *seq;
    uint32_t sample_rate;
    bool master_enable;
    float sample_delta_t;
    float duty1;
    float duty2;
    uint8_t power3;
    uint8_t vol3;
    int channel3_shift;
    uint8_t poly4;
    float r4;
    uint8_t s4;
    bool seven_bit;
    uint8_t nr50;
    float master_left;
    float master_right;
    uint8_t nr51;
    float chan_l[4];
    float chan_r[4];
    float fhz1;
    float fhz2;
    float fhz3;
    uint32_t i;
    int c;

    if (sample_count == 0 || sample_count > VIRTUAAPU_MAX_RENDER_SAMPLES || regs == NULL) {
        return;
    }

    layout = mode1_get_layout_mut();
    seq = &g_mode1_audio.seq;

    virtuapu_mode1_sync_audio_registers(regs);

    sample_rate = (regs->sample_rate > 0) ? regs->sample_rate : 48000u;
    master_enable = ((layout->nr52 >> 7) & 1u) != 0;
    if (!master_enable) {
        memset(virtuapu_audio_buffer, 0, sample_count * 2u * sizeof(int16_t));
        return;
    }

    sample_delta_t = 1.0f / (float)sample_rate;

    duty1 = duty_lookup[(layout->nr11 >> 6) & 3u];
    duty2 = duty_lookup[(layout->nr21 >> 6) & 3u];

    power3 = layout->nr30;
    vol3 = layout->nr32;
    channel3_shift = ((vol3 >> 5) & 3u) - 1;
    if (((power3 >> 7) & 1u) == 0 || channel3_shift == -1) {
        channel3_shift = 4;
    }

    poly4 = layout->nr43;
    r4 = (float)(poly4 & 7u);
    s4 = (uint8_t)((poly4 >> 4) & 0xFu);
    seven_bit = ((poly4 >> 3) & 1u) != 0;
    if (r4 == 0.0f) {
        r4 = 0.5f;
    }

    nr50 = layout->nr50;
    master_left = (float)((nr50 >> 4) & 7u) / 7.0f;
    master_right = (float)(nr50 & 7u) / 7.0f;

    nr51 = layout->nr51;
    for (c = 0; c < 4; ++c) {
        chan_r[c] = (float)((nr51 >> c) & 1u);
        chan_l[c] = (float)((nr51 >> (c + 4)) & 1u);
    }

    fhz1 = (seq->frequency[1] < 2048u) ? 131072.0f / (float)(2048u - seq->frequency[1]) : 131072.0f;
    fhz2 = (seq->frequency[2] < 2048u) ? 65536.0f / (float)(2048u - seq->frequency[2]) : 65536.0f;
    fhz3 = 524288.0f / r4 / (float)(1u << (s4 + 1u));

    for (i = 0; i < sample_count; ++i) {
        float fhz0;
        float v[4];
        float channels[4];
        float sample_l = 0.0f;
        float sample_r = 0.0f;
        float out_l;
        float out_r;

        fhz0 = (seq->frequency[0] < 2048u) ? 131072.0f / (float)(2048u - seq->frequency[0]) : 131072.0f;

        seq->chan_t[0] += sample_delta_t * fhz0;
        seq->chan_t[1] += sample_delta_t * fhz1;
        seq->chan_t[2] += sample_delta_t * fhz2;
        seq->chan_t[3] += sample_delta_t * fhz3;

        while (seq->chan_t[3] >= 1.0f) {
            int bit = (seq->lfsr4 ^ (seq->lfsr4 >> 1)) & 1;

            seq->chan_t[3] -= 1.0f;
            seq->lfsr4 >>= 1;
            seq->lfsr4 |= (uint16_t)(bit << 14);
            if (seven_bit) {
                seq->lfsr4 &= (uint16_t)~(1u << 7);
                seq->lfsr4 |= (uint16_t)(bit << 6);
            }
        }

        for (c = 0; c < 3; ++c) {
            seq->chan_t[c] -= (float)((int)seq->chan_t[c]);
        }

        for (c = 0; c < 4; ++c) {
            v[c] = seq->active[c] ? (float)seq->volume[c] / 15.0f : 0.0f;
        }
        v[2] = 1.0f;

        channels[0] = bandlimited_square(seq->chan_t[0], duty1, sample_delta_t * fhz0) * v[0];
        channels[1] = bandlimited_square(seq->chan_t[1], duty2, sample_delta_t * fhz1) * v[1];

        {
            unsigned wav_samp = ((unsigned)(seq->chan_t[2] * 32.0f)) % 32u;
            int dat_byte = read_wave_ram(layout, (int)(wav_samp / 2u));
            int offset = (wav_samp & 1u) ? 0 : 4;
            int sample4 = (dat_byte >> offset) & 0xFu;
            int dat = sample4 >> channel3_shift;
            int wav_offset = 8 >> channel3_shift;

            channels[2] = (float)(dat - wav_offset) / 8.0f;
        }

        channels[3] = ((float)(seq->lfsr4 & 1u) * 2.0f - 1.0f) * v[3];

        for (c = 0; c < 4; ++c) {
            float l = channels[c] * chan_l[c];
            float r = channels[c] * chan_r[c];

            if (l >= -2.0f && l <= 2.0f) {
                sample_l += l;
            }
            if (r >= -2.0f && r <= 2.0f) {
                sample_r += r;
            }
        }

        sample_l *= 0.25f;
        sample_r *= 0.25f;
        sample_l *= master_left;
        sample_r *= master_right;

        if (sample_l > 1.0f) {
            sample_l = 1.0f;
        }
        if (sample_l < -1.0f) {
            sample_l = -1.0f;
        }
        if (sample_r > 1.0f) {
            sample_r = 1.0f;
        }
        if (sample_r < -1.0f) {
            sample_r = -1.0f;
        }

        if (!(g_mode1_audio.capacitor_l < 2.0f && g_mode1_audio.capacitor_l > -2.0f)) {
            g_mode1_audio.capacitor_l = 0.0f;
        }
        if (!(g_mode1_audio.capacitor_r < 2.0f && g_mode1_audio.capacitor_r > -2.0f)) {
            g_mode1_audio.capacitor_r = 0.0f;
        }

        out_l = sample_l - g_mode1_audio.capacitor_l;
        out_r = sample_r - g_mode1_audio.capacitor_r;
        g_mode1_audio.capacitor_l = (sample_l - out_l) * 0.996f;
        g_mode1_audio.capacitor_r = (sample_r - out_r) * 0.996f;

        virtuapu_audio_buffer[i * 2u + 0u] = (int16_t)(out_l * 32760.0f);
        virtuapu_audio_buffer[i * 2u + 1u] = (int16_t)(out_r * 32760.0f);
    }
}
