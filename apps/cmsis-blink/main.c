#include "stm32f103xb.h"

#define LED_PIN 13u
#define BLINK_DELAY 800000u

static void delay(volatile uint32_t count) {
  while (count-- > 0u) {
    /* Keep the CPU busy for one instruction so the loop is not optimized away.
     */
    __asm volatile("nop");
  }
}

int main(void) {
  // Enabling GPIO Port C Clock
  RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

  // Configuring GPIO Port C as general purpose output push-pull (CNF - 00), max
  // speed 2MHz (Mode - 10)
  // 1 - Setting bits 23:20 - 0x0000
  GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
  // 2 - Setting bit 21 to 0x1
  GPIOC->CRH |= GPIO_CRH_MODE13_1;

  // Turn off the LED setting the value to 1
  GPIOC->BSRR = GPIO_BSRR_BS13;

  while (1) {
    GPIOC->ODR ^= GPIO_BSRR_BS13;
    delay(BLINK_DELAY);
  }
}
