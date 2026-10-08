#include "gdt.h"

uint8_t user_stack[4096];

__attribute__((naked)) void user_code(void) {
    asm volatile (
        "mov $0x00, %%eax\n"
        "out %%al, $0x64\n"
        "hlt\n"
        ::: "eax"
    );
}

void enter_user_mode(void) {
    asm volatile (
        "cli\n"

        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"

        "mov $0x10, %%ax\n"
        "mov %%ax, %%ss\n"

        "mov %0, %%esp\n"

        "pushl $0x23\n"
        "pushl %%esp\n"
        "pushfl\n"
        "pushl $0x1B\n"
        "pushl $user_code\n"
        "iret\n"
        :
        : "r"(user_stack + 4096)
        : "eax"
    );
}

void kmain(void) {
    gdt_init();

    volatile unsigned char *vid = (unsigned char *)0xB8000;
    
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        vid[i]     = ' ';
        vid[i + 1] = 0x07;
    }
    
    const char *msg = "DROPPING TO RING3...";
    int i = 0;
    for (int j = 0; msg[j]; j++) {
        vid[i++] = msg[j];
        vid[i++] = 0x07;
    }
    
    // user_stack のアドレスを16進数で表示
uint32_t sp_addr = (uint32_t)(user_stack + 4096);
for (int k = 0; k < 8; k++) {
    int shift = (7 - k) * 4;
    int nibble = (sp_addr >> shift) & 0xF;
    vid[160 + k * 2] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
    vid[161 + k * 2] = 0x0E;
}

    enter_user_mode();

    // ここには絶対来ない
    for (;;) asm ("hlt");
}

