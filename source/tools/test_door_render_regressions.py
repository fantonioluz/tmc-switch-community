"""Exercise production room-property and terrain-overlay code against a local USA ROM.

Run with Python + g++: python tools/test_door_render_regressions.py
Or emit a standalone C++ harness: --emit build/door_regressions.cpp
The harness takes the path to the user's baserom.gba; no ROM bytes are embedded.
"""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def function(source, signature):
    start = source.index(signature + " {")
    end = source.index("\n}", start) + 2
    return source[start:end]


def harness():
    draw = (ROOT / "port/port_draw.c").read_text()
    loader = (ROOT / "port/port_asset_loader.cpp").read_text()
    index = (ROOT / "port/port_asset_index.c").read_text()
    entries = re.findall(r'\{ 0x[0-9A-Fa-f]+, 0x[0-9A-Fa-f]+, "[^"]+" \}', index)
    table_size = re.search(r'sShoesOverlayPtrs\[(\d+)\]', draw)[1]
    prefix = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
using u8 = uint8_t; using u16 = uint16_t; using u32 = uint32_t;
using s8 = int8_t; using s16 = int16_t; using s32 = int32_t;
#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))
u8* gRomData; u32 gRomSize;
struct EmbeddedAssetEntry { u32 offset, size; const char* path; };
static const EmbeddedAssetEntry entries[] = { ENTRIES };
const EmbeddedAssetEntry* EmbeddedAssetIndex_Get() { return entries; }
u32 EmbeddedAssetIndex_Count() { return ARRAY_COUNT(entries); }
union Position { struct { u16 LO; s16 HI; } HALF; };
struct Entity {
    Position x{}, y{}, z{};
    u8 spriteSettings{}, spritePriority{}, collisionLayer{};
};
struct { s16 origin_x{}, origin_y{}; } gRoomControls;
struct { u8 updated{}, field_0x1{}, rest[30]{}; } gOAMControls;
struct DeferredEntry { s16 packed0, packed1; };
struct { u8 count{}; DeferredEntry entries[64]; } sDeferredList;
static const u8* sShoesOverlayPtrs[TABLE_SIZE]{};
static int sShoesOverlayTableLoaded;
static const u8* overlay;
static int overlay_calls, sprite_calls, sprite_y, overlay_y;
static u32 tile;
u32 GetActTileAtTilePos(u16, u8) { return tile; }
static void ResolveEntitySpriteParams(Entity* e, s32* x, s32* y, u32* f, u16* ex) {
    *x=e->x.HALF.HI; *y=e->y.HALF.HI; *f=0; *ex=0xAC00;
}
static void RenderSpritePieces(const u8* p, s16, s16 y, u32 f, u16 ex) {
    assert(f == 0 && ex == 0xC00);
    overlay=p; overlay_y=y; ++overlay_calls;
}
static void DrawEntitySprites(Entity*, s32, s32 y, u32, u16) {
    sprite_y=y; ++sprite_calls;
}
'''.replace("ENTRIES", ",\n".join(entries)).replace("TABLE_SIZE", table_size)
    functions = [function(loader, "const u8* ResolveFragmentedRoomProperty(const std::string& path)"),
                 function(draw, "static const u8* TranslateIwramOrRomPointer(u32 ptr)"),
                 function(draw, "static u32 ReadRomU32LE(u32 offset)"),
                 function(draw, "static void LoadShoesOverlayTableFromRom(void)"),
                 function(draw, "static void ProcessEntityForDraw(Entity* entity)")]
    tests = r'''
