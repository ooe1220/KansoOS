typedef unsigned int uint32_t;

/*
  - 1024要素 × 4バイト = 4KB
  - CR3レジスタがこの配列の物理アドレスを指す
  - 配列の各要素が頁表を指す
  - 仮想アドレスの bits 31:22(上位10bit) が索引
  - 1つの頁表が 4KB × 1024要素 = 4MB を担当
  - 1024項目 × 4MB(1頁表当たりの担当) = 4GB（仮想アドレス空間全体）
*/
__attribute__((aligned(4096)))
static uint32_t page_dir[1024];

__attribute__((aligned(4096)))
static uint32_t page_tables[1024 * 1024];  // 1024枚 × 1024項目/4KB

static inline void enable_paging(void) {

    // page_table_0 と page_table_c0 の1024個全て仮想=物理      bit0 : 1で有効    bit1 : 1で書き込み可
    // page_table_0[0]    = 0x00000003   → 仮想0番目 → 物理0x00000000
    // page_table_0[1]    = 0x00001003   → 仮想1番目 → 物理0x00001000
    // page_table_0[2]    = 0x00002003   → 仮想2番目 → 物理0x00002000
    // ...
    // page_table_0[1023] = 0x003FF003   → 仮想1023番目 → 物理0x003FF000
    // 
    // 例)仮想→物理
    // 0x00002003(仮想) = 0000 0000 0000 0010 0000 | 0000 0000 0011
    // 下位12bitを捨てると0x00002003 & 0xFFFFF000 = 0x00002000(物理)
    for (int i = 0; i < 1024; i++) {
        page_table_0[i] = (i * 0x1000) | 3;
        page_table_c0[i] = (i * 0x1000) | 3;
    }

    page_dir[0]   = (uint32_t)page_table_0 | 3;  // 0x00000000 〜 0x003FFFFFの変換に使用
    page_dir[768] = (uint32_t)page_table_c0 | 3; // 0xC0000000 〜 0xC03FFFFFの変換に使用
    for (int i = 1; i < 1024; i++) {
        if (i != 768) page_dir[i] = 0; // 未使用領域は無効化
    }

    /*
      objdump -d -M intel tmp/kernel.o結果
      mov cr3, eax              ; CR3に page_dir のアドレスを格納
      mov eax, cr0              ; CR0のbit31（PGビット）を1にする
      or  eax, 0x80000000       ; 0x80000000 = 1000 0000 0000 0000 0000 0000 0000 0000
      mov cr0, eax
    */
    asm volatile (
        "mov %0, %%cr3\n\t"
        "mov %%cr0, %%eax\n\t"
        "or  $0x80000000, %%eax\n\t"
        "mov %%eax, %%cr0\n\t"
        : : "r"(page_dir) : "eax"
    );
}

static inline uint32_t read_cr0(void) {
    /*
      objdump -d -M intel tmp/kernel.o結果
      mov    eax,cr0
    */
    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    return cr0;
}

void kmain(void) {
    enable_paging();

    uint32_t cr0 = read_cr0();
    int pg_on = (cr0 >> 31) & 1;

    volatile unsigned char *vid = (unsigned char *)0xC00B8000;  // ← 0xB8000 から変える
    
    if (pg_on) {
        const char *msg = "0xC00B8000 OK!";
        int i = 0;
        for (int j = 0; msg[j]; j++) {
            vid[i++] = msg[j];
            vid[i++] = 0x0A;
        }
    } else {
        const char *msg = "PAGING OFF!";
        int i = 0;
        for (int j = 0; msg[j]; j++) {
            vid[i++] = msg[j];
            vid[i++] = 0x04;
        }
    }

    for (;;) asm ("hlt");
}
