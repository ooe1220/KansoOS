#include "vbe.h"
#include "x86/console.h"
#include "lib/stddef.h"
#include "fontdata.h"

vbe_mode_info_t *vbe = NULL;

/* ---------- ここから下を追加 ---------- */

static uint8_t  *fb_base = NULL;
static uint32_t fb_pitch = 0;
static uint32_t fb_width = 0;
static uint32_t fb_height = 0;

void vbe_video_init(void) {
    uint32_t addr = MODEINFO_PTR;
    vbe = (vbe_mode_info_t *)addr;
    if (!vbe) return;

    fb_base   = (uint8_t *)(uintptr_t)vbe->vram_addr;
    fb_pitch  = vbe->lfb_pitch;
    fb_width  = vbe->x_res;
    fb_height = vbe->y_res;

    /* 黒クリア */
    for (uint32_t i = 0; i < fb_pitch * fb_height; i++) {
        fb_base[i] = 0x00;
    }
}

void vbe_clear(uint32_t bg_color) {
    if (!fb_base) return;

    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            uint8_t *p = fb_base + y * fb_pitch + x * 3;
            p[0] = bg_color & 0xFF;
            p[1] = (bg_color >> 8) & 0xFF;
            p[2] = (bg_color >> 16) & 0xFF;
        }
    }
}

static void putpixel(int x, int y, uint32_t color) {
    if (!fb_base) return;
    if (x < 0 || x >= (int)fb_width || y < 0 || y >= (int)fb_height) return;

    uint8_t *p = fb_base + (uint32_t)y * fb_pitch + (uint32_t)x * 3;
    p[0] = color & 0xFF;
    p[1] = (color >> 8) & 0xFF;
    p[2] = (color >> 16) & 0xFF;
}

void draw_char_8x16(int x, int y, char c, uint32_t fg_color, uint32_t bg_color) {
    if (!fb_base) return;

    const uint8_t *glyph = &fontdata_8x16[(unsigned char)c * 16];

    for (int row = 0; row < 16; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                putpixel(x + col, y + row, fg_color);
            } else if (bg_color != 0xFFFFFFFF) {
                putpixel(x + col, y + row, bg_color);
            }
        }
    }
}
