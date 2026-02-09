// syscall.c
#include "console.h"
#include "idt.h"
#include "lib/stdint.h"
#include "drivers/ata.h"
#include "../fs/fs_file.h"

// -------------------------
// システムコール用関数（ハンドラ）
// INT 0x80 割り込みで呼ばれる
// 各システムコール番号に応じた処理に分岐する
// -------------------------
void syscall_handler(void);

// -------------------------
// システムコール初期化
// 0x80 割り込みに syscall_handler を登録
// 以降、INT 0x80 でユーザ空間からシステムコールが呼べる
// -------------------------
void init_syscall(void) {
    idt_set_gate(0x80, (uint32_t)syscall_handler);
}

// -------------------------
// システムコール: 書き込み処理
// 画面に文字列を出力
// -------------------------
uint32_t handle_write(const char *str) {

    /* ==== 確認用：VRAMに 'C' を出す ==== */
    //volatile unsigned char* vram = (unsigned char*)0xB8000;
    //vram[4] = 'C';      // 文字
    //vram[5] = 0x0F;     // 白文字・黒背景
    /* ================================== */
    
    if (str) {
        kputs(str);  // カーネルの画面出力関数
    }
    return 0;
}

// ユーザの権限には不相応　今後削除予定
int handle_read_sector(uint32_t lba, uint8_t* buffer) {
    return ata_read_lba28(lba, 1, buffer); // 1セクタ読み込み
}

int handle_fs_open(const char* filename)
{
    return fs_open(filename);  // カーネル内部の fs_open を呼ぶ
    //int fd = fs_open(filename);
    //kprintf("handle_fs_open fd=%d ",fd);
    //return fd;
}

int handle_fs_read(int fd, void* buf, int size)
{
    return fs_read(fd, buf, size);  // カーネル内部の fs_read を呼ぶ
}

int handle_fs_close(int fd)
{
    return fs_close(fd);  // カーネル内部の fs_close を呼ぶ
}




