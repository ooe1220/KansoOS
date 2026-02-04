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

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {  // 両方の文字が同じなら進む
        s1++;
        s2++;
    }
    // 最初に違う文字が出た時の差を返す
    return (uint8_t)(*s1) - (uint8_t)(*s2);
}

// 文字列比較関数
int memcmp(const char* s1, const char* s2, int length) {
    int i;
    for (i = 0; i < length; i++) {
        if (s1[i] != s2[i]) {
            return 0;
        }
    }
    return 1;
}

