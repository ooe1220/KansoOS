#ifndef FS_OPS_H
#define FS_OPS_H

#include <stdint.h>

struct kfile;

/* ファイルシステムの操作メニュー（FS 実装ごとに差し替える） */
typedef struct {
    int  (*open)(struct kfile *kf, const char *name);
    int  (*read)(struct kfile *kf, void *buf, int len); 
    int  (*close)(struct kfile *kf); 
} fs_ops_t;

/* 今、どの FS が刺さってるか（fs_init_from_boot_sig でセット） */
extern fs_ops_t *g_fs_ops;

#endif