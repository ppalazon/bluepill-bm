#include "gpio.h"
#include "tim.h"

int main(void) {
    board_led_init();
    board_led_off();

    // Initialize timer TIM2
    tim2_1hz_init();
    while (1) {
        // The first UIF is very quick, almost immediately, so it seems like it start on
        tim2_wait_uif();
        board_led_toggle();
    }
}
