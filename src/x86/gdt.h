#ifndef GDT_H
#define GDT_H

#include "lib/stdint.h"

void gdt_init(void);

// セレクタ定数（他ファイルから使いやすく）
#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_CODE   0x18
#define GDT_USER_DATA   0x20
#define GDT_TSS         0x28

#endif
