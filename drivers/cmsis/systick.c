/*
 * Copyright (C) 2026 Phyxor Microsystems
 * SPDX-FileContributor: Pablo Palazon
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "systick.h"
#include "board_clock.h"
#include "stm32f103xb.h"
#include <stdint.h>

#define ONE_MSEC_LOAD (HCLK_HZ / 1000u)

void systick_msec_delay(uint32_t delay) {
    // Load number of clock cycles per millisecond
    SysTick->LOAD = ONE_MSEC_LOAD - 1;

    // Clear systick current value register
    SysTick->VAL = 0;

    // Select internal clock source (bit 2) and enable it (bit 0)
    SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk);

    for (uint32_t i = 0; i < delay; i++) {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0) {}
    }

    // Disable systick
    SysTick->CTRL = 0u;
}
