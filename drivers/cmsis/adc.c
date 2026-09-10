#include "adc.h"
#include "stm32f103xb.h"
#include <stdint.h>

void pb0_adc_init(void) {
    // Enable GPIOB to enable ADC channel 8 and 9
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    // Set pins PB0 (ADC8) as input analog (CNF: 00 and MODE: 00)
    GPIOB->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);

    // Enable ADC module in the APB2 bus
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    // Set conversion sequence start with channel 8 (3bit enable 0x8)
    ADC1->SQR3 &= ~(ADC_SQR3_SQ1);
    ADC1->SQR3 |= ADC_SQR3_SQ1_3;

    // Set conversion sequence length (Set 0 to make 1 conversion)
    ADC1->SQR1 &= ~(ADC_SQR1_L);

    // Enable ADC module
    ADC1->CR2 |= ADC_CR2_ADON;
}

void calibration(void) {
    // Reset calibration
    ADC1->CR2 |= ADC_CR2_RSTCAL;

    // Wait until finishes calibration
    while (ADC1->CR2 & ADC_CR2_RSTCAL) {
    }

    // Initialize calibration
    ADC1->CR2 |= ADC_CR2_CAL;

    /* Wait until ADC is calibrated */
    while (ADC1->CR2 & ADC_CR2_CAL) {
    }
}

void start_conversion(void) {
    // Software trigger with EXTSEL=111
    // Continous conversion
    // Trigger software start
    ADC1->CR2 |= (ADC_CR2_CONT | ADC_CR2_SWSTART | ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG);
}

uint32_t adc_read(void) {
    // Wait for conversion to be complete
    while (!(ADC1->SR & ADC_SR_EOC)) {
    }

    /* Read converted value, getting 16 LSB */
    return (ADC1->DR & 0xFFFF);
}
