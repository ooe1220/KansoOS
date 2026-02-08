#include "lib/mystdio.h"
#include "lib/stdint.h"

int main(int argc, char **argv){ 

    if(argc < 2){
        write("Usage: cat <filename>\n");
        return -1;
    }  
    
    uint8_t buf[4096];

    // ファイルを開く
    int fd = open(argv[1]);
    if(fd < 0){
        write("open error\n");
        return -1;
    }

    // 読み込み
    int n = read(fd, buf, sizeof(buf));
    if(n < 0){
        write("read error\n");
        close(fd);
        return -1;
    }

    // 表示
    write((char*)buf);

    // 閉じる
    close(fd);

    return 0;
}

