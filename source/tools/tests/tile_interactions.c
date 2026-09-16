/* Native tile-interaction regression test. From the repository root:
 * gcc -std=gnu11 -DPC_PORT -DUSA -DENGLISH -DNON_MATCHING -I. -Iinclude -Iport -Ibuild/USA \
 *     -O1 -g -ffunction-sections -fdata-sections -Wl,--gc-sections -fsanitize=address,undefined \
 *     tools/tests/tile_interactions.c src/data/data_080046A4.c src/data/mapActTileToSurfaceType.c \
 *     port/port_gameplay_stubs.c src/physics.c -o build/tile_interactions_test
 * build/tile_interactions_test baserom.gba
 * The local USA baserom is the oracle; no game data is embedded here. */
#include "asm.h"
#include "effects.h"
#include "map.h"
#include "object.h"
#include "physics.h"
#include "player.h"
#include "room.h"
#include "tiles.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

extern const KeyValuePair gUnk_080046A4[];
extern const KeyValuePair gMapActTileToSurfaceType[];
extern const u16 gUnk_080047F6[];
extern bool32 sub_0806FC24(u32 tileType, u32 interaction);
extern const u16* sub_0806FC50(u32 tileType, u32 interaction);

RoomControls gRoomControls;
static MapLayer testLayer;
static Entity created;
static u32 tileType, createCalls, changeCalls, restoreCalls;
static u32 createdId, createdType, createdType2, changedType, changedPos, changedLayer;
static u16 expectedValues[65536];
static unsigned char expectedDefinitions[60 * 8];

u32 GetTileTypeAtWorldCoords(s32 x, s32 y, u32 layer) { return tileType; }
Entity* CreateObject(u32 id, u32 type, u32 type2) {
    ++createCalls;
    createdId = id;
    createdType = type;
    createdType2 = type2;
    memset(&created, 0, sizeof(created));
    return &created;
}
void UpdateSpriteForCollisionLayer(Entity* entity) {}
MapLayer* GetLayerByIndex(u32 layer) { return &testLayer; }
void RestorePrevTileEntity(u32 pos, u32 layer) { ++restoreCalls; }
void sub_0807B7D8(u32 type, u32 pos, u32 layer) {
    ++changeCalls;
    changedType = type;
    changedPos = pos;
    changedLayer = layer;
}

static void ResetEffects(void) {
    createCalls = changeCalls = restoreCalls = 0;
    memset(&testLayer, 0x5a, sizeof(testLayer));
}

static unsigned ReadU16(FILE* rom) {
    int lo = fgetc(rom), hi = fgetc(rom);
    assert(lo >= 0 && hi >= 0);
    return (unsigned)(lo | (hi << 8));
}

