#ifndef MEM_H
#define MEM_H

#include <stddef.h>

void  kheap_init(void);
void* kmalloc(size_t size);
void  kfree(void* ptr);

void  uheap_init(void);
void* umalloc(size_t size);
void  ufree(void* ptr);

#endif
