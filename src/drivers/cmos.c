// cmos.c
#include "cmos.h"
#include "x86/io.h"  // inb/outb

static uint8_t cmos_read(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

//  BCD (Binary-Coded Decimal) を通常の2進数に変換
// 上位4bit: 10の位 下位4bit: 1の位    
static uint8_t bcd_to_bin(uint8_t val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}

// 現在時刻をstruct RTCに読み込む
void cmos_read_rtc(struct RTC* rtc) {
    rtc->second = bcd_to_bin(cmos_read(0x00));  // 秒
    rtc->minute = bcd_to_bin(cmos_read(0x02));  // 分
    rtc->hour   = bcd_to_bin(cmos_read(0x04));  // 時
    rtc->day    = bcd_to_bin(cmos_read(0x07));  // 日
    rtc->month  = bcd_to_bin(cmos_read(0x08));  // 月
    rtc->year   = bcd_to_bin(cmos_read(0x09));  // 西暦(下2桁)
    rtc->century = bcd_to_bin(cmos_read(0x32)); // 西暦(上2桁)
}

// RTC データを CMOS から読み込んで文字列に変換
void format_date_time(char* buf) {
    struct RTC rtc;

    // CMOS から日時を取得
    cmos_read_rtc(&rtc);

    // "YYYY/MM/DD HH:MM:SS" 形式に整形
    buf[0]  = '0' + (rtc.century / 10);
    buf[1]  = '0' + (rtc.century % 10);
    buf[2]  = '0' + (rtc.year / 10);
    buf[3]  = '0' + (rtc.year % 10);
    buf[4]  = '/';
    buf[5]  = '0' + (rtc.month / 10);
    buf[6]  = '0' + (rtc.month % 10);
    buf[7]  = '/';
    buf[8]  = '0' + (rtc.day / 10);
    buf[9]  = '0' + (rtc.day % 10);
    buf[10] = ' ';
    buf[11] = '0' + (rtc.hour / 10);
    buf[12] = '0' + (rtc.hour % 10);
    buf[13] = ':';
    buf[14] = '0' + (rtc.minute / 10);
    buf[15] = '0' + (rtc.minute % 10);
    buf[16] = ':';
    buf[17] = '0' + (rtc.second / 10);
    buf[18] = '0' + (rtc.second % 10);
    buf[19] = '\0';
}
