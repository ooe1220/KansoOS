#include "command.h"
#include "x86/console.h"
#include "x86/io.h"
#include "lib/string.h"
#include "lib/stdint.h"
#include "lib/stddef.h" // NULLの定義
#include "fs/dir.h"
#include "fs/fat16_file.h"
#include "user_exec.h"
#include "drivers/ata.h"
#include "../fs/fat16.h"
#include "x86/pic.h"

#define USER_PROG_MEM 0x10000 // ユーザプログラムを置くアドレス
#define USER_ARG_MEM  0x20000  // コマンドライン引数を置くアドレス、ユーザプログラムには実体でなくアドレスを渡す

// 内部コマンド実行
int run_builtin_command(const char *line){

if (strcmp(line, "help") == 0) {
        kputs("\nAvailable commands:\n");
        kputs("  help     - Show this message\n");
        kputs("  clear    - Clear screen\n");
        kputs("  reboot   - Reboot CPU\n");
        kputs("  shutdown - Shutdown system (QEMU only)\n");
        kputs("  ls       - List files in current directory\n");
        kputs("  dir      - Same as 'ls'\n");
        return 0;
    }
    
    if (strcmp(line, "clear") == 0) {
        console_clear();
        return 0;
    }
    
    if (strcmp(line, "reboot") == 0) {
        outb(0x64, 0xFC);
        return 0;
    }
    
    if (strcmp(line, "shutdown") == 0) {
        outw(0x604, 0x2000);//QEMU専用
        return 0;
    }
    
    if (strcmp(line, "ls") == 0 || strcmp(line, "dir") == 0) {
        kputs("\n");
        fs_dir_list();
        return 0;
    }
    
    return -1;
}

void run_file(const char *line){
    if (line[0] == 0) return; // 空行なら何もしない
    
    char filename[64];
    const char *argstr = NULL;
    
    // 空白で分割 (コマンドライン引数があった場合はファイル名の抽出が必要。例: hello.bin param1 param2 -> filename=hello.bin)
    int i = 0;
    while (line[i] && line[i] != ' ' && i < sizeof(filename)-1) {
        filename[i] = line[i];
        i++;
    }
    filename[i] = 0;
    
    // 末尾が ".bin" か確認して、なければ追加
    int len = strlen(filename);
    if(len < 4 || strcmp(filename + len - 4, ".bin") != 0){
        if(len + 4 < sizeof(filename)){
            filename[len] = '.';
            filename[len+1] = 'b';
            filename[len+2] = 'i';
            filename[len+3] = 'n';
            filename[len+4] = 0;
        } else {
            kputs("Filename too long: ");
            kputs(filename);
            return;
        }
    }
    
    //ファイルが存在するかを確認しない場合は抜ける
    uint32_t start_cluster, file_size;
    kputs("\n");    
    if (!fat16_find_file(filename, &start_cluster, &file_size)) {
        kputs("Unknown command or file not found: ");
        kputs(line);
        return;
    }
    
    // 空白の次から引数部分
    if (line[i] == ' ') {
        argstr = line + i + 1;
    }
    
    // 引数をユーザプログラムが読む場所に複製
    char *p = (char*)USER_ARG_MEM;
    for (int j = 0; j < 256; j++) p[j] = 0; // 前回のデータを初期化
    if (argstr) {
        strcpy((char*)USER_ARG_MEM, argstr); // 引数文字列コピー
    }
    
    // argc を計算
    int argc = 1; // argv[0] = filename
    if (argstr) {
        int in_word = 0;
        for (int j = 0; argstr[j]; j++) {
            if (argstr[j] != ' ' && !in_word) {
                in_word = 1;   // 新しい単語開始
                argc++;        // 引数加算
            } else if (argstr[j] == ' ') {
                in_word = 0;   // 単語終了
            }
        }
    }
    
    // argv 配列を作る
    char *argv[argc];
    argv[0] = filename;
    
    if (argstr) {
        int arg_index = 1;
        char *s = (char*)USER_ARG_MEM; // 複製済みの引数文字列
        argv[arg_index] = s;
        for (int j = 0; s[j]; j++) {
            if (s[j] == ' ') {
                s[j] = 0; // 空白を終端に置換
                arg_index++;
                if (arg_index < argc) {
                    argv[arg_index] = &s[j+1]; // 次の引数先頭番地
                }
            }
        }
    }    
                 
    // ファイルを毎回固定でメモリ0x10000上へ展開して実行
    // 1クラスタ=8セクタ FAT表未実装により、固定で8セクタを読み込む
    // 前提:データ領域LBA126〜、1クラスタ=8セクタ
    //kprintf_d("start_cluster=%d\n",start_cluster);
    uint32_t start_sector = 126 + (start_cluster - 2) * 8;//開始クラスタ→開始セクタ変換式
    ata_read_lba28(start_sector, 8, (void*)USER_PROG_MEM); // ユーザプログラムをメモリ0x10000上へ展開
    pic_mask_irq(1); // IRQ1キーボード無効化
    int ret = user_exec((void*)USER_PROG_MEM, argc, argv);// ユーザプログラムへ遷移
    //kprintf("ret = %d\n",ret);
    pic_unmask_irq(1); // IRQ1キーボード有効化
}



