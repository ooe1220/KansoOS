#include "user_exec.h"
#include "lib/string.h"
#include "x86/console.h"
#include "lib/stddef.h" // NULLの定義
#include "fs/fat16.h"
#include "fs/fs_file.h"
#include "x86/pic.h"

/* 20261007いつか複数プロセスを導入する時はこの関数を廃止する */

// argc: 引数個数, argv: 引数配列
// 引数を指定しない場合、argc=1, argv[0]=ファイル名
int user_exec(void* entry, int argc, char **argv)
{
    int ret;
    
    //kprintf("user_exec.c argc: %d\n", argc); // test:引数の数

    asm volatile (
        "push %[argv]\n"   // argv のアドレスを push
        "push %[argc]\n"   // argc を push
        "call *%[entry]\n" // エントリポイントを呼び出す
        "add $8, %%esp\n"  // argv、argcの分のスタックを戻す
        : "=a"(ret)        // EAX に返り値を受け取る
        : [entry]"r"(entry),
          [argc]"r"(argc),
          [argv]"r"(argv)
        : "memory"
    );
    
    return ret;
}


/*20261007
objdump -d -M intel build/user_exec.o

00000000 <user_exec>:
   0:	8b 44 24 04          	mov    eax,DWORD PTR [esp+0x4]
   4:	8b 54 24 08          	mov    edx,DWORD PTR [esp+0x8]
   8:	8b 4c 24 0c          	mov    ecx,DWORD PTR [esp+0xc]
   c:	51                   	push   ecx
   d:	52                   	push   edx
   e:	ff d0                	call   eax
  10:	83 c4 08             	add    esp,0x8
  13:	c3                   	ret    
*/

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
    uint32_t size;
    fs_get_file_size(fd, &size); // ユーザプログラムの大きさ取得
    fs_read(fd, (void*)USER_PROG_MEM, size); // ユーザプログラムをメモリ0x10000上へ展開 (drivers/ata.h)
    fs_close(fd);
    
    pic_mask_irq(1); // IRQ1キーボード無効化 (x86/pic.h)
    int ret = user_exec((void*)USER_PROG_MEM, argc, argv);// ユーザプログラムへ遷移 (kernel/user_exec.h)
    //kprintf("ret = %d\n",ret);
    pic_unmask_irq(1); // IRQ1キーボード有効化 (x86/pic.h)
    
    return 0;
}

