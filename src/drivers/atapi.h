#ifndef ATAPI_H
#define ATAPI_H

#include "stdint.h"

/*
 * ATAPI デバイス情報構造体
 */
struct atapi_dev {
    int present;                  // デバイスが存在するか (1 = あり)
    char model[41];               // モデル名（NULL終端、最大40文字）
    // 必要に応じて他のフィールド（容量、LBA対応など）を追加可能
};


struct atapi_dev atapi_detect(void);

int atapi_read_sector(uint32_t lba, uint8_t *buf);

#endif /* ATAPI_H */
