#include "gpio.h"
#include "systick.h"

#define BLINK_DELAY 2000

int main(void) {
    board_led_init();
    board_led_off();
    while (1) {
        board_led_toggle();
        systick_msec_delay(BLINK_DELAY);
    }
}
