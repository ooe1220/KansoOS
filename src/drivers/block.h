#ifndef _DRIVERS_BLOCK_H
#define _DRIVERS_BLOCK_H

#include "../lib/stdint.h"

/* ブロックデバイスの操作関数群（読み込み専用、今回は read のみ） */
typedef struct block_dev {
    /* dev: ドライバ固有のコンテキスト（ATAならNULL/チャンネル番号、ATAPIならドライブ構造体） */
    int  (*read)(void *dev, uint32_t lba, uint32_t count, void *buf);
    void *dev;
    uint32_t block_size;   /* 512 固定で開始 */
} block_dev_t;

/* 統一ラッパ：LBA を count セクタ読む */
static inline int block_read(block_dev_t *b,
                             uint32_t lba,
                             uint32_t count,
                             void *buf)
{
    if (!b || !b->read) return -1;
    return b->read(b->dev, lba, count, buf);
}

#endif /* _DRIVERS_BLOCK_H */