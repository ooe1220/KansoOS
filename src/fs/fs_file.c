#include "fs_file.h"
#include "fs_ops.h"
#include "fs_mount.h" 

static struct kfile kfiles[MAX_KFILES]; // 現在開いているファイルの配列

// ファイルを読み込む際に空いている番号を探してFD発行
static int alloc_fd(void)
{
    int i;

    for (i = 0; i < MAX_KFILES; i++) {
        if (!kfiles[i].used) {
            kfiles[i].used = 1;
            kfiles[i].size = 0;
            kfiles[i].pos = 0;
            return i;
        }
    }

    return -1;
}

// ファイルを開く
int fs_open(const char *filename)
{
    int fd;

    if (!filename || !g_fs_ops || !g_fs_ops->open)
        return -1;

    fd = alloc_fd(); // 空いているFDが無い場合
    if (fd < 0)
        return -1;

    // FAT16 にファイルを開かせ、失敗したら fd を回収する
    if (g_fs_ops->open(&kfiles[fd], filename) != 0) {
        kfiles[fd].used = 0;
        return -1;
    }

    return fd;
}

// ファイルを読む
int fs_read(int fd, void *buf, int size)
{
    // fdの番号範囲、開いているかを確認
    if (fd < 0 || fd >= MAX_KFILES || !kfiles[fd].used)
        return -1;

    // bufferあるか、ファイルシステム設定済みか、読む機能はあるか
    if (!buf || size < 0 || !g_fs_ops || !g_fs_ops->read)
        return -1;

    return g_fs_ops->read(&kfiles[fd], buf, size);
}

// ファイル（FD）を閉じる
int fs_close(int fd)
{
    int result = 0;

    if (fd < 0 || fd >= MAX_KFILES || !kfiles[fd].used)
        return -1;

    if (g_fs_ops && g_fs_ops->close)
        result = g_fs_ops->close(&kfiles[fd]);

    kfiles[fd].used = 0;
    return result;
}

// ファイルの大きさを返す
int fs_get_file_size(int fd, uint32_t *out_size)
{
    if (fd < 0 || fd >= MAX_KFILES || !kfiles[fd].used || !out_size)
        return -1;

    *out_size = kfiles[fd].size;
    return 0;
}