#include "arch/x86/idt.h"
#include "arch/x86/console.h"

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];

extern void isr6(void);

void idt_init(void) {
    struct idt_ptr idtp;

    uint32_t addr = (uint32_t)isr6;

    idt[6].offset_low  = addr & 0xFFFF;
    idt[6].selector    = 0x08;   // kernel code segment
    idt[6].zero        = 0;
    idt[6].type_attr   = 0x8E;   // interrupt gate
    idt[6].offset_high = addr >> 16;

    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint32_t)&idt;

    asm volatile("lidt (%0)" :: "r"(&idtp));

    kputs("IDT loaded\n");
}

void exception_handler(void) {
    kputs("EXCEPTION!\n");
    while (1) {
        asm volatile("hlt");
    }
}

