#include "lib/string.h"

/* 文字列長 */
size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

/*
 * 複製
 * 引数:
 *   dst : コピー先の文字列バッファ（先頭アドレス）
 *   src : コピー元の NULL終端文字列（先頭アドレス）
 *
 * 返り値:
 *   dst（コピー先バッファの先頭アドレス）
 */
char *strcpy(char *dst, const char *src) {
    char *ret = dst;
    while (*src != '\0') {
        *dst = *src;
        dst++;
        src++;
    }
    *dst = '\0';
    return ret;
}

/*
 * 連結(dst の文字列の後ろに srcを繋げる)
 * 引数:
 *   dst : 連結先の文字列バッファ（先頭アドレス）
 *         末尾に src を追加できる十分な空きがあること
 *   src : 連結する NULL終端文字列（先頭アドレス）
 *
 * 返り値:
 *   dst（連結先バッファの先頭アドレス）
 */
char *strcat(char *dst, const char *src) {
    char *p = dst + strlen(dst); // pはdst の終端文字 '\0' の位置を指す
    while (*src != '\0') {
        *p = *src;
        p++;
        src++;
    }
    *p = '\0';
    return dst;
}


