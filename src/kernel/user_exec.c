#include "user_exec.h"
#include "lib/string.h"
#include "x86/console.h"
#include "lib/stddef.h" // NULLの定義
#include "fs/fs_file.h"
#include "debug/serial.h"
#include "debug/debug_print.h"
#include "lib/memmap.h"

extern uint32_t saved_kernel_esp;
extern uint32_t saved_kernel_ebp;

int run_file(const char *line){
    if (line[0] == 0) return 1; // 空行なら何もしない
    
    char filename[64];
    const char *argstr = NULL;
    
    // 空白で分割 (入力文字列から「最初の空白まで」をファイル名として切り出す。例: hello.bin param1 param2 -> filename=hello.bin)
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
            return 2;
        }
    }
    
    //ファイルが存在するかを確認しない場合は抜ける
    int fd = fs_open(filename);
    kputs("\n");
    if (fd < 0) {   
        return -1;
    }
    
    // 空白の次から引数部分
    if (line[i] == ' ') {
        argstr = line + i + 1;
    }
    
    // 引数をユーザプログラムが読む場所に複製
    char *p = (char*)USER_ARG_ADDR;
    for (int j = 0; j < 256; j++) p[j] = 0; // 前回のデータを初期化
    if (argstr) {
        strcpy((char*)USER_ARG_ADDR, argstr); // 引数文字列コピー
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
        char *s = (char*)USER_ARG_ADDR; // 複製済みの引数文字列
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
    
    /* 例
    argv[0] → filename[64] の先頭（カーネルスタック上）
    argv[1] → USER_ARG_MEM + 0   （"param1" の先頭）
    argv[2] → USER_ARG_MEM + 7   （"param2" の先頭）
    argv[3] → NULL
    
    char **argv;二重ポインタになっている意味
    argv[i] で i 番目の文字列の先頭アドレス参照
    argv[i][j] で i 番目の文字列の j 文字目参照
    */
                 
    // ファイルを毎回固定でメモリ0x10000上へ展開して実行
    uint32_t size;
    fs_get_file_size(fd, &size); // ユーザプログラムの大きさ取得
    fs_read(fd, (void*)USER_PROG_ADDR, size); // ユーザプログラムをメモリ0x10000上へ展開 (drivers/ata.h)
    fs_close(fd);

    uint32_t esp;
    asm volatile ("mov %%esp, %0" : "=r"(esp));
    debug_printf("ESP = 0x%x\n", esp);
    
    int ret = user_exec((void*)USER_PROG_ADDR, argc, argv);// ユーザプログラムへ遷移 (kernel/user_exec.h)

    asm volatile ("mov %%esp, %0" : "=r"(esp));
    debug_printf("ESP = 0x%x\n", esp);

    //kprintf("ret = %d\n",ret);
    
    return 0;
}

