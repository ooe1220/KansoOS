#include "lib/mystdio.h"
#include "lib/stdint.h"

int main(int argc, char **argv){   
    //検証：syscallを直接呼ぶ
    
    /*
    uint8_t buf[8*512];  // 1クラスタ分バッファ
    
    uint32_t lba = 1838;
    for(int i=0;i<8;i++)
        read_sector(lba+i, buf + i*512);

    write(buf);
    */
    
    uint8_t buf[4096];

    // ファイルを開く
    int fd = open("TEST.TXT");
    if(fd < 0){
        write("open error\n");
        return -1;
    }
    
    printf_d("test4 %d",fd);

    // 読み込み
    int n = read(fd, buf, sizeof(buf));
    if(n < 0){
        write("read error\n");
        close(fd);
        return -1;
    }
    
    //printf_d("n = %d",n);

    // 表示
    write((char*)buf);

    // 閉じる
    close(fd);


    return 0;
}

