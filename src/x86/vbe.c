#include "vbe.h"
#include "x86/console.h"
#include "lib/stddef.h"
#include "fontdata.h"
#include "./kernel/penguin_bmp.h"

vbe_mode_info_t *vbe = NULL;

static uint8_t  *fb_base = NULL;
static uint32_t fb_pitch = 0;
static uint32_t fb_width = 0;
static uint32_t fb_height = 0;

// VRAMの構造
// vbe->vram_addr = 0xFD000000(QEMU実測値)の場合
// ピクセル(x=0, y=0):  [B][G][R]  ← 0xFD000000, 0xFD000001, 0xFD000002
// ピクセル(x=1, y=0):  [B][G][R]  ← 0xFD000003, 0xFD000004, 0xFD000005

void vbe_video_init(void) {
    uint32_t addr = MODEINFO_PTR;
    vbe = (vbe_mode_info_t *)addr;
    if (!vbe) return;

    fb_base   = (uint8_t *)(uintptr_t)vbe->vram_addr;
    fb_pitch  = vbe->lfb_pitch; // 1行のバイト数
    fb_width  = vbe->x_res; // 画面幅
    fb_height = vbe->y_res; // 画面高さ

    vbe_clear(0x00000000); //黒で画面クリア 
}

// 画面クリア
// 0x00RRGGBBのbg_colorをそれぞれRGBのピクセルに書き込む
void vbe_clear(uint32_t bg_color) {
    if (!fb_base) return;

    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            uint8_t *p = fb_base + y * fb_pitch + x * 3; // putpixel()呼ぶとcallやスタック生成で遅くなるのでべた書き
            p[0] = bg_color & 0xFF;
            p[1] = (bg_color >> 8) & 0xFF;
            p[2] = (bg_color >> 16) & 0xFF;
        }
    }
}

static void putpixel(int x, int y, uint32_t color) {
    if (!fb_base) return;
    if (x < 0 || x >= (int)fb_width || y < 0 || y >= (int)fb_height) return;

    uint8_t *p = fb_base + y * fb_pitch + x * 3;
    p[0] = color & 0xFF;         // 青
    p[1] = (color >> 8) & 0xFF;  // 緑
    p[2] = (color >> 16) & 0xFF; // 赤
}

void draw_char(int x, int y, char c, uint32_t fg_color) {
    if (!fb_base) return;
    const uint8_t *glyph = &fontdata_8x16[(unsigned char)c * 16]; // 表示する字体の取り出し、1文字16バイトでASCII順の配列だから添字はC×16

    for (int row = 0; row < 16; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                putpixel(x + col, y + row, fg_color);
            }
        }
    }
}

// 矩形
void draw_rect(int x, int y, int w, int h, uint32_t color) {
    if (!fb_base) return;
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            putpixel(x + col, y + row, color);
        }
    }
}

// 円
void draw_circle(int cx, int cy, int r, uint32_t color) {
    if (!fb_base) return;
    int x = r, y = 0;
    int err = 0;

    while (x >= y) {
        putpixel(cx + x, cy + y, color);
        putpixel(cx + y, cy + x, color);
        putpixel(cx - y, cy + x, color);
        putpixel(cx - x, cy + y, color);
        putpixel(cx - x, cy - y, color);
        putpixel(cx - y, cy - x, color);
        putpixel(cx + y, cy - x, color);
        putpixel(cx + x, cy - y, color);
        y++;
        err += 1 + 2 * y;
        if (2 * (err - x) + 1 > 0) {
            x--;
            err += 1 - 2 * x;
        }
    }
}

// BMPを (dest_x, dest_y) に描画する
void draw_bmp(int dest_x, int dest_y, const uint8_t *bmp_data) {
    if (!bmp_data || !fb_base) return;

    // BMPヘッダー読み取り（リトルエンディアン）
    uint16_t bf_type = *(uint16_t *)(bmp_data + 0x00);
    if (bf_type != 0x4D42) return; // 'BM' でなければ終了

    uint32_t bf_offbits = *(uint32_t *)(bmp_data + 0x0A);
    int32_t  bi_width   = *(int32_t  *)(bmp_data + 0x12);
    int32_t  bi_height  = *(int32_t  *)(bmp_data + 0x16);
    uint16_t bi_bitcount = *(uint16_t *)(bmp_data + 0x1C);

    if (bi_bitcount != 24) return; // 24BPP以外は未対応

    int is_topdown = (bi_height < 0);
    int height = is_topdown ? -bi_height : bi_height;
    int width = bi_width;

    // 各行のバイト数（パディング込み）
    int row_bytes = ((width * 3 + 3) / 4) * 4;

    const uint8_t *pixel_data = bmp_data + bf_offbits;

    for (int y = 0; y < height; y++) {
        // BMPはボトムアップの場合、下の行から格納されている
        int bmp_row = is_topdown ? y : (height - 1 - y);
        const uint8_t *src_row = pixel_data + bmp_row * row_bytes;

        for (int x = 0; x < width; x++) {
            int fb_x = dest_x + x;
            int fb_y = dest_y + y;

            if (fb_x < 0 || fb_x >= (int)fb_width || fb_y < 0 || fb_y >= (int)fb_height)
                continue;

            uint8_t *fb = fb_base + fb_y * fb_pitch + fb_x * 3;
            const uint8_t *px = src_row + x * 3;

            // BMP: BGR → VBE: BGR（そのままコピー！）
            fb[0] = px[0];  // B
            fb[1] = px[1];  // G
            fb[2] = px[2];  // R
        }
    }
}

void show_penguin(void) {
    // 画面中央に表示
    //int x = (fb_width - 80) / 2;
    //int y = (fb_height - 80) / 2;
    int x = 0;
    int y = 0;
    draw_bmp(x, y, src_kernel_penguin_bmp);
}

