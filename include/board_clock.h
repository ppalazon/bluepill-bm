// SPDX-FileCopyrightText: 2026 Pablo Palazon
// SPDX-License-Identifier: BSD-3-Clause

#ifndef BOARD_CLOCK_H
#define BOARD_CLOCK_H

/* SystemInit() keeps the MCU on the default 8 MHz HSI clock. */
#define HSI_HZ 8000000u

#define SYSCLK_HZ HSI_HZ
#define HCLK_HZ SYSCLK_HZ
#define PCLK1_HZ HCLK_HZ
#define PCLK2_HZ HCLK_HZ
#define ADCCLK_HZ (PCLK2_HZ / 2u)

#endif /* BOARD_CLOCK_H */
