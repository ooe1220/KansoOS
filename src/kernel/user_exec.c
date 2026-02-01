#include "user_exec.h"
#include "x86/console.h"

/*
int user_exec(void* entry)
{
    int ret;

    asm volatile (
        "call *%1"
        : "=a"(ret)   // EAXに入った返り値を受け取りretへ格納
        : "r"(entry)
        : "memory", "cc"
    );

    return ret;
}*/

// argc: 引数個数, argv: 引数配列
// 引数を指定しない場合、argc=1, argv[0]=ファイル名
int user_exec(void* entry, int argc, char **argv)
{
    int ret;
    
    kprintf("user_exec.c argc: %d\n", argc); // test:引数の数

    asm volatile (
        "push %[argv]\n"   // argv のアドレスを push
        "push %[argc]\n"   // argc を push
        "call *%[entry]\n" // エントリポイントを呼び出す
        : "=a"(ret)        // EAX に返り値を受け取る
        : [entry]"r"(entry),
          [argc]"r"(argc),
          [argv]"r"(argv)
        : "memory"
    );
    
    return ret;
}
