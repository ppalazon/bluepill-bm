// SPDX-FileCopyrightText: 2026 Pablo Palazon
// SPDX-License-Identifier: BSD-3-Clause

#include "spi.h"
#include "spi1_nokia_5110.h"
#include "stm32f103xb.h"
#include "systick.h"
#include <stdint.h>

void spi1_n5110_init(void) {
    // Enabling SPI1
    spi1_init();

    // Drive reset low before changing PA1 to an output.
    GPIOA->BSRR = GPIO_BSRR_BR1;

    // Clean GPIO default configuration for PA1, PA2, and PA3.
    GPIOA->CRL &= ~(0x0FFF0);

    // Assign PA1 to reset the display (2 MHz push-pull output).
    GPIOA->CRL |= GPIO_CRL_MODE1_1;

    // Assign a PA3 to send Command / Data information to LCD (Output)
    GPIOA->CRL |= GPIO_CRL_MODE3_1;

    // Assign to PA2 to Chip Select for this display
    GPIOA->CRL |= GPIO_CRL_MODE2_1;

    // Leave the display deselected and in command mode.
    GPIOA->BSRR = GPIO_BSRR_BS2 | GPIO_BSRR_BR3;
}

void spi1_n5110_config(void) {
    spi1_config();
}

void spi1_n5110_reset(void) {
    GPIOA->BSRR = GPIO_BSRR_BR1;
    systick_msec_delay(1u);
    GPIOA->BSRR = GPIO_BSRR_BS1;
    systick_msec_delay(1u);
}

void spi1_n5110_start(void) {
    spi1_n5110_send_command(N5110_CMD_FUNCTION_SET | N5110_FUNCTION_ACTIVE |
                            N5110_FUNCTION_HORIZONTAL | N5110_FUNCTION_EXTENDED);
    spi1_n5110_send_command(N5110_CMD_SET_BIAS(N5110_BIAS_MUX_1_48));
    spi1_n5110_send_command(N5110_CMD_SET_VOP(60u));
    spi1_n5110_send_command(N5110_CMD_SET_TEMPERATURE_COEFFICIENT(N5110_TEMPERATURE_COEFFICIENT_0));
    spi1_n5110_send_command(N5110_CMD_FUNCTION_SET | N5110_FUNCTION_ACTIVE |
                            N5110_FUNCTION_HORIZONTAL | N5110_FUNCTION_BASIC);
    spi1_n5110_send_command(N5110_CMD_DISPLAY_NORMAL);
}

void spi1_n5110_cs_enable(void) {
    GPIOA->BSRR = GPIO_BSRR_BR2;
}

void spi1_n5110_cs_disable(void) {
    GPIOA->BSRR = GPIO_BSRR_BS2;
}

void spi1_n5110_send_command(uint8_t command) {
    // Set PA3 to low level to indicate a command
    GPIOA->BSRR = GPIO_BSRR_BR3;
    spi1_n5110_cs_enable();
    spi1_transmit(&command, 1);
    spi1_n5110_cs_disable();
}
