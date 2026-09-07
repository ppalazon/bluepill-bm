#include "uart.h"
#include "stm32f103xb.h"
#include <stdint.h>
#define DBG_UART_BAUDRATE 115200
#define SYS_REQ 8000000
#define APB1_CLK SYS_REQ

static void uart_set_baudrate(uint32_t periph_clk, uint32_t baudrate);
static void uart_write(int ch);

int board_putchar(int ch) {
    uart_write(ch);
    return ch;
}

void uart_init(void) {
    // Enable GPIOA clock in the APB2 peripherials
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // Set mode to Alternative functions on corresponds GPIOA pins
    // 1 - Reseting to 0 from bit 19:0
    GPIOA->CRL &= ~(0x000FFFFF);
    // PA0: CTS2 - Floating input (CNF: 01, Mode: 00),
    GPIOA->CRL |= (GPIO_CRL_CNF0_0);
    // PA1: RTS2 - Output push-pull (CNF: 10, Mode: 10) (2MHz)
    GPIOA->CRL |= (GPIO_CRL_CNF1_1 | GPIO_CRL_MODE1_1);
    // PA2: TX2 - Output push-pull (CNF: 10, Mode: 10) (2MHz)
    GPIOA->CRL |= (GPIO_CRL_CNF2_1 | GPIO_CRL_MODE2_1);
    // PA3: RX2 - Floating input (CNF: 01, Mode: 00),
    GPIOA->CRL |= (GPIO_CRL_CNF3_0);
    // PA4: CK2 - Output push-pull (CNF: 10, Mode: 10) (2MHz)
    GPIOA->CRL |= (GPIO_CRL_CNF4_1 | GPIO_CRL_MODE4_1);

    // Enable clock access to USART2 (Using GPIOA pins)
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // Configure uart baudrate
    uart_set_baudrate(APB1_CLK, DBG_UART_BAUDRATE);

    // Enable and configure transfer direction (Only transmitter enable)
    USART2->CR1 = (USART_CR1_UE | USART_CR1_TE);
}

static void uart_write(int ch) {
    // Make sure transmit data register is empty
    while (!(USART2->SR & USART_SR_TXE)) {
    }

    // Write to transmit dat register
    USART2->DR = (ch & 0xFF); // Write the LSB Byte
}

static uint16_t compute_uart_bd(uint32_t periph_clk, uint32_t baudrate) {
    return ((periph_clk + (baudrate / 2U)) / baudrate);
}

static void uart_set_baudrate(uint32_t periph_clk, uint32_t baudrate) {
    USART2->BRR = compute_uart_bd(periph_clk, baudrate);
}
