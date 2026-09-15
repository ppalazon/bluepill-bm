#ifndef RTC_H
#define RTC_H

#include <stdbool.h>
#include <stdint.h>

bool rtc_init(void);
bool rtc_set_prescaler_1hz(void);
bool rtc_set_epoch_time(uint32_t epoch_time_sec);
uint32_t rtc_get_epoch_time(void);
bool rtc_set_second_interrupt(void);
void rtc_clear_second_flag(void);

#endif /* end of include guard: RTC_H */
