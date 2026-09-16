#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    VIRTUAAPU_MAX_RENDER_SAMPLES = 4096,
    VIRTUAAPU_AMEM_SIZE = 256
};

typedef struct VirtuaAPUMemory {
    uint8_t mode;
    uint32_t sample_rate;
    uint32_t fifo_sample_rate;
    bool double_speed;
} VirtuaAPUMemory;

extern int16_t virtuapu_audio_buffer[VIRTUAAPU_MAX_RENDER_SAMPLES * 2];
extern uint8_t virtuapu_amem[VIRTUAAPU_AMEM_SIZE];
extern VirtuaAPUMemory virtuapu_registers;

#ifdef __cplusplus
}
#endif
