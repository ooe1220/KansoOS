// syscall.c
#include "console.h"
#include "idt.h"
#include "lib/stdint.h"

#define SYS_WRITE 1

void syscall_handler(void);

void init_syscall(void) {
    idt_set_gate(0x80, (uint32_t)syscall_handler);
}

// システムコールハンドラ
__attribute__((naked))
void syscall_handler(void) {
    asm volatile(
        "pusha\n"               // レジスタ保存
        
        /* ==== 確認用：VRAMに 'B' を出す ==== */
        "mov $0xB8002, %%edi\n"
        "movb $'B', (%%edi)\n"
        "movb $0x0F, 1(%%edi)\n"
        /* ================================== */
        
        //"cmp $1, %%eax\n"       // syscall番号チェック
        //"jne .syscall_done\n"   // 1でなければ何もしない
        
        "push %%ebx\n"          // 引数（文字列ポインタ）
        "call handle_write\n"
        "add $4, %%esp\n"       // スタック調整
        
        ".syscall_done:\n"
        "popa\n"                // レジスタ復元
        "iret\n"
        :
        :
        : "memory"
    );
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
