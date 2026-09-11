#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "stm32f1xx.h"

void pb0_adc_init(void);
void adc1_temperature_init(void);
void start_conversion(void);
void calibration(void);
uint32_t adc_read(void);
uint32_t adc1_channel8_read_raw(void);
uint32_t adc1_temperature_read_raw(void);
uint32_t adc1_vrefint_read_raw(void);

#endif /* end of include guard: ADC_H */
