#include "atapi.h"         /* ドライバの世界 */
#include "block.h"         /* ブロックの世界 ← このファイルだけがまたぐ */
#include "../lib/stddef.h"
#include "../lib/stdint.h"
#include "x86/console.h"

/*=============================================================
 * atapi_block_read
 *-------------------------------------------------------------
 * 役割 : ATAPI を「ブロックデバイス」に見せる。
 *        ATAPI ドライバ自身はブロックの存在を知らない。
 *
 * 引数 :
 *   dev   : 未使用
 *   lba   : 論理ブロック番号（1 ブロック ＝ 2048 バイト）
 *   count : ブロック数
 *   buf   : 読み込み先（2048 × count バイト）
 *
 * 戻り値 : 0 成功 / -1 失敗
 *=============================================================*/
static int atapi_block_read(void *dev, uint32_t lba, uint32_t count, void *buf)
{
    (void)dev;
    uint8_t *p = (uint8_t *)buf;
    for (uint32_t i = 0; i < count; i++) {
        if (atapi_read_sector(lba + i, p) < 0) {
            kprintf("atapi_block: blk %d fail\n", lba + i);
            return -1;
        }
        p += 2048;
    }
    return 0;
}

/*=============================================================
 * atapi_blkdev
 *-------------------------------------------------------------
 * 役割 : 「ATAPI はこう使えばブロックに見える」という適応層。
 *        ata_blkdev と並んで fs_mount から選ばれる。
 *
 *   .read        : atapi_block_read
 *   .dev         : NULL（単一ドライブ固定）
 *   .block_size  : 2048 ← ATA の 512 と混ぜない命の値
 *=============================================================*/
block_dev_t atapi_blkdev = {
    .read       = atapi_block_read,
    .dev        = NULL,
    .block_size = 2048,
};