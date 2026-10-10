#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

void console_init(void);
void console_clear(void);
void kputc(char c);
void kputs(const char *s);
void kprintf(const char *format, ...);

#endif
