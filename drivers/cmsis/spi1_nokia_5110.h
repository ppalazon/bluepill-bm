#ifndef SPI1_NOKIA_5110_H
#define SPI1_NOKIA_5110_H

#include "stm32f1xx.h"
#include <stdint.h>

/* Commands available in both instruction sets. */
#define N5110_CMD_NOP 0x00u
#define N5110_CMD_FUNCTION_SET 0x20u
#define N5110_FUNCTION_ACTIVE 0x00u
#define N5110_FUNCTION_POWER_DOWN 0x04u
#define N5110_FUNCTION_HORIZONTAL 0x00u
#define N5110_FUNCTION_VERTICAL 0x02u
#define N5110_FUNCTION_BASIC 0x00u
#define N5110_FUNCTION_EXTENDED 0x01u

/* Basic instruction set (H = 0). */
#define N5110_CMD_DISPLAY_BLANK 0x08u
#define N5110_CMD_DISPLAY_ALL_ON 0x09u
#define N5110_CMD_DISPLAY_NORMAL 0x0Cu
#define N5110_CMD_DISPLAY_INVERSE 0x0Du

#define N5110_Y_ADDRESS_MAX 5u
#define N5110_X_ADDRESS_MAX 83u
#define N5110_CMD_SET_Y_ADDRESS(y) ((uint8_t)(0x40u | ((uint8_t)(y) & 0x07u)))
#define N5110_CMD_SET_X_ADDRESS(x) ((uint8_t)(0x80u | ((uint8_t)(x) & 0x7Fu)))

/* Extended instruction set (H = 1). */
#define N5110_CMD_SET_TEMPERATURE_COEFFICIENT(tc) ((uint8_t)(0x04u | ((uint8_t)(tc) & 0x03u)))
#define N5110_TEMPERATURE_COEFFICIENT_0 0u
#define N5110_TEMPERATURE_COEFFICIENT_1 1u
#define N5110_TEMPERATURE_COEFFICIENT_2 2u
#define N5110_TEMPERATURE_COEFFICIENT_3 3u

#define N5110_CMD_SET_BIAS(bias) ((uint8_t)(0x10u | ((uint8_t)(bias) & 0x07u)))
#define N5110_BIAS_MUX_1_100 0u
#define N5110_BIAS_MUX_1_80 1u
#define N5110_BIAS_MUX_1_65 2u
#define N5110_BIAS_MUX_1_48 3u
#define N5110_BIAS_MUX_1_40_OR_1_34 4u
#define N5110_BIAS_MUX_1_24 5u
#define N5110_BIAS_MUX_1_18_OR_1_16 6u
#define N5110_BIAS_MUX_1_10_OR_1_9_OR_1_8 7u

#define N5110_VOP_MAX 127u
#define N5110_CMD_SET_VOP(vop) ((uint8_t)(0x80u | ((uint8_t)(vop) & 0x7Fu)))

void spi1_n5110_init(void);
void spi1_n5110_config(void);
void spi1_n5110_reset(void);
void spi1_n5110_start(void);
void spi1_n5110_cs_enable(void);
void spi1_n5110_cs_disable(void);
void spi1_n5110_send_command(uint8_t command);

#endif /* SPI1_NOKIA_5110_H */
