#include "fs_file.h"
#include "fat16_file.h"
#include "drivers/ata.h"
#include "x86/console.h"

kfile_t kfiles[MAX_KFILES]; // 開いているファイルの情報 fs_file.h中構造体定義

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

static uint32_t cluster_to_lba(uint32_t start_cluster)
{
    return 126 + (start_cluster - 2) * 8;
}

// filenameが存在したらFDを返す
int fs_open(const char* filename)
{
    uint32_t start_cluster, file_size;
    if(!fat16_find_file(filename, &start_cluster, &file_size)) // ファイル名からファイル検索
        return -1;

    int fd = alloc_fd(); // FD割り当て
    if(fd<0) return -1; // 割り当て失敗(既に８ファイル開いている)

    kfiles[fd].start_cluster = start_cluster; // ファイルの開始クラスタ
    kfiles[fd].size = file_size;              // ファイルの大きさ
    
    return fd;
}

// FDが存在したら、ディスクから読み込みbufへ格納
// 読み込んだバイト数を返す　今は固定
// ＊＊＊＊＊＊＊＊＊＊＊＊
// FAT未実装により1クラスタ(8セクタ)のみ読み込む　今後拡張予定！！！！！！！！！！
// ＊＊＊＊＊＊＊＊＊＊＊＊
int fs_read(int fd, void* buf, int size)
{

    if(fd<0||fd>=MAX_KFILES) return -1; // FDが存在しない fs_open未実行
    if(!kfiles[fd].used) return -1; // 開かれていない若しくは閉じた    

    uint32_t lba = cluster_to_lba(kfiles[fd].start_cluster);
    ata_read_lba28(lba,8,buf);

    kfiles[fd].pos += 8*512;  // 1クラスタだけ
    return 8*512;
}

// FDを閉じる
int fs_close(int fd)
{
    if(fd<0||fd>=MAX_KFILES) return -1;
    kfiles[fd].used = 0;
    return 0;
}