static u16 le16(const u8* p) { return p[0] | (u16(p[1]) << 8); }
static void reset() {
    overlay=nullptr; overlay_calls=sprite_calls=0;
    sprite_y=overlay_y=-1; sDeferredList.count=0;
}
int main(int argc, char** argv) {
    assert(argc == 2);
    std::ifstream file(argv[1], std::ios::binary);
    std::vector<u8> rom((std::istreambuf_iterator<char>(file)), {});
    assert(rom.size() == 0x1000000);
    assert(std::string((char*)rom.data()+0xAC, 4) == "BZME");
    gRomData=rom.data(); gRomSize=rom.size();

    const char* path="data_080D5360/gUnk_additional_c_HyruleTown_0.bin";
    const u8* doors=ResolveFragmentedRoomProperty(path);
    assert(doors == gRomData+0xEF9E4);
    const u16 expected[][2] = {
        {296,51},{696,291},{136,339},{40,560},{920,515},
        {648,577},{128,689},{392,721},{584,721},{728,731},
        {824,739},{936,835},{296,882},{680,843},{48,419}
    };
    for (u32 i=0; i<15; ++i) {
        assert(le16(doors+i*12)==expected[i][0]);
        assert(le16(doors+i*12+2)==expected[i][1]);
    }
    assert(le16(doors+15*12)==0xFFFF);
    assert(ReadRomU32LE(0xEF9E4+3*12+8)==0x0800EF40);
    assert(ReadRomU32LE(0xEF9E4+13*12+8)==0x0801090C);
    assert(ResolveFragmentedRoomProperty("data_080D5360/gUnk_additional_8_HyruleTown_1.bin")==nullptr);
    assert(ResolveFragmentedRoomProperty("unknown.bin")==nullptr);
    gRomData=nullptr;
    assert(ResolveFragmentedRoomProperty(path)==nullptr);
    gRomData=rom.data();
    // Door 346 uses static frame-object lists, not the animation table.
    for (u32 frame=0; frame<2; ++frame) {
        u32 table=ReadRomU32LE(0x2F3D74+346*4);
        u32 data=ReadRomU32LE(0x2F3D74+table+frame*4);
        assert(data<200045 && rom[0x2F3D74+data]>0);
    }

    Entity e{}; e.spritePriority=8; e.collisionLayer=1;
    // Independent oracle: byte-address calculation in asm/src/intr.s,
    // USA sub_080B25D8/sub_080B25E8. Cover all rows and animation phases.
    for (tile=0xF; tile<=0x2F; tile+=0x20) {
        for (u32 row=0; row<4; ++row) {
            e.spriteSettings=row<<4;
            for (u32 phase=0; phase<4; ++phase) {
                reset(); e.x.HALF.HI=32+phase*2; e.y.HALF.HI=80;
                e.x.HALF.LO=0xFE00; e.y.HALF.LO=0; // must not affect the frame
                gOAMControls.field_0x1=phase*8;
                ProcessEntityForDraw(&e);
                u32 r2=tile==0xF ? ((phase*8 & 0x18)+0x80)>>2
                                          : (e.x.HALF.HI^e.y.HALF.HI)&6;
                u32 byte_offset=(e.spriteSettings&0x30)+(r2<<1);
                const u8* expected_frame=TranslateIwramOrRomPointer(ReadRomU32LE(0xB2B58+byte_offset));
                assert(overlay_calls==1 && overlay==expected_frame);
                assert(overlay_y==80+(tile==0xF ? 2 : 0));
                assert(sprite_calls==1 && sprite_y==overlay_y);
                assert(sDeferredList.count==0);
            }
        }
    }
    reset(); tile=0; ProcessEntityForDraw(&e);
    assert(overlay_calls==0 && sprite_calls==1 && sDeferredList.count==1);
    reset(); tile=0x2F; e.z.HALF.HI=-4; ProcessEntityForDraw(&e);
    assert(overlay_calls==0 && sprite_calls==1 && sDeferredList.count==1);
    reset(); e.z.HALF.HI=0; tile=0x19; ProcessEntityForDraw(&e);
    assert(sprite_y==82 && overlay_calls==0);
    puts("PASS: 15 Hyrule doors, 2 inline scripts, static door frames, 32 terrain-overlay cases, normal/jumping paths");
}
'''
    return prefix + "\n".join(functions) + tests


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--emit", type=Path)
    parser.add_argument("--rom", type=Path, default=ROOT / "baserom.gba")
    args = parser.parse_args()
    if args.emit:
        args.emit.write_text(harness())
    else:
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "test.cpp"
            binary = Path(directory) / "test"
            source.write_text(harness())
            subprocess.run(["g++", "-std=c++17", "-O1", "-fsanitize=address,undefined",
                            str(source), "-o", str(binary)], check=True)
            subprocess.run([str(binary), str(args.rom)], check=True)
