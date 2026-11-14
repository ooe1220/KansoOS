#include "../lib/stdint.h"
#include "vbe.h"    /* vbe_mode_info_t *vbe を使う */

/* ユーザ側 gfx.h の struct gfx_buf と「並びだけ」合わせる。
   カーネルは gfx.h を include しない
   ２重定義はいつか解決しないといけない*/
typedef struct {
    uint32_t base;    /* DRAWBUF_ADDR (0xB00000) */
    int      width;   /* 800 */
    int      height;  /* 600 */
} gfx_buf_t;

int gfx_do_flip(const void *arg)
{
    const gfx_buf_t *b = (const gfx_buf_t *)arg;
    if (!b || !vbe) return -1;

    /* ピクセル列の先頭（raw 16bpp、ヘッダなしなので base そのもの） */
    const uint8_t *src = (const uint8_t *)b->base;
    int      w  = b->width;
    int      h  = b->height;
    uint32_t rs = (uint32_t)w * 2;        /* 1行バイト数 = width * 2 */

    /* VRAM の先頭と pitch は VBE 構造体から取る */
    uint8_t *vram = (uint8_t *)vbe->vram_addr;
    uint32_t vram_pitch = (uint32_t)vbe->lfb_pitch;

/*
    for (int y = 0; y < h; y++) {
        const uint8_t *s = src + (uint32_t)y * rs;
        uint8_t       *d = vram + (uint32_t)y * vram_pitch;
        for (uint32_t x = 0; x < rs; x++)
            d[x] = s[x];
    }
*/
    // 4バイトずつ複製するようにしたらかなり速くなった
    for (int y = 0; y < h; y++) {
       const uint8_t *s = src + (uint32_t)y * rs;
       uint8_t       *d = vram + (uint32_t)y * vram_pitch;

        for (uint32_t x = 0; x < rs; x += 4)
            *(uint32_t *)(d + x) = *(const uint32_t *)(s + x);
    }
    return 0;
}