#ifndef SYSTICK_H
#define SYSTICK_H

#include "stm32f1xx.h"
#include <stdint.h>

void systick_msec_delay(uint32_t delay);

#endif /* SYSTICK_H */
