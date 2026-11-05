#include "drivers/ata.h"
#include "lib/string.h"
#include "fat16.h"
#include "../x86/console.h"

static uint32_t g_fat_lba  = 0;
static uint32_t g_root_lba = 0;
static uint32_t g_data_lba = 0;
static uint32_t g_root_sec = 0;   /* ルートディレクトリのセクタ数 */
static uint8_t  g_sec_per_clus = 0;


//************************************************
// BPB(0x7c00) から FAT16 の各領域 LBA を計算
// 実値値: res=8, fats=2, fatsec=16, rent=512, spc=8, hidden=0
//   FAT  = 8
//   ROOT = 8 + 2 * 16      = 40  (0x28)
//   DATA = 40 + 32(root) = 72  (0x48)
//************************************************
void fat16_init(void) {
    const bpb_t* b = VBR_ADDR;

    uint32_t bps    = b->bytes_per_sec;   // 0x200 = 512 （1セクタあたりのバイト数）
    uint32_t res    = b->reserved_sec;    // 8　（予約セクタ）
    uint32_t nfats  = b->num_fats;       // 2　（FATの数　通常2）
    uint32_t rent   = b->root_ent_cnt;    // 512セクタ　（ルートディレクトリの要素数）
    uint32_t fatsec = b->fat_size16;      // 16　（FAT1つが占めるセクタ数）

    uint32_t root_sec = (rent * 32u + bps - 1u) / bps;   // 32　（ルートディレクトリが占めているセクタ数）

    g_fat_lba       = res;
    g_root_lba      = g_fat_lba + nfats * fatsec;
    g_data_lba      = g_root_lba + root_sec;
    g_sec_per_clus  = b->sec_per_clus;    // 8
    g_root_sec      = root_sec;

    kprintf("FAT16 init ok: fat=%u root=%u data=%u spc=%u\n",g_fat_lba, g_root_lba, g_data_lba, (uint32_t)g_sec_per_clus);
}

uint32_t fat16_data_lba(void)
{
    return g_data_lba;   /* static の中身を外に出さずに「値だけ」渡す */
}

uint32_t fat16_sec_per_cluster(void)
{
    return g_sec_per_clus;
}


//************************************************
// 文字を大文字に変換（自作）
//************************************************
static char to_upper_char(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c;
}

//************************************************
// ファイル名を8.3形式に変換　例）"test2.bin"→"TEST2   BIN"
//************************************************
void to_83_format(const char* filename, char* result) {
    int i, j = 0;
    
    // ファイル名部分（8文字）
    for (i = 0; i < 8 && filename[i] && filename[i] != '.'; i++) {
        result[j++] = to_upper_char(filename[i]);
    }
    
    // ファイル名空白埋め
    while (j < 8) {
        result[j++] = ' ';
    }
    
    // 拡張子部分 - 修正箇所
    int ext_found = 0;
    for (i = 0; filename[i]; i++) {
        if (filename[i] == '.') {
            ext_found = 1;
            i++; // '.'を跳ばす
            break;
        }
    }
    
    // 拡張子をコピー（最大3文字）
    if (ext_found) {
        for (int k = 0; k < 3 && filename[i]; k++, i++) {
            result[j++] = to_upper_char(filename[i]);
        }
    }
    
    // 拡張子空白埋め
    while (j < 11) {
        result[j++] = ' ';
    }
    
    result[11] = '\0';
}


//************************************************
// 指定したファイルの存在有無を確認
//************************************************
int fat16_find_file(const char* filename, uint32_t* start_cluster, uint32_t* file_size) {
    uint8_t buf[512];
    fat_dirent_t* ent;
    char target_name[12];
    int s, i;
    
    to_83_format(filename, target_name); // target_name=8.3変換後
    
    for (s = 0; s < g_root_sec; s++) { // 32セクタ分操作
        ata_read_sector(g_root_lba + s, buf);
        ent = (fat_dirent_t*)buf;

        for (i = 0; i < 512 / sizeof(fat_dirent_t); i++) {
            /* 終端または削除済み要素は跳ばす */
            if (ent[i].name[0] == 0x00 || ent[i].name[0] == 0xE5)
                continue;

            /* Volume label/systemは跳ばす */
            if (ent[i].attr & 0x08)
                continue;

            /* ファイル名比較 */
            if (memcmp(ent[i].name, target_name, 11)==0) {
                if (start_cluster) *start_cluster = ent[i].clus_lo | (ent[i].clus_hi << 16);
                if (file_size) *file_size = ent[i].size;
                return 1; // ファイルが見つかった
            }
        }
    }
    
    return 0; // ファイルが見つからない
}

//************************************************
// 8.3形式のファイル名へ変換し直接コンソールへ表示
//************************************************
static void print_name(const char* name) {
    int i;

    /* ファイル名 */
    for (i = 0; i < 8; i++) {
        if (name[i] == ' ') break;
        kputc(name[i]);
    }

    /* 拡張子 */
    if (name[8] != ' ') {
        kputc('.');
        for (i = 8; i < 11; i++) {
            if (name[i] == ' ') break;
            kputc(name[i]);
        }
    }
}

//************************************************
// ディレクトリの中身を直接コンソールへ表示
//************************************************
void fs_dir_list(void) {
    uint8_t buf[512];
    fat_dirent_t* ent;
    int s, i;

    for (s = 0; s < g_root_sec; s++) {
        ata_read_sector(g_root_lba + s, buf);
        ent = (fat_dirent_t*)buf;

        for (i = 0; i < 512 / sizeof(fat_dirent_t); i++) {
        
            /* 終端 */
            if (ent[i].name[0] == 0x00)
                return;

            /* 削除済み */
            if (ent[i].name[0] == 0xE5)
                continue;

            /* Volume label / system */
            if (ent[i].attr & 0x08)
                continue;

            print_name(ent[i].name);
            //kputs("\n"); // 縦表示
            kputs(" "); // 横表示
        }
    }
}

uint32_t fat16_cluster_to_lba(uint32_t cluster)
{
    return g_data_lba + (cluster - 2) * g_sec_per_clus;
}
