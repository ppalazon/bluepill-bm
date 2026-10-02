/*
 * Copyright (C) 2026 Phyxor Microsystems
 * SPDX-FileContributor: Pablo Palazon
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "spi.h"
#include "stm32f103xb.h"
#include <stdint.h>

void spi1_init(void) {
    // Enabling GPIO A/B and SPI1
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // Clean GPIO Default configuration for PA4:7
    GPIOA->CRL &= ~(0xFFFF0000);

    // SPI1_NSS at PA4 as Master (Output CNF=10, Mode=11)
    GPIOA->CRL |= (GPIO_CRL_CNF4_1 | GPIO_CRL_MODE4);
    // SPI1_SCK at PA5 as Master (Output CNF=10, Mode=11)
    GPIOA->CRL |= (GPIO_CRL_CNF5_1 | GPIO_CRL_MODE5);
    // SPI1_MISO at PA6 as Input CNF=01, Mode=00
    GPIOA->CRL |= (GPIO_CRL_CNF6_0);
    // SPI1_MOSI at PA7 as Output (Output CNF=10, Mode=11)
    GPIOA->CRL |= (GPIO_CRL_CNF7_1 | GPIO_CRL_MODE7);
}

void spi1_config(void) {
    // Enable SPI1 clock
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Set clock on SCK Baud rate to fPCLK/4
    SPI1->CR1 |= 0x1 << SPI_CR1_BR_Pos;

    // Set CPOL and CPHA to 1
    SPI1->CR1 |= (SPI_CR1_CPOL | SPI_CR1_CPHA);
    // SPI1->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA); // SPI Mode 0

    // Enable full duplex (Clean bit 10)
    SPI1->CR1 &= ~(SPI_CR1_RXONLY);

    // Set MSB first
    SPI1->CR1 &= ~(SPI_CR1_LSBFIRST);

    // Set mode to Master
    SPI1->CR1 |= SPI_CR1_MSTR;

    // Set 8 bit mode
    SPI1->CR1 &= ~(SPI_CR1_DFF);

    // Select software slave management by setting SSM=1 and SSI=1
    SPI1->CR1 |= (SPI_CR1_SSM | SPI_CR1_SSI);

    // Enable SPI Module
    SPI1->CR1 |= (SPI_CR1_SPE);
}

void spi1_transmit(uint8_t *data, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        // Wait until TXE is set
        while (!(SPI1->SR & SPI_SR_TXE)) {}

        // Write the command in the data register
        SPI1->DR = data[i];

        // Full-duplex SPI receives one byte for every transmitted byte.
        while (!(SPI1->SR & SPI_SR_RXNE)) {}
        (void)SPI1->DR;
    }

    // Wait until TXE is set
    while (!(SPI1->SR & SPI_SR_TXE)) {}

    // Wait for BUSY flag to reset
    while (SPI1->SR & SPI_SR_BSY) {}

    // Clear OVR with the required DR-then-SR read sequence if it was already set.
    if (SPI1->SR & SPI_SR_OVR) {
        (void)SPI1->DR;
        (void)SPI1->SR;
    }
}

void spi1_receive(uint8_t *data, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        // Send dummy data
        SPI1->DR = 0;

        // Wait for RXNE flag to be set
        while (!(SPI1->SR & SPI_SR_RXNE)) {}

        // Read data from data register
        *data++ = SPI1->DR;
    }
}
