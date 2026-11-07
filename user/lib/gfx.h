#ifndef GFX_H
#define GFX_H

#include "os.h"

/* ---- 描画先（ユーザ側で持つ画面バッファ） ---- */
// mallocが安定したら可変にする
#ifndef DRAWBUF_ADDR
#define DRAWBUF_ADDR  ((uint32_t)0x00B00000u)
#endif

struct gfx_buf {
    uint32_t base;    /* 描画先バッファのアドレス */
    int       width;   /* バッファの幅（ピクセル） */
    int       height;  /* バッファの高さ */
};

// 今は固定
static struct gfx_buf g_gfx = {
    .base   = DRAWBUF_ADDR, 
    .width  = 800,
    .height = 600,
};

static inline uint16_t *g_get_drawbuf(void)
{
    return (uint16_t *)g_gfx.base;
}

/* ---- 色（RGB565） ---- */
#define COLOR_BLACK 0x0000
#define COLOR_BLUE  0x001F
#define COLOR_GREEN 0x07E0
#define COLOR_RED   0xF800
#define COLOR_WHITE 0xFFFF

/* ---- 画面情報　ピッチはOS側で考慮する ---- */
#ifndef FB_WIDTH
#define FB_WIDTH  800
#endif
#ifndef FB_HEIGHT
#define FB_HEIGHT 600
#endif

/* ---- 低レベル：バッファへ描く（syscall なし） ---- */
static inline void buf_pixel(uint16_t *buf, int x, int y, uint16_t c)
{
    if (x < 0 || x >= FB_WIDTH || y < 0 || y >= FB_HEIGHT) return;
    buf[y * FB_WIDTH + x] = c;
}

static inline void buf_rect(uint16_t *buf,int x, int y, int w, int h, uint16_t c)
{
    if (w < 0) { x += w; w = -w; }
    if (h < 0) { y += h; h = -h; }
    for (int row = 0; row < h; row++)
        for (int col = 0; col < w; col++)
            buf_pixel(buf, x + col, y + row, c);
}

/* 線*/
static inline void buf_line(uint16_t *buf,int x0, int y0, int x1, int y1, uint16_t c)
{
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    while (1) {
        buf_pixel(buf, x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int e2 = err << 1;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

/* ---- 描画関数 ---- */
static inline void g_pixel(int x, int y, uint16_t c) { buf_pixel(g_get_drawbuf(), x, y, c); }
static inline void g_rect (int x,int y,int w,int h,uint16_t c){ buf_rect (g_get_drawbuf(),x,y,w,h,c); }
static inline void g_line (int x0,int y0,int x1,int y1,uint16_t c){ buf_line(g_get_drawbuf(),x0,y0,x1,y1,c); }

/* ---- システムコール経由で画面へ描画 ---- */
static inline void g_flip(void)   /* 画面に表示（syscall 1回） */
{
    asm volatile(
        "mov %[num], %%eax\n"
        "int $0x80\n"
        :
        : [num] "i"(SYS_GFX_FLIP),   // EAX ← 番号
          "b"(&g_gfx)                // EBX ← struct gfx_buf のアドレス
        : "eax", "memory"
    );
}

#endif