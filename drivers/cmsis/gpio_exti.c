#include "gpio_exti.h"
#include "stm32f103xb.h"

void pb15_exti_init(void) {
    // Disable global interrupt to avoid corruption
    __disable_irq();

    // Enable clock access for GPIO B
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    // Set PB15 as a floating input; the HW-483 module drives the signal high.
    GPIOB->CRH &= ~(GPIO_CRH_CNF15 | GPIO_CRH_MODE15);
    GPIOB->CRH |= GPIO_CRH_CNF15_0;

    // Enable clock access for to enable PB15 as EXTI
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->EXTICR[3] &= ~AFIO_EXTICR4_EXTI15;
    AFIO->EXTICR[3] |= AFIO_EXTICR4_EXTI15_PB;

    // Select falling edge trigger
    EXTI->RTSR &= ~EXTI_RTSR_RT15;
    EXTI->FTSR |= EXTI_FTSR_FT15;

    // Clear a stale request before unmasking EXTI15.
    EXTI->PR = EXTI_PR_PIF15;
    EXTI->IMR |= EXTI_IMR_IM15;

    // Enables EXTI15 line in NVIC
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    // Enable global interruption
    __enable_irq();
}
