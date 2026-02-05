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


/* =======================
   open
   syscall番号 = 3
   EBX = filename
   返り値 = fd
   ======================= */
static inline int open(const char* filename) {
    int ret;
    asm volatile(
        "mov $3, %%eax\n"
        "int $0x80\n"
        : "=a"(ret)
        : "b"(filename)
    );

    return ret;
}

/* =======================
   read
   syscall番号 = 4
   EBX = fd
   ECX = buf
   EDX = size
   返り値 = 読み込んだ大きさ
   ======================= */
static inline int read(int fd, void* buf, int size) {
    int ret;
    asm volatile(
        "mov $4, %%eax\n"
        "int $0x80\n"
        : "=a"(ret)
        : "b"(fd), "c"(buf), "d"(size)
    );
    return ret;
}

/* =======================
   close
   syscall番号 = 5
   EBX = fd
   返り値 = 成功0 / 失敗-1
   ======================= */
static inline int close(int fd) {
    int ret;
    asm volatile(
        "mov $5, %%eax\n"
        "int $0x80\n"
        : "=a"(ret)
        : "b"(fd)
    );
    return ret;
}
