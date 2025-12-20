#include "arch/x86/io.h"
#include "arch/x86/console.h"
#include "arch/x86/cmos.h"
#include "arch/x86/pic.h"
#include "arch/x86/idt.h"
#include "arch/x86/ata.h"
#include "lib/stdint.h"
#include "lib/string.h"

void format_date_time(char* buf);

void kernel_main() {

    // 起動時間取得
    char boot_time[20];
    format_date_time(boot_time);
    
    kputs("-----------------------------------\n");
    kputs("         C Kernel Booted           \n");
    kputs("         ");
    kputs(boot_time);
    kputs("\n-----------------------------------\n");
        
    idt_init(); // IDT初期化
    kputs("IDT initialized\n");
    pic_init(); // PIC初期化
    kputs("PIC initialized\n");
    
    pic_unmask_irq(1); // キーボード IRQ1初期化
    
    asm volatile("sti");  // 割り込みを有効にする(PIC初期化しないと割り込みが常時発生)
    
    // asm volatile("ud2");  // 割り込み動作確認
    // ata_read_lba28(0, 1, (void*)0x10000); // ATAドライバ動作確認
    
    while(1){
        asm volatile("hlt");  // 割り込みが来るまでCPU停止
    }

}

// RTC データを CMOS から読み込んで文字列に変換
void format_date_time(char* buf) {
    struct RTC rtc;

    // CMOS から日時を取得
    cmos_read_rtc(&rtc);

    // "YYYY/MM/DD HH:MM:SS" 形式に整形
    buf[0]  = '0' + (rtc.century / 10);
    buf[1]  = '0' + (rtc.century % 10);
    buf[2]  = '0' + (rtc.year / 10);
    buf[3]  = '0' + (rtc.year % 10);
    buf[4]  = '/';
    buf[5]  = '0' + (rtc.month / 10);
    buf[6]  = '0' + (rtc.month % 10);
    buf[7]  = '/';
    buf[8]  = '0' + (rtc.day / 10);
    buf[9]  = '0' + (rtc.day % 10);
    buf[10] = ' ';
    buf[11] = '0' + (rtc.hour / 10);
    buf[12] = '0' + (rtc.hour % 10);
    buf[13] = ':';
    buf[14] = '0' + (rtc.minute / 10);
    buf[15] = '0' + (rtc.minute % 10);
    buf[16] = ':';
    buf[17] = '0' + (rtc.second / 10);
    buf[18] = '0' + (rtc.second % 10);
    buf[19] = '\0';
}

