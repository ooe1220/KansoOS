#include "command.h"
#include "x86/console.h"
#include "x86/io.h"
#include "lib/string.h"
#include "lib/stdint.h"
#include "fs/dir.h"
#include "user_exec.h"
#include "x86/ata.h"
#include "../fs/fat16.h"
#include "x86/pic.h"


// 文字を大文字に変換（自作）
static char to_upper_char(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c;
}

// ファイル名を8.3形式に変換　例）"test2.bin"→"TEST2   BIN"
static void to_83_format(const char* filename, char* result) {
    int i, j = 0;
    
    // ファイル名部分（8文字）
    for (i = 0; i < 8 && filename[i] && filename[i] != '.'; i++) {
        result[j++] = to_upper_char(filename[i]);
    }
    
    // ファイル名空白埋め
    while (j < 8) {
        result[j++] = ' ';
    }
    
    // 拡張子部分 - 修正箇所
    int ext_found = 0;
    for (i = 0; filename[i]; i++) {
        if (filename[i] == '.') {
            ext_found = 1;
            i++; // '.'を跳ばす
            break;
        }
    }
    
    // 拡張子をコピー（最大3文字）
    if (ext_found) {
        for (int k = 0; k < 3 && filename[i]; k++, i++) {
            result[j++] = to_upper_char(filename[i]);
        }
    }
    
    // 拡張子空白埋め
    while (j < 11) {
        result[j++] = ' ';
    }
    
    result[11] = '\0';
}

// 文字列比較関数
static int string_compare(const char* s1, const char* s2, int length) {
    int i;
    for (i = 0; i < length; i++) {
        if (s1[i] != s2[i]) {
            return 0;
        }
    }
    return 1;
}

// ファイルを検索して情報を取得
static int find_file_info(const char* filename, uint32_t* start_cluster, uint32_t* file_size) {
    uint8_t buf[512];
    fat_dirent_t* ent;
    char target_name[12];
    int s, i;
    
    to_83_format(filename, target_name); // target_name=8.3変換後
    
    for (s = 0; s < ROOT_DIR_SECTORS; s++) {
        ata_read_sector(ROOT_DIR_LBA + s, buf);
        ent = (fat_dirent_t*)buf;

        for (i = 0; i < 512 / sizeof(fat_dirent_t); i++) {
            /* 終端または削除済み要素は跳ばす */
            if (ent[i].name[0] == 0x00 || ent[i].name[0] == 0xE5)
                continue;

            /* Volume label/systemは跳ばす */
            if (ent[i].attr & 0x08)
                continue;

            /* ファイル名比較 */
            if (string_compare(ent[i].name, target_name, 11)) {
                if (start_cluster) *start_cluster = ent[i].clus_lo | (ent[i].clus_hi << 16);
                if (file_size) *file_size = ent[i].size;
                return 1; // ファイルが見つかった
            }
        }
    }
    
    return 0; // ファイルが見つからない
}

void execute_command(const char *line) {
    if (strcmp(line, "help") == 0) {
        kputs("\nAvailable commands:\n");
        kputs("  help   - Show this message\n");
        kputs("  clear  - Clear screen\n");
        kputs("  reboot - Reboot CPU\n");
    } else if (strcmp(line, "clear") == 0) {
        console_clear();
    } else if (strcmp(line, "reboot") == 0) {
        outb(0x64, 0xFC);
    } else if (strcmp(line, "ls") == 0 || strcmp(line, "dir") == 0) {
        kputs("\n");
        fs_dir_list();
    } else if (line[0] != 0) {
        // 空行でなければ、それをファイル名として扱う
        uint32_t start_cluster, file_size;
        kputs("\n");
        
        if (find_file_info(line, &start_cluster, &file_size)) {            
            // ファイルを毎回固定でメモリ0x10000上へ展開して実行
            // 1クラスタ=8セクタ FAT表未実装により、固定で8セクタを読み込む
            ata_read_lba28(1800, 8, (void*)0x10000); // 後から1800→start_clusterへ変更予定
            pic_mask_irq(1); // IRQ1キーボード無効化
            user_exec((void*)0x10000);
            pic_unmask_irq(1); // IRQ1キーボード有効化
        } else {
            kputs("Unknown command or file not found: ");
            kputs(line);
        }
    }
}



