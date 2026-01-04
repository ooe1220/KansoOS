// malloc.h
#ifndef _MALLOC_H
#define _MALLOC_H

#include "../lib/stdint.h"  // for size_t

#ifndef NULL
#define NULL ((void*)0)
#endif

// アドレス8倍数に揃える
#define ALIGN_SIZE 8
#define ALIGN(size) (((size) + (ALIGN_SIZE-1)) & ~(ALIGN_SIZE-1))

// メモリブロック構造体
typedef struct mem_block {
    size_t size;            // 大きさ
    int free;               // 1=空，0=使用中
    struct mem_block* next; // 次のブロック
} mem_block;

#define BLOCK_META_SIZE sizeof(mem_block)
#define MIN_BLOCK_SIZE 16

// ヒープ開始アドレス及び大きさ
#define HEAP_START 0x200000  // 2MB〜
#define HEAP_SIZE   (4 * 1024 * 1024)  // ヒープの大きさは4MB

void* kmalloc(size_t size);
void kfree(void* ptr);
void* kcalloc(size_t num, size_t size);
void* krealloc(void* ptr, size_t size);

struct mem_block* find_free_block(size_t size);
void split_block(struct mem_block* block, size_t size);
void merge_free_blocks(void);

#endif
