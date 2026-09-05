#include <stdint.h>

#include "stm32f103c8t6.h"

/* Blue Pill onboard LED: PC13, usually active-low. */
#define LED_PIN 13u

static void delay(volatile uint32_t count) {
  while (count-- > 0u) {
    /* Keep the CPU busy for one instruction so the loop is not optimized away. */
    __asm volatile("nop");
  }
}

int main(void) {
  /*
   * Enable GPIOC's APB2 peripheral clock.
   * RCC_APB2ENR address: 0x40021018
   * RCC_APB2ENR_IOPCEN: bit 4, mask 0x00000010
   * Effect: sets bit 4 and leaves all other bits unchanged.
   * Reset value 0x00000000 becomes 0x00000010.
   */
  RCC_APB2ENR |= RCC_APB2ENR_IOPCEN;

  /*
   * PC13 is configured by GPIOC_CRH because CRH controls pins 8-15.
   * GPIOC_CRH address: 0x40011004
   * PC13 field: bits 23:20, mask 0x00F00000
   * Effect: clears CNF13[1:0] and MODE13[1:0] to 0000, leaving other pins unchanged.
   * Reset value 0x44444444 becomes 0x44044444.
   */
  GPIOC_CRH &= ~GPIO_CFG_MASK(LED_PIN);

  /*
   * Configure PC13 as a 2 MHz push-pull output.
   * GPIO_MODE_OUTPUT_2MHZ: 0b10, GPIO_CNF_OUTPUT_PP: 0b00
   * GPIO_CFG(13, 0b10, 0b00): bits 23:20 = 0010, value 0x00200000
   * Effect: sets MODE13[1:0] to 10 and leaves CNF13[1:0] clear at 00.
   * Previous value 0x44044444 becomes 0x44244444.
   */
  GPIOC_CRH |= GPIO_CFG(LED_PIN, GPIO_MODE_OUTPUT_2MHZ, GPIO_CNF_OUTPUT_PP);

  while (1) {
    /*
     * Toggle PC13 output data.
     * GPIOC_ODR address: 0x4001100C
     * GPIO_PIN(13): bit 13, mask 0x00002000
     * Effect: flips bit 13 only; 0 drives PC13 low, 1 drives PC13 high.
     * Reset value 0x00000000 first becomes 0x00002000, then 0x00000000, and repeats.
     * The onboard LED is usually active-low: low turns it on, high turns it off.
     */
    GPIOC_ODR ^= GPIO_PIN(LED_PIN);
    delay(800000u);
  }
}
