#include "bootdev.h"
#include "block.h"      /* ata_blkdev */
#include "ata.h"
/* #include "atapi.h" */    /* CD 対応できたら atapi_cd_blkdev */
#include "../lib/stddef.h"       // NULL のため
#include "../lib/stdint.h"


#define VBR_BASE        0x00007c00u
#define ROOTDEV_SIG_OFF 0x1FD
#define ROOTDEV_HDD     'H'
#define ROOTDEV_CD      'C'

static bootdev_type_t cached_type = BOOTDEV_NONE;
static int            cached      = 0;

extern block_dev_t ata_blkdev;

/* VBR の sig を見て種別を決める（一度だけ判定） */
static bootdev_type_t detect(void)
{
    const uint8_t sig = *(const uint8_t *)(VBR_BASE + ROOTDEV_SIG_OFF);
    switch (sig) {
    case ROOTDEV_CD:
        return BOOTDEV_CD;
    default:                /* 不正値も安全側で HDD 扱い */
        return BOOTDEV_HDD;
    }
}

bootdev_type_t bootdev_type(void)
{
    if (!cached) {
        cached_type = detect();
        cached      = 1;
    }
    return cached_type;
}

const char *bootdev_name(void)
{
    switch (bootdev_type()) {
    case BOOTDEV_HDD: return "HDD";
    case BOOTDEV_CD:  return "CD/DVD";
    default:          return "none";
    }
}

/* 「今どの dev を使うか」だけを返す。FS のことは知らない */
block_dev_t *bootdev_get(void)
{
    switch (bootdev_type()) {
    case BOOTDEV_HDD:
        return &ata_blkdev;      /* HDD = ATA 経由 */
    case BOOTDEV_CD:
        /* return &atapi_cd_blkdev; */   /* 開発中：まだ使えない */
        return NULL;
    default:
        return NULL;
    }
}