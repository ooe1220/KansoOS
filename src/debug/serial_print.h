#ifndef DEBUG_SERIAL_PRINT_H
#define DEBUG_SERIAL_PRINT_H
#include "lib/stdint.h"

void serial_print(const char *fmt, ...);

void debug_dump_regs(void);

void debug_enter(const char *func);

void debug_leave(const char *func);

void debug_dump_mem(uint32_t addr, int bytes) ;

void debug_dump_stack(uint32_t esp, int below, int above);

#endif
