#include "memory.h"

// メモリ分配
void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    // 8の倍数に揃える(例:6→8、15→16)
    size = ALIGN(size);
    
    struct mem_block* block;
    
    // 空きブロックを探す
    block = find_free_block(size);
    
    if (block) { // 使用可能なブロック発見
        // 分割の必要があれば分割
        split_block(block, size);
        block->free = 0; // 使用中
        return (void*)((char*)block + BLOCK_META_SIZE); // ブロック構造体直後(保存領域開始)アドレスを返す
    }
    
    return NULL; // 空きブロック無し
}
