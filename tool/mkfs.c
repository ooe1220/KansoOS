#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../src/fs/fat16.h" // fat16の構造体はOS側に合わせる、DATA_START_LBA使用　※DATA_START_LBA再調整必須

#define SECTOR_SIZE  512
#define TOTAL_SECTORS 2048
#define DISK_SIZE    (SECTOR_SIZE * TOTAL_SECTORS)  // 1,048,576 bytes = 1MB


struct root_entry {
    const char *filename;   // Linux側のファイル名 "test2.bin"
    const char *fatname;    // FAT上の8+3名 "TEST2   BIN"
    int lba;                // 配置先LBR
    int size;
};

struct fat16_bpb {
    uint8_t  jmp[3];          // 0: jmp start nopの機械語
    char     oem[8];          // 3: 名前

    uint16_t byts_per_sec;    // 11: 1セクタあたりのバイト数（通常512）
    uint8_t  sec_per_clus;    // 13: 1クラスタあたりのセクタ数
    uint16_t rsvd_sec_cnt;    // 14: VBRからFAT開始までの予約セクタ数
    uint8_t  num_fats;        // 16: FATの数（通常2）
    uint16_t root_ent_cnt;    // 17: ルートディレクトリのエントリ数
    uint16_t tot_sec_16;      // 19: ボリューム総セクタ数（小容量用）
    uint8_t  media;           // 21: メディア種別（HDDなら通常0xF8）
    uint16_t fat_sz_16;       // 22: 1つのFATが占めるセクタ数
    uint16_t sec_per_trk;     // 24: 1トラックあたりのセクタ数（BIOS CHS用）
    uint16_t num_heads;       // 26: ヘッド数（BIOS CHS用）
    uint32_t hid_sec;         // 28: パーティション開始位置までの隠しセクタ数
    uint32_t tot_sec_32;      // 32: ボリューム総セクタ数（大容量用）
} __attribute__((packed));

struct fat16_ebpb {
    uint8_t  drive_num;       // 36: BIOSブートドライブ番号（0x80=HDD等）
    uint8_t  reserved;        // 37: 予約領域
    uint8_t  boot_sig;        // 38: 拡張BPBが存在することを示す署名（0x29）
    uint32_t vol_id;          // 39: ボリューム固有のシリアル番号
    char     vol_lab[11];     // 43: ボリュームラベル（11文字）
    char     fil_sys_type[8]; // 54: ファイルシステム種別文字列（"FAT16   "）
} __attribute__((packed));


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
    //printf("[mkfs] %s -> LBA %d (%d sectors, %ld bytes)\n", path, lba, sectors, size);
    return (int)size;
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
        fat_dirent_t entry = {0};
        memcpy(entry.name, files[i].fatname, 11);// ファイル名
        entry.attr = 0x20;// 属性
        entry.clus_lo = (files[i].lba - DATA_START_LBA) / 8 + 2;// 開始クラスタ (LBA → クラスタ番号)
        entry.size = files[i].size;// ファイルサイズ
        
        // ★ 確認用
        printf("[rootdir] %s -> clus_lo=%u (lba=%d)\n",files[i].fatname, entry.clus_lo, files[i].lba);
        
        memcpy(root + i * 32, &entry, 32);// RootDirに書き込み
    }

    printf("[mkfs] RootDir written at LBA 66 (%d entries)\n", count);
}

// VBRのPBPをここで書き換える
static void patch_vbr(uint8_t *vbr)
{
    struct fat16_bpb  *bpb  = (struct fat16_bpb *)vbr;
    struct fat16_ebpb *ebpb = (struct fat16_ebpb *)(vbr + 36);
    
    // jmp + nop
    vbr[0] = 0xEB;
    vbr[1] = 0x3C;
    vbr[2] = 0x90;

    // OEM名
    memcpy(bpb->oem, "KANSO OS", 8);

    bpb->byts_per_sec = 512;            // 1セクタあたりのバイト数（通常512）
    bpb->sec_per_clus = 8;              // 1クラスタあたりのセクタ数
    bpb->rsvd_sec_cnt = 1;              // VBRからFAT開始までの予約セクタ数
    bpb->num_fats     = 2;              // FATの数
    bpb->root_ent_cnt = 512;            // ルートディレクトリのエントリ数
    bpb->tot_sec_16   = TOTAL_SECTORS;  // ボリューム総セクタ数（小容量用）
    bpb->media        = 0xF8;           // 媒体種別
    bpb->fat_sz_16    = 8;              // 1つのFATが占めるセクタ数
    bpb->sec_per_trk  = 63;             // 1トラックあたりのセクタ数（BIOS CHS用）
    bpb->num_heads    = 255;            // ヘッド数（BIOS CHS用）
    bpb->hid_sec      = 63;             // パーティション開始位置までの隠しセクタ数
    bpb->tot_sec_32   = 0;              // ボリューム総セクタ数（大容量用）

    ebpb->drive_num = 0x80;             // BIOSブートドライブ番号（0x80=HDD等）
    ebpb->reserved  = 0;                // 予約領域
    ebpb->boot_sig  = 0x29;             // 拡張BPBが存在することを示す署名

    ebpb->vol_id = 0x12345678;          // ボリューム固有の番号

    memcpy(ebpb->vol_lab, "KANSO OS    ", 11); //ボリュームラベル（11文字）
    memcpy(ebpb->fil_sys_type, "FAT16   ", 8); // ファイルシステム種別文字列
}

int main(void) {
    
    uint8_t *disk = calloc(1, DISK_SIZE); // 1MB の0埋めバッファ確保
    printf("[mkfs] 1MB disk image buffer allocated (zero-filled)\n");
    
    write_file(disk, "build/mbr.bin", 0);
    write_file(disk, "build/vbr.bin", 63);
    patch_vbr(disk + 63 * SECTOR_SIZE); // dd if=build/disk.img bs=512 skip=63 count=1 | hexdump -C
    write_file(disk, "build/kernel.bin", DATA_START_LBA);
    
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
