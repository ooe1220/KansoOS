#include "arch/x86/io.h"    // inb/outb
#include "arch/x86/console.h"
#include "arch/x86/rtc.h"
#include "lib/stdint.h"
#include "arch/x86/pic.h"
#include "arch/x86/a20.h"

void kernel_main() {

    // 起動時間取得
    char buf[20];
    format_date_time(buf);
    
    //char title[100];

    puts("-----------------------------------\n");
    puts("         C Kernel Booted           \n");
    puts("         ");
    puts(buf);
    puts("\n-----------------------------------\n");
    

    enable_a20();
    puts("A20 initialized\n");
    pic_init();
    pic_set_mask(0xFFFD);
    puts("Interrupt initialized\n");
    
    while(1){
        asm volatile("sti");  // 割り込みを有効にする(PIC初期化しないと割り込みが常時発生)
        asm volatile("hlt");  // 割り込みが来るまでCPU停止
    }

}
