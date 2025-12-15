#include "arch/x86/io.h"    // inb/outb
#include "arch/x86/console.h"
#include "arch/x86/rtc.h"
#include "arch/x86/pic.h"
#include "arch/x86/a20.h"
#include "lib/stdint.h"
#include "lib/string.h"

void kernel_main() {

    // 起動時間取得
    char boot_time[20];
    format_date_time(boot_time);
    
    kputs("-----------------------------------\n");
    kputs("         C Kernel Booted           \n");
    kputs("         ");
    kputs(boot_time);
    kputs("\n-----------------------------------\n");

    enable_a20();
    kputs("A20 initialized\n");
    pic_init();
    pic_set_mask(0xFFFD);
    kputs("Interrupt initialized\n");
    
    
    asm volatile("sti");  // 割り込みを有効にする(PIC初期化しないと割り込みが常時発生)
    while(1){
        asm volatile("hlt");  // 割り込みが来るまでCPU停止
    }

}
