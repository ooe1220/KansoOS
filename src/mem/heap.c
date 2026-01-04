#include "memory.h"

static struct mem_block* free_list = NULL;
static void* heap_start = (void*)HEAP_START; // ヒープ領域開始アドレス
static void* heap_end = (void*)(HEAP_START + HEAP_SIZE); // ヒープ領域終了アドレス
static void* heap_current = (void*)HEAP_START;

// ヒープ初期化
void heap_init(void) {
    // 初めに生成するブロックは全ヒープ領域を指す
    struct mem_block* first_block = (struct mem_block*)heap_start;
    
    first_block->size = HEAP_SIZE - BLOCK_META_SIZE; // ヒープ全体-ブロック情報=保存領域の大きさ
    first_block->free = 1; // 未使用
    first_block->next = NULL; // ブロックは一つのみ、次ブロック無し
    
    heap_current = (void*)((char*)heap_start + BLOCK_META_SIZE + first_block->size);
    free_list = first_block;
}

// 十分に大きな空きブロックを2つに分割
void split_block(struct mem_block* block, size_t size) {
    if (block->size > size + BLOCK_META_SIZE + 4) {
        // 新ブロック位置計算
        struct mem_block* new_block = (struct mem_block*)(
            (char*)block + BLOCK_META_SIZE + size
        );
        
        // [元:構造体|保存領域] → 次
        // [元:構造体|保存領域] → [新:構造体|保存領域] → 次
        
        // 新ブロック(未使用に設定)
        new_block->size = block->size - size - BLOCK_META_SIZE;
        new_block->free = 1; // 未使用
        new_block->next = block->next; // 「元」が指していた「次」を指す
        
        // 元のブロック調整
        block->size = size;
        block->next = new_block;
    }
}

// 隣り合う未使用ブロックを結合
void merge_free_blocks(void) {
    struct mem_block* curr = free_list;
    
    while (curr && curr->next) {//走査中ブロック及びその次のブロックが存在する間ループ
        if (curr->free && curr->next->free) { // 走査中ブロック及び次のブロックが未使用
            char* curr_end = (char*)curr + BLOCK_META_SIZE + curr->size;
            if (curr_end == (char*)curr->next) {// curr及びcurr->nextが物理的に連続している
            
                // [1:構造体|保存領域] → [2:構造体|保存領域]
                // 1ブロックの大きさに2ブロックの構造体及び保存領域の大きさを足す
                curr->size += BLOCK_META_SIZE + curr->next->size;
                curr->next = curr->next->next;
                continue;
            }
        }
        curr = curr->next;
    }
}

// 適切な大きさのブロックを探す
struct mem_block* find_free_block(size_t size) {
    struct mem_block* curr = free_list;
    
    while (curr) {
        if (curr->free && curr->size >= size) { // 未使用且つ必要な大きさ以上
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}
