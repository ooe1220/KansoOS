#include "console.h"
#include "pic.h"
#include "timer.h"
#include "pit.h"
#include "idt.h"

void irq0_handler(void);

static volatile uint32_t tick_count = 0;

void timer_tick(void) {
    tick_count++;
    // ここに将来スケジューラ呼び出しを入れる
    
    cursor_blink();
}

uint32_t timer_get_ticks(void) {
    return tick_count;
}

void timer_init(void) {
    pit_init(1193);                         // ハードウェア初期化
    idt_set_gate(0x20, (uint32_t)irq0_handler);
    pic_unmask_irq(0);                    // IRQ0 許可
}

__attribute__((naked))
void irq0_handler(void) {
    asm volatile(
        "pusha\n"
        "push %ds\n"
        "push %es\n"
        "push %fs\n"
        "push %gs\n"
        "call timer_tick\n"
        "movb $0x20, %al\n"
        "outb %al, $0x20\n"
        "pop %gs\n"
        "pop %fs\n"
        "pop %es\n"
        "pop %ds\n"
        "popa\n"
        "iret\n"
    );
}
