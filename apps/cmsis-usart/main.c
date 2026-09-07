#include "gpio.h"
#include "systick.h"
#include "uart.h"
#include <stdint.h>
#include <stdio.h>

#define BLINK_DELAY 1000

int main(void) {
    // Initialization
    board_led_init();
    uart_init();

    // Turn off the board led;
    board_led_off();
    uint32_t count = 0;
    while (1) {
        printf("Hello from STM32 (%ld)...\r\n", count);
        board_led_toggle();
        systick_msec_delay(BLINK_DELAY);
        count++;
    }
}
