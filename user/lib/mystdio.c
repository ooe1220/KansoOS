#pragma once
#include "syscall.h"

// -----------------
// itoa (整数→文字列)
// -----------------
static void itoa(int value, char *buffer) {
    char temp[12];
    int i = 0, j;
    int negative = 0;

    if (value < 0) {
        negative = 1;
        value = -value;
    }

    do {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    } while (value);

    if (negative)
        temp[i++] = '-';

    for (j = 0; j < i; j++)
        buffer[j] = temp[i - j - 1];

    buffer[i] = '\0';
}

// -----------------
// printf_d (%d 1個だけ)
// -----------------
static void printf_d(const char *fmt, int val) {
    char buffer[128];
    char numbuf[12];
    int i = 0, j = 0;

    while (fmt[i] != '\0' && j < sizeof(buffer) - 1) {
        if (fmt[i] == '%' && fmt[i+1] == 'd') {
            itoa(val, numbuf);

            int k = 0;
            while (numbuf[k] != '\0' && j < sizeof(buffer) - 1)
                buffer[j++] = numbuf[k++];

            i += 2;
        } else {
            buffer[j++] = fmt[i++];
        }
    }
    buffer[j] = '\0';

    write(buffer);
}

int write(const char *str) {
    return sys_write(str);
}

int open(const char *filename) {
    return sys_open(filename);
}

int read(int fd, void *buf, int size) {
    return sys_read(fd, buf, size);
}

int close(int fd) {
    return sys_close(fd);
}
