#ifndef MEM_H
#define MEM_H

#include <stddef.h>

void  uheap_init(void);
void* umalloc(size_t size);
void  ufree(void* ptr);

#endif
