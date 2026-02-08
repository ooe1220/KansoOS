#pragma once

typedef struct {

    int (*write)(const char*);
    int (*open)(const char*);
    int (*read)(int, void*, int);
    int (*close)(int);

    void (*printf_d)(const char*, int);

} libc_table_t;


/* 共有アドレス */
#define LIBC_TABLE_ADDR 0x200000

#define libc ((libc_table_t*)LIBC_TABLE_ADDR)

