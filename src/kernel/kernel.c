#include "x86/pic.h"
#include "x86/idt.h"
#include "x86/gdt.h"
#include "x86/vbe.h"
#include "x86/io.h"
#include "x86/console.h"
#include "x86/vbe.h"
#include "x86/timer.h"
#include "x86/panic.h"
#include "x86/syscall.h"
#include "x86/paging.h"
#include "drivers/keyboard.h"
#include "drivers/cmos.h"
#include "lib/stdint.h"
#include "lib/string.h"
#include "lib/bmp.h"
#include "command.h"
#include "mem.h"
#include "x86/cpu.h"
#include "user_exec.h"
#include "debug/serial.h"
#include "debug/debug_print.h"
#include "debug/detect.h"
#include "drivers/atapi.h"

void kernel_main() { 

    gdt_init();
    tss_init(0x9FC00); // swich32.asmと2重定義になるのでいつか纏める

    // 起動時間取得
    char boot_time[20];
    format_date_time(boot_time);
    
    // CPU名取得
    char cpuname[48];
    get_cpu_name(cpuname);
        
    // ページングを入れるとoutb(0x64, 0xFC)に失敗する
    // 実機ではまともに動かない
    //enable_paging(); // 物理=仮想
    //console_clear(); // ページングの際にVRAMにゴミが残る為、消す(原因不明)
    
    console_init();      // VBE 初期化 + 画面クリア
            
    // 起動画面ペンギン
    show_boot_logo();
    for (uint32_t i = 0; i < 5000 * 100000; i++) {
        asm volatile ("nop");
    }
    
    console_clear();
    
    kputs("       C Kernel Booted           \n");
    kputs("       "); kputs(cpuname); kputs("\n");
    kputs("       "); kputs(boot_time); kputs(" (UTC)\n");

    kputs("GDT initialized\n");
    kputs("TSS initialized\n");
    kputs("VBE initialized\n");
    
    show_penguin();// shell画面左上にペンギン画像を表示
    
    cpu_cache_disable();// キャッシュを切っておく
    kputs("CPU cache disabled\n");
    
    // 光学ドライブ検出
    struct atapi_dev dev = atapi_detect();
    if (dev.present) {
        kputs("atapi: optical drive detected: ");
        kputs(dev.model);
        kputs("\n");
    } else {
        kputs("atapi: no optical drive found\n");
    }
    
    // 1セクタ読み出す
    uint8_t sector[2048];
    atapi_read_sector(35, sector);
    //debug_dump_mem((uint32_t)sector, 2048);
    atapi_read_sector(16, sector);
    /* type と CD001  */
    debug_printf("[PVD] type=%x magic=%c%c%c%c%c",
             sector[0],
             sector[1], sector[2], sector[3], sector[4], sector[5]);

    /* ボリュームラベル（32バイト、印字可能文字だけ表示） */
    debug_printf("[PVD] magic=%c%c%c%c%c <<<",
             sector[1], sector[2], sector[3], sector[4], sector[5]);

    /* 論理セクタサイズ（リトルエンディアン） */
    debug_printf("[PVD] block_size=%u",
             (unsigned)(sector[0x9C]
                     | (sector[0x9D] << 8)
                     | (sector[0x9E] << 16)
                     | (sector[0x9F] << 24)));
    
     // メモリ管理初期化　QEMUは通るが実機では固まる
    //kheap_init(); // カーネル用
    //uheap_init(); // ユーザ用
    //kputs("Heap initialized\n");
        
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
    
    //タイマーを入れるとスタックが壊れる。一旦保留
    //timer_init();
      
    char line[128]; // コマンド入力バッファ
    int len = 0; // 現在の入力位置（文字数）
    
    asm volatile("sti");  // 割り込みを有効にする(PIC初期化しないと割り込みが常時発生)
    
    while (keyboard_available())keyboard_getchar(); // 実機起動時ゴミが表示される為、完全に捨てる
           
    kputs("\n>");
    while(1){
        // キーボード入力を待つ (内部的にはhlt→IRQ1割り込み) (drivers/keyboard.h)
        char c = keyboard_getchar(); 
        
        if (c == '\n') { // ENTER : 命令実行及び改行
            line[len] = 0; // バッファを空に(実際には消さず末尾文字を先頭に付加)
            
            int buildin_res = run_builtin_command(line); // 内部コマンド
            int runfile_res = 0;
            if(buildin_res!=0){
                runfile_res = run_file(line); // 実行ファイル実行
            }
            if(buildin_res==-1 && runfile_res==-1){ // 内部コマンドでも実行ファイルでもない
                kputs("Unknown command or file not found: ");
                kputs(line);
            }
            
            len = 0; // 次の命令入力の為に入力バッファを空にする
            kputs("\n>");
        } else if (c == '\b') { // BACKSPACE : 一文字削除
            if (len > 0) {
                // BACKSPACE処理
                // 1. バッファから1文字削除
                // 2. カーソルを左へ
                // 3. その位置を空白で上書き（文字を消す）
                // 4. 空白表示時にカーソルが右へ動く為カーソルを再び左へ
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



