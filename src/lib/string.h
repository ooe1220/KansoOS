#ifndef STRING_H
#define STRING_H

#include "lib/stdint.h"   // size_t を使う場合

/* 文字列長取得 */
size_t strlen(const char *s);

/* 文字列コピー */
char *strcpy(char *dst, const char *src);

/* 文字列連結 */
char *strcat(char *dst, const char *src);

int strcmp(const char *s1, const char *s2);

#endif

