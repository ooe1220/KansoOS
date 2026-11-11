#include "x86/pic.h"
#include "x86/idt.h"
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
#include "fs/fs_mount.h" 
#include "fs/iso9660.h" // OPS作ったら消す 
#include "drivers/bootdev.h"
#include "kernel/shell.h"

void kernel_main() { 

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
    
    kputs("       KansoOS           \n");
    kputs("       "); kputs(cpuname); kputs("\n");
    kputs("       "); kputs(boot_time); kputs(" (UTC)\n");

    kputs("VBE initialized\n");
    
    show_penguin();// shell画面左上にペンギン画像を表示
    
    cpu_cache_disable();// キャッシュを切っておく
    kputs("CPU cache disabled\n");

    fs_mount_root(); //起動ディスクに応じてATA+FAT16かATAPI+ISO9660かを決める（安定したら固定をやめる）

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

   // 本来はmount_fsの中でやるべきだが、実機の場合ここでやらないと何故か失敗する
    struct atapi_dev dev = atapi_detect();
        if (dev.present) {
            kputs("atapi: optical drive detected: ");
            kputs(dev.model);
            kputs("\n");
        } else {
            kputs("atapi: no optical drive found\n");
    } 
      
    char line[128]; // コマンド入力バッファ
    int len = 0; // 現在の入力位置（文字数）
    
    asm volatile("sti");  // 割り込みを有効にする(PIC初期化しないと割り込みが常時発生)
    
    shell_run(); // 今後独立プロセスへする予定
}

