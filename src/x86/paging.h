// paging.h
#ifndef PAGING_H
#define PAGING_H

#include "lib/stdint.h"

// static外してh中でexternつけないと他のファイルから見えない。
extern uint32_t kernel_page_dir[];
extern uint32_t kernel_page_tables[];

uint32_t virt_to_phys(uint32_t virt_addr);

void enable_paging(void);

uint32_t read_cr0(void);

#endif // PAGING_H
