#include "vbe.h"
#include "x86/console.h"
#include "lib/stddef.h"
#include "fontdata.h"
#include "debug/debug_print.h"

// RGB565採用

vbe_mode_info_t *vbe = NULL;

static uint16_t *fb_base = NULL;   // 16bpp 固定（1ピクセル = 2バイト）
static uint32_t  fb_pitch = 0;     // 1行のバイト数
static uint32_t  fb_width  = 0;
static uint32_t  fb_height = 0;

/* ---------------------------------------------------------
 * VBE 初期化
 * --------------------------------------------------------- */
void vbe_video_init(void)
{
    uint32_t addr = MODEINFO_PTR;
    vbe = (vbe_mode_info_t *)addr;
    if (!vbe) return;

    fb_base   = (uint16_t *)(uintptr_t)vbe->vram_addr;
    fb_pitch  = vbe->lfb_pitch;
    fb_width  = vbe->x_res;
    fb_height = vbe->y_res;

    vbe_clear(0x0000); /* 黒 */
}

/* ---------------------------------------------------------
 * 画面クリア
 *
 * color : RGB565 の 16bit カラー値そのもの
 *         （変換・マスク・解釈は一切しない）
 * --------------------------------------------------------- */
void vbe_clear(uint16_t color)
{
    if (!fb_base) return;

    for (uint32_t y = 0; y < fb_height; y++) {
        uint8_t *row_bytes = (uint8_t *)fb_base + y * fb_pitch; // バイト単位で探す為に8ビット
        uint16_t *row = (uint16_t *)row_bytes; // 16bppなので2バイトずつ進めたい

        for (uint32_t x = 0; x < fb_width; x++) {
            row[x] = color;
        }
    }
}

/* ---------------------------------------------------------
 * 1ピクセル描画（16bpp・変換なし）
 * --------------------------------------------------------- */
static void putpixel(int x, int y, uint16_t color)
{
    if (!fb_base) return;
    if (x < 0 || x >= (int)fb_width || y < 0 || y >= (int)fb_height) return;

    uint8_t *row_bytes = (uint8_t *)fb_base + y * fb_pitch;
    uint16_t *p = (uint16_t *)(row_bytes + x * 2);

    *p = color;
}

/* ---------------------------------------------------------
 * 文字描画
 * --------------------------------------------------------- */
void draw_char(int x, int y, char c, uint16_t fg_color)
{
    if (!fb_base) return;

    const uint8_t *glyph = &fontdata_8x16[(unsigned char)c * 16];

    for (int row = 0; row < 16; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                putpixel(x + col, y + row, fg_color);
            }
        }
    }
}

/* ---------------------------------------------------------
 * 矩形
 * --------------------------------------------------------- */
void draw_rect(int x, int y, int w, int h, uint16_t color)
{
    if (!fb_base) return;

    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            putpixel(x + col, y + row, color);
        }
    }
}
