#include "arch/x86/io.h"
#include "arch/x86/console.h"
#include "arch/x86/cmos.h"
#include "arch/x86/pic.h"
#include "arch/x86/idt.h"
#include "arch/x86/ata.h"
#include "arch/x86/keyboard.h"
#include "arch/x86/panic.h"
#include "lib/stdint.h"
#include "lib/string.h"
#include "command.h"

void format_date_time(char* buf);
static inline void jump_to_user(void* entry);

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
    
    keyboard_init(); //  IDT(0x21=33)へIRQ1（キーボード）処理を登録
    exception_init();// IDT(0〜31=0x00〜0x1F)へCPU例外処理を登録(panic)
    
    asm volatile("sti");  // 割り込みを有効にする(PIC初期化しないと割り込みが常時発生)
    
    // asm volatile("ud2");  // 割り込み動作確認
    
    /* ************************************************************** */
    // test.binを固定の位置から読み込み実行する
    // 今後はXXX.binの名前を自分で入力して実行するようにする予定 
    // disk.img LBA1800から10セクタ分読み込みメモリ0x10000上へ展開する
    
    ata_read_lba28(1800, 10, (void*)0x10000);
    jump_to_user((void*)0x10000);
    
    /* ************************************************************** */
    
    
    char line[128]; // コマンド入力バッファ
    int len = 0; // 現在の入力位置（文字数）
    
    kputs("\n>");
    while(1){
        char c = keyboard_getchar(); // キーボード

        if (c == '\n') {
            line[len] = 0;
            execute_command(line);
            len = 0;
            kputs("\n>");
        } else if (c == '\b') {
            if (len > 0) {
                len--;
                kputc('\b'); kputc(' '); kputc('\b');
            }
        } else {
            if (len < sizeof(line)-1) {
                line[len++] = c;
                kputc(c);
            }
        }
    }

}

static inline void jump_to_user(void* entry)
{
    asm volatile (
        "pushf\n"        /* EFLAGS 保存 */
        "call *%0\n"     /* ユーザコード実行 */
        "popf\n"         /* EFLAGS 復元 */
        :
        : "r"(entry)
        : "memory", "cc"
    );
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

