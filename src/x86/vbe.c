#include "vbe.h"
#include "x86/console.h"
#include "lib/stddef.h"
#include "fontdata.h"
#include "debug/debug_print.h"

vbe_mode_info_t *vbe = NULL;

static uint8_t  *fb_base = NULL;
static uint32_t fb_pitch = 0; // QEMU2400 実機3200となっていた
static uint32_t fb_width = 0;
static uint32_t fb_height = 0;

 // 1ピクセルのバイト数。VBEを24bppで設定した筈が、QEMU24bpp, 実機32bppとなり1ピクセル毎に1バイトずれていた
static uint8_t bytes_per_pixel = 0;

static uintptr_t max_write_addr = 0; // VRAM WRAPの原因調査用

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
    bytes_per_pixel = vbe->bpp / 8; // bpp=24なら3、bpp=32なら4

    vbe_clear(0x00000000); //黒で画面クリア 
}

// 画面クリア
// 0x00RRGGBBのbg_colorをそれぞれRGBのピクセルに書き込む
void vbe_clear(uint32_t bg_color) {
    if (!fb_base) return;

    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            uint8_t *p = fb_base + y * fb_pitch + x * bytes_per_pixel; // putpixel()呼ぶとcallやスタック生成で遅くなるのでべた書き
            p[0] = bg_color & 0xFF;
            p[1] = (bg_color >> 8) & 0xFF;
            p[2] = (bg_color >> 16) & 0xFF;
        }
    }
}

static void putpixel(int x, int y, uint32_t color) {
    if (!fb_base) return;
    if (x < 0 || x >= (int)fb_width || y < 0 || y >= (int)fb_height) return;

    uint8_t *p = fb_base + y * fb_pitch + x * bytes_per_pixel;
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

/********************************
* 以下VBEデバッグ用　実装とは無関係
*********************************/

//VRAMと画面の有効範囲を調査
void vbe_fill_test(void) {
    if (!fb_base) return;
    
    // 画面全体を赤（0x000000FF）で塗りつぶす
    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            uint8_t *p = fb_base + y * fb_pitch + x * bytes_per_pixel;
            p[0] = 0xFF;  // B
            p[1] = 0x00;  // G
            p[2] = 0x00;  // R → 赤
        }
    }
}

// 一定のアドレスを越えたら描画済み領域を侵食する現象。ラップしていないか→していなかった
void vram_wrap_check(void) {
    uintptr_t base = (uintptr_t)fb_base;
    uintptr_t prev_line_addr = 0;

    for (int y = 0; y < fb_height; y++) {
        uintptr_t line_addr = base + y * fb_pitch;

        if (prev_line_addr > 0) {
            uintptr_t expected = prev_line_addr + fb_pitch;
            if (line_addr != expected) {
                // アドレスが飛んでる → WRAP or 不連続
                debug_printf("WRAP at y=%d: expected=0x%x got=0x%x\n",
                             y, expected, line_addr);
                // その行を真っ赤にする
                for (int x = 0; x < fb_width; x++) {
                    uint8_t *p = (uint8_t *)(base + y * fb_pitch + x * bytes_per_pixel);
                    p[0] = 0x00;
                    p[1] = 0x00;
                    p[2] = 0xFF;
                    if (bytes_per_pixel == 4) p[3] = 0x00;
                }
                return;
            }
        }
        prev_line_addr = line_addr;
    }

    debug_printf("VRAM linear OK\n");
}

#define HIT_COUNT_BASE 0x200000
#define HIT_COUNT_SIZE (800 * 600 * 4)

//　実機で衝突した。fb_width=800 fb_height=600 fb_pitch=2400 bytes_per_pixel=3
void vram_collision_check(void) {
    uintptr_t base = (uintptr_t)fb_base;
    uintptr_t vram_size = (uintptr_t)(fb_height * fb_pitch);

    uint8_t *hit_count = (uint8_t *)HIT_COUNT_BASE;

    // ゼロクリア
    for (uintptr_t i = 0; i < HIT_COUNT_SIZE; i++) {
        hit_count[i] = 0;
    }

    for (int y = 0; y < (int)fb_height; y++) {
        for (int x = 0; x < (int)fb_width; x++) {
            uintptr_t addr = base + y * fb_pitch + x * bytes_per_pixel;
            uintptr_t offset = addr - base;

            if (offset >= HIT_COUNT_SIZE) {
                debug_printf("OUT_OF_VRAM y=%d x=%d offset=0x%x\n", y, x, offset);
                __asm__ volatile ("hlt");
            }

            hit_count[offset]++;

            if (hit_count[offset] > 1) {
                debug_printf("COLLISION y=%d x=%d offset=0x%x pitch=%d bpp=%d width=%d\n",
                 y, x, offset, fb_pitch, bytes_per_pixel, fb_width);
                __asm__ volatile ("hlt");
            }
        }
    }

    // 全クリア
    debug_printf("VRAM_OK\n");
    __asm__ volatile ("hlt");
}

void vram_alias_check(void) {
    uintptr_t base = (uintptr_t)fb_base;

    // VRAM 全体を 0 クリア
    for (uintptr_t i = 0; i < (uintptr_t)(fb_height * fb_pitch); i += 4) {
        *(volatile uint32_t *)(base + i) = 0;
    }

    debug_printf("VRAM cleared\n");

    // 全座標を順に 0xFFFFFFFF でマークしていく
    for (int y = 0; y < (int)fb_height; y++) {
        for (int x = 0; x < (int)fb_width; x++) {
            uintptr_t addr = base + y * fb_pitch + x * bytes_per_pixel;
            volatile uint32_t *p = (uint32_t *)addr;

            if (*p != 0) {
                // 既に書き込み済み → 衝突検出
                // 衝突した座標のアドレスに特徴的なパターンを書く
                *p = 0xAAAAAAAA;  // 10101010...

                // 少し待つ（メモリの反映を確実にするため）
                for (volatile int i = 0; i < 1000000; i++);

                // 衝突相手の座標を探す
                // 全座標をもう一度スキャンして 0xAAAAAAAA が見える場所を探す
                for (int yy = 0; yy < (int)fb_height; yy++) {
                    for (int xx = 0; xx < (int)fb_width; xx++) {
                        uintptr_t addr2 = base + yy * fb_pitch + xx * bytes_per_pixel;
                        volatile uint32_t *p2 = (uint32_t *)addr2;

                        if (*p2 == 0xAAAAAAAA && !(yy == y && xx == x)) {
                            debug_printf("ALIAS: (%d,%d) and (%d,%d) share addr 0x%x\n",
                                         y, x, yy, xx, addr);
                            __asm__ volatile ("hlt");
                        }
                    }
                }

                debug_printf("ALIAS partner not found??\n");
                __asm__ volatile ("hlt");
            }

            *p = 0xFFFFFFFF;
        }
    }

    debug_printf("NO_ALIAS\n");
    __asm__ volatile ("hlt");
}
