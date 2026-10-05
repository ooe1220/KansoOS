#include "stdint.h"
#include "string.h"

#define HEAP_START  0x00200000  // 2MBから開始
#define BLOCK_SIZE  4096        // 1塊 = 4KB
#define BLOCK_COUNT 3072        // 3072塊管理する（約12MB）

// 0 = 空き, 1 = 使用中
static unsigned char heap_state[BLOCK_COUNT];

void heap_init(void) {

    // 管理表を空きにする
    for (int i = 0; i < BLOCK_COUNT; i++) {
        heap_state[i] = 0;  // 全部空きにする
    }
    
    // ヒープ本体を0埋め
    memset((void*)HEAP_START, 0, BLOCK_SIZE * BLOCK_COUNT);
}

void* kmalloc(size_t size) {
    (void)size;  // 今はsize無視(後から拡張)
    for (int i = 0; i < BLOCK_COUNT; i++) {
        if (heap_state[i] == 0) { // 空きの塊を発見
            heap_state[i] = 1;  // 使用中にする
            return (void*)(HEAP_START + (i * BLOCK_SIZE));
        }
    }
    return NULL;  // メモリ不足
}

// 渡されたアドレスが、何番目の塊かを計算して、その塊を空きにする
void kfree(void* ptr) {
    if (!ptr) return;
    unsigned int addr = (unsigned int)ptr;
    int block = (addr - HEAP_START) / BLOCK_SIZE;
    heap_state[block] = 0;  // 空きに戻す
}
