#ifndef ARCH_DETECT_H
#define ARCH_DETECT_H

// 実機で動いているか
// 戻り値: 0 = エミュレータ（仮）, 非0 = 実機
int is_bare_metal(void);

#endif
