// SPDX-FileCopyrightText: 2026 Pablo Palazon
// SPDX-License-Identifier: BSD-3-Clause

#ifndef TIM_H
#define TIM_H

#include "stm32f1xx.h"

void tim2_1hz_init(void);
void tim2_wait_uif(void);

#endif // TIM_H
