#include "x86/idt.h"
#include "x86/pic.h"
#include "x86/io.h"
#include "x86/console.h"
#include "lib/stdint.h"

extern void irq1(void);

/* ------------------------------
 * キーマップ（最小）
 * ------------------------------ */
static const char keymap[128] = {
    0,  27,'1','2','3','4','5','6','7','8','9','0','-','=',
    '\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',
};

/* ------------------------------
 * 入力バッファ
 * ------------------------------ */
#define KBD_BUF_SIZE 128

static char buf[KBD_BUF_SIZE];
static volatile int head = 0;
static volatile int tail = 0;

static void buf_push(char c) {
    int next = (head + 1) % KBD_BUF_SIZE;
    if (next != tail) {   // 満杯でなければ
        buf[head] = c;
        head = next;
    }
}

int keyboard_available(void) {
    return head != tail;
}

char keyboard_getchar(void) {
    while (!keyboard_available())
        asm volatile("hlt");   // 静かに待つ（ポーリングしない）

    char c = buf[tail];
    tail = (tail + 1) % KBD_BUF_SIZE;
    return c;
}

/* ------------------------------
 * IRQ1 Cハンドラ
 * ------------------------------ */
void keyboard_handler(void) {
    uint8_t sc = inb(0x60);
    
    //kputs("yobareta"); // 割り込みが呼ばれたかの確認一回押すと2回表示される

    /* キーリリースは無視 */
    if (sc & 0x80) {
        pic_send_eoi(1);
        return;
    }

    char c = keymap[sc];
    if (c) {
        buf_push(c);
    }

    pic_send_eoi(1);
}

/* ------------------------------
 * IRQ1 Cハンドラ呼び出し
 * ------------------------------ */
__attribute__((naked))
void irq1(void) {
    __asm__ volatile (
        "pusha\n"
        "call keyboard_handler\n"
        "popa\n"
        "iret\n"
    );
}


/* ------------------------------
 * 初期化
 * ------------------------------ */
void keyboard_init(void) {
    idt_set_gate(0x21, (uint32_t)irq1); // IRQ1
}

