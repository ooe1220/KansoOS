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
#include "command.h"
#include "user_exec.h"
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
    
    init_cursor_from_hardware();
    
    /*
    // 8042初期化（自作BIOS）
// 1. 出力バッファをクリア
while (inb(0x64) & 0x01) inb(0x60);

// 2. キーボードリセット
while (inb(0x64) & 0x02);  // 入力バッファ空きを待つ
outb(0x60, 0xFF);          // キーボードリセット

// 3. ACK + 自己テスト完了読み取り
while (!(inb(0x64) & 0x01)); // 出力バッファに何か来るまで待つ
inb(0x60);  // ACK (0xFA)
while (!(inb(0x64) & 0x01));
inb(0x60);  // Self-test OK (0xAA)

// 4. IRQ有効化
while (inb(0x64) & 0x02);  // 入力バッファ空きを待つ
outb(0x64, 0xAE);          // IRQ1有効化
*/

#include "x86/io.h"
#include "x86/console.h"

void check_keyboard_buffer(void) {
    kputs("Press keys. Output buffer check:\n");

    while (1) {
        uint8_t status = inb(0x64);

        // bit0 = 出力バッファフル
        if (status & 0x01) {
            uint8_t scancode = inb(0x60);
            kputs("Scancode: ");
            char buf[4];
            itoa(scancode, buf, 16); // 16進で表示
            kputs(buf);
            kputs("\n");
        }
    }
}

    
    asm volatile("sti");  // 割り込みを有効にする(PIC初期化しないと割り込みが常時発生)
        
    char line[128]; // コマンド入力バッファ
    int len = 0; // 現在の入力位置（文字数）
    
    //test_code();
    
    kputs("\n>");
    while(1){
        char c = keyboard_getchar(); // キーボード入力を待つ (内部的にはhlt→IRQ1割り込み)

        if (c == '\n') { // ENTER : 命令実行及び改行
            line[len] = 0;
            if(do_builtin(line) != 0){ // 内部コマンド実行
                run_file(line); // 内部コマンドと一致しない場合、実行ファイルとして実行を試みる
            }
            len = 0;
            kputs("\n>");
        } else if (c == '\b') { // SPACE : 一文字削除
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

