#ifndef MOUNT_H
#define MOUNT_H

#include "block.h"   /* block_dev_t のため */

/* 起動ディスクの種別（fs 層に伝える用。数値に意味はない） */
typedef enum {
    BOOTDEV_NONE = 0,
    BOOTDEV_HDD,
    BOOTDEV_CD,
} bootdev_type_t;

/* 起動ディスクに対応するブロックデバイスを返す（未サポートなら NULL） */
block_dev_t *bootdev_get(void);

/* 起動ディスクの種別を返す（表示用・分岐用） */
bootdev_type_t bootdev_type(void);

/* 種別の名前（kputs 用） */
const char *bootdev_name(void);

#endif