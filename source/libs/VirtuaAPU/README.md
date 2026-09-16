# VirtuaAPU

Pure C Game Boy APU library.

Public API:
- `include/virtuapu.h`
- `include/apu_memory.h`

Implemented modes:
- `mode 1`: DMG
- `mode 2`: CGB
- `mode 3`: GBA PSG + Direct Sound FIFO foundation

The library exposes global APU state and a render buffer, mirroring the style
used by `VirtuaPPU`.
