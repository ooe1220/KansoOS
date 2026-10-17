#ifndef CONSOLE_H
#define CONSOLE_H

#include "../lib/stdint.h"

void console_init(void);
void console_clear(void);
void cursor_blink(void);
void kputc(char c);
void kputs(const char *s);
void kprintf(const char *format, ...);

#endif
