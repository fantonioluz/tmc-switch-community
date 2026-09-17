"""Exercise production clock conversion and menu affine save/restore on host."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def function(text, signature):
    start = text.index(signature)
    brace = text.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


def run():
    shim = (ROOT / 'platforms/switch/compat/sdl3_to_sdl2.h').read_text(encoding='utf-8')
    menu = (ROOT / 'src/subtask.c').read_text(encoding='utf-8')
    # Compile the actual production operations, including their PC_PORT branches.
    save = menu.split('MemCopy(&gRoomControls, &gUI.roomControls, sizeof(RoomControls));', 1)[1]
    save = save.split('MemCopy(&gActiveScriptInfo', 1)[0]
    restore = menu.split('MemCopy(&gUI.activeScriptInfo, &gActiveScriptInfo, sizeof(ActiveScriptInfo));', 1)[1]
    restore = restore.split('MemCopy(gUI.palettes', 1)[0]
    code = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "structures.h"
static uint64_t counter, frequency;
static uint64_t SDL_GetPerformanceCounter(void) { return counter; }
static uint64_t SDL_GetPerformanceFrequency(void) { return frequency; }
UI gUI;
OAMControls gOAMControls;
/* Deliberately separate, as in the native port. */
unsigned char gUnk_03000420[0x800];
void MemCopy(const void* src, void* dst, u32 size) { memcpy(dst, src, size); }
'''
    code += function(shim, 'static inline uint64_t sdl3compat_GetTicksNS(void)')
    code += '\nstatic void save_menu(void) {\n' + save + '\n}\n'
    code += '\nstatic void restore_menu(void) {\n' + restore + '\n}\n'
    code += r'''
int main(void) {
    const uint64_t frequencies[] = {19200000, 1000000000, 10000000};
    for (unsigned f = 0; f < sizeof(frequencies)/sizeof(*frequencies); f++) {
        frequency = frequencies[f];
        uint64_t boundary = UINT64_MAX / 1000000000ULL;
        uint64_t previous = 0;
        /* The old conversion jumps backwards at precisely this boundary. */
        assert((boundary * 1000000000ULL) / frequency >
               ((boundary + 1) * 1000000000ULL) / frequency);
        for (counter = boundary - 100; counter <= boundary + 100; counter++) {
            uint64_t now = sdl3compat_GetTicksNS();
            assert(now >= previous);
            assert(now == (counter / frequency) * 1000000000ULL +
                          (counter % frequency) * 1000000000ULL / frequency);
            previous = now;
        }
        for (unsigned days = 1; days <= 365; days++) {
            counter = frequency * 86400ULL * days;
            assert(sdl3compat_GetTicksNS() == 86400ULL * days * 1000000000ULL);
        }
        /* A frame deadline spanning the old wrap must finish in ~one frame. */
        counter = boundary - frequency / 120;
        uint64_t deadline = sdl3compat_GetTicksNS() + 16666667;
        unsigned waits = 0;
        while (sdl3compat_GetTicksNS() < deadline && waits < 100) {
            counter += frequency / 1000;
            waits++;
        }
        assert(waits >= 16 && waits <= 18);
    }
    _Static_assert(sizeof(gUI.unk_2a8) == 32 * sizeof(OAMObj), "32 affine slots");
    for (unsigned cycle = 0; cycle < 3; cycle++) {
        memset(gOAMControls.unk, 0, sizeof(gOAMControls.unk));
        for (unsigned i = 1; i < 32; i++) {
            /* Static book scale and occupied-slot marker, plus other actors. */
            uint16_t params[] = {0x80, (uint16_t)(0x80 + i), (uint16_t)(i << 8)};
            memcpy(&gOAMControls.unk[i], params, sizeof(params));
            gOAMControls.unk[i].unk6 = 1;
        }
        unsigned char expected[256];
        memcpy(expected, gOAMControls.unk, sizeof(expected));
        expected[7] = 1; /* Restoration must request CopyOAM to rebuild matrices. */
        save_menu();
        memset(gOAMControls.unk, 0, 256); /* Subtask_Init */
        memset(gOAMControls.unk, 0x5a, 256); /* Menu uses its own transforms. */
        restore_menu();
        assert(memcmp(expected, gOAMControls.unk, sizeof(expected)) == 0);
    }
    puts("PASS: clock wrap, year-long uptime, frame deadline, repeated menu affine restoration");
}
'''
    with tempfile.TemporaryDirectory() as tmp:
        src = Path(tmp) / 'test.c'
        exe = Path(tmp) / 'test'
        src.write_text(code, encoding='utf-8')
        subprocess.run(['gcc', '-std=gnu11', '-DPC_PORT', '-DUSA', '-DENGLISH',
                        '-Iinclude', '-Iport', '-I.', '-O1', '-g',
                        '-fsanitize=address,undefined', str(src), '-o', str(exe)],
                       cwd=ROOT, check=True)
        subprocess.run([str(exe)], check=True)


if __name__ == '__main__':
    run()
