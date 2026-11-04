#pragma once
#include <stdint.h>

extern uint32_t g_data_lba;
extern uint8_t  g_sec_per_clus;

#define VBR_ADDR ((const bpb_t*)0x00007c00u)

#pragma pack(push,1)
typedef struct {
    uint8_t  jmp[3];
    char     oem[8];
    uint16_t bytes_per_sec;   // 0x0B
    uint8_t  sec_per_clus;    // 0x0D
    uint16_t reserved_sec;    // 0x0E
    uint8_t  num_fats;        // 0x10
    uint16_t root_ent_cnt;    // 0x11
    uint16_t tot_sec16;       // 0x13
    uint8_t  media;           // 0x15
    uint16_t fat_size16;      // 0x16
    uint16_t sec_per_trk;     // 0x18
    uint16_t num_heads;       // 0x1A
    uint32_t hidden_sec;      // 0x1C
    uint32_t tot_sec32;       // 0x20
} bpb_t;
#pragma pack(pop)

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

void fat16_init(void);

/* dir / ls */
void fs_dir_list(void);

// ファイル名を8.3形式に変換　例）"test2.bin"→"TEST2   BIN"
void to_83_format(const char* filename, char* result);

// ファイルを検索して情報を取得
int fat16_find_file(const char* filename, uint32_t* start_cluster, uint32_t* file_size);

void fs_dir_list(void);