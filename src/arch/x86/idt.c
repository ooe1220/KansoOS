#include "arch/x86/idt.h"
#include "arch/x86/console.h"

/*
 IDT 1個分の構造体
*/
struct idt_entry {
    uint16_t offset_low;;  // ISR アドレス下位 16bit
    uint16_t selector;     // コードセグメント
    uint8_t  zero;         // 常に 0
    uint8_t  type_attr;
    uint16_t offset_high;  // ISR アドレス上位 16bit
} __attribute__((packed));

/*
 * IDTR に渡す構造体
 * lidt 命令は
 *   limit : IDT 全体の大きさ - 1
 *   base  : IDT 配列の先頭アドレス
 *  が必要
 */
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));


/*
 * IDT 本体
 * 256 個分（CPU 仕様）
 *
 * 0〜31   : CPU 例外
 * 32〜47  : IRQ (PIC リマップ後)
 * 48〜255 : 
 */
static struct idt_entry idt[256];

/*
 * IDTに1個分処理を設定する関数
 *
 * n       : IDT 番号（例外番号 / IRQ 番号）
 * handler : ISR のアドレス
 */
void idt_set_gate(int n, uint32_t handler) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = 0x08;
    idt[n].zero        = 0;
    idt[n].type_attr   = 0x8E;
    idt[n].offset_high = handler >> 16;
}

/*
 * IDTの空表をCPUへ登録
 */
void idt_init(void) {
    struct idt_ptr idtp;

    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint32_t)&idt;

    asm volatile("lidt (%0)" :: "r"(&idtp));
}

