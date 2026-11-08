#include "lib/string.h"
#include "../x86/console.h"
//#include "fs_mount.h" 
//#include "fs_file.h" 
//#include "fs_mount.h" 

#include "iso9660.h" 
#include "../x86/console.h"
#include "../lib/stdint.h"

#include "drivers/block.h"    /* block_read */
#include "fs/fs_mount.h"      /* g_fs_blk / block_dev_t がここにあるはず */
#include "drivers/bootdev.h"  /* bootdev_get */


//lba_le=19 一旦固定値を使う
/*
isoinfo -l -i build/os.iso
dd if=build/os.iso of=/tmp/pvd.bin bs=2048 skip=16 count=1 status=none
xxd -s 0x9c -l 8 /tmp/pvd.bin
isoinfo -l -i build/os.iso | grep KERNEL
*/


/* テスト呼び出し
    uint8_t buf[2048];
    read_blk(19, buf);                 // 実際の読み込み関数に合わせて
    iso9660_list_names_only(buf, 2048);
*/
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

    if (block_read(dev, 19, 1, root) != 0) {
        kputs("iso9660: read root(19) failed\n");
        return;
    }

    iso9660_list_names_only(root, 2048);
}