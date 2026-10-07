#include "user_exec.h"
#include "x86/console.h"

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
