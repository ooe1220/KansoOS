#ifndef BOOTDEV_H
#define BOOTDEV_H

#include "block.h" 

typedef enum {
    BOOTDEV_HDD = 0,
    BOOTDEV_CD,
    BOOTDEV_NONE,
} bootdev_type_t;

bootdev_type_t bootdev_type(void);
const char    *bootdev_name(void);
block_dev_t   *bootdev_get(void);

#endif