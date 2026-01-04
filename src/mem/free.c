#include "memory.h"

// メモリ開放
void kfree(void* ptr) {
    if (!ptr) {
        return;
    }
    
    // 開放するメモリブロック情報の開始アドレスを計算
    struct mem_block* block = (struct mem_block*)(
        (char*)ptr - BLOCK_META_SIZE
    );
    
    // 未使用(1)に設定
    block->free = 1;
    
    // 隣り合う空のブロックがあれば結合
    merge_free_blocks();
}
