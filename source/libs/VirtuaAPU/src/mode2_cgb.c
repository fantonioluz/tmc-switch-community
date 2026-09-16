#include "cpu/mode2_cgb.h"

void virtuapu_mode2_reset(void)
{
    virtuapu_mode1_reset();
}

void virtuapu_mode2_sync_audio_registers(const VirtuaAPUMemory *regs)
{
    virtuapu_mode1_sync_audio_registers(regs);
}

void virtuapu_mode2_step_frame_sequencer(void)
{
    virtuapu_mode1_step_frame_sequencer();
}

void virtuapu_mode2_render_audio(const VirtuaAPUMemory *regs, uint32_t sample_count)
{
    virtuapu_mode1_render_audio(regs, sample_count);
}

VirtuaAPUPCMReadback virtuapu_mode2_get_pcm_readback(void)
{
    const FrameSequencer *seq = virtuapu_mode1_get_frame_sequencer();
    VirtuaAPUPCMReadback readback;

    readback.pcm12 = (uint8_t)((seq->active[1] ? (seq->volume[1] & 0x0Fu) : 0u) << 4);
    readback.pcm12 |= (uint8_t)(seq->active[0] ? (seq->volume[0] & 0x0Fu) : 0u);
    readback.pcm34 = (uint8_t)((seq->active[3] ? (seq->volume[3] & 0x0Fu) : 0u) << 4);
    return readback;
}
