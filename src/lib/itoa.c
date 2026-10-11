/**
 * 符号付き整数を文字列に変換
 * @param value 入力整数
 * @param buffer 出力文字列バッファ
 */
void itoa(int value, char *buffer) {
    char temp[12];
    int i = 0, j;
    int negative = 0;

    if (value < 0) {
        negative = 1;
        value = -value;
    }

    do {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    } while (value);

    if (negative)
        temp[i++] = '-';

    for (j = 0; j < i; j++)
        buffer[j] = temp[i - j - 1];

    buffer[i] = '\0';
}

void utoa(unsigned int value, char *buffer, int base) {
    char temp[12];
    int i = 0, j;

    do {
        int remainder = value % base;
        temp[i++] = (remainder < 10) ? ('0' + remainder) : ('a' + remainder - 10);
        value /= base;
    } while (value);

    for (j = 0; j < i; j++)
        buffer[j] = temp[i - j - 1];

    buffer[i] = '\0';
}
