#include "x86/io.h"
#include "x86/console.h"
#include "drivers/cmos.h"
#include "x86/pic.h"
#include "x86/idt.h"
#include "drivers/keyboard.h"
#include "x86/panic.h"
#include "x86/syscall.h"
#include "drivers/vga.h"
#include "lib/stdint.h"
#include "lib/string.h"
#include "command.h"
#include "mem/memory.h"
#include "debug.h"

void format_date_time(char* buf);
void irq0_handler(void);
void timer_tick(void);

void kernel_main() {

    // 起動時間取得
    char boot_time[20];
    format_date_time(boot_time);
    
    kputs("-----------------------------------\n");
    kputs("         C Kernel Booted           \n");
    kputs("         ");
    kputs(boot_time);
    kputs("\n-----------------------------------\n");
        
    idt_init(); // IDT初期化 (x86/idt.h)
    kputs("IDT initialized\n");
    
    pic_init(); // PIC初期化 (x86/pic.h)
    kputs("PIC initialized\n");
    
    pic_unmask_irq(1); // PICのIRQ1(キーボード割り込み)を有効化 (x86/pic.h)
    kputs("IRQ1 (keyboard) unmasked\n");
    
    keyboard_init(); //  IDT(0x21=33)へIRQ1（キーボード）処理を登録 (drivers/keyboard.h)
    kputs("Keyboard interrupt handler registered\n");
    
    exception_init(); // IDT(0〜31=0x00〜0x1F)へCPU例外処理を登録 (x86/panic.h)
    kputs("CPU exception handlers registered\n");
    
    init_syscall(); // IDT 0x80へシステムコールを登録　Linuxの様にint0x80経由でシステムコールを呼び出す (x86/syscall.h)
    kputs("System call handler (int 0x80) registered\n");
    
    init_cursor_from_hardware();
    
    asm volatile("sti");  // 割り込みを有効にする(PIC初期化しないと割り込みが常時発生)
        
    char line[128]; // コマンド入力バッファ
    int len = 0; // 現在の入力位置（文字数）
    
    //test_code();
       
    kputs("\n>");
    while(1){
        char c = keyboard_getchar(); // キーボード入力を待つ (内部的にはhlt→IRQ1割り込み) (drivers/keyboard.h)
        if (c == '\n') { // ENTER : 命令実行及び改行
            line[len] = 0;
            if(run_builtin_command(line) != 0){ // 内部コマンド実行 (kernel/command.h)
                //asm volatile("cli");
                //asm volatile("hlt"); // EBP=0009fbf8 ESP=0009fb50  0009fb50: 0x00000000 0x74000000 0x33747365 0x00000000
                run_file(line); // 内部コマンドと一致しない場合、実行ファイルとして実行を試みる (kernel/command.h)
                //asm volatile("cli");
                //asm volatile("hlt");//EBP=0009fb48 ESP=0009fb34 0009fb34: 0x00009f06 0x0009fbf8 0x000081b5 0x0000ad95
            }
            len = 0;
            kputs("\n>");
        } else if (c == '\b') { // BACKSPACE : 一文字削除
            if (len > 0) {
                len--;
                kputc('\b'); kputc(' '); kputc('\b');
            }
        } else { // 入力された文字のASCIIコードをバッファに入れ、画面上に表示
            if (len < sizeof(line)-1) {
                line[len++] = c;
                kputc(c);
            }
        }
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

