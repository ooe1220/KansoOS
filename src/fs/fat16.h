#pragma once
#include <stdint.h>

/* 固定ディスク配置 */
#define FAT_BASE_LBA        64
#define FAT_COUNT          2
#define SECTORS_PER_FAT    1
#define ROOT_ENTRIES       32
#define ROOT_DIR_SECTORS   2

#define ROOT_DIR_LBA (FAT_BASE_LBA + FAT_COUNT)

#pragma pack(push,1)
typedef struct {
    char name[11];
    uint8_t attr;
    uint8_t ntres;
    uint8_t crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t acc_date;
    uint16_t clus_hi;
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t clus_lo;
    uint32_t size;
} fat_dirent_t;
#pragma pack(pop)

/* dir / ls */
void fs_dir_list(void);

