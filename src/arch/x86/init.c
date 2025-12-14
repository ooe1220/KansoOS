#include "arch/x86/pic.h"
#include "arch/x86/a20.h"
#include "arch/x86/console.h"

void init_irq(void) {

    pic_init();

    // キーボードのみ有効 (IRQ1)
    // マスタPIC: 0b11111101 = 0xFD
    // スレーブPIC: 0b11111111 = 0xFF
    pic_set_mask(0xFFFD);
}

//カーネルから直接x86固有の関数を呼ばない
void arch_init(void) {
    enable_a20();
    puts("A20 initialized\n");
    init_irq();
    puts("Interrupt initialized\n");

}
