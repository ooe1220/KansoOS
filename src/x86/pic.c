// PIC.c
#include "io.h"

// 第二ブートローダで初期化済み

void pic_send_eoi(uint8_t irq) {
    (void)irq;              // 今は IRQ1 しか使わない
    outb(0x20, 0x20);
}

// -------------------------
// 指定したIRQを無効化
// -------------------------
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

// -------------------------
// 指定したIRQを有効化
// -------------------------
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
