#include "mystdio.h"
#include "libc_table.h"

/* ユーザから見える関数はここを通る */

int write(const char *str) {
    return libc->write(str);
}

int open(const char *filename) {
    return libc->open(filename);
}

int read(int fd, void *buf, int size) {
    return libc->read(fd, buf, size);
}

int close(int fd) {
    return libc->close(fd);
}

void printf_d(const char *fmt, int val) {
    libc->printf_d(fmt, val);
}

