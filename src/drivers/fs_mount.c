#include "fs_file.h"
//#include "fs_ops.h"
#include "fat16.h"
#include "drivers/bootdev.h"
#include "x86/console.h"

/* bootdev と fs 実装を繋ぐだけの init */
void fs_mount_root(void)
{
    block_dev_t *dev = bootdev_get();

    if (!dev) {
        kputs("file system: no boot device / unsupported media\n");
       // g_fs_ops = NULL;
        return;
    }

    fat16_set_blkdev(dev);      /* HDD でも CD(対応後) でも、dev を FAT に渡す */
    //g_fs_ops = &fat16_ops;

    kputs("file system: mounted via ");
    kputs(bootdev_name());
    kputs("\n");
}