#include "io.h"

#define PIT_CMD  0x43
#define PIT_CH0  0x40

void pit_init(uint32_t freq) {
    uint32_t divisor = 1193180 / freq;
    outb(PIT_CMD, 0x36);                  // チャネル0, 方式3, バイナリ
    outb(PIT_CH0, divisor & 0xFF);        // LSB
    outb(PIT_CH0, (divisor >> 8) & 0xFF); // MSB
}
