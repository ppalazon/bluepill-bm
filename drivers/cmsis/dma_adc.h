/*
 * Copyright (C) 2026 Phyxor Microsystems
 * SPDX-FileContributor: Pablo Palazon
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DMA_ADC_H
#define DMA_ADC_H

#include "stm32f1xx.h"

#define ADC_BUFFER_SAMPLES 100u
#define ADC_HALF_SAMPLES 50u

void dma_adc_mem_init(void);

#endif /* end of include guard: DMA_ADC_H */
