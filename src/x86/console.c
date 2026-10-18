#include "console.h"
#include "vbe.h"
#include "string.h"
#include "timer.h"
#include "./lib/itoa.h"

// 文字サイズ・画面構成（800x600 / フォント8x16）
#define CHAR_W  8
#define CHAR_H  16
#define COLS    (800 / CHAR_W)
#define ROWS    (600 / CHAR_H)

/* =========================================================
 * 色定義（RGB565 の uint16_t のみ）
 * ========================================================= */
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0

#define FG_COLOR  COLOR_WHITE   // 文字色：白
#define BG_COLOR  COLOR_BLUE    // 背景色：青

void cursor_clear(void);

// カーソル位置
int cx = 0;
int cy = 0;

// 文字バッファ（色に関係ないのでそのまま）
static char text_buf[ROWS][COLS];

/* ---------------------------------------------------------
 * コンソール初期化
 * --------------------------------------------------------- */
void console_init(void) {
    cx = 0;
    cy = 0;
    vbe_video_init();
    vbe_clear(BG_COLOR);

    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            text_buf[y][x] = ' ';
}

/* ---------------------------------------------------------
 * スクロール（1行分上にシフト）
 * --------------------------------------------------------- */
static void scroll(void) {
    uint8_t *fb = (uint8_t *)vbe->vram_addr;
    uint32_t pitch = vbe->lfb_pitch;

    // 上に 16px シフト（memmove で自己上書き対策）
    for (int y = CHAR_H; y < 600; y += CHAR_H) {
        uint8_t *dst = fb + (y - CHAR_H) * pitch;
        uint8_t *src = fb + y * pitch;
        memcpy(dst, src, pitch * CHAR_H);(dst, src, pitch * CHAR_H);
    }

    // 最下行を背景色でクリア
    draw_rect(0, (ROWS - 1) * CHAR_H, COLS * CHAR_W, CHAR_H, BG_COLOR);

    cy = ROWS - 1;
    cx = 0;
}

/* ---------------------------------------------------------
 * カーソル描画 / 消去
 * --------------------------------------------------------- */
void cursor_clear(void) {
    draw_rect(cx * CHAR_W, cy * CHAR_H, 2, CHAR_H, BG_COLOR);
}

void cursor_draw(void) {
    draw_rect(cx * CHAR_W, cy * CHAR_H, 2, CHAR_H, FG_COLOR);
}

/* ---------------------------------------------------------
 * 1文字出力
 * --------------------------------------------------------- */
void kputc(char c) {
    cursor_clear();

    if (c == '\n') {
        cx = 0;
        cy++;
    } else if (c == '\b') {
        if (cx > 0) {
            cx--;
            if (cy < ROWS && cx < COLS) {
                draw_rect(cx * CHAR_W, cy * CHAR_H, CHAR_W, CHAR_H, BG_COLOR);
            }
        }
    } else {
        draw_char(cx * CHAR_W, cy * CHAR_H, c, FG_COLOR);
        cx++;
    }

    if (cy >= ROWS) {
        scroll();
    }

    cursor_draw();
}

/* ---------------------------------------------------------
 * 文字列出力
 * --------------------------------------------------------- */
void kputs(const char *s) {
    while (*s) {
        kputc(*s++);
    }
}

/* ---------------------------------------------------------
 * 画面クリア
 * --------------------------------------------------------- */
void console_clear(void) {
    vbe_clear(BG_COLOR);
    cx = 0;
    cy = 0;

    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            text_buf[y][x] = ' ';
}



void kprintf(const char* format, ...) {
    // 可変数引数
    char* arg_ptr = (char*)&format + sizeof(char*);
    const char* p = format;
    char buffer[32];
    
    while (*p) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case '%':  // %% -> %
                    kputc('%');
                    break;
                    
                case 'c':  // %c -> 文字
                    kputc(*((char*)arg_ptr));
                    arg_ptr += sizeof(char);
                    break;
                    
                case 's':  // %s -> 文字列
                    kputs(*((char**)arg_ptr));
                    arg_ptr += sizeof(char*);
                    break;
                    
                case 'd':  // %d -> 符号付き整数
                case 'i':
                    itoa(*((int*)arg_ptr), buffer);
                    kputs(buffer);
                    arg_ptr += sizeof(int);
                    break;
                    
                case 'u':  // %u -> 符号無し整数
                    utoa(*((unsigned int*)arg_ptr), buffer, 10);
                    kputs(buffer);
                    arg_ptr += sizeof(unsigned int);
                    break;
                    
                case 'x':  // %x -> 16進数
                    utoa(*((unsigned int*)arg_ptr), buffer, 16);
                    kputs(buffer);
                    arg_ptr += sizeof(unsigned int);
                    break;
                    
                default:  // 未定義はそのまま出す
                    kputc('%');
                    kputc(*p);
                    break;
            }
        } else {
            kputc(*p);
        }
        p++;
    }
}
