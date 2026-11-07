// 今は静的リンク
#pragma once
#include "../../src/lib/stdint.h"
#include "../../src/x86/syscall_num.h" // 後から-I使う

static inline void write(const char *str) {
    // 現在は画面表示専用、ディスク書き込み実装時にfdを足す
    asm volatile(
        "mov %[num], %%eax\n"
        "int $0x80\n"
        :
        : [num] "i"(SYS_WRITE), "b"(str)   // ★ EBX 固定はそのまま
        : "eax", "memory"
    );
}

static inline int read_sector(uint32_t lba, void* buffer) {
    int ret;
    asm volatile(
        "mov %[num], %%eax\n" 
        "int $0x80\n"
        : "=a"(ret)            // EAX に返り値を入れる
        : [num] "i"(SYS_READ_SECT), "b"(lba), "c"(buffer)
    );
    return ret;
}

/* =======================
   open
   EBX = filename
   返り値 = fd
   ======================= */
static inline int open(const char* filename) {
    int ret;
    asm volatile(
        "mov %[num], %%eax\n"
        "int $0x80\n"
        : "=a"(ret)
        : [num] "i"(SYS_FS_OPEN), "b"(filename)
    );
    return ret;
}

/* =======================
   read
   EBX = fd
   ECX = buf
   EDX = size
   返り値 = 読み込んだ大きさ
   ======================= */
static inline int read(int fd, void* buf, int size) {
    int ret;
    asm volatile(
        "mov %[num], %%eax\n"
        "int $0x80\n"
        : "=a"(ret)
        : [num] "i"(SYS_FS_READ), "b"(fd), "c"(buf), "d"(size)
    );
    return ret;
}


/* =======================
   close
   EBX = fd
   返り値 = 成功0 / 失敗-1
   ======================= */
static inline int close(int fd) {
    int ret;
    asm volatile(
        "mov %[num], %%eax\n"
        "int $0x80\n"
        : "=a"(ret)
        : [num] "i"(SYS_FS_CLOSE), "b"(fd)
    );
    return ret;
}


static inline void* malloc(int size) {
    void* ret;
    asm volatile(
        "mov %[num], %%eax\n"
        "int $0x80\n"
        : "=a"(ret)
        : [num] "i"(SYS_UMALLOC), "b"(size)
    );
    return ret;
}

static inline void free(void* ptr) {
    asm volatile(
        "mov %[num], %%eax\n"
        "int $0x80\n"
        :
        : [num] "i"(SYS_UFREE), "b"(ptr)
        : "eax"          // EAX を壊すからそのまま書いておく
    );
}
