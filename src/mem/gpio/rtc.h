#ifndef GBA_RTC_H
#define GBA_RTC_H

#include "../../common/util.h"
#include "../gbabus.h"

gpio_port_t gba_rtc_read();
void gba_rtc_write(gpio_port_t value, byte mask);

#endif //GBA_RTC_H
