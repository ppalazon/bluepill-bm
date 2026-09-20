#include "standby_mode.h"
#include "cmsis_gcc.h"
#include <stdint.h>

// Set default standby mode as Power down
#define PWR_MODE_STANDBY PWR_CR_PDDS

static void set_power_mode(uint32_t pwr_mode);

void wakeup_pin_init(void) {
    // Enable clock access for GPIO A
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // Setting WAKE Up pin as input (not pull-up nor pull-down)
    GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
    GPIOA->CRL |= GPIO_CRL_CNF0_0;
}

void standby_wakeup_pin_setup(void) {
    // Wait for wakeup pin to be released
    while (get_wakeup_pin_state() == 0u) {}

    // Disable wake up pin
    PWR->CSR &= ~(PWR_CSR_WUF);

    // Clear all wake up flags
    PWR->CR |= PWR_CR_CWUF;

    // Enable wakeup pin
    PWR->CSR |= PWR_CSR_EWUP;

    // Enter Standby mode
    set_power_mode(PWR_MODE_STANDBY);

    // Set SLEEPDEEP bit in the CorteX System Control Register
    SCB->SCR |= (SCB_SCR_SLEEPDEEP_Msk);

    // Wait for interrupt
    __WFI();
}

uint32_t get_wakeup_pin_state(void) {
    return ((GPIOA->IDR & GPIO_IDR_IDR0) == GPIO_IDR_IDR0);
}

static void set_power_mode(uint32_t pwr_mode) {
    // Power down deepsleep and low-power deepsleep bits
    MODIFY_REG(PWR->CR, (PWR_CR_PDDS | PWR_CR_LPDS), pwr_mode);
}
