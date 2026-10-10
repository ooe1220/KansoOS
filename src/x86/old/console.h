#pragma once
#include "lib/stdint.h"

void kputc(char c);
void kputs(const char* s);
void console_clear();
void kprintf_d(const char *fmt, int val);
void kprintf(const char* format, ...);
