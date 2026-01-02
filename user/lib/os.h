#pragma once

static inline void write(const char *str) {
    asm volatile(
        "mov $1, %%eax\n"
        "int $0x80\n"
        :
        : "b"(str)      // ★ EBX 固定
        : "eax"
    );
}

