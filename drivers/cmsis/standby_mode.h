/*
 * Copyright (C) 2026 Phyxor Microsystems
 * SPDX-FileContributor: Pablo Palazon
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STANDBY_MODE_H
#define STANDBY_MODE_H

#include "stm32f1xx.h"

uint32_t get_wakeup_pin_state(void);
void wakeup_pin_init(void);
uint32_t get_wakeup_pin_state(void);
void standby_wakeup_pin_setup(void);

#endif /* end of include guard: STANDBY_MODE_H */
