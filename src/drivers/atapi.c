#include "x86/io.h"
#include "atapi.h"
#include "string.h"
#include "debug/debug_print.h"

#define ATA_DATA    0x170
#define ATA_FEAT    0x171
#define ATA_BC_LOW  0x174
#define ATA_BC_HIGH 0x175
#define ATA_DEV     0x176
#define ATA_CMD     0x177

#define DEV_MASTER  0xA0
#define ST_ERR      0x01
#define ST_DRQ      0x08
#define ST_BSY      0x80

#define CMD_IDENTIFY    0xA1
#define CMD_PACKET      0xA0
#define CMD_READ_10     0x28

#define SECTOR_SIZE 2048

static inline void io_wait(void) {
    outb(0x80, 0);
}

/* 戻り値: 0=DRQ準備完了, -1=ERR または 時間切れ */
static int wait_drq(void) {
    for (int i = 0; i < 1000000; i++) {
        uint8_t s = inb(ATA_CMD);
        if (s & ST_ERR) return -1;
        if (!(s & ST_BSY) && (s & ST_DRQ)) return 0;
        io_wait();
    }
    return -1;
}

struct atapi_dev atapi_detect(void) {
    struct atapi_dev dev = {0};

    outb(ATA_DEV, DEV_MASTER);
    io_wait();
    io_wait();

    outb(ATA_CMD, CMD_IDENTIFY); 
    io_wait();
    io_wait();

    uint8_t st = inb(ATA_CMD);
    if (st == 0x00) {
        return dev;
    }

    dev.present = 1;
    
    wait_drq();

    /* IDENTIFY データ読み込み */
    uint16_t id[256];
    for (int i = 0; i < 256; i++) {
        id[i] = inw(ATA_DATA);  /* ← データポートは 0x170 */
    }

    /* 機種名（word 27–46） */
    int k = 0;
    for (int i = 27; i <= 46; i++) {
        uint16_t w = id[i];
            dev.model[k++] = (w >> 8) & 0xFF;
            dev.model[k++] = w & 0xFF;
    }
    dev.model[k] = '\0';

    return dev;
}

int atapi_read_sector(uint32_t lba, uint8_t *buf)
{
    memset(buf, 0xAA, SECTOR_SIZE);

    /* 1. 待機：BSY も DRQ も落ちていること */
    while (inb(ATA_CMD) & (ST_BSY | ST_DRQ)) {
        io_wait();
    }

    /* 2. レジスタ設定 */
    outb(ATA_FEAT, 0x00);          /* Features = 0 (PIO) */
    outb(ATA_BC_LOW, 0x00);          /* Byte Count Low  = 0x0800 */
    outb(ATA_BC_HIGH, 0x08);          /* Byte Count High = 0x0800 */
    outb(ATA_DEV, 0xA0);          /* マスター。モデル名が取れた値と同じにする */
    io_wait();

    /* 3. PACKET 発行 */
    outb(ATA_CMD, CMD_PACKET);
    io_wait();

    /* 4. コマンド受付：BSY 落ち + DRQ 立ち */
    wait_drq();

    if (!(inb(ATA_CMD) & 0x08))return -1;

    /* 5. CDB 送出（READ(10)） */
    uint8_t cdb[12] = {0};
    cdb[0] = CMD_READ_10;                     /* READ(10) */
    /* cdb[1] = 0; FUA 等なし */
    cdb[2] = (lba >> 24) & 0xff;       /* LBA は 4 バイト */
    cdb[3] = (lba >> 16) & 0xff;
    cdb[4] = (lba >>  8) & 0xff;
    cdb[5] = (lba >>  0) & 0xff;
    cdb[6] = 0;
    cdb[7] = 0;                        /* Transfer Length */
    cdb[8] = 1;
    cdb[9] = 0;                        /* Control */

    for (int i = 0; i < 6; i++) {
        uint16_t w = cdb[i*2] | ((uint16_t)cdb[i*2+1] << 8); /* リトルエンディアン送出 */
        outw(ATA_DATA,w);
    }
    io_wait();

    /* 6. データ転送：BSY 落ち + DRQ 立ち */
    wait_drq();

    if (!(inb(ATA_CMD) & ST_DRQ))return -1;

    /* 7. 実際の転送バイト数を取得 */
    uint16_t limit = inb(ATA_BC_LOW) | (inb(ATA_BC_HIGH) << 8);
    if (limit == 0 || limit > SECTOR_SIZE) limit = SECTOR_SIZE;
    limit &= ~1;

    volatile uint16_t *p = (volatile uint16_t *)buf;
    for (int i = 0; i < limit / 2; i++) p[i] = inw(ATA_DATA);

    return 0;
}
