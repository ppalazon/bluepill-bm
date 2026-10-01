// SPDX-FileCopyrightText: 2026 Pablo Palazon
// SPDX-License-Identifier: BSD-3-Clause

#include "tim.h"
#include "board_clock.h"
#include "stm32f103xb.h"

#define TIM_PRESCALER_HZ 1000u
#define TIM_PSC_1HZ ((PCLK1_HZ / TIM_PRESCALER_HZ) - 1u)
#define TIM_ARR_1HZ 999 // 1000 - 1

void tim2_1hz_init(void) {
    // Enable TIM2 on APB1 peripherial bus
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Set prescaler value
    TIM2->PSC = TIM_PSC_1HZ;

    // Set auto-reload value
    TIM2->ARR = TIM_ARR_1HZ;

    // Clear counter
    TIM2->CNT = 0;

    // Enable timer
    TIM2->CR1 = TIM_CR1_CEN;
}

void tim2_wait_uif(void) {
    // Wait for UIF
    while (!(TIM2->SR & TIM_SR_UIF)) {}

    // Clear UIF
    TIM2->SR &= ~(TIM_SR_UIF);
}
