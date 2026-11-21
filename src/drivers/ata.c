#include "ata.h"
#include "x86/io.h"
#include "../x86/console.h" // debug

#define HPC 16   // ヘッド数
#define SPT 63   // セクタ/トラック

/* BUSY待ち */
static int ata_wait_ready(void) {
    while (1) {
        uint8_t s = inb(0x1F7);
        if (s == 0xFF) return -1;          // デバイス無ければ抜ける
        if (!(s & 0x80) && (s & 0x40)) return 0; // !BSY && DRDY
    }
}

static int ata_wait_drq(void) {
    for (int t = 0; t < 1000000; t++) {
        uint8_t s = inb(0x1F7);
        if (s & 0x80)       continue;    // BSY中は全ビット無効
        if (s & 0x20)       return -2;    // DF
        if (s & 0x08)       return 0;     // DRQ 成立 → 優先
        if (s & 0x01)       return -1;    // DRQなしのERRだけが真のエラー
    }
    return -3;
}

int ata_read_chs(uint32_t lba, uint8_t sector_cnt, void* buffer) {
    if (sector_cnt == 0) return -1;
    //kprintf("chs cnt=%d\n", sector_cnt);

    uint8_t  sec  = (lba % SPT) + 1;
    uint32_t tmp  = lba / SPT;
    uint8_t  head = tmp % HPC;
    uint16_t cyl  = tmp / HPC;

    uint16_t* buf = (uint16_t*)buffer;

    if (ata_wait_ready() != 0) return -1;
    outb(0x1F6, 0xA0 | (head & 0x0F));
    inb(0x3F6); inb(0x3F6); inb(0x3F6); inb(0x3F6);

    if (ata_wait_ready() != 0) return -1;
    outb(0x1F2, sector_cnt);           // ★ 可变化
    outb(0x1F3, sec);
    outb(0x1F4, cyl & 0xFF);
    outb(0x1F5, cyl >> 8);
    outb(0x1F7, 0x20);
    inb(0x3F6); inb(0x3F6); inb(0x3F6); inb(0x3F6);

    for (int s = 0; s < sector_cnt; s++) {
        int r = ata_wait_drq();
        if (r != 0) {
            kprintf("[ATA CHS] ERR status=0x%x err=0x%x\n", inb(0x1F7), inb(0x1F1));
            return -1;
        }
        for (int i = 0; i < 256; i++) buf[i] = inw(0x1F0);
        buf += 256;
    }
    return 0;
}


/* LBA28読み込み（1〜255セクタ） */
int ata_read_lba28(uint32_t lba, uint8_t sector_cnt, void* buffer) {
    uint16_t* buf = (uint16_t*)buffer;

    //kprintf("[ATA] lba=%u cnt=%d sel=0x%x st=0x%x err=0x%x\n",
    //                            lba, sector_cnt, 0xE0 | ((lba >> 24) & 0x0F),inb(0x1F7), inb(0x1F1));
    //kprintf("lba");
    if(ata_wait_ready()!=0)return -1;

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); // ドライブ選択
    inb(0x3F6); inb(0x3F6); inb(0x3F6); inb(0x3F6);

     if(ata_wait_ready()!=0)return -1;

    outb(0x1F2, sector_cnt);                   // セクタ数
    outb(0x1F3, lba & 0xFF);                   // LBA低8bit
    outb(0x1F4, (lba >> 8) & 0xFF);            // LBA中8bit
    outb(0x1F5, (lba >> 16) & 0xFF);           // LBA高8bit
    outb(0x1F7, 0x20);                         // READ SECTORS コマンド

    inb(0x3F6); inb(0x3F6); inb(0x3F6); inb(0x3F6);

    for (int s = 0; s < sector_cnt; s++) {

        int r = ata_wait_drq();
        if (r < 0){
            if (r == -1)kprintf("[ATA Driver] device ERR, status=0x%x  err=0x%x \n",inb(0x1F7),inb(0x1F1));
            else if (r == -2) kputs("[ATA Driver] drive fault (DF)\n");
            else              kputs("[ATA Driver] DRQ timeout \n");
            return -1;
        }

        for (int i = 0; i < 256; i++) { // 16bit(1wordずつ読み込む) 1セクタ(512バイト)=256ワード
            buf[i] = inw(0x1F0);
        }
        buf += 256;
         if(ata_wait_ready()!=0)return -1;
    }

    return 0;
}

/* LBA28 書き込み（1〜255セクタ） */
int ata_write_lba28(uint32_t lba, uint8_t sector_cnt, void* buffer) {
    const uint16_t* buf = (const uint16_t*)buffer;

    ata_wait_ready();

    // ドライブ選択
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, sector_cnt);           // セクタ数
    outb(0x1F3, lba & 0xFF);           // LBA低8bit
    outb(0x1F4, (lba >> 8) & 0xFF);    // LBA中8bit
    outb(0x1F5, (lba >> 16) & 0xFF);   // LBA高8bit
    outb(0x1F7, 0x30);                 // WRITE SECTORS コマンド

    for (int s = 0; s < sector_cnt; s++) {
        if (ata_wait_drq() < 0)
            return -1;

        for (int i = 0; i < 256; i++) { // 1セクタ512バイト=256ワード
            outw(0x1F0, buf[i]);
        }
        buf += 256;
    }

    // 書き込み完了を待つ
    ata_wait_ready();

    return 0;
}
/* 1セクタ読み込み */
int ata_read_sector(uint32_t lba, void* buffer) {
    return ata_read_lba28(lba, 1, buffer);
    //return ata_read_chs(lba, buffer);
}

/* 1セクタ書き込み */
int ata_write_sector(uint32_t lba, void* buffer) {
    return ata_write_lba28(lba, 1, buffer);
}

