#include "x86/io.h"
#include "atapi.h"
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

/* LBA 指定で 1セクタ読む（PIO） */
int atapi_read_sector(uint32_t lba, uint8_t *buf) {
    /* 1. Device Select (Secondary Master) */
    outb(0x176, 0xA0);
    io_wait();
    io_wait();

    /* 2. READ(10) Command Packet (12 bytes) */
uint8_t cmd[12] = {
    0xA8,
    0x00,
    (lba >> 24) & 0xFF,   /* Big Endian のまま */
    (lba >> 16) & 0xFF,
    (lba >>  8) & 0xFF,
    (lba >>  0) & 0xFF,
    0x00,
    0x00,
    0x00, 0x01,           /* 1 sector */
    0x00
};

    for (int i = 0; i < 12; i++) {
        outb(0x170, cmd[i]);
        io_wait();
    }

    /* 3. Wait DRQ */
    int timeout = 100000;
    while (!(inb(0x177) & 0x08) && timeout--) {
        io_wait();
    }

    if (timeout == 0) {
        debug_printf("ATAPI DRQ TIMEOUT");
        return -1;
    }
    
uint8_t st = inb(0x177);
debug_printf("STATUS=%x", st);
uint16_t v = inw(0x170);

while (!(inb(0x177) & 0x08));

    /* 4. Read 2048 bytes */
    uint16_t *p = (uint16_t *)buf;
    for (int i = 0; i < 1024; i++) {
        *p++ = inw(0x170);
        io_wait();
        //kputc((char)((uintptr_t)p));
    }

    return 0;
}

