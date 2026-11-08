#include "../lib/stddef.h"
#include "fs_file.h"
#include "fs_ops.h"
#include "fat16.h"
#include "../drivers/bootdev.h"
#include "x86/console.h"
#include "../drivers/block.h" 

extern const fs_ops_t fat16_ops; 

/* ---- マウント状態はここが持つ ---- */
fs_ops_t    *g_fs_ops = NULL;
block_dev_t *g_fs_blk = NULL;

/* bootdev と fs 実装を繋ぐ ※暫定的に起動ディスクに固定して、kernelmainから1回呼ぶ*/
void fs_mount_root(void)
{
    block_dev_t *dev = bootdev_get(); // 起動ディスクを貰う

    if (!dev) {
        kputs("file system: no boot device / unsupported media\n");
        return;
    }

    // CD対応後にここを分岐にする。今はATA+FAT16固定
    switch (bootdev_type()) {
        case BOOTDEV_CD:
            g_fs_ops = NULL; // IS09660は開発中
            break;
        default:
            fat16_init();
            g_fs_ops = &fat16_ops;     /* HDD = FAT16 */
            break;
    }

    g_fs_blk = dev; // devmount.cの方で既に起動ディスク分岐している

    kputs("file system: mounted via ");
    kputs(bootdev_name());
    kputs("\n");
}