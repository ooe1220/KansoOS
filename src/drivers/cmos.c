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

