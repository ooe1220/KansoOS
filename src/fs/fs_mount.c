#include "../lib/stddef.h"
#include "../lib/stddef.h"
#include "fs_file.h"
#include "fs_ops.h"
#include "fat16.h"
#include "iso9660.h"
#include "../drivers/bootdev.h"
#include "x86/console.h"
#include "../drivers/block.h" 
#include "../drivers/atapi.h" 

extern const fs_ops_t fat16_ops; 
extern const fs_ops_t iso9660_ops; 

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

/*     
            // 実機では何故かkernel.c側で一度atapi_detectを読んで置かないとここ及びデータ読み込みに失敗する。kernel.cで一度呼ぶとここでも成功　QEMUはここ一回でも成功
            // 
            struct atapi_dev dev = atapi_detect();
            if (dev.present) {
                kputs("atapi: optical drive detected: ");
                kputs(dev.model);
                kputs("\n");
            } else {
                kputs("atapi: no optical drive found\n");
            } 
*/
            iso9660_init();
            g_fs_ops = &iso9660_ops; // IS09660は開発中
            break;
        default:
            fat16_init();
            g_fs_ops = &fat16_ops;     // HDD = FAT16
            break;
    }

    g_fs_blk = dev; // devmount.cの方で既に起動ディスク分岐している

    kputs("file system: mounted via ");
    kputs(bootdev_name());
    kputs("\n");
}