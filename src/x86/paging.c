#include "lib/stdint.h"

__attribute__((aligned(4096)))
uint32_t kernel_page_dir[1024];

__attribute__((aligned(4096)))
uint32_t kernel_page_tables[1024 * 1024];

inline uint32_t read_cr0(void) {
    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    return cr0;
}

uint32_t virt_to_phys(uint32_t virt_addr)
{
    /* ディレクトリインデックス・テーブルインデックス・オフセットを取り出し */
    uint32_t dir_index   = (virt_addr >> 22) & 0x3FF;
    uint32_t table_index = (virt_addr >> 12) & 0x3FF;
    uint32_t offset      = virt_addr & 0xFFF;

    /* ページディレクトリエントリ取得 */
    uint32_t pde = kernel_page_dir[dir_index];
    if ((pde & 1) == 0) {
        return (uint32_t)-1;   /* 存在しない */
    }

    /* ページテーブルへのポインタ（下位12bitをマスク） */
    uint32_t *pt = (uint32_t *)(pde & 0xFFFFF000);

    /* ページテーブルエントリ取得 */
    uint32_t pte = pt[table_index];
    if ((pte & 1) == 0) {
        return (uint32_t)-1;   /* 存在しない */
    }

    /* 物理アドレス構築 */
    return (uint32_t)((pte & 0xFFFFF000) | offset);
}


void enable_paging(void) {
    // 1024枚のページテーブルを初期化
    for (int dir = 0; dir < 1024; dir++) {
        uint32_t *pt = &kernel_page_tables[dir * 1024];

        for (int tbl = 0; tbl < 1024; tbl++) {
            uint32_t phys = (dir * 1024 * 4096) + (tbl * 4096);
            pt[tbl] = phys | 3;  // P=1, RW=1, US=0
        }

        // ディレクトリエントリに登録
        kernel_page_dir[dir] = ((uint32_t)pt) | 3;  // P=1, RW=1, US=0
    }
    
    asm volatile (
        "mov %0, %%cr3\n\t"
        "mov %%cr0, %%eax\n\t"
        "or  $0x80000000, %%eax\n\t"
        "mov %%eax, %%cr0\n\t"
        : : "r"(kernel_page_dir) : "eax"
    );
    
}


