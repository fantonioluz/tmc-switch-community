"""Emit a C regression harness using production NPC loading code and a local USA ROM."""
from pathlib import Path
import argparse
import re

ROOT = Path(__file__).resolve().parents[1]


def function(text, signature):
    start = text.index(signature + ' {')
    return text[start:text.index('\n}', start) + 2]


def harness():
    room = (ROOT / 'src/room.c').read_text(encoding='utf-8')
    room_init = (ROOT / 'src/roomInit.c').read_text(encoding='utf-8')
    town = (ROOT / 'src/npc/townsperson.c').read_text(encoding='utf-8')
    kid = (ROOT / 'src/npc/kid.c').read_text(encoding='utf-8')
    festari = (ROOT / 'src/npc/festari.c').read_text(encoding='utf-8')
    data = (ROOT / 'port/data_stubs_autogen.c').read_text(encoding='utf-8')
    library = re.search(r'EntityData gUnk_additional_a_TownMinishHoles_LibraryBookshelf\[\] = \{.*?\n};', room_init, re.S)[0]
    kid_tables = kid[kid.index('const SpriteLoadData gUnk_0810BD7C'):kid.index('static const SpriteLoadData* Kid_GetSpriteLoadData')]
    table_size = re.search(r'gUnk_0810B6EC\[(\d+)\]', data)[1]
    count_definition = re.search(r'const u32 gTownspersonSpriteLoadDataCount = .*?;', data)[0]
    festari_type = re.search(r'typedef struct \{.*?\} FestariEntity;', festari, re.S)[0]
    return r'''
#include "asm.h"
#include "entity.h"
#include "enemy.h"
#include "npc.h"
#include "physics.h"
#include "room.h"
#include "script.h"
#include "player.h"
#include "port_rom.h"
#include "port_scripts.h"
#include "port_entity_ctx.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

u8* gRomData; u32 gRomSize;
RoomControls gRoomControls;
PlayerEntity gPlayerEntity;
GenericEntity gAuxPlayerEntities[MAX_AUX_PLAYER_ENTITIES];
GenericEntity gEntities[MAX_ENTITIES];
ScriptExecutionContext* gEntityScriptCtxTable[PC_MAX_ENTITY_SLOTS];
static ScriptExecutionContext scriptContext;
static Script* startedScript;
static ScriptExecutionContext* executedContext;
static unsigned executeCalls;
SpriteLoadData* gUnk_0810B6EC[TABLE_SIZE];
COUNT_DEFINITION
FESTARI_TYPE
KID_TABLES
LIBRARY_ARRAY

void sub_0804AF0C(Entity* ent, const EntityData* dat);
void RegisterRoomEntity(Entity* ent, const EntityData* dat) { ent->next = ent; }
void* GetEmptyEntityByKind(u32 kind) { return &gEntities[0]; }
Entity* DeepFindEntityByID(u32 kind, u32 id) { return NULL; }
u32 ResolveCollisionLayer(Entity* ent) { ent->collisionLayer = 1; return 1; }
void DeleteEntity(Entity* ent) { assert(!"unexpected entity deletion"); }
ScriptExecutionContext* StartCutscene(Entity* ent, Script* script) {
    startedScript = script;
    ent->flags |= ENT_SCRIPTED;
    scriptContext.scriptInstructionPointer = script;
    Port_SetEntityScriptCtx(ent, &scriptContext);
    return &scriptContext;
}
void ExecuteScript(Entity* ent, ScriptExecutionContext* context) {
    ++executeCalls; executedContext = context;
}
void sub_0805FF2C(FestariEntity* ent, ScriptExecutionContext* context) {
    assert(context == &scriptContext);
}
void InitAnimationForceUpdate(Entity* ent, u32 animation) { ent->animIndex = animation; }
void UpdateAnimationSingleFrame(Entity* ent) {}
u32 GetFacingDirection(Entity* ent, Entity* other) { return 0; }
u32 GetAnimationStateForDirection4(u32 direction) { return 0; }
void InitializeNPCFusion(Entity* ent) {}
u32 sub_0806ED78(Entity* ent) { return 0; }
''' .replace('TABLE_SIZE', table_size).replace('COUNT_DEFINITION', count_definition).replace('FESTARI_TYPE', festari_type).replace('KID_TABLES', kid_tables).replace('LIBRARY_ARRAY', library) + '\n'.join([
        function(town, 'static const SpriteLoadData* Townsperson_GetSpriteLoadData(u32 type)'),
        function(kid, 'static const SpriteLoadData* Kid_GetSpriteLoadData(u32 type)'),
        function(room, 'Entity* LoadRoomEntity(const EntityData* dat)'),
        function(room, 'void sub_0804AF0C(Entity* ent, const EntityData* dat)'),
        function(festari, 'void sub_0805FE48(FestariEntity* this)'),
    ]) + r'''
int main(int argc, char** argv) {
    assert(argc == 2);
    FILE* rom = fopen(argv[1], "rb"); assert(rom);
    fseek(rom, 0, SEEK_END); gRomSize = ftell(rom); rewind(rom);
    gRomData = malloc(gRomSize); assert(gRomData);
    assert(fread(gRomData, 1, gRomSize, rom) == gRomSize); fclose(rom);

    /* The first library Minish is a native EntityData entry. Its flags must
     * match the cartridge, including 0x40 which starts its talking script. */
    const EntityData* data = gUnk_additional_a_TownMinishHoles_LibraryBookshelf;
    assert(sizeof(EntityData) == 16);
    assert(memcmp(data, gRomData + 0xDB820, sizeof(EntityData)) == 0);
    Entity* entity = LoadRoomEntity(data);
    assert(entity->kind == NPC && entity->id == TOWN_MINISH && entity->type == 2);
    assert(entity->flags & ENT_SCRIPTED);
    assert(startedScript == (Script*)(gRomData + 0xE6E8));
    assert(Port_GetEntityScriptCtx(entity) == &scriptContext);
    assert(entity->x.HALF.HI == 0x80 && entity->y.HALF.HI == 0x158);

    /* All byte-sized NPC types, including the reported Lake Hylia type 64. */
    for (unsigned type = 0; type < 21; ++type)
        gUnk_0810B6EC[type] = (SpriteLoadData*)(gRomData + 0x100 + type * 4);
    assert(gTownspersonSpriteLoadDataCount == 21);
    for (unsigned type = 0; type < 256; ++type) {
        assert(Townsperson_GetSpriteLoadData(type) == (type < 21 ? gUnk_0810B6EC[type] : NULL));
        assert(Kid_GetSpriteLoadData(type) == (type < ARRAY_COUNT(gUnk_0810BDC4) ? gUnk_0810BDC4[type] : NULL));
    }

    /* The native side table owns context pointers, even if the legacy mirror
     * is stale or temporarily absent during a room transition. */
    FestariEntity* festari = (FestariEntity*)&gEntities[1];
    festari->base.action = 1;
    Port_SetEntityScriptCtx(&festari->base, &scriptContext);
    festari->context = NULL;
    sub_0805FE48(festari);
    assert(executeCalls == 1 && executedContext == &scriptContext);
    Port_SetEntityScriptCtx(&festari->base, NULL);
    festari->context = (ScriptExecutionContext*)(uintptr_t)1;
    sub_0805FE48(festari);
    assert(executeCalls == 1);
    free(gRomData);
    puts("PASS: library NPC matches ROM and starts its script; 512 sprite lookups are bounded; Festari uses authoritative context and tolerates absence.");
    return 0;
}
'''


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--emit', type=Path, required=True)
    args = parser.parse_args()
    args.emit.write_text(harness(), encoding='utf-8')
