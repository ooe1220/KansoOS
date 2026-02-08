#pragma once
#include <stdint.h>

typedef struct {
    void (*write)(const char*);
    int  (*open)(const char*);
    int  (*read)(int, void*, int);
    int  (*close)(int);
} libc_table_t;

#define LIBC_TABLE_ADDR 0x400000

