#include "gpio.h"
#include "gpio_exti.h"
#include "systick.h"
#include "stm32f103xb.h"
#include "uart.h"
#include <stdint.h>
#include <stdio.h>

#define DEBOUNCE_DELAY_MS 20u

static uint32_t count = 0;
static volatile uint32_t button_pending = 0u;

static void exti_callback(void);

int main(void) {
    board_led_init();
    uart1_init();
    pb15_exti_init();

    printf("Welcome to app cmsis-ext1-btn\r\n");

    board_led_off();

    while (1) {
        if (button_pending == 0u) {
            continue;
        }

        systick_msec_delay(DEBOUNCE_DELAY_MS);
        if ((GPIOB->IDR & GPIO_IDR_IDR15) == 0u) {
            exti_callback();
        }

        /* Keep EXTI masked until the button has been released for 20 ms. */
        do {
            while ((GPIOB->IDR & GPIO_IDR_IDR15) == 0u) {
            }
            systick_msec_delay(DEBOUNCE_DELAY_MS);
        } while ((GPIOB->IDR & GPIO_IDR_IDR15) == 0u);

        EXTI->PR = EXTI_PR_PIF15;
        button_pending = 0u;
        EXTI->IMR |= EXTI_IMR_IM15;
    }
}

static void exti_callback(void) {
    count++;
    printf("BTN Pressed %ld\r\n", (long)count);
    board_led_toggle();
}

void EXTI15_10_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PIF15) {
        EXTI->IMR &= ~EXTI_IMR_IM15;
        EXTI->PR = EXTI_PR_PIF15;
        button_pending = 1u;
    }
}
