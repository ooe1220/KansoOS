#include "user_exec.h"
#include "lib/string.h"
#include "x86/console.h"
#include "lib/stddef.h" // NULLの定義
#include "fs/fat16.h"
#include "fs/fs_file.h"
#include "x86/pic.h"
#include "debug/serial.h"
#include "debug/serial_print.h"

/*
int user_exec(void* entry, int argc, char **argv)
{
    // ユーザースタックの準備
    // 0x20000 をユーザースタックトップとする
    uint32_t *ustack = (uint32_t *)0x20000;
    uint32_t esp = (uint32_t)ustack;
    
    // スタックを8バイト境界へ
    esp = esp & ~7;
    
    // ユーザースタックに argv の文字列をコピー
    // （argv[i] が指す先がカーネル側のアドレスだと
    //   Ring 3 から見えないので、スタック上に配置する）
    char *arg_strings[argc];
    for (int i = 0; i < argc; i++) {
        int len = strlen(argv[i]) + 1;
        esp -= len;
        strcpy((char*)esp, argv[i]);
        arg_strings[i] = (char*)esp;
    }
    
    // 8バイトアライン再調整
    esp = esp & ~7;
    
    // argv 配列をスタックに積む
    esp -= (argc + 1) * 4;
    uint32_t *argv_on_stack = (uint32_t*)esp;
    for (int i = 0; i < argc; i++) {
        argv_on_stack[i] = (uint32_t)arg_strings[i];
    }
    argv_on_stack[argc] = 0;  // NULL終端
    
    // argc をスタックに積む
    esp -= 4;
    *(uint32_t*)esp = argc;
    
    // 戻り先アドレス（ダミー、システムコールで終了する想定）
    esp -= 4;
    *(uint32_t*)esp = 0x00000000;  // ret addr = 0（落ちるように）
    
    uint32_t user_entry = (uint32_t)entry;
    
    // iret で Ring 3 へ
    asm volatile (
        // データセグメントをユーザー用に
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        
        // ユーザースタック設定
        "mov %[esp], %%ebx\n"
        "mov %%ebx, %%esp\n"
        
        // iret フレーム
        "push $0x23\n"          // SS
        "push %%ebx\n"          // ESP
        "pushf\n"               // EFLAGS
        "pop %%eax\n"
        "or $0x200, %%eax\n"    // IF=1
        "push %%eax\n"          // EFLAGS
        "push $0x1B\n"          // CS
        "push %[entry]\n"       // EIP
        "iret\n"
        :
        : [esp]"r"(esp), [entry]"r"(user_entry)
        : "eax", "ebx", "memory"
    );
    
    // ここには戻ってこない
    return -1;
}
*/



/* 20261007いつか複数プロセスを導入する時はこの関数を廃止する */

// argc: 引数個数, argv: 引数配列
// 引数を指定しない場合、argc=1, argv[0]=ファイル名
int user_exec(void* entry, int argc, char **argv)
{
    int ret;
    
    //debug_dump_mem((uint32_t)entry, 512); // 実行ファイルが読み込まれているか
    
    //kprintf("user_exec.c argc: %d\n", argc); // test:引数の数
    
    uint32_t esp;
    asm volatile ("mov %%esp, %0" : "=r"(esp));
    serial_print("ESP = 0x%x\n", esp);

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
    
    asm volatile ("mov %%esp, %0" : "=r"(esp));
    serial_print("ESP = 0x%x\n", esp);

    
    return ret;
}

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
    fs_read(fd, (void*)USER_PROG_MEM, size); // ユーザプログラムをメモリ0x10000上へ展開 (drivers/ata.h)
    fs_close(fd);
    
    pic_mask_irq(1); // IRQ1キーボード無効化 (x86/pic.h)
    int ret = user_exec((void*)USER_PROG_MEM, argc, argv);// ユーザプログラムへ遷移 (kernel/user_exec.h)
    //kprintf("ret = %d\n",ret);
    pic_unmask_irq(1); // IRQ1キーボード有効化 (x86/pic.h)
    
    return 0;
}

