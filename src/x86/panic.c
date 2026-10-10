#include "console.h"
#include "idt.h"

void cpu_exception_0(void)  { kputs("KERNEL PANIC! Exception #0\n");  while (1) asm volatile("hlt"); }
void cpu_exception_1(void)  { kputs("KERNEL PANIC! Exception #1\n");  while (1) asm volatile("hlt"); }
void cpu_exception_2(void)  { kputs("KERNEL PANIC! Exception #2\n");  while (1) asm volatile("hlt"); }
void cpu_exception_3(void)  { kputs("KERNEL PANIC! Exception #3\n");  while (1) asm volatile("hlt"); }
void cpu_exception_4(void)  { kputs("KERNEL PANIC! Exception #4\n");  while (1) asm volatile("hlt"); }
void cpu_exception_5(void)  { kputs("KERNEL PANIC! Exception #5\n");  while (1) asm volatile("hlt"); }
void cpu_exception_6(void)  { kputs("KERNEL PANIC! Exception #6\n");  while (1) asm volatile("hlt"); }
void cpu_exception_7(void)  { kputs("KERNEL PANIC! Exception #7\n");  while (1) asm volatile("hlt"); }
void cpu_exception_8(void)  { kputs("KERNEL PANIC! Exception #8\n");  while (1) asm volatile("hlt"); }
void cpu_exception_9(void)  { kputs("KERNEL PANIC! Exception #9\n");  while (1) asm volatile("hlt"); }
void cpu_exception_10(void) { kputs("KERNEL PANIC! Exception #10\n"); while (1) asm volatile("hlt"); }
void cpu_exception_11(void) { kputs("KERNEL PANIC! Exception #11\n"); while (1) asm volatile("hlt"); }
void cpu_exception_12(void) { kputs("KERNEL PANIC! Exception #12\n"); while (1) asm volatile("hlt"); }
void cpu_exception_13(void) { kputs("KERNEL PANIC! Exception #13\n"); while (1) asm volatile("hlt"); }

// CPUがエラーコードをスタックに積む、HALTを止める場合はずれないように注意
void cpu_exception_14(void) {
    uint32_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));
    kputs("PAGE FAULT! CR2=");
    kprintf("%x", cr2);
    kputs("\n");
    while(1) asm volatile("hlt");
}

void cpu_exception_15(void) { kputs("KERNEL PANIC! Exception #15\n"); while (1) asm volatile("hlt"); }
void cpu_exception_16(void) { kputs("KERNEL PANIC! Exception #16\n"); while (1) asm volatile("hlt"); }
void cpu_exception_17(void) { kputs("KERNEL PANIC! Exception #17\n"); while (1) asm volatile("hlt"); }
void cpu_exception_18(void) { kputs("KERNEL PANIC! Exception #18\n"); while (1) asm volatile("hlt"); }
void cpu_exception_19(void) { kputs("KERNEL PANIC! Exception #19\n"); while (1) asm volatile("hlt"); }
void cpu_exception_20(void) { kputs("KERNEL PANIC! Exception #20\n"); while (1) asm volatile("hlt"); }
void cpu_exception_21(void) { kputs("KERNEL PANIC! Exception #21\n"); while (1) asm volatile("hlt"); }
void cpu_exception_22(void) { kputs("KERNEL PANIC! Exception #22\n"); while (1) asm volatile("hlt"); }
void cpu_exception_23(void) { kputs("KERNEL PANIC! Exception #23\n"); while (1) asm volatile("hlt"); }
void cpu_exception_24(void) { kputs("KERNEL PANIC! Exception #24\n"); while (1) asm volatile("hlt"); }
void cpu_exception_25(void) { kputs("KERNEL PANIC! Exception #25\n"); while (1) asm volatile("hlt"); }
void cpu_exception_26(void) { kputs("KERNEL PANIC! Exception #26\n"); while (1) asm volatile("hlt"); }
void cpu_exception_27(void) { kputs("KERNEL PANIC! Exception #27\n"); while (1) asm volatile("hlt"); }
void cpu_exception_28(void) { kputs("KERNEL PANIC! Exception #28\n"); while (1) asm volatile("hlt"); }
void cpu_exception_29(void) { kputs("KERNEL PANIC! Exception #29\n"); while (1) asm volatile("hlt"); }
void cpu_exception_30(void) { kputs("KERNEL PANIC! Exception #30\n"); while (1) asm volatile("hlt"); }
void cpu_exception_31(void) { kputs("KERNEL PANIC! Exception #31\n"); while (1) asm volatile("hlt"); }

void exception_init(void) {
    idt_set_gate(0,  (uint32_t)cpu_exception_0);
    idt_set_gate(1,  (uint32_t)cpu_exception_1);
    idt_set_gate(2,  (uint32_t)cpu_exception_2);
    idt_set_gate(3,  (uint32_t)cpu_exception_3);
    idt_set_gate(4,  (uint32_t)cpu_exception_4);
    idt_set_gate(5,  (uint32_t)cpu_exception_5);
    idt_set_gate(6,  (uint32_t)cpu_exception_6);
    idt_set_gate(7,  (uint32_t)cpu_exception_7);
    idt_set_gate(8,  (uint32_t)cpu_exception_8);
    idt_set_gate(9,  (uint32_t)cpu_exception_9);
    idt_set_gate(10, (uint32_t)cpu_exception_10);
    idt_set_gate(11, (uint32_t)cpu_exception_11);
    idt_set_gate(12, (uint32_t)cpu_exception_12);
    idt_set_gate(13, (uint32_t)cpu_exception_13);
    idt_set_gate(14, (uint32_t)cpu_exception_14);
    idt_set_gate(15, (uint32_t)cpu_exception_15);
    idt_set_gate(16, (uint32_t)cpu_exception_16);
    idt_set_gate(17, (uint32_t)cpu_exception_17);
    idt_set_gate(18, (uint32_t)cpu_exception_18);
    idt_set_gate(19, (uint32_t)cpu_exception_19);
    idt_set_gate(20, (uint32_t)cpu_exception_20);
    idt_set_gate(21, (uint32_t)cpu_exception_21);
    idt_set_gate(22, (uint32_t)cpu_exception_22);
    idt_set_gate(23, (uint32_t)cpu_exception_23);
    idt_set_gate(24, (uint32_t)cpu_exception_24);
    idt_set_gate(25, (uint32_t)cpu_exception_25);
    idt_set_gate(26, (uint32_t)cpu_exception_26);
    idt_set_gate(27, (uint32_t)cpu_exception_27);
    idt_set_gate(28, (uint32_t)cpu_exception_28);
    idt_set_gate(29, (uint32_t)cpu_exception_29);
    idt_set_gate(30, (uint32_t)cpu_exception_30);
    idt_set_gate(31, (uint32_t)cpu_exception_31);
}
