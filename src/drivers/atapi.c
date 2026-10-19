#include "x86/io.h"
#include "atapi.h"
#include "debug/debug_print.h"

static inline void io_wait(void) {
    outb(0x80, 0);
}

struct atapi_dev atapi_detect_primary_master(void) {
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
    debug_printf("ATAPI PRESENT on SECONDARY, st=0x%x", st);

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
