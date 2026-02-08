#ifndef ATA_H
#define ATA_H

#include "lib/stdint.h"

/*
 * ATA PIO (LBA28) でディスクを読み込む
 *
 * @param lba         開始LBA（0〜0x0FFFFFFF）
 * @param sector_cnt  読み込むセクタ数（1〜255）
 * @param buffer      読み込み先メモリアドレス
 *
 * @return 0  成功
 *        -1  エラー
 */
int ata_read_lba28(uint32_t lba, uint8_t sector_cnt, void* buffer);

int ata_read_sector(uint32_t lba, void* buffer);

int ata_write_lba28(uint32_t lba, uint8_t sector_cnt, void* buffer);

int ata_write_sector(uint32_t lba, void* buffer);

#endif /* ATA_H */

