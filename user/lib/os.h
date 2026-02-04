#pragma once
#include "stdint.h"

static inline void write(const char *str) {
    asm volatile(
        "mov $1, %%eax\n"
        "int $0x80\n"
        :
        : "b"(str)      // ★ EBX 固定
        : "eax"
    );
}

static inline int read_sector(uint32_t lba, void* buffer) {
    int ret;
    asm volatile(
        "mov $2, %%eax\n"   // syscall番号 2
        "int $0x80\n"
        : "=a"(ret)         // EAX に返り値を入れる
        : "b"(lba), "c"(buffer) // EBX = lba, ECX = buffer
    );
    return ret;
}

