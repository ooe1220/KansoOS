// INIT.h
#ifndef INIT_H
#define INIT_H

#include "lib/stdint.h"

// IRQの初期化（必要なIRQだけ有効化）
void init_irq(void);

void arch_init(void);

#endif // INIT_H
