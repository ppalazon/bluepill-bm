// SPDX-FileCopyrightText: 2026 Pablo Palazon
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DMA_UART_H
#define DMA_UART_H

#include "stm32f1xx.h"

#define UART_DATA_BUFF_SIZE 5

void uart1_rx_tx_init(void);
void dma1_init(void);
void dma1_channel5_uart_rx_config(void);
void dma1_channel4_uart_tx_config(uint32_t msg_to_snd, uint32_t msg_len);

#endif /* end of include guard: DMA_UART_H */