int main(int argc, char** argv) {
    assert(argc == 2);
    FILE* rom = fopen(argv[1], "rb");
    assert(rom != NULL);
    assert(fseek(rom, 0x46a4, SEEK_SET) == 0);
    for (unsigned i = 0; i < 84; ++i) {
        unsigned key = ReadU16(rom), value = ReadU16(rom);
        assert(key != 0 && value < 60);
        expectedValues[key] = value;
        assert(gUnk_080046A4[i].key == key);
        assert(gUnk_080046A4[i].value == value);
    }
    assert(fseek(rom, 0x47f6, SEEK_SET) == 0);
    assert(fread(expectedDefinitions, 1, sizeof(expectedDefinitions), rom) == sizeof(expectedDefinitions));
    fclose(rom);
    assert(gTileInteractionDefinitionCount == 60);
    assert(memcmp(gUnk_080047F6, expectedDefinitions, sizeof(expectedDefinitions)) == 0);

    /* Every absent key must stop inside the array, regardless of linker order.
     * Before the fix ASan reports a global-buffer-overflow on the first lookup. */
    for (unsigned key = 0; key < 65536; ++key) {
        assert(FindValueForKey(key, gUnk_080046A4) == expectedValues[key]);
    }
    assert(gUnk_080046A4[84].key == 0);
    assert(FindValueForKey(0xffff, gMapActTileToSurfaceType) == 0);
    assert(FindValueForKey(ACT_TILE_13, gMapActTileToSurfaceType) == SURFACE_PIT);

    Entity player = { 0 };
    player.x.HALF.HI = 176;
    player.y.HALF.HI = 368;
    /* These ordinary Hyrule tiles became false row 256 in the broken Switch
     * ELF, spawning GROUND_ITEM / ITEM_SMITH_SWORD and replacing the floor. */
    const u32 ordinaryTiles[] = { 0x3e0, 0x420, 0x120, 0x140, 0x1c0, 0x200, 0xffff };
    for (unsigned i = 0; i < ARRAY_COUNT(ordinaryTiles); ++i) {
        tileType = ordinaryTiles[i];
        assert(expectedValues[tileType] == 0);
        for (unsigned layer = 1; layer <= 2; ++layer) {
            player.collisionLayer = layer;
            for (unsigned interaction = 0; interaction < 16; ++interaction) {
                ResetEffects();
                assert(DoTileInteraction(&player, interaction, 176, 368) == NULL);
                assert(sub_0806FC24(tileType, interaction) == 0);
                assert(sub_0806FC50(tileType, interaction) == NULL);
                assert(createCalls == 0 && changeCalls == 0 && restoreCalls == 0);
                for (unsigned pos = 0; pos < ARRAY_COUNT(testLayer.mapData); ++pos)
                    assert(testLayer.mapData[pos] == 0x5a5a);
            }
        }
    }

    /* Valid bush cutting/lifting must still update the tile. */
    tileType = 67;
    player.collisionLayer = 1;
    ResetEffects();
    assert(DoTileInteraction(&player, 0, 176, 368) == &gUnk_080047F6[3 * 4]);
    assert(createCalls == 1 && createdId == SPECIAL_FX && createdType == FX_BUSH);
    assert(createdType2 == 0x80 && created.parent == &player && created.collisionLayer == 1);
    assert(created.x.HALF.HI == 184 && created.y.HALF.HI == 376);
    assert(changeCalls == 1 && changedType == 0x79 && changedPos == 11 + 23 * 64 && changedLayer == 1);
    assert(sub_0806FC24(tileType, 6) == 1);
    assert(sub_0806FC50(tileType, 6) == &gUnk_080047F6[3 * 4]);
    ResetEffects();
    assert(DoTileInteraction(&player, 6, 176, 368) != NULL);
    assert(createCalls == 0 && changeCalls == 1 && changedType == 0x79);

    tileType = 353;
    ResetEffects();
    assert(DoTileInteraction(&player, 0, 176, 368) == &gUnk_080047F6[47 * 4]);
    assert(createCalls == 1 && createdId == SPECIAL_FX && createdType == FX_GRASS_CUT);
    assert(changeCalls == 1 && changedType == 29);

    const u32 invalidInteractions[] = { 16, 31, 32, 0xffffffff };
    for (unsigned i = 0; i < ARRAY_COUNT(invalidInteractions); ++i) {
        ResetEffects();
        assert(DoTileInteraction(&player, invalidInteractions[i], 176, 368) == NULL);
        assert(sub_0806FC24(tileType, invalidInteractions[i]) == 0);
        assert(sub_0806FC50(tileType, invalidInteractions[i]) == NULL);
        assert(createCalls == 0 && changeCalls == 0 && restoreCalls == 0);
    }
    gRoomControls.reload_flags = 1;
    assert(DoTileInteraction(&player, 0, 176, 368) == NULL);
    puts("PASS: 65536 tile lookups match USA ROM; 224 ordinary-floor interactions preserve tiles and spawn nothing; bush cutting/lifting, grass cutting and invalid inputs pass.");
    return 0;
}
