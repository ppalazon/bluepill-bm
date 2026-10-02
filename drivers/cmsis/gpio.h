/*
 * Copyright (C) 2026 Phyxor Microsystems
 * SPDX-FileContributor: Pablo Palazon
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPIO_H_
#define GPIO_H_

#include "stm32f1xx.h"

void board_led_init(void);
void board_led_on(void);
void board_led_off(void);
void board_led_toggle(void);

#endif /* GPIO_H_ */
