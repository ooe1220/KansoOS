#include "arch/x86/io.h"

/* 8042 input buffer が空になるまで待つ */
static inline void wait_8042_input_empty(void)
{
    while (inb(0x64) & 0x02);
}

/* 8042 output buffer に溜まるまで待つ */
static inline void wait_8042_output_full(void)
{
    while (!(inb(0x64) & 0x01));
}

/* 8042 キーボードコントローラ方式 */
static void enable_a20_8042(void)
{
    uint8_t data;

    /* キーボード無効化 */
    wait_8042_input_empty();
    outb(0x64, 0xAD);

    /* 出力ポート読み出し */
    wait_8042_input_empty();
    outb(0x64, 0xD0);

    wait_8042_output_full();
    data = inb(0x60);

    /* A20 (bit1) を立てる */
    data |= 0x02;

    /* 出力ポート書き込み */
    wait_8042_input_empty();
    outb(0x64, 0xD1);

    wait_8042_input_empty();
    outb(0x60, data);

    /* キーボード再有効化 */
    wait_8042_input_empty();
    outb(0x64, 0xAE);
    
}

/* 外部公開用 */
void enable_a20(void)
{
    /* 念のため 8042 も実行 */
    enable_a20_8042();
}

