/*
 * Copyright (C) 2026 Phyxor Microsystems
 * SPDX-FileContributor: Pablo Palazon
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IWDG_H
#define IWDG_H

#include "stm32f1xx.h"

void iwdg_init(void);
void iwdg_refresh(void);

#endif /* end of include guard: IWDG_H */
