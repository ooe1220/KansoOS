#include "console.h"
#include "vbe.h"
#include "string.h"
#include "timer.h"

// 本来はこれらもVBE構造体から取り出すべき、今後の課題
#define CHAR_W  8
#define CHAR_H  16
#define COLS    (800 / CHAR_W)
#define ROWS    (600 / CHAR_H)

//#define FG_COLOR  0x00AAAA00
//#define BG_COLOR  0x00000000

#define FG_COLOR  0x00FFFFFF   // 白
#define BG_COLOR  0x000000FF   // 青

#define CONSOLE_ROWS (600 / 16)   // 37行（800x600, フォント16px高）

void cursor_clear(void);

//カーソル
int cx = 0;
int cy = 0;
static int cursor_visible = 1;

static char text_buf[ROWS][COLS];

void console_init(void) {
    cx = 0;
    cy = 0;
    vbe_video_init();
    vbe_clear(BG_COLOR);

    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            text_buf[y][x] = ' ';
}

static void scroll(void) {
    
    uint8_t *fb = (uint8_t *)vbe->vram_addr;
    uint32_t pitch = vbe->lfb_pitch;   // 2400

    // 1. 上に 1 文字行（16px）シフト
    //    16px 単位でブロックコピー（安全かつ高速）
    for (int y = CHAR_H; y < 600; y += CHAR_H) {
        uint8_t *dst = fb + (y - CHAR_H) * pitch;
        uint8_t *src = fb + y * pitch;

        // 1行分（16 × 2400 = 38400 バイト）
        memcpy(dst, src, pitch * CHAR_H);
    }

    // 2. 最下行（37行目）を背景色でクリア
    //    座標: y = (ROWS - 1) * CHAR_H = 592
    draw_rect(
        0,
        (ROWS - 1) * CHAR_H,
        COLS * CHAR_W,
        CHAR_H,
        BG_COLOR
    );

    // 3. カーソル位置リセット
    cy = ROWS - 1;
    cx = 0;
}

// カーソル非表示
void cursor_clear(void) {
    draw_rect(cx * CHAR_W, cy * CHAR_H, 2, 16, BG_COLOR);
}

// カーソル表示
void cursor_draw(void) {
    draw_rect(cx * CHAR_W, cy * CHAR_H, 2, 16, FG_COLOR);
}

// 点滅させると競合してCPU例外13が頻発　一旦コメントアウト
void cursor_blink(void) {
    
    static uint32_t last_tick = 0;
    uint32_t tick = timer_get_ticks();

    if (tick - last_tick >= 500) {
        last_tick = tick;
        if (cursor_visible) {
            //cursor_clear();
            cursor_visible = 0;
        } else {
            //cursor_draw();
            cursor_visible = 1;
        }
    }
}

void cursor_init(void) {
    cursor_visible = 1;
    cursor_draw();
}

void kputc(char c) {
    cursor_clear();

    if (c == '\n') {
        cx = 0;
        cy++;
    } else if (c == '\b') {
        // BACKSPACE: 左へ1文字分戻って背景色で上書き
        if (cx > 0) {
            cx--;
            //draw_char(cx * CHAR_W, cy * CHAR_H, ' ', BG_COLOR);
            //draw_rect(cx * CHAR_W, cy * CHAR_H, CHAR_W, CHAR_H, BG_COLOR);
        // 念のため範囲内だけ塗る
        if (cy < ROWS && cx < COLS) {
            draw_rect(cx * CHAR_W, cy * CHAR_H, CHAR_W, CHAR_H, BG_COLOR);
        }
        }
        // カーソル位置はそのまま（左に戻った位置に留まる）
    } else {
        draw_char(cx * CHAR_W, cy * CHAR_H, c, FG_COLOR);
        cx++;
    }

    if (cy >= ROWS) {
        scroll();
    }

    cursor_draw();
    
}

void kputs(const char *s) {
    while (*s) {
        kputc(*s++);
    }
}

void console_clear(void) {
    vbe_clear(BG_COLOR);
    cx = 0;
    cy = 0;

    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            text_buf[y][x] = ' ';
}

/**
 * 符号付き整数を文字列に変換
 * @param value 入力整数
 * @param buffer 出力文字列バッファ
 */
static void itoa(int value, char *buffer) {
    char temp[12];
    int i = 0, j;
    int negative = 0;

    if (value < 0) {
        negative = 1;
        value = -value;
    }

    do {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    } while (value);

    if (negative)
        temp[i++] = '-';

    for (j = 0; j < i; j++)
        buffer[j] = temp[i - j - 1];

    buffer[i] = '\0';
}

static void utoa(unsigned int value, char *buffer, int base) {
    char temp[12];
    int i = 0, j;

    do {
        int remainder = value % base;
        temp[i++] = (remainder < 10) ? ('0' + remainder) : ('a' + remainder - 10);
        value /= base;
    } while (value);

    for (j = 0; j < i; j++)
        buffer[j] = temp[i - j - 1];

    buffer[i] = '\0';
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
