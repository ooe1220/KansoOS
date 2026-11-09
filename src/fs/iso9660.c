#include "lib/string.h"
#include "../x86/console.h"

#include "iso9660.h" 
#include "../x86/console.h"
#include "../lib/stdint.h"
#include "../lib/stddef.h"

#include "drivers/block.h"    /* block_read */
#include "fs/fs_mount.h"      /* g_fs_blk / block_dev_t がここにあるはず */
#include "drivers/bootdev.h"  /* bootdev_get */

#define ISO_PVD_LBA       16u
#define ISO_SECTOR_SIZE   2048u
#define ISO_ROOT_OFFSET   156u
#define ISO_MAGIC         "CD001"

/* レコード内オフセット */
#define ISO_REC_EXTENT    2u
#define ISO_REC_SIZE      10u

/* ---- 全局（ファイル内 static）: FAT16 の g_fat_lba 等と同じ役割 ---- */
static uint32_t g_iso_root_lba  = 0;
static uint32_t g_iso_root_size = 0;

/* リトルエンディアンの u32 をホスト順に（環境になければこれで OK） */
static uint32_t le32(const uint8_t *b)
{
    return (uint32_t)b[0]
         | ((uint32_t)b[1] << 8)
         | ((uint32_t)b[2] << 16)
         | ((uint32_t)b[3] << 24);
}

/* 名前の開始位置 = 固定部直後 + 拡張属性分 */
static const uint8_t *iso_name_ptr(const iso_dir_rec_t *rec)
{
    return (const uint8_t *)rec + ISO_DIR_REC_FIXED_SZ + rec->ext_attr_len;
}

int iso9660_init(void)
{
    uint8_t pvd[ISO_SECTOR_SIZE];
    const uint8_t *rec;
    uint32_t extent, size;
    block_dev_t *dev = bootdev_get();

    if (!dev || !dev->read)
        return -1;

    /* CD は 2048 バイト/セクタであることを確認 */
    if (dev->block_size != ISO_SECTOR_SIZE) {
        kputs("iso9660: unexpected block size\n");
        return -1;
    }

    /* ① PVD を読む */
    if (block_read(dev, ISO_PVD_LBA, 1, pvd) != 0) {
        kputs("iso9660: read PVD(16) failed\n");
        return -1;
    }

    /* ② シグネチャ確認: type=1, "CD001" */
    if (pvd[0] != 0x01 || memcmp(pvd + 1, ISO_MAGIC, 5) != 0) {
        kputs("iso9660: not an ISO9660 volume (no PVD)\n");
        return -1;
    }

    /* ③ PVD+156 のルートレコードから extent / size を取出す */
    rec = pvd + ISO_ROOT_OFFSET;

    if (rec[0] < 34) {
        kputs("iso9660: bad root record\n");
        return -1;
    }

    extent = le32(rec + 2);            /* extent_loc (LE) */
    size   = le32(rec + 10);           /* size (LE) */

    if (extent == 0 || size == 0) {
        kputs("iso9660: root extent is empty\n");
        return -1;
    }

    g_iso_root_lba  = extent;
    g_iso_root_size = size;

    kprintf("ISO9660 init ok: root=%u size=%u\n", extent, size);
    return 0;
}

/* 1 レコードの名前だけ表示（;1 はそのまま / 定義すれば削る） */
static void iso_print_name(const iso_dir_rec_t *rec)
{
    const uint8_t *name = iso_name_ptr(rec);
    int i;

    for (i = 0; i < rec->id_len; i++) {
#ifdef ISO_STRIP_SEMICOLON
        if (name[i] == ';') break;
#endif
        kputc(name[i]);
    }
}

void iso9660_list_names_only(const void *buf, int bufsz)
{
    const uint8_t *p   = (const uint8_t *)buf;
    const uint8_t *end = p + bufsz;

    while (p < end) {
        const iso_dir_rec_t *rec = (const iso_dir_rec_t *)p;

        if (rec->len == 0)                       /* 終端 */
            break;
        if (rec->len < ISO_DIR_REC_FIXED_SZ || p + rec->len > end)
            break;                               /* 壊れ防止 */

        /* カレント(0x00) / 親(0x01) はスキップしないならこの if を消す */
        const uint8_t *name = iso_name_ptr(rec);
        if (rec->id_len == 1 && (name[0] == 0x00 || name[0] == 0x01)) {
            p += rec->len;
            continue;
        }

        iso_print_name(rec);
        kputs(rec->flags & ISO_FILE_DIR ? "/\n" : "\n");

        p += rec->len;                           /* 可変長なので len ずつ進む */
    }
}

void iso9660_list_root(void)
{
    uint8_t root[2048];
    block_dev_t *dev = bootdev_get();

    if (block_read(dev, g_iso_root_lba, 1, root) != 0) {
        kputs("iso9660: read root failed\n");
        return;
    }

    iso9660_list_names_only(root, 2048);
}

const fs_ops_t iso9660_ops = {
    .open  = NULL,
    .read  = NULL,
    .close = NULL,
    .dir   = iso9660_list_root,
};