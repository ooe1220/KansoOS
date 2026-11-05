#include "fs_file.h"
#include "fat16.h"
#include "drivers/block.h"
#include "x86/console.h"
#include "../lib/stddef.h"
#include "../lib/stdint.h"

static kfile_t kfiles[MAX_KFILES]; // 開いているファイルの情報 fs_file.h中構造体定義

extern block_dev_t ata_blkdev;
static block_dev_t *g_fs_blk = &ata_blkdev;

// 新しいFDを割り当てる
// 返り値 int → 割り当てた fd 番号（0～MAX_KFILES-1）、空きがなければ -1
static int alloc_fd(void)
{
    for(int i=0;i<MAX_KFILES;i++){
        if(!kfiles[i].used){ // 未使用なら
            kfiles[i].used = 1;
            kfiles[i].pos = 0;  // 読み込み位置を先頭に合わす
            return i;
        }
    }
    return -1;
}

// filenameが存在したらFDを返す
int fs_open(const char* filename)
{
    uint32_t first_block, file_size;
    if(!fat16_find_file(filename, &first_block, &file_size)) // ファイル名からファイル検索
        return -1;

    int fd = alloc_fd(); // FD割り当て
    if(fd<0) return -1; // 割り当て失敗(既に８ファイル開いている)
    kfiles[fd].first_block = first_block; // ファイルの開始クラスタ
    kfiles[fd].size = file_size;              // ファイルの大きさ
    return fd;
}

/*=============================================================
 * fs_read
 *-------------------------------------------------------------
 * 役割 : 既に開いているファイルからデータを読む。
 *        FAT16 の「クラスタチェーン」はまだ未実装なので、
 *        現状は「開始クラスタ 1 つぶん（= 1 クラスタ）」しか読まない。
 *
 * 引数 :
 *   fd   : fs_open で貰ったファイルディスクリプタ
 *   buf  : 読み込んだデータを置くバッファ
 *   size : 読みたいバイト数（今は無視して 1 クラスタ分だけ読む）
 *
 * 戻り値 :
 *   正の値 : 実際に読んだバイト数（今は常に 8×512 = 4096）
 *   -1     : 失敗（FD が不正 / 未オープン / デバイス未マウント / 読み込みエラー）
 *
 * 処理の流れ :
 *   1. FD の正当性確認
 *   2. デバイスがマウントされてるかチェック（g_fs_blk が NULL なら弾く）
 *   3. 開始クラスタ → LBA に変換
 *   4. block_read で 1 クラスタ（8 セクタ）読む
 *   5. 読んだぶん読み取り位置(pos)を進めてバイト数を返す
 *=============================================================*/
/*
int fs_read(int fd, void* buf, int size)
{
    if(fd<0||fd>=MAX_KFILES) return -1; // FDが存在しない fs_open未実行
    if(!kfiles[fd].used) return -1; // 開かれていない若しくは閉じた    
    if(!g_fs_blk) return -1;          // デバイスがセットされてない

    uint32_t lba = fat16_cluster_to_lba(kfiles[fd].first_block);

    if (block_read(g_fs_blk, lba, g_sec_per_clus, buf) != 0)return -1;  // FATを辿り１クラスタ以上呼ぶ場合はここを複数回呼び出す

    kfiles[fd].pos += 8*512;  // 1クラスタだけ
    return 8*512;
}*/

int fs_read(int fd, void* buf, int size)
{
    if (fd < 0 || fd >= MAX_KFILES) return -1; // FDが存在しない fs_open未実行
    if (!kfiles[fd].used) return -1;// 開かれていない若しくは閉じた   
    if (!g_fs_blk) return -1;         // デバイスがセットされてない

    uint32_t lba  = fat16_cluster_to_lba(kfiles[fd].first_block);
    uint32_t sec_per_cluster = fat16_sec_per_cluster(); 

    if (block_read(g_fs_blk, lba, sec_per_cluster, buf) != 0) return -1;

    uint32_t got = sec_per_cluster * 512;
    kfiles[fd].pos += got;
    return (int)got;
}

// FDを閉じる
int fs_close(int fd)
{
    if(fd<0||fd>=MAX_KFILES) return -1;
    kfiles[fd].used = 0;
    return 0;
}


int fs_get_file_size(int fd, uint32_t *out_size) {
    if (fd < 0 || fd >= MAX_KFILES) return -1;
    if (!kfiles[fd].used) return -1;
    *out_size = kfiles[fd].size;
    return 0;
}
