#include "lib/mystdio.h"
#include "lib/stdint.h"

int main(int argc, char **argv){   
    //検証：syscallを直接呼ぶ
    
    uint8_t buf[8*512];  // 1クラスタ分バッファ
    
    uint32_t lba = 1838;
    for(int i=0;i<8;i++)
        read_sector(lba+i, buf + i*512);

    write(buf);

    return 0;
}

