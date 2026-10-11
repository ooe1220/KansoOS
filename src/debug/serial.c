#include "debug/serial.h"
#include "x86/io.h"

#define COM1 0x3F8

static int serial_transmit_empty(void) {
    return (inb(COM1 + 5) & 0x20) != 0;
}

void serial_init(void) {
    outb(COM1 + 1, 0x00);  // 割り込み無効
    outb(COM1 + 3, 0x80);  // DLAB オン
    outb(COM1 + 0, 0x03);  // 除数低位: 3 (38400 baud)
    outb(COM1 + 1, 0x00);  // 除数高位: 0
    outb(COM1 + 3, 0x03);  // 8bit, no parity, 1 stop bit
    outb(COM1 + 2, 0xC7);  // FIFO 有効, 14byte しきい値
    outb(COM1 + 4, 0x0B);  // DTR + RTS + OUT2
    outb(COM1 + 1, 0x00);  // 念のため再び割り込み無効
}

void serial_putc(char c) {
    while (!serial_transmit_empty());
    outb(COM1, c);
}

void serial_puts(const char *s) {
    while (*s) {
        if (*s == '\n') {
            serial_putc('\r');
        }
        serial_putc(*s++);
    }
}
