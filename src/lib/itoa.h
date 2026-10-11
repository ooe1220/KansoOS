#ifndef LIB_ITOA_H
#define LIB_ITOA_H

/**
 * 符号付き整数を文字列に変換
 * @param value   入力整数
 * @param buffer  出力文字列バッファ（呼び出し側で確保すること）
 */
void itoa(int value, char *buffer);

/**
 * 符号無し整数を指定基数で文字列に変換
 * @param value   入力（符号無し整数）
 * @param buffer  出力文字列バッファ（呼び出し側で確保すること）
 * @param base    基数（2〜16）
 */
void utoa(unsigned int value, char *buffer, int base);

#endif
