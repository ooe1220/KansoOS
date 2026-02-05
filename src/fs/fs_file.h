#pragma once
#include "../lib/stdint.h"

#define MAX_KFILES 8 // 同時に開けるファイル数
#define DATA_LBA (ROOT_DIR_LBA + ROOT_DIR_SECTORS)
//#define SECTORS_PER_CLUSTER 1

typedef struct {
    uint32_t start_cluster; // ファイルの最初のクラスタ番号
    uint32_t size;          // ファイルのサイズ（バイト単位） 
    uint32_t pos;           // 読み込み中の位置（現在のオフセット）
    int used;               // この fd が使われているかどうか（0 = 空き, 1 = 使用中）
} kfile_t;

extern kfile_t kfiles[MAX_KFILES];

// syscall 経由で呼ばれる
int fs_open(const char* filename);
int fs_read(int fd, void* buf, int size);
int fs_close(int fd);

