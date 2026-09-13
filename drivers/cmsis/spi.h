
#ifndef SPI1_H
#define SPI1_H

#include "stm32f1xx.h"
#include <stdint.h>

void spi1_init(void);
void spi1_config(void);
void spi1_transmit(uint8_t *data, uint32_t size);
void spi1_receive(uint8_t *data, uint32_t size);

#endif /* end of include guard: SPI1_H */
