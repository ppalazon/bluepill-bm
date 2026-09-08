#include "tim.h"
#include "stm32f103xb.h"

#define SYSCLK 8000000
#define APB1_CLK SYSCLK
#define TIM_CLK APB1_CLK

#define TIM_PSC_1HZ 7999 // 8000 - 1
#define TIM_ARR_1HZ 999  // 1000 - 1

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
    while (!(TIM2->SR & TIM_SR_UIF)) {
    }

    // Clear UIF
    TIM2->SR &= ~(TIM_SR_UIF);
}
