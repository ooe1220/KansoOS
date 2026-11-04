#include "ata.h"
#include "block.h"
#include "../lib/stddef.h"
#include "../lib/stdint.h"
#include "x86/console.h"

/*=============================================================
 * ata_block_read
 *-------------------------------------------------------------
 * 役割 : ブロックデバイス層の「読み込み」関数。
 *        block_dev_t.read に登録して使う（fs_read から間接的に呼ばれる）
 *
 * 引数 :
 *   dev   : 「どのデバイスか」を示すポインタ。
 *            今は ATA を 1 台しか無いので未使用（常に NULL）。
 *   lba   : 読み込み開始セクタ番号（LBA）
 *   count : 読み込むセクタ数（例: 8 = 1クラスタ分）
 *   buf   : 読み込んだデータを置くバッファ（512×count バイト必要）
 *
 * 戻り値 :
 *   0  : 成功
 *  -1  : 失敗（ata_read_lba28 の結果をそのまま返す）
 *
 * 流れ :
 *   fs_read → block_read → ここ → ata_read_lba28（実際のディスク読み込み）
 *=============================================================*/
static int ata_block_read(void *dev, uint32_t lba, uint32_t count, void *buf)
{
    (void)dev; 
    //kprintf("count=%d",count);
    return ata_read_lba28(lba, count, buf);  /* 成功0 / 失敗-1 を想定 */
}

/*=============================================================
 * ata_blkdev
 *-------------------------------------------------------------
 * 役割 : ATA ドライブを「ブロックデバイス」として扱うための
 *        設定シート（block_dev_t 型の実体）。
 *
 *        fs_read などは「ディスクの種類」を知らなくていい。
 *        この構造体を通して read を呼ぶだけで、
 *         ・ATA なら ata_block_read
 *         ・SD カードなら sd_block_read
 *         ……に自動で繋がるようになる。
 *
 * メンバ :
 *   .read        : 読み込み関数へのポインタ。
 *                   ata_block_read を登録する。
 *                   block_read() が 「b->read(...)」でここを呼ぶ。
 *
 *   .dev         : 「どのドライブか」を読み込み関数に教えるための
 *                   ポインタ。今はドライブを 1 台しか無いので NULL。
 *                   （複数ドライブ化する時は、チャンネル番号や
 *                     ドライブ構造体のアドレスを入れる）
 *
 *   .block_size  : 1 ブロック＝何バイトか。ATA は 512 固定。
 *                   呼び出し側が「1単位＝何バイト」を知るための目安。
 *
 * 使われ方 :
 *   fs_file.c で  g_fs_blk = &ata_blkdev;  として登録
 *      ↓
 *   block_read(g_fs_blk, lba, count, buf)
 *      ↓
 *   ata_blkdev.read(...)  →  ata_block_read(...)
 *=============================================================*/
block_dev_t ata_blkdev = {
    .read       = ata_block_read,
    .dev        = NULL,
    .block_size = 512,
};