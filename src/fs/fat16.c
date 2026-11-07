#include "lib/string.h"
#include "fat16.h"
#include "../x86/console.h"
#include "fs_mount.h" 
#include "fs_file.h" 
#include "fs_mount.h" 

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
    
    for (s = 0; s < g_root_sec; s++) { // ルートの大きさ（セクタ）分走査

        if (block_read(g_fs_blk, g_root_lba + s, 1, buf) != 0) {
            kputs("fat16: read root dir failed\n");
            return 0;
        }

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
/*
void fs_dir_list(void) {
    uint8_t buf[512];
    fat_dirent_t* ent;
    int s, i;

    for (s = 0; s < g_root_sec; s++) {
        if (block_read(g_fs_blk, g_root_lba + s, 1, buf) != 0) {
            kputs("fat16: read root dir failed\n");
            return;
        }
        ent = (fat_dirent_t*)buf;

        for (i = 0; i < 512 / sizeof(fat_dirent_t); i++) {
        
            //終端 
            if (ent[i].name[0] == 0x00)
                return;

            // 削除済み
            if (ent[i].name[0] == 0xE5)
                continue;

            // Volume label / system 
            if (ent[i].attr & 0x08)
                continue;

            print_name(ent[i].name);
            //kputs("\n"); // 縦表示
            kputs(" "); // 横表示
        }
    }
}*/

//>>>>> fs_dir_list MS-DOS版

/* ---- 数値を文字列化（カンマ区切り、末尾から書くので自己完結） ---- */
static void num_with_commas(uint32_t n, char *out, int *len)
{
    char tmp[16];
    int  idx = 0;

    do {
        tmp[idx++] = '0' + (n % 10);
        n /= 10;
        if (n && idx % 3 == 0) tmp[idx++] = ',';
    } while (n);

    *len = idx;
    while (idx--) *out++ = tmp[idx];
}

/* ---- 右詰で出力（幅に足りなければ左をスペース埋め） ---- */
static void pr_right(uint32_t n, int width)
{
    char buf[16] = {0};      // ゴミを 0 にする
    int  len;

    num_with_commas(n, buf, &len);

    int pad = (width > len) ? (width - len) : 0;
    for (int i = 0; i < pad; i++) kputc(' ');   // 空白埋め
    for (int i = 0; i < len; i++) kputc(buf[i]); //  書いた分だけ出力
}

/* ---- DOS の日付(u16)・時刻(u16) を "MM-DD-YYYY  H:MMa/p" で出力 ---- */
static void pr_dos_datetime(uint16_t date, uint16_t time)
{
    int day  = date & 0x1F;
    int mon  = (date >> 5) & 0x0F;
    int year = 1980 + (date >> 9);

    int sec  = (time & 0x1F) * 2;
    int min  = (time >> 5) & 0x3F;
    int hr   = (time >> 11) & 0x1F;

    const char *ampm = (hr >= 12) ? "p" : "a";
    int h = hr % 12; if (h == 0) h = 12;

    kprintf("%d-%d-%d  %d:%d%s", mon, day, year, h, min, ampm);
}

/* ----FAT を走査して空きクラスタを数える ---- */
static uint32_t fat16_free_bytes(void)
{
    uint8_t fat[512];
    uint32_t free_clusters = 0;
    uint32_t fat_entries  = g_root_lba - g_fat_lba;   // 1 FAT のセクタ数 × (512/2)
    fat_entries *= 256;                               // 1 セクタあたり 256 エントリ(FAT16)

    for (uint32_t e = 2; e < fat_entries; e++) {
        uint32_t sec = e / 256;
        uint32_t off = (e % 256) * 2;

        if (sec == 0 || off == 0) {   // セクタ先頭なら読み直す（簡易キャッシュなし）
            if (block_read(g_fs_blk, g_fat_lba + sec, 1, fat) != 0)
                break;
        }
        uint16_t val = *(uint16_t *)&fat[off];
        if (val == 0x0000) free_clusters++;
    }
    return free_clusters * g_sec_per_clus * 512u;
}

/* ============ MS-DOS 風 dir ============ */
void fs_dir_list(void)
{
    uint8_t buf[512];
    fat_dirent_t *ent;
    int s, i;
    uint32_t nfiles = 0, total = 0;
    char vol_label[12] = "NO NAME";

    // 1 パス目でボリュームラベルを拾う（先頭付近にあるはず）
    for (s = 0; s < (int)g_root_sec; s++) {
        if (block_read(g_fs_blk, g_root_lba + s, 1, buf) != 0) break;
        ent = (fat_dirent_t *)buf;
        for (i = 0; i < (int)(512 / sizeof(fat_dirent_t)); i++) {
            if (ent[i].name[0] == 0x00) goto header;
            if (ent[i].attr & 0x08) {          // Volume label
                memcpy(vol_label, ent[i].name, 11);
                vol_label[11] = '\0';
                goto header;
            }
        }
    }
header:

    kprintf(" Volume in drive A is %s\n", vol_label);
    kputs(" Directory of A:\\\n\n");

    // 2 パス目：エントリを列挙 
    for (s = 0; s < (int)g_root_sec; s++) {
        if (block_read(g_fs_blk, g_root_lba + s, 1, buf) != 0) {
            kputs("fat16: read root dir failed\n");
            return;
        }
        ent = (fat_dirent_t *)buf;

        for (i = 0; i < (int)(512 / sizeof(fat_dirent_t)); i++) {
            if (ent[i].name[0] == 0x00) goto footer;
            if (ent[i].name[0] == 0xE5)  continue;
            if (ent[i].attr & 0x08)      continue;   // label はここではスキップ
            if (ent[i].attr & 0x0F)      continue;   // LFN

            //名前: "FILE    EXT" （8.3 をそのまま DOS 表記）
            for (int k = 0; k < 11; k++) kputc(ent[i].name[k]);

            //サイズ or <DIR> 
            if (ent[i].attr & 0x10) {
                kputs("       <DIR>  ");
            } else {
                pr_right(ent[i].size, 10);
                kputs("  ");
            }

            //日付・時刻 
            pr_dos_datetime(ent[i].wrt_date, ent[i].wrt_time);
            kputs("\n");

            if (!(ent[i].attr & 0x10)) {
                nfiles++;
                total += ent[i].size;
            }
        }
    }

footer:
    kputs("\n");
    pr_right(nfiles, 0); kputs(" file(s)    ");
    pr_right(total, 0);  kputs(" bytes\n");

    uint32_t free = fat16_free_bytes();
    kputs("                ");   // 上の行と桁を揃える（適当なパディング）
    pr_right(free, 0);   kputs(" bytes free\n");
}
//>>>>>>

uint32_t fat16_cluster_to_lba(uint32_t cluster)
{
    return g_data_lba + (cluster - 2) * g_sec_per_clus;
}

static int fat16_read(struct kfile *file, void *buf, int size)
{
    if (!file || !file->used) return -1;
    if (!g_fs_blk) return -1;

    uint32_t lba   = fat16_cluster_to_lba(file->first_unit);
    uint32_t sec_p = fat16_sec_per_cluster();

    if (block_read(g_fs_blk, lba, sec_p, buf) != 0) return -1;

    uint32_t got = sec_p * 512;
    file->pos += got;
    return (int)got;
}

static int fat16_open_op(struct kfile *file, const char *name)
{
    uint32_t cluster, size;

    if (!file || !name) return -1;
    if (!fat16_find_file(name, &cluster, &size)) return -1;

    file->first_unit = cluster;   // ★ FAT16 は「開始クラスタ」として使う
    file->size        = size;
    file->pos         = 0;
    return 0;
}

static int fat16_close_op(struct kfile *file)
{
    (void)file;
    return 0;
}

const fs_ops_t fat16_ops = {
    .open  = fat16_open_op,
    .read  = fat16_read,
    .close = fat16_close_op,
};