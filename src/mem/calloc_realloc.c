#include "memory.h"

// メモリ確保後0で初期化
void* kcalloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = kmalloc(total);
    
    if (ptr) {
        char* p = (char*)ptr;
        for (size_t i = 0; i < total; i++) {
            p[i] = 0;
        }
    }
    
    return ptr;
}

// 確保したメモリを拡張
// 例）
// char* buffer = kmalloc(100);
// strcpy(buffer, "Hello World");
// buffer = krealloc(buffer, 200); // 100→200へ拡張
void* krealloc(void* ptr, size_t new_size) {
    if (!ptr) {
        return kmalloc(new_size);
    }
    
    if (new_size == 0) { // 0の場合解放
        kfree(ptr);
        return NULL;
    }
    
    // ブロック構造体のアドレス
    struct mem_block* old_block = (struct mem_block*)(
        (char*)ptr - BLOCK_META_SIZE
    );
    
    // 拡張後の大きさが元の大きさ以下の場合はそのまま返す
    if (new_size <= old_block->size) {
        return ptr;
    }
    
    // 拡張後の大きさでメモリ確保
    void* new_ptr = kmalloc(new_size);
    if (!new_ptr) {
        return NULL;
    }
    
    // 新しく確保したブロックに元ブロックのデータを複製
    char* src = (char*)ptr;
    char* dst = (char*)new_ptr;
    for (size_t i = 0; i < old_block->size; i++) {
        dst[i] = src[i];
    }
    
    // 元ブロック解放
    kfree(ptr);
    
    return new_ptr;
}
