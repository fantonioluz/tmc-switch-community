#include "cpu/mode2.h"

#include <string.h>

#include "cpu/mode1.h"
#include "virtuappu.h"

#ifdef __SWITCH__
/* Persistent libnx worker pool (platforms/switch/switch_render_pool.c). */
extern void switch_render_pool_run(size_t total, void (*body)(void *, size_t), void *ctx);
#endif

/* mode2 (GBA mode 1/2: text BG0/BG1 + affine BG2 + OBJ) does NOT run the
 * per-line HDMA callback, so io_mem is constant across the frame — every
 * scanline reads the same registers. That makes the lines trivially
 * independent (concurrent io READS are safe, no per-line snapshot needed), so
 * the render loop body is hoisted into this per-line function and fanned out
 * across the worker pool on Switch. dispcnt is read once and passed via ctx. */
typedef struct {
    uint16_t dispcnt;
} Mode2LineCtx;

static void mode2_render_one_line(void *ctxp, size_t line_idx)
{
    static const int affine_sizes[4] = {128, 256, 512, 1024};
    const Mode2LineCtx *c = (const Mode2LineCtx *)ctxp;
    uint16_t dispcnt = c->dispcnt;
    int line = (int)line_idx;
    {
        uint32_t bg_layers[MODE1_GBA_BG_COUNT][MODE1_GBA_WIDTH];
        uint8_t bg_priority[MODE1_GBA_BG_COUNT][MODE1_GBA_WIDTH];
        uint32_t obj_layer[MODE1_GBA_WIDTH];
        uint8_t obj_priority[MODE1_GBA_WIDTH];
        VirtuaPPUMode1GbaMemory memory;
        bool obj_1d = (dispcnt & MODE1_DISP_OBJ_1D) != 0u;

        memset(bg_layers, 0, sizeof(bg_layers));
        memset(bg_priority, 0, sizeof(bg_priority));
        memset(obj_layer, 0, sizeof(obj_layer));
        memset(obj_priority, 0xFF, sizeof(obj_priority));

        if ((dispcnt & MODE1_DISP_BG0_ON) != 0u) {
            virtuappu_mode1_render_text_bg_line(0, line, bg_layers[0], bg_priority[0]);
        }
        if ((dispcnt & MODE1_DISP_BG1_ON) != 0u) {
            virtuappu_mode1_render_text_bg_line(1, line, bg_layers[1], bg_priority[1]);
        }

        if ((dispcnt & MODE1_DISP_BG2_ON) != 0u) {
            uint16_t bgcnt;
            uint8_t bg_priority_value;
            uint32_t char_base;
            uint32_t screen_base;
            bool wrap;
            uint16_t size_flag;
            int map_size;
            int map_tiles;
            int16_t pa;
            int16_t pb;
            int16_t pc;
            int16_t pd;
            int32_t ref_x;
            int32_t ref_y;
            int x;

            virtuappu_mode1_get_bound_gba_memory(&memory);
            bgcnt = virtuappu_mode1_io_read16(MODE1_IO_BG2CNT);
            bg_priority_value = (uint8_t)(bgcnt & 3u);
            char_base = (uint32_t)((bgcnt >> 2u) & 3u) * 0x4000u;
            screen_base = (uint32_t)((bgcnt >> 8u) & 0x1Fu) * 0x800u;
            wrap = ((bgcnt >> 13u) & 1u) != 0u;
            size_flag = (uint16_t)((bgcnt >> 14u) & 3u);
            map_size = affine_sizes[size_flag];
            map_tiles = map_size / 8;
            pa = (int16_t)virtuappu_mode1_io_read16(0x20u);
            pb = (int16_t)virtuappu_mode1_io_read16(0x22u);
            pc = (int16_t)virtuappu_mode1_io_read16(0x24u);
            pd = (int16_t)virtuappu_mode1_io_read16(0x26u);
            ref_x = (int32_t)virtuappu_mode1_io_read32(0x28u);
            ref_y = (int32_t)virtuappu_mode1_io_read32(0x2Cu);

            if ((ref_x & 0x08000000u) != 0u) {
                ref_x |= (int32_t)0xF0000000u;
            }
            if ((ref_y & 0x08000000u) != 0u) {
                ref_y |= (int32_t)0xF0000000u;
            }

            for (x = 0; x < MODE1_GBA_WIDTH; ++x) {
                int32_t tex_x = ref_x + pb * line + pa * x;
                int32_t tex_y = ref_y + pd * line + pc * x;
                int32_t src_x = tex_x >> 8;
                int32_t src_y = tex_y >> 8;
                int tile_col;
                int tile_row;
                int pixel_x;
                int pixel_y;
                uint32_t map_addr;
                uint8_t tile_index;
                uint32_t tile_addr;
                uint8_t color_index;

                if (wrap) {
                    src_x = ((src_x % map_size) + map_size) % map_size;
                    src_y = ((src_y % map_size) + map_size) % map_size;
                } else if (src_x < 0 || src_x >= map_size || src_y < 0 || src_y >= map_size) {
                    continue;
                }

                tile_col = src_x / 8;
                tile_row = src_y / 8;
                pixel_x = src_x % 8;
                pixel_y = src_y % 8;
                map_addr = screen_base + (uint32_t)(tile_row * map_tiles + tile_col);
                tile_index = (map_addr < MODE1_VRAM_SIZE) ? memory.vram[map_addr] : 0u;
                tile_addr = char_base + (uint32_t)tile_index * 64u + (uint32_t)pixel_y * 8u + (uint32_t)pixel_x;
                color_index = (tile_addr < MODE1_VRAM_SIZE) ? memory.vram[tile_addr] : 0u;

                if (color_index == 0u) {
                    continue;
                }

                bg_layers[2][x] = virtuappu_mode1_rgb555_to_abgr8888(memory.bg_palette[color_index]);
                bg_priority[2][x] = bg_priority_value;
            }
        }

        if ((dispcnt & MODE1_DISP_OBJ_ON) != 0u) {
            virtuappu_mode1_render_obj_line(line, obj_1d, obj_layer, obj_priority);
        }

        virtuappu_mode1_composite_line(line, bg_layers, bg_priority, obj_layer, obj_priority, dispcnt);
    }
}

void virtuappu_mode2_render_frame(const PPUMemory *ppu)
{
    uint16_t dispcnt;
    Mode2LineCtx ctx;

    (void)ppu;

    dispcnt = virtuappu_mode1_io_read16(MODE1_IO_DISPCNT);
    if ((dispcnt & MODE1_DISP_FORCED_BLANK) != 0u) {
        memset(virtuappu_frame_buffer, 0xFF, MODE1_GBA_WIDTH * MODE1_GBA_HEIGHT * sizeof(uint32_t));
        return;
    }

    ctx.dispcnt = dispcnt;
#ifdef __SWITCH__
    /* io_mem is constant across the mode2 frame (no per-line callback), so the
     * lines fan out across the worker pool with no snapshot needed. */
    switch_render_pool_run((size_t)MODE1_GBA_HEIGHT, mode2_render_one_line, &ctx);
#else
    {
        int line;
        for (line = 0; line < MODE1_GBA_HEIGHT; ++line) {
            mode2_render_one_line(&ctx, (size_t)line);
        }
    }
#endif
}
