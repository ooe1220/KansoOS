#include "lib/os.h"

void main(void) {
    //volatile unsigned char* vram = (unsigned char*)0xB8000;
    //vram[0] = 'A';      // 文字
    //vram[1] = 0x0F;     // 白文字・黒背景
    write("hello from user!\n");
    return;
}
