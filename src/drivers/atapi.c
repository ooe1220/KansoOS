#include "x86/io.h"
#include "atapi.h"
#include "string.h"
#include "debug/debug_print.h"

static inline void io_wait(void) {
    outb(0x80, 0);
}

struct atapi_dev atapi_detect_slave(void) {
    struct atapi_dev dev = {0};

    /* Secondary Master に固定 */
    outb(0x176, 0xA0);
    io_wait();
    io_wait();

    outb(0x177, 0xA1);  /* ATAPI IDENTIFY */
    io_wait();
    io_wait();

    uint8_t st = inb(0x177);
    if (st == 0x00) {
        debug_printf("NOTHING on Secondary");
        return dev;
    }

    dev.present = 1;
    //debug_printf("ATAPI PRESENT on SECONDARY, st=0x%x", st);
    
   while (inb(0x177) & 0x80); // BSY
   
   while (!(inb(0x177) & 0x08)); // DRQ


    /* IDENTIFY データ読み込み */
    uint16_t id[256];
    for (int i = 0; i < 256; i++) {
        id[i] = inw(0x170);  /* ← データポートは 0x170 */
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



#define D 0x170
#define F 0x171
#define M 0x174   /* Byte Count Low  */
#define H 0x175   /* Byte Count High */
#define V 0x176
#define C 0x177

static uint8_t wait_clr(uint8_t m) { uint8_t s; while ((s=inb(C))&m) io_wait(); return s; }

int atapi_read_sector(uint32_t lba, uint8_t *buf)
{
    memset(buf, 0xAA, 2048);

    debug_printf("[1] pre  st=%x", inb(C));

    /* 1. 待機：BSY も DRQ も落ちていること */
    wait_clr(0x80 | 0x08);
    debug_printf("[2] idle st=%x", inb(C));

    /* 2. レジスタ設定 */
    outb(F, 0x00);          /* Features = 0 (PIO) */
    outb(M, 0x00);          /* Byte Count Low  = 0x0800 */
    outb(H, 0x08);          /* Byte Count High = 0x0800 */
    outb(V, 0xA0);          /* マスター。モデル名が取れた値と同じにする */
    io_wait();

    /* 3. PACKET 発行 */
    outb(C, 0xA0);
    io_wait();

    /* 4. コマンド受付フェーズ：BSY 落ち + DRQ 立ち */
    for (int i = 0; i < 1000000; i++) {
        uint8_t s = inb(C);
        if (s & 0x01) {
            debug_printf("[3] ERR st=%x err=%x", s, inb(F));
            return -1;
        }
        if (!(s & 0x80) && (s & 0x08)) {
            debug_printf("[3] CDB-ready st=%x bcl=%x%x",
                         s, inb(H), inb(M));
            break;
        }
        io_wait();
    }
    if (!(inb(C) & 0x08)) {
        debug_printf("[3] NO CDB DRQ st=%x", inb(C));
        return -1;
    }

/* 5. CDB 送出（READ(10)） */
uint8_t cdb[12] = {0};
cdb[0] = 0x28;                     /* READ(10) */
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
    outw(D, w);
}
io_wait();
    debug_printf("[4] cdb sent, st=%x", inb(C));

    /* 6. データ転送フェーズ：BSY 落ち + DRQ 立ち */
    for (int i = 0; i < 1000000; i++) {
        uint8_t s = inb(C);
        if (s & 0x01) {
            debug_printf("[5] ERR st=%x err=%x", s, inb(F));
            return -1;
        }
        if (!(s & 0x80) && (s & 0x08)) {
            debug_printf("[5] DATA-ready st=%x bcl=%x%x",
                         s, inb(H), inb(M));
            break;
        }
        io_wait();
    }
    if (!(inb(C) & 0x08)) {
        debug_printf("[5] NO DATA DRQ st=%x", inb(C));
        return -1;
    }

    /* 7. 実際の転送バイト数を取得 */
    uint16_t limit = inb(M) | (inb(H) << 8);
    debug_printf("[6] limit=%x", limit);
    if (limit == 0 || limit > 2048) limit = 2048;
    limit &= ~1;

    /* 8. 先頭 1 ワードだけ試し読み */
    uint16_t first = inw(D);
    debug_printf("[7] FIRST=%x", first);

    volatile uint16_t *p = (volatile uint16_t *)buf;
    p[0] = first;
    for (int i = 1; i < limit / 2; i++) p[i] = inw(D);

    debug_printf("[8] done head=%x %x %x %x",
                 buf[0], buf[1], buf[2], buf[3]);
    return 0;
}
