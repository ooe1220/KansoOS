// gdt.h
#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// セレクタ値
#define GDT_KERNEL_CS  0x08
#define GDT_KERNEL_DS  0x10
#define GDT_USER_CS    0x1B   // (3 << 3) | 3
#define GDT_USER_DS    0x23   // (4 << 3) | 3
#define GDT_TSS        0x28   // (5 << 3)  ← 将来用

void gdt_init(void);

#endif
