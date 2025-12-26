// inlineにすることでPUSH、CALL、RET、STACK等操作の無駄を省く
#ifndef IO_H
#define IO_H

#include "stdint.h"

// 8bit I/O
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0"
                      : "=a"(ret)
                      : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1"
                      :
                      : "a"(value), "Nd"(port));
}

// 16bit I/O
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0"
                      : "=a"(ret)
                      : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1"
                      :
                      : "a"(value), "Nd"(port));
}

#endif

