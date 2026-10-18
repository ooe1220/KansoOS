#include "./kernel/penguin_bmp.h"
#include "./x86/vbe.h"

/* ---------------------------------------------------------
 * RGB888 (BGR順バイト列) → RGB565
 * r/g/b : 0x00 - 0xFF
 * --------------------------------------------------------- */
static inline uint16_t rgb888_to_rgb565(uint8_t b, uint8_t g, uint8_t r) {
    return
        ((r & 0xF8) << 8) |   // R: 8bit → 上位5bit
        ((g & 0xFC) << 3) |   // G: 8bit → 中間6bit
        ((b & 0xF8) >> 3);    // B: 8bit → 下位5bit
}

/* 共通: BMP ヘッダ解析 */
static int parse_bmp_header(const uint8_t *bmp_data,
                            const uint8_t **out_pixels,
                            int *out_w, int *out_h, int *out_row_bytes)
{
    if (*(uint16_t *)(bmp_data + 0x00) != 0x4D42) return -1; // 'BM'
    uint32_t off = *(uint32_t *)(bmp_data + 0x0A);
    int32_t  w   = *(int32_t  *)(bmp_data + 0x12);
    int32_t  h   = *(int32_t  *)(bmp_data + 0x16);
    uint16_t bpp = *(uint16_t *)(bmp_data + 0x1C);
    if (bpp != 24) return -1; // 24bpp のみ

    int topdown = (h < 0);
    *out_w = w;
    *out_h = topdown ? -h : h;
    *out_row_bytes = ((w * 3 + 3) / 4) * 4;
    *out_pixels = bmp_data + off;
    return topdown ? 1 : 0; // 0=bottomup, 1=topdown
}

/* ---------------------------------------------------------
 * BMP を (dest_x, dest_y) に 1:1 描画（16bpp）
 * --------------------------------------------------------- */
void draw_bmp(int dest_x, int dest_y, const uint8_t *bmp_data) {
    if (!bmp_data || !vbe) return;

    uint16_t *fb_base = (uint16_t *)(uintptr_t)vbe->vram_addr;
    uint32_t pitch = vbe->lfb_pitch;
    uint32_t sw = vbe->x_res, sh = vbe->y_res;

    int w, h, row_bytes;
    const uint8_t *pixels;
    int topdown = parse_bmp_header(bmp_data, &pixels, &w, &h, &row_bytes) < 0 ? 0 :
                  parse_bmp_header(bmp_data, &pixels, &w, &h, &row_bytes);

    for (int y = 0; y < h; y++) {
        int bmp_row = topdown ? y : (h - 1 - y);
        const uint8_t *src = pixels + bmp_row * row_bytes;

        uint8_t *row_bytes_addr = (uint8_t *)fb_base + (dest_y + y) * pitch;
        uint16_t *row = (uint16_t *)row_bytes_addr;

        for (int x = 0; x < w; x++) {
            int fx = dest_x + x;
            int fy = dest_y + y;
            if (fx < 0 || fx >= (int)sw || fy < 0 || fy >= (int)sh) continue;

            const uint8_t *px = src + x * 3;
            row[fx] = rgb888_to_rgb565(px[0], px[1], px[2]);
        }
    }
}

/* ---------------------------------------------------------
 * BMP を (dest_x, dest_y) に scale 倍して描画（16bpp）
 * --------------------------------------------------------- */
void draw_bmp_scaled(int dest_x, int dest_y, const uint8_t *bmp_data, int scale) {
    if (!bmp_data || !vbe) return;

    uint16_t *fb_base = (uint16_t *)(uintptr_t)vbe->vram_addr;
    uint32_t pitch = vbe->lfb_pitch;
    uint32_t sw = vbe->x_res, sh = vbe->y_res;

    int w, h, row_bytes;
    const uint8_t *pixels;
    if (parse_bmp_header(bmp_data, &pixels, &w, &h, &row_bytes) < 0) return;
    int topdown = ( *(int32_t*)(bmp_data+0x16) < 0 );

    for (int y = 0; y < h; y++) {
        int bmp_row = topdown ? y : (h - 1 - y);
        const uint8_t *src_row = pixels + bmp_row * row_bytes;

        for (int sy = 0; sy < scale; sy++) {
            int fy = dest_y + y * scale + sy;
            if (fy < 0 || fy >= (int)sh) continue;
            uint8_t *row_bytes_addr = (uint8_t *)fb_base + fy * pitch;
            uint16_t *row = (uint16_t *)row_bytes_addr;

            for (int x = 0; x < w; x++) {
                const uint8_t *px = src_row + x * 3;
                uint16_t c = rgb888_to_rgb565(px[0], px[1], px[2]);

                for (int sx = 0; sx < scale; sx++) {
                    int fx = dest_x + x * scale + sx;
                    if (fx < 0 || fx >= (int)sw) continue;
                    row[fx] = c;
                }
            }
        }
    }
}

/* ---------------------------------------------------------
 * 起動画像表示
 * --------------------------------------------------------- */
void show_boot_logo(void) {
    if (!vbe) return;
    int w = vbe->x_res, h = vbe->y_res;
    draw_bmp_scaled((w - 80)/2 - 150, (h - 80)/2 - 150,
                    src_kernel_penguin_bmp, 8);
}

void show_penguin(void) {
    draw_bmp(0, 0, src_kernel_penguin_bmp);
}
