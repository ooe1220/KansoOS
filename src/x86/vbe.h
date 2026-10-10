
#ifndef VBE_H
#define VBE_H

#include "stdint.h"

#define MODEINFO_PTR (*(volatile uint32_t *)0x7DFA) // VBR内で取得した情報構造体のアドレス置き場

typedef struct {
    /* === 0x00 - 0x27: 基本情報 === */
    uint16_t mode_attr;        // +0x00 モード属性
    uint8_t  win_attr[2];      // +0x02 ウィンドウA/B属性
    uint16_t win_gran;         // +0x04 ウィンドウグラニュラリティ
    uint16_t win_size;         // +0x06 ウィンドウサイズ (KB)
    uint16_t win_seg[2];       // +0x08 ウィンドウA/Bセグメント
    uint32_t win_func;         // +0x0C ウィンドウ関数ポインタ(FAR)
    uint16_t pitch;            // +0x10 バイト/ライン
    uint16_t x_res;            // +0x12 水平解像度
    uint16_t y_res;            // +0x14 垂直解像度
    uint8_t  x_charsz;         // +0x16 文字セル幅
    uint8_t  y_charsz;         // +0x17 文字セル高
    uint8_t  planes;           // +0x18 プレーン数
    uint8_t  bpp;              // +0x19 色深度
    uint8_t  banks;            // +0x1A バンク数
    uint8_t  mem_model;        // +0x1B メモリモデルタイプ
    uint8_t  bank_size;        // +0x1C バンクサイズ (KB)
    uint8_t  img_pages;        // +0x1D イメージページ数
    uint8_t  reserved1;        // +0x1E 予約
    uint8_t  red_mask;         // +0x1F 赤マスク幅
    uint8_t  red_pos;          // +0x20 赤ビット位置
    uint8_t  green_mask;       // +0x21 緑マスク幅
    uint8_t  green_pos;        // +0x22 緑ビット位置
    uint8_t  blue_mask;        // +0x23 青マスク幅
    uint8_t  blue_pos;         // +0x24 青ビット位置
    uint8_t  rsvd_mask;        // +0x25 予約マスク幅
    uint8_t  rsvd_pos;         // +0x26 予約ビット位置
    uint8_t  directcolor;      // +0x27 ダイレクトカラー情報

    /* === 0x28 - 0x33: LFB（リニアフレームバッファ）情報 === */
    uint32_t vram_addr;        // +0x28 LFB物理アドレス ★
    uint32_t offscreen_ptr;    // +0x2C オフスクリーンメモリオフセット
    uint16_t offscreen_size;   // +0x30 オフスクリーンメモリサイズ (KB)

    /* === 0x32 - 0xFF: VBE 2.0+ 拡張 === */
    uint16_t lfb_pitch;        // +0x32 LFB用バイト/ライン（VBE 2.0+）
    uint8_t  bpp2;             // +0x34 LFB用 bpp（VBE 2.0+）
    uint8_t  reserved2[189];   // +0x35 - 0xFF 予約（ゼロ）
} __attribute__((packed)) vbe_mode_info_t;

#define MODEINFO_PTR (*(volatile uint32_t *)0x7DFA)

extern vbe_mode_info_t *vbe;

void vbe_video_init(void);
void vbe_clear(uint32_t bg_color);
void draw_char(int x, int y, char c, uint32_t fg_color);

void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_circle(int cx, int cy, int r, uint32_t color);

#endif
