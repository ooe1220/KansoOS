#include "debug.h"
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
#include "x86/vga.h"

void test_code(void) {

    set_mode13();
    uint8_t *vram = (uint8_t*)0xA0000;
    for (int y = 0; y < 200; y++)
        for (int x = 0; x < 320; x++)
            vram[y * 320 + x] = 3;   // 赤
    for (;;);

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

