#include "gdt.h"
#include "console.h"
#include <string.h>

/*
 * GDT 1個分の構造体
 * 32bit プロテクションモード用
 */
struct gdt_entry {
    uint16_t limit_low;      // セグメント限界 (下位 16bit)
    uint16_t base_low;       // ベースアドレス (下位 16bit)
    uint8_t  base_middle;    // ベースアドレス (中位 8bit)
    uint8_t  access;         // アクセス権
    uint8_t  granularity;    // 上限 + フラグ
    uint8_t  base_high;      // ベースアドレス (上位 8bit)
} __attribute__((packed));

/*
 * GDTR に渡す構造体
 * lgdt 命令は
 *   limit : GDT 全体の大きさ - 1
 *   base  : GDT 配列の先頭アドレス
 *  が必要
 */
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/*
 * GDT 本体
 *
 * 0 : NULL ディスクリプタ（必須）
 * 1 : カーネル用コードセグメント (Ring 0)
 * 2 : カーネル用データセグメント (Ring 0)
 * 3 : ユーザ用コードセグメント   (Ring 3)
 * 4 : ユーザ用データセグメント   (Ring 3)
 * 5 : TSS（タスク切り替え用、今は予約だけでもOK）
 */
static struct gdt_entry gdt[6];

struct tss_entry tss;

/*
 * GDT に 1 個分セグメントを設定する関数
 *
 * num        : GDT 番号
 * base       : ベースアドレス
 * limit      : セグメント限界
 * access     : アクセス権
 * gran       : グラニュラリティ + フラグ
 */
static void gdt_set_gate(int num, uint32_t base, uint32_t limit,
                         uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= (gran & 0xF0);

    gdt[num].access = access;
}

/*
 * GDT の空表を CPU へ登録
 */
void gdt_init(void) {
    struct gdt_ptr gdtp;

    // NULL ディスクリプタ
    gdt_set_gate(0, 0, 0, 0, 0);

    // カーネル用 (Ring 0)
    // base=0, limit=4GB, コード/データ
    gdt_set_gate(1, 0, 0xFFFFFFFF,
                 0x9A, 0xCF);  // コード: P=1, DPL=0, S=1, Type=1010
    gdt_set_gate(2, 0, 0xFFFFFFFF,
                 0x92, 0xCF);  // データ: P=1, DPL=0, S=1, Type=0010

    // ユーザ用 (Ring 3)
    gdt_set_gate(3, 0, 0xFFFFFFFF,
                 0xFA, 0xCF);  // コード: P=1, DPL=3, S=1, Type=1010
    gdt_set_gate(4, 0, 0xFFFFFFFF,
                 0xF2, 0xCF);  // データ: P=1, DPL=3, S=1, Type=0010

    // TSS 用（今はダミー、タスク切り替え実装時に使う）
    gdt_set_gate(5, 0, 0, 0, 0);

    gdtp.limit = sizeof(gdt) - 1;
    gdtp.base  = (uint32_t)&gdt;

    asm volatile("lgdt (%0)" :: "r"(&gdtp));

    /*
     * セグメントレジスタを再ロード
     * CS は far jmp でしか変更できない
     */
    asm volatile(
        "ljmp $0x08, $reload_cs\n"
        "reload_cs:\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        ::: "eax"
    );
}

void tss_init(uint32_t kernel_stack) {
    // TSS 用のメモリをゼロクリア
    memset(&tss, 0, sizeof(tss));
    
    // Ring 0 用スタック（カーネルスタック）
    tss.esp0 = kernel_stack;
    tss.ss0  = 0x10;  // GDT_KERNEL_DATA
    
    // TSS を GDT の 5番目に設定(ユーザー（Ring 3）からカーネル（Ring 0）に戻る時、どのスタックを使うかを CPU に教えておく)
    // base = &tss, limit = sizeof(tss)-1
    gdt_set_gate(5,
                 (uint32_t)&tss,
                 sizeof(tss) - 1,
                 0x89,    // P=1, DPL=0, Type=1001 (32-bit TSS, not busy)
                 0x00);   // granularity は 0
    
    // TR レジスタにロード
    asm volatile("ltr %%ax" :: "a"(GDT_TSS));
}

