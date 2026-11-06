#ifndef FS_MOUNT_H
#define FS_MOUNT_H

#include "fs_ops.h"
#include "drivers/ata.h"
#include "drivers/block.h"

/* ルートファイルシステムをマウントする（起動デバイス判定 → FS 選択までやる） */
void fs_mount_root(void);

/* fs_mount.c が持っているマウント状態を見えるようにする */
extern fs_ops_t *g_fs_ops;
extern block_dev_t *g_fs_blk;

#endif