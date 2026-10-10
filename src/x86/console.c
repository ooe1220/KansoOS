#include "console.h"
#include "vbe.h"
#include "string.h"

// 本来はこれらもVBE構造体から取り出すべき、今後の課題
#define CHAR_W  8
#define CHAR_H  16
#define COLS    (800 / CHAR_W)
#define ROWS    (600 / CHAR_H)

//#define FG_COLOR  0x00AAAA00
//#define BG_COLOR  0x00000000

#define FG_COLOR  0x00FFFFFF   // 白
#define BG_COLOR  0x000000FF   // 青

static int cursor_x = 0;
static int cursor_y = 0;
static char text_buf[ROWS][COLS];

void console_init(void) {
    cursor_x = 0;
    cursor_y = 0;
    vbe_video_init();
    vbe_clear(BG_COLOR);

    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            text_buf[y][x] = ' ';
}

static void scroll(void) {
    if (cursor_y < ROWS)
        return;

    uint8_t *fb = (uint8_t *)vbe->vram_addr;
    uint32_t pitch = vbe->lfb_pitch;
    uint32_t width_bytes = 800 * 3;

    for (uint32_t y = CHAR_H; y < 600; y++) {
        memcpy(fb + (y - CHAR_H) * pitch, fb + y * pitch, width_bytes);
    }

    for (uint32_t y = 600 - CHAR_H; y < 600; y++) {
        memset(fb + y * pitch, 0x00, width_bytes);
    }

    for (int y = 1; y < ROWS; y++)
        memcpy(text_buf[y-1], text_buf[y], COLS);
    for (int x = 0; x < COLS; x++)
        text_buf[ROWS-1][x] = ' ';

    cursor_y = ROWS - 1;
}

void kputc(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        scroll();
        return;
    }

    if (c == '\b') {
        if (cursor_x > 0) cursor_x--;
        return;
    }

    if (c == '\r') {
        cursor_x = 0;
        return;
    }

    draw_char_8x16(cursor_x * CHAR_W, cursor_y * CHAR_H, c, FG_COLOR, BG_COLOR);
    text_buf[cursor_y][cursor_x] = c;

    cursor_x++;
    if (cursor_x >= COLS) {
        cursor_x = 0;
        cursor_y++;
        scroll();
    }
}

void kputs(const char *s) {
    while (*s) {
        kputc(*s++);
    }
}

void console_clear(void) {
    vbe_clear(BG_COLOR);
    cursor_x = 0;
    cursor_y = 0;

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
