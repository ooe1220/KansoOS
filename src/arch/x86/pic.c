// PIC.c
#include "arch/x86/io.h"

// PIC初期化
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

void pic_mask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t mask;

    if (irq < 8) {
        port = 0x21;
        mask = inb(port);
        mask |= (1 << irq); // 例 IRQ=1 : 00000001b << 1 = 00000010
    } else {
        port = 0xA1;
        irq -= 8;
        mask = inb(port);
        mask |= (1 << irq);
    }

    outb(port, mask);
}


void pic_unmask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t mask;

    if (irq < 8) {
        port = 0x21;
        mask = inb(port);
        mask &= ~(1 << irq);  // ← 0 にする
    } else {
        port = 0xA1;
        irq -= 8;
        mask = inb(port);
        mask &= ~(1 << irq);
    }

    outb(port, mask);
}
