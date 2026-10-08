// gdt.c
#include "gdt.h"

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// 6個: NULL + カーネルCS/DS + ユーザCS/DS + TSS(予約)
static struct gdt_entry gdt[6];
static struct gdt_ptr   gdtp;

static void gdt_set_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = base & 0xFFFF;
    gdt[num].base_mid    = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    gdt[num].limit_low   = limit & 0xFFFF;
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[num].access      = access;
}

void gdt_init(void) {
    gdtp.limit = sizeof(gdt) - 1;
    gdtp.base  = (uint32_t)&gdt;

    // 0: NULL
    gdt_set_entry(0, 0, 0, 0, 0);

    // 1: カーネルコード  DPL=0
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // 2: カーネルデータ  DPL=0
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // 3: ユーザコード    DPL=3
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    // 4: ユーザデータ    DPL=3
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    // 5: TSS (予約、後で使う)
    gdt_set_entry(5, 0, 0, 0, 0);

    // CPU に登録
    asm volatile("lgdt (%0)" :: "r"(&gdtp));

    // セグメントレジスタ再ロード
    asm volatile (
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "ljmp $0x08, $1f\n"
        "1:\n"
        :
        :
        : "eax"
    );
}
