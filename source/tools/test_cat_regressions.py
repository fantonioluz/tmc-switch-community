"""Exercise the production cat attack with actual packed table bytes and a local ROM."""
from pathlib import Path
import argparse
import re

ROOT = Path(__file__).resolve().parents[1]

def function(text, signature):
    start = text.index(signature + ' {')
    return text[start:text.index('\n}', start) + 2]

def harness():
    cat = (ROOT / 'src/npc/cat.c').read_text(encoding='utf-8')
    data = (ROOT / 'port/data_const_stubs.c').read_text(encoding='utf-8')
    packed = re.search(r'const u8 gUnk_08111154\[\d+\].*?= \{(.*?)\};', data, re.S)[1]
    words = re.findall(r'0x[0-9a-fA-F]+', packed)[:32]
    assert len(words) == 32
    typedef = re.search(r'typedef struct \{.*?\} CatEntity;', cat, re.S)[0]
    fallback = re.search(r'static const Hitbox gUnk_08110EF0 = .*?;', cat)[0]
    return '''#include "entity.h"
#include "enemy.h"
#include "port_rom.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
u8* gRomData; u32 gRomSize;
''' + typedef + '\n' + fallback + '\nconst u8 gUnk_08111154[32] = {' + ','.join(words) + '''};
static unsigned resetCalls, idleCalls;
void UpdateAnimationSingleFrame(Entity* e) { (void)e; }
bool32 sub_08067D20(CatEntity* e) { (void)e; return 0; }
void sub_08067B80(CatEntity* e, u32 v) { (void)e; (void)v; assert(0); }
void sub_08067C24(CatEntity* e) { (void)e; resetCalls++; }
void sub_08067DDC(Entity* e) { (void)e; idleCalls++; }
''' + function(cat, 'static const Hitbox* Cat_GetAttackHitbox(u32 index)') + '\n' + function(cat, 'void sub_08067A0C(CatEntity* this)') + '''
int main(int argc, char** argv) {
    assert(argc == 2);
    FILE* f = fopen(argv[1], "rb"); assert(f);
    fseek(f, 0, SEEK_END); gRomSize = ftell(f); rewind(f);
    gRomData = malloc(gRomSize); assert(gRomData);
    assert(fread(gRomData, 1, gRomSize, f) == gRomSize); fclose(f);
    assert(memcmp(gUnk_08111154, gRomData + 0x111154, 32) == 0);
    const u8 expected[8][8] = {
        {5,255,0,0,0,0,4,2}, {1,3,0,0,0,0,4,4},
        {249,4,0,0,0,0,8,4}, {244,3,0,0,0,0,4,4},
        {251,255,0,0,0,0,4,2}, {255,3,0,0,0,0,4,4},
        {7,4,0,0,0,0,8,4}, {12,3,0,0,0,0,4,4}
    };
    for (unsigned i = 0; i < 8; i++) {
        const Hitbox* h = Cat_GetAttackHitbox(i);
        assert((const u8*)h == gRomData + 0x111114 + i * 8);
        assert(memcmp(h, expected[i], sizeof(Hitbox)) == 0);
    }
    CatEntity cat = {0};
    for (unsigned flip = 0; flip < 2; flip++) {
        for (unsigned frame = 1; frame <= 7; frame++) {
            cat.base.spriteSettings.flipX = flip;
            cat.base.frame = frame;
            sub_08067A0C(&cat);
            unsigned idx = frame - 1 + flip * 4;
            if (idx >= 8) idx = 7;
            assert(cat.base.hitbox == Cat_GetAttackHitbox(idx));
            assert(memcmp(cat.base.hitbox, expected[idx], sizeof(Hitbox)) == 0);
        }
    }
    cat.base.frame = 0; sub_08067A0C(&cat); assert(idleCalls == 1);
    cat.base.frame = ANIM_DONE; sub_08067A0C(&cat); assert(resetCalls == 1);
    assert(Cat_GetAttackHitbox(8) == &gUnk_08110EF0);
    assert(Cat_GetAttackHitbox(~0u) == &gUnk_08110EF0);
    u32 saved = gRomSize;
    gRomSize = 0x11114c + sizeof(Hitbox) - 1;
    assert(Cat_GetAttackHitbox(7) == &gUnk_08110EF0);
    gRomSize = 0; assert(Cat_GetAttackHitbox(0) == &gUnk_08110EF0);
    gRomSize = saved; free(gRomData); gRomData = NULL;
    assert(Cat_GetAttackHitbox(0) == &gUnk_08110EF0);
    puts("Cat regression passed: 8 ROM hitboxes, 14 attack frames, bounds and missing ROM.");
}
'''

if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('--emit', type=Path, required=True)
    args = p.parse_args()
    args.emit.write_text(harness(), encoding='utf-8')
