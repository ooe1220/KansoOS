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
    
    char title[128];
    strcpy(title, "-----------------------------------\n");
    strcat(title, "         C Kernel Booted           \n");
    strcat(title, "         ");
    strcat(title, boot_time);
    strcat(title, "\n-----------------------------------\n");
    puts(title);

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
