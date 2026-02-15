#include "debug.h"
#include "x86/io.h"
#include "x86/console.h"
#include "drivers/cmos.h"
#include "x86/pic.h"
#include "x86/idt.h"
#include "drivers/ata.h"
#include "drivers/keyboard.h"
#include "x86/panic.h"
#include "x86/syscall.h"
#include "lib/stdint.h"
#include "lib/string.h"
#include "command.h"
//#include "user_exec.h"
#include "mem/memory.h"
#include "drivers/vga.h"

void test_code(void) {


    //xxd -g 2 -s -1024 build/disk.img
    const uint32_t total_sectors = 2048;    // 1MBディスク / 512B
    const uint32_t lba_start = total_sectors - 2; // 最後から2セクタ分
    uint16_t buffer[512];   // 2セクタ分 = 512ワード（1ワード=2B）
    // 書き込みバッファを0x1234で埋める
    for (int i = 0; i < 512; i++) {
        buffer[i] = 0x1234;
    }
    // 書き込み実行
    if (ata_write_lba28(lba_start, 2, buffer) != 0) {
        kprintf("ATA書き込みエラー\n");
        return;
    }

/*
    set_mode13();
    uint8_t *vram = (uint8_t*)0xA0000;
    for (int y = 0; y < 200; y++)
        for (int x = 0; x < 320; x++)
            vram[y * 320 + x] = 4;   // 赤
    for (;;);
*/

/*
    kputs("==== DEBUG TEST START ====\n");

    //************************
    // メモリ管理系関数動作確認
    //************************
    {
        char* buffer = kmalloc(100);
        strcpy(buffer, "Hello World");
        buffer = krealloc(buffer, 200);
        kputs(buffer);
        kputs("\n");
    }

    //************************
    // kprintf 動作確認
    //************************
    {
        kprintf("test1: %c\n", 'A');
        kprintf("test2: %s\n", "hello");
        kprintf("test3: %d\n", 123);
        kprintf("test4: %d\n", -456);
        kprintf("test5: %u\n", 789);
        kprintf("test6: %x\n", 255);
        kprintf("test7: %%\n");
        kprintf("test8: %c %s %d %u %x\n",'X', "test", -100, 200, 0xABCD);
    }
    
    // asm volatile("ud2");  // 割り込み動作確認

    kputs("==== DEBUG TEST END ====\n");
    */
}

