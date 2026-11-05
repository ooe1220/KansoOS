#pragma once
#include "../lib/stdint.h"

#define MAX_KFILES 8 // 同時に開けるファイル数

typedef struct {
    uint32_t first_block; // ファイルの最初のブロック FAT16=クラスタ，ISO9600=LBA
    uint32_t size;          // ファイルのサイズ（バイト単位） 
    uint32_t pos;           // 読み込み中の位置（現在のオフセット）
    int used;               // この fd が使われているかどうか（0 = 空き, 1 = 使用中）
} kfile_t;

// syscall経由、カーネル内使用
int fs_open(const char* filename);
int fs_read(int fd, void* buf, int size);
int fs_close(int fd);

int fs_get_file_size(int fd, uint32_t *out_size);

void fs_init_from_boot_sig(void);
