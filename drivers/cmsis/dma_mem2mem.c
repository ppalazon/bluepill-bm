// SPDX-FileCopyrightText: 2026 Pablo Palazon
// SPDX-License-Identifier: BSD-3-Clause

#include "dma_mem2mem.h"
#include "stm32f103xb.h"

void dma1_mem2mem_config(void) {
    // Enable clock access to DMA
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    // Disable DMA Channel to configure it
    DMA1_Channel2->CCR &= ~DMA_CCR_EN;

    // Wailt until DMA Channel is disabled
    while ((DMA1_Channel2->CCR & DMA_CCR_EN)) {}

    // Clear interrupt flags for Channel 2
    DMA1->IFCR = DMA_IFCR_CGIF2 | DMA_IFCR_CTEIF2 | DMA_IFCR_CHTIF2 | DMA_IFCR_CTCIF2;

    // Set MSIZE to half word (16 bits)
    DMA1_Channel2->CCR &= ~(DMA_CCR_MSIZE);
    DMA1_Channel2->CCR |= DMA_CCR_MSIZE_0;

    // Set PSIZE to half word (16 bits)
    DMA1_Channel2->CCR &= ~(DMA_CCR_PSIZE);
    DMA1_Channel2->CCR |= DMA_CCR_PSIZE_0;

    // Enable memory addr increment
    DMA1_Channel2->CCR |= DMA_CCR_MINC;

    // Enable peripherial addr increment
    DMA1_Channel2->CCR |= DMA_CCR_PINC;

    // Configure mem2mem transaction
    DMA1_Channel2->CCR |= DMA_CCR_MEM2MEM;

    // Enable transfer complete interruption
    DMA1_Channel2->CCR |= (DMA_CCR_TCIE | DMA_CCR_TEIE);

    // Disable Circular mode (It's not used with Memory mode)
    DMA1_Channel2->CCR &= ~(DMA_CCR_CIRC);

    // Set Direction from peripherial to memory (Peripherial if source)
    DMA1_Channel2->CCR &= ~(DMA_CCR_DIR);

    // Enable DMA Channel 2 Interrupt NVIC
    NVIC_EnableIRQ(DMA1_Channel2_IRQn);
}

void dma_transfer_start(uint32_t src_buff, uint32_t dest_buff, uint32_t len) {
    // DMA is configured from peripeherial to memory
    // Set peripheral address
    DMA1_Channel2->CPAR = src_buff;

    // Set memory address
    DMA1_Channel2->CMAR = dest_buff;

    // Set number of transactions
    DMA1_Channel2->CNDTR = len;

    // Enable DMA Stream
    DMA1_Channel2->CCR |= DMA_CCR_EN;
}
