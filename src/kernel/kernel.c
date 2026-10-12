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
#include "x86/cpuid.h"
#include "user_exec.h"
#include "debug/serial.h"
#include "debug/serial_print.h"

void kernel_main() {

    gdt_init();
    tss_init(0x9FC00); // swich32.asmと2重定義になるのでいつか纏める

    // 起動時間取得
    char boot_time[20];
    format_date_time(boot_time);
    
    // CPU名取得
    char cpuname[48];
    get_cpu_name(cpuname);
    
    // ページング有効にすると、"reboot"命令のみ失敗する不可解な不具合が発生
    // strcmp(line, "reboot") は問題ない
    // outをコメントアウトすると>へ戻れるがstrcmp直後のkputsを2つ置くと2つめしか表示されない
    // kputsが無くてもoutで固まる。
    // reboot自体は仮想=物理となっている　outを別の場所へ置くと再起動可能
    // 他の部分のコードを色々整理してもこのエラーは健在、偶然ではない
    // 解決するまでコメントアウト
    // ページング有効(複数プロセスをする時の為に仕組みのみ入れておく)
    //enable_paging();
    //console_clear(); // ページングの際にVRAMにゴミが残る為、消す(原因不明)
    //serial_print("pd at 0x%x\n", (uint32_t)kernel_page_dir);
    //serial_print("pt at 0x%x\n", (uint32_t)kernel_page_tables);
    //serial_print("pde[0] = 0x%x\n", kernel_page_dir[0]);
    //serial_print("pde[1] = 0x%x\n", kernel_page_dir[1]);
    //enable_paging();
    
    console_init();      // VBE 初期化 + 画面クリア
    
    kputs("       C Kernel Booted           \n");
    kputs("       "); kputs(cpuname); kputs("\n");
    kputs("       "); kputs(boot_time); kputs(" (UTC)\n");

    kputs("GDT initialized\n");
    kputs("TSS initialized\n");
    kputs("VBE initialized\n");
    
    // デバッグ用COM1有効
    serial_init();
    kputs("Serial initialized (COM1)\n");
    serial_puts("\nSerial initialized (COM1)\n");
    
    // テスト
    //serial_print("test: argc=%d str=%s hex=0x%x\n", 3, "hello", 0xDEAD);
    //debug_dump_regs();
    //debug_dump_mem(0x0600, 512); // MBR
    //debug_dump_mem(0x7C00, 512); // VBR
    // dd if=build/disk.img bs=512 skip=63 count=1 2>/dev/null | xxd | tail -4でVBRには0xBB66があるのにメモリ上には無い
    //uint32_t esp;
    //asm volatile ("mov %%esp, %0" : "=r"(esp));
    //debug_dump_stack(esp, 32, 32);
    
     // メモリ管理初期化
    kheap_init(); // カーネル用
    uheap_init(); // ユーザ用
    kputs("Heap initialized\n");
        
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
    
    show_penguin();
    
    asm volatile("sti");  // 割り込みを有効にする(PIC初期化しないと割り込みが常時発生)
        
    char line[128]; // コマンド入力バッファ
    int len = 0; // 現在の入力位置（文字数）
           
    kputs("\n>");
    while(1){
    
        // キーボード入力を待つ (内部的にはhlt→IRQ1割り込み) (drivers/keyboard.h)
        char c = keyboard_getchar(); 
        
        if (c == '\n') { // ENTER : 命令実行及び改行
            line[len] = 0; // バッファを空に(実際には消さず末尾文字を先頭に付加)
            
            int buildin_res = run_builtin_command(line); // 内部コマンド
            int runfile_res = run_file(line); // 実行ファイル実行
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



