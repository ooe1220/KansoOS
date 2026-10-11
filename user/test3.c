#include "lib/mystdio.h"

/*
    printf_d("test3 argc = %d\n", argc);
    
    int i=0;
    for(;i<argc;i++){
        write(argv[i]);
    }
    return 0;
}
*/    

//ユーザプログラム動作確認用 syscall不使用版
int main(int argc, char **argv){
    volatile unsigned char* vram = (unsigned char*)0xB8000;
    vram[0] = 'A';      // 文字
    vram[1] = 0x0F;     // 白文字・黒背景
}

