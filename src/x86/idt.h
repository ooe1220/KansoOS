#ifndef IDT_H
#define IDT_H

#include "lib/stdint.h"

void idt_init(void);
void idt_set_gate(int n, uint32_t handler);

#endif

