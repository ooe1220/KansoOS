// syscall.c
#include "console.h"
#include "idt.h"
#include "lib/stdint.h"

void syscall_handler(void);

void init_syscall(void) {
    idt_set_gate(0x80, (uint32_t)syscall_handler);
}

// 実際のwrite処理
uint32_t handle_write(const char *str) {

    /* ==== 確認用：VRAMに 'C' を出す ==== */
    volatile unsigned char* vram = (unsigned char*)0xB8000;
    vram[4] = 'C';      // 文字
    vram[5] = 0x0F;     // 白文字・黒背景
    /* ================================== */
    if (str) {
        //kputs("handle_write was called");
        kputs(str);  // カーネルの画面出力関数
    }
    return 0;
}
