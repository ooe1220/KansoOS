#include "ata.h"
#include "x86/io.h"

/* BUSY待ち */
static void ata_wait_busy(void) {
    while (inb(0x1F7) & 0x80);  // 0x80 = 1000 0000b bit7=1:ディスクがまだ処理中
}

/* DRQ待ち */
static int ata_wait_drq(void) {
    uint8_t s;
    while (1) {
        s = inb(0x1F7);
        if (s & 0x01) return -1; // bit0=1:エラー発生
        if (s & 0x08) return 0;
    }
}

/* LBA28読み込み（1〜255セクタ） */
int ata_read_lba28(uint32_t lba, uint8_t sector_cnt, void* buffer) {
    uint16_t* buf = (uint16_t*)buffer;

    ata_wait_busy();

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); // ドライブ選択
    outb(0x1F2, sector_cnt);                   // セクタ数
    outb(0x1F3, lba & 0xFF);                   // LBA低8bit
    outb(0x1F4, (lba >> 8) & 0xFF);            // LBA中8bit
    outb(0x1F5, (lba >> 16) & 0xFF);           // LBA高8bit
    outb(0x1F7, 0x20);                         // READ SECTORS コマンド

    for (int s = 0; s < sector_cnt; s++) {
        if (ata_wait_drq() < 0)
            return -1;

        for (int i = 0; i < 256; i++) { // 16bit(1wordずつ読み込む) 1セクタ(512バイト)=256ワード
            buf[i] = inw(0x1F0);
        }
        buf += 256;
    }

    return 0;
}

/* LBA28 書き込み（1〜255セクタ） */
int ata_write_lba28(uint32_t lba, uint8_t sector_cnt, void* buffer) {
    const uint16_t* buf = (const uint16_t*)buffer;

    ata_wait_busy();

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
    ata_wait_busy();

    return 0;
}
/* 1セクタ読み込み */
int ata_read_sector(uint32_t lba, void* buffer) {
    return ata_read_lba28(lba, 1, buffer);
}

/* 1セクタ書き込み */
int ata_write_sector(uint32_t lba, void* buffer) {
    return ata_write_lba28(lba, 1, buffer);
}

