#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define SECTOR_SIZE  512
#define TOTAL_SECTORS 2048
#define DISK_SIZE    (SECTOR_SIZE * TOTAL_SECTORS)  // 1,048,576 bytes = 1MB


struct root_entry {
    const char *filename;   // Linux側のファイル名 "test2.bin"
    const char *fatname;    // FAT上の8+3名 "TEST2   BIN"
    int lba;                // 配置先LBR
    int size;
};

// src/fs/fat16.h中の構造体に合わせる
struct __attribute__((packed)) dir_entry {
    char     name[11];          // 0
    uint8_t  attr;              // 11
    uint8_t  ntres;             // 12
    uint8_t  crt_time_tenth;    // 13
    uint16_t crt_time;          // 14
    uint16_t crt_date;          // 16
    uint16_t acc_date;          // 18
    uint16_t clus_hi;           // 20
    uint16_t wrt_time;          // 22
    uint16_t wrt_date;          // 24
    uint16_t cluster;           // 26 ← ここが正解
    uint32_t size;              // 28
};

//備忘録 : fread(書き込み先, 1要素の大きさ（バイト）, 要素数, FILEポインタ);
static int write_file(uint8_t *disk, const char *path, int lba) {
    FILE *f = fopen(path, "rb");
    
    // ファイル大きさを取得
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // セクタ境界に切り上げ
    int sectors = (size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    
    fread(disk + lba * SECTOR_SIZE, 1, size, f);
    fclose(f);
    printf("[mkfs] %s -> LBA %d (%d sectors, %ld bytes)\n", path, lba, sectors, size);
}


static void write_fat(uint8_t *disk) {
    uint8_t *fat1 = disk + 64 * SECTOR_SIZE;
    uint8_t *fat2 = disk + 65 * SECTOR_SIZE;
    
    // FATヘッダ: メディア記述子 + 終端
    fat1[0] = 0xF8; fat1[1] = 0xFF; fat1[2] = 0xFF;
    fat2[0] = 0xF8; fat2[1] = 0xFF; fat2[2] = 0xFF;
    
    // クラスタ2以降は全部 0xFFFF（未使用/終端）
    for (int i = 2; i < 256; i++) {
        int offset = i * 2;
        fat1[offset] = 0xFF;
        fat1[offset + 1] = 0xFF;
        fat2[offset] = 0xFF;
        fat2[offset + 1] = 0xFF;
    }
    
    printf("[mkfs] FAT1/2 initialized at LBA 64/65\n");
}

static void write_rootdir(uint8_t *disk, struct root_entry *files, int count) {
    uint8_t *root = disk + 66 * SECTOR_SIZE;
    memset(root, 0, 32 * SECTOR_SIZE);

    for (int i = 0; i < count; i++) {
        struct dir_entry entry = {0};
        memcpy(entry.name, files[i].fatname, 11);// ファイル名
        entry.attr = 0x20;// 属性
        entry.cluster = (files[i].lba - 126) / 8 + 2;// 開始クラスタ (LBA → クラスタ番号)
        entry.size = files[i].size;// ファイルサイズ
        
        // ★ 確認用
        printf("[rootdir] %s -> cluster=%u (lba=%d)\n",files[i].fatname, entry.cluster, files[i].lba);
        
        memcpy(root + i * 32, &entry, 32);// RootDirに書き込み
    }

    printf("[mkfs] RootDir written at LBA 66 (%d entries)\n", count);
}

int main(void) {
    
    uint8_t *disk = calloc(1, DISK_SIZE); // 1MB の0埋めバッファ確保
    printf("[mkfs] 1MB disk image buffer allocated (zero-filled)\n");
    
    write_file(disk, "build/mbr.bin", 0);
    write_file(disk, "build/vbr.bin", 63);
    write_file(disk, "build/kernel.bin", 126);
    
    write_fat(disk); // FAT表生成(現在未使用)
    
// ユーザープログラム配置
    struct root_entry files[] = {
        {"test2.bin", "TEST2   BIN", 1814, 0},
        {"test3.bin", "TEST3   BIN", 1822, 0},
        {"cat.bin",   "CAT     BIN", 1830, 0},
        {"test.txt",  "TEST    TXT", 1838, 0},
        {"memtest.bin","MEMTEST BIN",1846, 0},
    };
    int file_count = 5;
    
    // ファイル配置 + 大きさ自動取得
    for (int i = 0; i < file_count; i++) {
        char path[64];
        snprintf(path, sizeof(path), "build/%s", files[i].filename); // pathの生成
        files[i].size = write_file(disk, path, files[i].lba);
    }

    write_rootdir(disk, files, file_count);

    FILE *out = fopen("build/disk.img", "wb"); //  disk.img 出力

    fwrite(disk, 1, DISK_SIZE, out);
    fclose(out);

    printf("[mkfs] build/disk.img written (%d bytes)\n", DISK_SIZE);

    free(disk);
    return 0;
}
