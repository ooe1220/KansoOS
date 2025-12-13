#include "arch/x86/io.h"    // inb/outb
#include "arch/x86/console.h"
#include "arch/x86/rtc.h"
#include "arch/x86/init.h"
#include "lib/stdint.h"

void kernel_main() {

    // 起動時間取得
    char buf[20];
    format_date_time(buf);

    puts("-----------------------------------\n");
    puts("         C Kernel Booted           \n");
    puts("         ");
    puts(buf);
    puts("\n-----------------------------------\n");
    
    arch_init();
    
    while(1){
        asm volatile("sti");  // 割り込みを有効にする(PIC初期化しないと割り込みが常時発生)
        asm volatile("hlt");  // 割り込みが来るまでCPU停止
    }

}
