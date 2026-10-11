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
int main(int argc, char **argv) {
    volatile uint32_t* fb = (uint32_t*)0xFD000000;
    
    int width = 800;
    
    // 左上から 100x100 の赤い四角
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            fb[y * width + x] = 0x000000FF;
        }
    }
    
     //return 0;
    while(1);
}
