#include "iwdg.h"
#include "stm32f103xb.h"

#define IWDG_KEY_ENABLE 0xCCCCu
#define IWDG_KEY_RELOAD 0xAAAAu
#define IWDG_KEY_WR_ACCESS_ENABLE 0x5555u
#define IWDG_RELOAD_VAL 0xFFFu

void iwdg_init() {
    // Enable the IWDG by writing 0xCCCC in the IWDG_KR register
    IWDG->KR = IWDG_KEY_ENABLE;

    // Enable register access
    IWDG->KR = IWDG_KEY_WR_ACCESS_ENABLE;

    // Set the IWDG prescaler, write only once without clearing it
    IWDG->PR = 0x4; // Divided by 64

    // Set the reload register (IWDG_RLR) to the largetst value 0xFFF
    IWDG->RLR = IWDG_RELOAD_VAL;

    // Wait for the registers to be updated
    while ((IWDG->SR & (IWDG_SR_PVU | IWDG_SR_RVU)) != 0u) {}

    // Refresh the counter
    iwdg_refresh();
}

void iwdg_refresh() {
    // Refresh the counter value
    IWDG->KR = IWDG_KEY_RELOAD;
}
