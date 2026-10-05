#include "stdint.h"
#include "string.h"

// ============================================
// カーネルヒープ
// ============================================
#define KHEAP_START  0x00200000  // 2MBから開始
#define KBLOCK_SIZE  4096        // 1塊 = 4KB
#define KBLOCK_COUNT 3072        // 3072塊管理する（約12MB）

// 0 = 空き, 1 = 使用中
static unsigned char kheap_state[KBLOCK_COUNT];

void kheap_init(void) {

    // 管理表を空きにする
    for (int i = 0; i < KBLOCK_COUNT; i++) {
        kheap_state[i] = 0;  // 全部空きにする
    }
    
    // ヒープ本体を0埋め
    memset((void*)KHEAP_START, 0, KBLOCK_SIZE * KBLOCK_COUNT);
}

void* kmalloc(size_t size) {
    (void)size;  // 今はsize無視(後から拡張)
    for (int i = 0; i < KBLOCK_COUNT; i++) {
        if (kheap_state[i] == 0) { // 空きの塊を発見
            kheap_state[i] = 1;  // 使用中にする
            return (void*)(KHEAP_START + (i * KBLOCK_SIZE));
        }
    }
    return NULL;  // メモリ不足
}

// 渡されたアドレスが、何番目の塊かを計算して、その塊を空きにする
void kfree(void* ptr) {
    if (!ptr) return;
    unsigned int addr = (unsigned int)ptr;
    int block = (addr - KHEAP_START) / KBLOCK_SIZE;
    kheap_state[block] = 0;  // 空きに戻す
}


// ============================================
// ユーザヒープ
// ============================================
#define UHEAP_START  0x00600000  // 2MBから開始
#define UKBLOCK_SIZE  4096        // 1塊 = 4KB
#define UBLOCK_COUNT 256        // 256塊管理する

// 0 = 空き, 1 = 使用中
static unsigned char uheap_state[UBLOCK_COUNT];

void uheap_init(void) {

    // 管理表を空きにする
    for (int i = 0; i < UBLOCK_COUNT; i++) {
        uheap_state[i] = 0;  // 全部空きにする
    }
    
    // ヒープ本体を0埋め
    memset((void*)UHEAP_START, 0, UKBLOCK_SIZE * UBLOCK_COUNT);
}

void* umalloc(size_t size) {
    (void)size;  // 今はsize無視(後から拡張)
    for (int i = 0; i < UBLOCK_COUNT; i++) {
        if (uheap_state[i] == 0) { // 空きの塊を発見
            uheap_state[i] = 1;  // 使用中にする
            return (void*)(UHEAP_START + (i * UKBLOCK_SIZE));
        }
    }
    return NULL;  // メモリ不足
}

// 渡されたアドレスが、何番目の塊かを計算して、その塊を空きにする
void ufree(void* ptr) { // 後から範囲外freeの例外を追加予定
    if (!ptr) return;
    unsigned int addr = (unsigned int)ptr;
    int block = (addr - UHEAP_START) / UKBLOCK_SIZE;
    uheap_state[block] = 0;  // 空きに戻す
}
