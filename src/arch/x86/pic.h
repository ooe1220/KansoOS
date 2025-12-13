// PIC.h
#ifndef PIC_H
#define PIC_H

#include "lib/stdint.h"

// PIC初期化（リマップ＋全IRQマスク）
void pic_init(void);

// IRQマスク設定（16bit: 下位8bit=マスタ, 上位8bit=スレーブ）
void pic_set_mask(uint16_t mask);

#endif // PIC_H

