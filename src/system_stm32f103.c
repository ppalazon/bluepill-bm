#include <stdint.h>

#define RCC_CR      (*(volatile uint32_t *)0x40021000u)
#define RCC_CFGR    (*(volatile uint32_t *)0x40021004u)
#define FLASH_ACR   (*(volatile uint32_t *)0x40022000u)

void SystemInit(void)
{
    RCC_CR |= 0x00000001u;
    RCC_CFGR = 0x00000000u;
    FLASH_ACR = 0x00000000u;
}
