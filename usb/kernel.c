#include "x86/io.h"
#include "x86/console.h"
#include "x86/cmos.h"
#include "x86/pic.h"
#include "x86/idt.h"
#include "x86/ata.h"
#include "x86/keyboard.h"
#include "x86/panic.h"
#include "x86/syscall.h"
#include "lib/stdint.h"
#include "lib/string.h"
#include "../src/kernel/command.h"
#include "../src/kernel/user_exec.h"
#include "mem/memory.h"

void format_date_time(char* buf);

void kernel_main() {

    // 起動時間取得
    kputs("-----------------------------------\n");
    kputs("         C Kernel Booted           \n");
    kputs("         ");
    kputs("\n-----------------------------------\n");
        
    idt_init(); // IDT初期化
    kputs("IDT initialized\n");
    
    pic_init(); // PIC初期化
    kputs("PIC initialized\n");
    
    pic_unmask_irq(1); // PICのIRQ1(キーボード割り込み)を有効化
    kputs("IRQ1 (keyboard) unmasked\n");
    
    keyboard_init(); //  IDT(0x21=33)へIRQ1（キーボード）処理を登録
    kputs("Keyboard interrupt handler registered\n");
    
    exception_init(); // IDT(0〜31=0x00〜0x1F)へCPU例外処理を登録(panic)
    kputs("CPU exception handlers registered\n");
    
    init_syscall(); // IDT 0x80へシステムコールを登録　Linuxの様にint0x80経由でシステムコールを呼び出す
    kputs("System call handler (int 0x80) registered\n");
    
    asm volatile("sti");  // 割り込みを有効にする(PIC初期化しないと割り込みが常時発生)
        
    char line[128]; // コマンド入力バッファ
    int len = 0; // 現在の入力位置（文字数）
    
    for(;;);

}

