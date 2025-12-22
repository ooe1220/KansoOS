/* fs/dir.c */

#include "fat16.h"
#include "../arch/x86/ata.h"
#include "../arch/x86/console.h"

// 8.3形式のファイル名を表示する
static void print_name(const char* name) {
    int i;

    /* ファイル名 */
    for (i = 0; i < 8; i++) {
        if (name[i] == ' ') break;
        kputc(name[i]);
    }

    /* 拡張子 */
    if (name[8] != ' ') {
        kputc('.');
        for (i = 8; i < 11; i++) {
            if (name[i] == ' ') break;
            kputc(name[i]);
        }
    }
}

void fs_dir_list(void) {
    uint8_t buf[512];
    fat_dirent_t* ent;
    int s, i;

    for (s = 0; s < ROOT_DIR_SECTORS; s++) {
        ata_read_sector(ROOT_DIR_LBA + s, buf);
        ent = (fat_dirent_t*)buf;

        for (i = 0; i < 512 / sizeof(fat_dirent_t); i++) {
        
            /* 終端 */
            if (ent[i].name[0] == 0x00)
                return;

            /* 削除済み */
            if (ent[i].name[0] == 0xE5)
                continue;

            /* Volume label / system */
            if (ent[i].attr & 0x08)
                continue;

            print_name(ent[i].name);
            
            if(i != i < 512 / sizeof(fat_dirent_t) - 1){ // 最後のファイルの後は改行しない
                kputs("\n");
            }
        }
    }
}

