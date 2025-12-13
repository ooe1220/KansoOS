// PIC.c
#include "arch/x86/io.h"

// PIC初期化（リマップ＋全マスク）
void pic_init(void) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // 初期状態は全IRQ禁止
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

// IRQマスク更新（8bit: 下位8bitはマスタ、上位8bitはスレーブ）
void pic_set_mask(uint16_t mask) {
    outb(0x21, (uint8_t)(mask & 0xFF));    // マスタ
    outb(0xA1, (uint8_t)((mask >> 8) & 0xFF)); // スレーブ
}

