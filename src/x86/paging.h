// paging.h
#ifndef PAGING_H
#define PAGING_H

#include "lib/stdint.h"

uint32_t virt_to_phys(uint32_t virt_addr);

void enable_paging(void);

uint32_t read_cr0(void);

#endif // PAGING_H
