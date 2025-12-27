//ユーザプログラム動作確認用 syscall不使用版
void main(void) {
    volatile unsigned char* vram = (unsigned char*)0xB8000;
    vram[0] = 'A';      // 文字
    vram[1] = 0x0F;     // 白文字・黒背景
}
