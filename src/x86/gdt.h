#ifndef GDT_H
#define GDT_H

#include "lib/stdint.h"

void gdt_init(void);
void tss_init(uint32_t kernel_stack);

// セレクタ定数（他ファイルから使いやすく）
#define GDT_KERNEL_CODE 0x08   // index=1, RPL=0
#define GDT_KERNEL_DATA 0x10   // index=2, RPL=0
#define GDT_USER_CODE   0x1B   // Index=3, RPL=3
#define GDT_USER_DATA   0x23   // Index=4, RPL=3
#define GDT_TSS         0x28   // Index=5, RPL=0

struct tss_entry {
    uint32_t prev_tss;   // 前のTSS（使わない）
    uint32_t esp0;       // Ring 0 用スタック
    uint32_t ss0;        // Ring 0 用スタックセグメント
    uint32_t esp1;       // Ring 1（使わない）
    uint32_t ss1;
    uint32_t esp2;       // Ring 2（使わない）
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

extern struct tss_entry tss;

#endif
