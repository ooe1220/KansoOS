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
        
        "cmp $1, %%eax\n"       // syscall番号チェック
        "jne .syscall_done\n"   // 1でなければ何もしない
        
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
    if (str) {
        //kputs("handle_write was called");
        kputs(str);  // カーネルの画面出力関数
    }
    return 0;
}
