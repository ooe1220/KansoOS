#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SECTOR_SIZE 512

/* BPB（オフセット 11 から）。パックドでそのままマップ */
struct bpb {
    uint8_t  _jmp[3];
    uint8_t  oem[8];
    uint16_t bytes_per_sec;   // 0x0B
    uint8_t  sec_per_clus;    // 0x0D
    uint16_t rsvd_sec;        // 0x0E
    uint8_t  num_fats;        // 0x10
    uint16_t root_ent_cnt;    // 0x11
    uint16_t tot_sec_16;      // 0x13
    uint8_t  media;           // 0x15
    uint16_t fat_sec;         // 0x16
    uint16_t sec_per_trk;     // 0x18
    uint16_t heads;           // 0x1A
    uint32_t hid_sec;         // 0x1C
    uint32_t tot_sec_32;      // 0x20
} __attribute__((packed));

/* ルートディレクトリの 32B エントリ */
struct dirent {
    uint8_t  name[11];
    uint8_t  attr;
    uint8_t  _pad[10];
    uint16_t clus_lo;        // +0x1A
    uint32_t size;           // +0x1C
} __attribute__((packed));

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "rb");

    /* セクタ 0 を読んで BPB をマップ */
    uint8_t sect0[SECTOR_SIZE];
    fread(sect0, 1, SECTOR_SIZE, f);
    struct bpb *b = (struct bpb *)sect0;

    printf("bytes/sec = %u\n", b->bytes_per_sec);
    printf("sec/clus  = %u\n", b->sec_per_clus);
    printf("rsvd      = %u\n", b->rsvd_sec);
    printf("num_fats  = %u\n", b->num_fats);
    printf("fat_sec   = %u\n", b->fat_sec);
    printf("root_ent  = %u\n", b->root_ent_cnt);

    uint32_t root_start = b->rsvd_sec + b->num_fats * b->fat_sec;
    uint32_t root_sec   = (b->root_ent_cnt * 32 + b->bytes_per_sec - 1)
                          / b->bytes_per_sec;
    uint32_t data_start = root_start + root_sec;

    printf("root LBA  = %u\n", root_start);
    printf("data LBA  = %u\n", data_start);

    /* ルートディレクトリのセクタ列をまるごと読む */
    uint8_t *root = malloc(root_sec * SECTOR_SIZE);
    fseek(f, root_start * SECTOR_SIZE, SEEK_SET);
    fread(root, 1, root_sec * SECTOR_SIZE, f);

    printf("\n# name       attr clus size\n");
    for (int i = 0; i < b->root_ent_cnt; i++) {
        struct dirent *e = (struct dirent *)(root + i * 32);

        if (e->name[0] == 0x00) break;          // 空きの終端
        if (e->name[0] == 0xE5) continue;       // 削除済みはスキップ

        char name[12] = {0};
        int j;
        for (j = 0; j < 8 && e->name[j] && e->name[j] != ' '; j++)
            name[j] = e->name[j];
        if (e->name[8] != ' ') {
            name[j++] = '.';
            for (int k = 0; k < 3 && e->name[8+k] && e->name[8+k] != ' '; k++)
                name[j++] = e->name[8+k];
        }
        name[j] = '\0';

        printf("%-11s %02x   %4u %4u\n", name, e->attr, e->clus_lo, e->size);
    }

    return 0;
}