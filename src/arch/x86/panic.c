#include "arch/x86/console.h"
#include "arch/x86/idt.h"

void exception_handler(void) {
    kputs("KERNEL PANIC!\n");
    while (1) {
        asm volatile("hlt");
    }
}

__attribute__((naked))
void isr_stub(void) {
    __asm__ volatile (
        "call exception_handler\n"
        "iret\n"
    );
}

/*
 * CPU 例外の初期化
 * IDT の 0〜31 番は CPU 例外として予約されている。
 * 今の OS では全て同じ ISR に割り当て、
 * 発生したら必ず hltする。
 */
void exception_init(void) {
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (uint32_t)isr_stub);
    }
}
