// cmos.h
#ifndef CMOS_H
#define CMOS_H

#include "stdint.h"

struct RTC {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint16_t century;
};

void cmos_read_rtc(struct RTC* rtc);

#endif

