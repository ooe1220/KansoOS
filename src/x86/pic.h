// PIC.h
#ifndef PIC_H
#define PIC_H

#include "lib/stdint.h"

// PIC初期化（リマップ＋全IRQマスク）
void pic_init(void);

// IRQマスク設定
void pic_mask_irq(uint8_t irq);
void pic_unmask_irq(uint8_t irq);

void pic_send_eoi(uint8_t irq);

#endif // PIC_H

