#include "stm32f103c8t6.h"

void SystemInit(void) {
    /*
     * Keep the internal high-speed oscillator enabled.
     * RCC_CR address: 0x40021000
     * RCC_CR_HSION: bit 0, mask 0x00000001
     * Effect: sets HSION and leaves every other RCC_CR bit unchanged. After
     * reset this bit is normally already set, but writing it makes the startup
     * clock source explicit for this minimal system setup.
     */
    RCC_CR |= RCC_CR_HSION;

    /*
     * Reset the clock configuration register to the default clock tree.
     * RCC_CFGR address: 0x40021004
     * Value 0x00000000 selects HSI as SYSCLK, disables PLL selection, and keeps
     * the AHB/APB prescalers at their reset defaults.
     */
    RCC_CFGR = 0x00000000u;

    /*
     * Reset Flash access configuration.
     * FLASH_ACR address: 0x40022000
     * Value 0x00000000 means zero wait states and no prefetch configuration.
     * That is enough for the default 8 MHz HSI clock used by this project.
     */
    FLASH_ACR = 0x00000000u;
}
