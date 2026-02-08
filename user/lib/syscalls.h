#pragma once
#include "stdint.h"

int sys_write(const char *str);
int sys_open(const char *filename);
int sys_read(int fd, void *buf, int size);
int sys_close(int fd);

