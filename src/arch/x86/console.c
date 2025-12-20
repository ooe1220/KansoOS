#include "console.h"
#include "io.h"

static void hw_move_cursor(int x, int y);

#define VRAM       ((volatile unsigned short*)0xB8000)
#define COLS       80
#define ROWS       25
#define ATTR       0x0F00  // 白文字・黒背景

static int cursor_x = 0;
static int cursor_y = 0;

/* 内部関数：画面スクロール */
static void scroll() {
    // 最下行に到達していない場合はスクロール不要
    if (cursor_y < ROWS)
        return;

    // 1行分上に詰める (2行目 → 1行目)
    for (int y = 1; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            VRAM[(y - 1) * COLS + x] = VRAM[y * COLS + x];
        }
    }

    // 最終行を空白で埋める
    for (int x = 0; x < COLS; x++) {
        VRAM[(ROWS - 1) * COLS + x] = ATTR | ' ';
    }

    cursor_y = ROWS - 1;  // 最終行にセット
}

/* 1文字出力 */
void kputc(char c) {

    if (c == '\n') {        // 改行処理
        cursor_x = 0;
        cursor_y++;
        scroll();
        return;
    }

    // VRAM に書き込み
    VRAM[cursor_y * COLS + cursor_x] = ATTR | c;

    cursor_x++;

    // 行末を超えたら折り返し
    if (cursor_x >= COLS) {
        cursor_x = 0;
        cursor_y++;
        scroll();
    }
    
     hw_move_cursor(cursor_x, cursor_y);
}

/* 文字列出力 */
void kputs(const char* s) {
    while (*s) {
        kputc(*s++);
    }
}

/* 画面クリア（任意） */
void console_clear() {
    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            VRAM[y * COLS + x] = ATTR | ' ';
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

/* ハードウェアカーソル移動 */
/*
 x: 0〜79（列）
 y: 0〜24（行）
*/
static void hw_move_cursor(int x, int y) {
    unsigned short pos = y * COLS + x;

    outb(0x3D4, 0x0F);           // 下位バイト
    outb(0x3D5, pos & 0xFF);
    outb(0x3D4, 0x0E);           // 上位バイト
    outb(0x3D5, (pos >> 8) & 0xFF);
}


