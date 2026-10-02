/*
 * Copyright (C) 2026 Phyxor Microsystems
 * SPDX-FileContributor: Pablo Palazon
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "dma_adc.h"
#include "adc.h"
#include "stm32f103xb.h"
#include <stdint.h>

uint16_t adc_raw_data[ADC_BUFFER_SAMPLES];

void dma_adc_mem_init(void) {
    // Enable GPIO PB0 as input analog as adc.c
    // Enable GPIOB for ADC channel 8
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    // Set PB0 to analog input mode
    GPIOB->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);

    adc1_clock_init();

    // Select channel 8 as the first regular conversion
    ADC1->SQR3 = (8u << ADC_SQR3_SQ1_Pos);

    // Set the regular sequence length to one conversion
    ADC1->SQR1 = 0u;

    // Use a long sample time for a simple external sensor
    ADC1->SMPR2 = ADC_SMPR2_SMP8;

    // Select to use DMA
    ADC1->CR2 = ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG | ADC_CR2_DMA | ADC_CR2_CONT | ADC_CR2_ADON;
    adc_startup_delay();

    calibration();

    // DMA Configuration
    // Enable clock access to DMA
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    // Disable DMA Stream 0 used by ADC (See table)
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;

    // Wait until the Channel 1 is disable
    while ((DMA1_Channel1->CCR & DMA_CCR_EN)) {}

    // Enable circular mode
    DMA1_Channel1->CCR |= DMA_CCR_CIRC;

    // Set MSIZE to half-word (16bits) as ADC uses 12 bits
    DMA1_Channel1->CCR &= ~(DMA_CCR_MSIZE);
    DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0;

    // Set PSIZE to half-word (16bits) as ADC uses 12 bits
    DMA1_Channel1->CCR &= ~(DMA_CCR_PSIZE);
    DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0;

    // Enable memory ADDR increment
    DMA1_Channel1->CCR |= DMA_CCR_MINC;

    // Disable peripheral ADDR increment
    DMA1_Channel1->CCR &= ~DMA_CCR_PINC;

    // Set peripheral address
    DMA1_Channel1->CPAR = (uint32_t)(&(ADC1->DR));

    // Set memory address
    DMA1_Channel1->CMAR = (uint32_t)(&adc_raw_data);

    // Set number of transfers
    DMA1_Channel1->CNDTR = (uint16_t)ADC_BUFFER_SAMPLES;

    // Enable DMA Stream
    DMA1_Channel1->CCR |= DMA_CCR_EN;

    // Start ADC conversion
    ADC1->CR2 |= ADC_CR2_SWSTART;
}
