# Analog-to-digital converter

An analog-to-digital converter (ADC) measures an input voltage and returns a
number. The STM32F103 ADCs use 12-bit results, so a regular conversion produces a
value from `0` to `4095`.

For a single-ended input, the ideal result is approximately:

```text
result = voltage_in / voltage_reference * 4095
voltage_in = result / 4095 * voltage_reference
```

The ADC does not identify a sensor or a physical unit. It only measures voltage.
The application converts that voltage into temperature, light, position, current,
or another quantity using the sensor circuit and its calibration data.

## STM32F103 ADC hardware

The STM32F103C8T6 provides `ADC1` and `ADC2`. They share the external analog input
channels. On the LQFP48 package, the external channels used by this project are:

| ADC channel | Pin | GPIO configuration |
|---|---|---|
| `ADC12_IN0` to `ADC12_IN7` | `PA0` to `PA7` | Analog input |
| `ADC12_IN8` to `ADC12_IN9` | `PB0` to `PB1` | Analog input |

The complete package mapping and pin conflicts are in the [STM32F103C8T6
pinout](../targets/stm32f103c8t6/pinout.md). An ADC pin must use analog GPIO mode,
which disconnects the digital input and output logic.

`ADC1` also provides the internal temperature-sensor and `VREFINT` channels. They
are internal ADC1 channels, not external package pins. Use the device datasheet for
the electrical characteristics and the reference manual for the internal-channel
procedure.

The ADC input clock comes from `PCLK2` through the ADC prescaler and must not
exceed 14 MHz. This project currently uses `PCLK2 = 8 MHz`, so the `/2` ADC
prescaler gives `ADCCLK = 4 MHz`.

## Basic conversion path

For one external sensor channel, use this sequence:

```text
enable GPIO and ADC clocks
  -> set the GPIO pin to analog mode
  -> select the ADC clock prescaler
  -> select channel and sample time
  -> select the regular conversion sequence
  -> power on the ADC
  -> calibrate the ADC
  -> start a conversion
  -> wait for EOC
  -> read ADC_DR
```

Calibration is recommended after power-up. The sample time must be long enough for
the ADC sample capacitor to charge through the sensor circuit. STM32F1 sample-time
options range from `1.5` to `239.5` ADC clock cycles. A higher source impedance
usually needs a longer sample time.

## Registers used most

| Register | Purpose |
|---|---|
| `RCC_APB2ENR` | Enables the GPIO and ADC clocks |
| `RCC_CFGR` | Selects the ADC clock prescaler |
| `GPIOx_CRL` or `GPIOx_CRH` | Selects analog mode for the input pin |
| `ADCx_SR` | Reports `EOC`, `JEOC`, and analog-watchdog status |
| `ADCx_CR1` | Selects scan, discontinuous, interrupt, watchdog, and dual modes |
| `ADCx_CR2` | Powers, calibrates, triggers, aligns, and enables DMA |
| `ADCx_SMPR1` and `ADCx_SMPR2` | Selects the sample time for each channel |
| `ADCx_SQR1` to `ADCx_SQR3` | Defines the regular conversion sequence |
| `ADCx_JSQR` | Defines the injected conversion sequence |
| `ADCx_DR` | Holds the latest regular conversion result |
| `ADCx_JDR1` to `ADCx_JDR4` | Hold injected conversion results |
| `ADCx_HTR` and `ADCx_LTR` | Set analog-watchdog thresholds |

The registers are memory-mapped and must be accessed as `volatile` objects. The
CMSIS device header provides the names used in the example below.

## Minimal polling example

This example reads `PA0`, which is `ADC12_IN0`, using `ADC1`. It assumes the
project's current 8 MHz clock configuration and a 3.3 V analog supply.

```c
#include "stm32f1xx.h"

static void short_delay(void)
{
    for (volatile unsigned int count = 0u; count < 1000u; ++count) {
        __asm volatile("nop");
    }
}

static void adc1_init_pa0(void)
{
    /* GPIOA and ADC1 are both on APB2. */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_ADC1EN;

    /* PA0: analog input, MODE=00 and CNF=00. */
    GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);

    /* PCLK2 is 8 MHz here. ADCCLK becomes 4 MHz. */
    RCC->CFGR &= ~RCC_CFGR_ADCPRE;
    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV2;

    /* One regular conversion: rank 1 is ADC channel 0. */
    ADC1->CR1 = 0u;
    ADC1->SQR1 = 0u;
    ADC1->SQR2 = 0u;
    ADC1->SQR3 = 0u;

    /* Use the longest sample time for a simple sensor experiment. */
    ADC1->SMPR2 = ADC_SMPR2_SMP0;

    /* Select software start for regular conversions. */
    ADC1->CR2 = ADC_CR2_EXTSEL;
    ADC1->CR2 |= ADC_CR2_ADON;
    short_delay();

    /* Reset and run the ADC self-calibration sequence. */
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while ((ADC1->CR2 & ADC_CR2_RSTCAL) != 0u) {
    }
    ADC1->CR2 |= ADC_CR2_CAL;
    while ((ADC1->CR2 & ADC_CR2_CAL) != 0u) {
    }
}

static unsigned int adc1_read_pa0(void)
{
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while ((ADC1->SR & ADC_SR_EOC) == 0u) {
    }

    return ADC1->DR;
}
```

The application can convert the result to an approximate voltage:

```c
unsigned int raw = adc1_read_pa0();
unsigned int millivolts = (raw * 3300u) / 4095u;
```

The calculation assumes that the ADC reference is 3.3 V. For accurate results,
measure the actual reference voltage and account for sensor tolerance, resistor
tolerance, ADC error, and calibration data.

## Conversion modes and use cases

| Mode | Use it for | Main mechanism |
|---|---|---|
| Single conversion | Read a potentiometer, button voltage, or slow sensor on demand | Software starts one regular conversion |
| Continuous conversion | Keep one sensor value updated continuously | `CONT` starts the next conversion automatically |
| Scan conversion | Read several sensors in a fixed order | `SCAN` and `ADC_SQRx` define the channel sequence |
| Timer-triggered conversion | Sample at a precise fixed rate | A timer event starts the regular group |
| End-of-conversion interrupt | Process a result without polling | `EOCIE` and the ADC IRQ handler |
| DMA conversion | Fill a buffer with samples with low CPU overhead | ADC regular results transfer to SRAM |
| Discontinuous conversion | Split a longer sequence across several triggers | `DISCEN` limits conversions per trigger |
| Injected conversion | Give an urgent measurement priority over regular sampling | `ADC_JSQR` and injected triggers |
| Analog watchdog | Detect an input outside a voltage window | `ADC_HTR`, `ADC_LTR`, and `AWD` |
| Dual ADC mode | Increase sampling rate or sample two ADCs together | `ADC1` master and `ADC2` slave |

Use polling for a first experiment. Use a timer and DMA for repeatable sampling of
several channels, waveform capture, or control loops. Use the analog watchdog for
threshold alarms where the CPU does not need every sample.

## Sensor circuits

The ADC measures the voltage presented at its pin. Common applications are:

| Sensor or circuit | ADC application |
|---|---|
| Potentiometer | Position or user-adjustable parameter |
| LDR or photodiode circuit | Light level |
| Thermistor divider | Temperature estimate |
| Resistive pressure or flex sensor | Force, pressure, or bend estimate |
| Joystick axis | User input position |
| Battery divider | Battery-voltage monitoring |
| Current shunt and amplifier | Current measurement |
| Analog microphone or conditioned signal | Low-rate waveform or level measurement |
| Internal temperature sensor | MCU die-temperature estimate |
| `VREFINT` | Supply or ADC-reference diagnostics |

For a voltage divider, keep the divided voltage inside the ADC input range. For an
I2C, SPI, or digital sensor, use the digital peripheral instead of an ADC input.

## Practical rules

- Connect analog ground and supply according to the datasheet.
- Keep the input voltage between `VREF-` and `VREF+`.
- Do not connect a sensor output directly if its voltage can exceed the ADC range.
- Check the sensor source impedance before choosing the sample time.
- Add an RC filter when the signal contains unwanted noise, but account for its
  settling time.
- Average repeated readings when the signal is slow and noisy.
- Use timer-triggered conversion when the sample interval must be stable.
- Use DMA when software must process a buffer instead of one result at a time.
- Calibrate the sensor system if the application needs units rather than a raw
  relative value.
- Keep ADC pins in analog mode while they are used by the ADC.

## Reference manual pages

| Topic | Direct link |
|---|---|
| ADC introduction and main features | [RM0008 pages 215-216](../refs/stm32f103x8-reference.pdf#page=215) |
| ADC clock, channels, and internal sources | [RM0008 pages 218-219](../refs/stm32f103x8-reference.pdf#page=218) |
| Analog watchdog and scan mode | [RM0008 pages 220-221](../refs/stm32f103x8-reference.pdf#page=220) |
| Calibration and data alignment | [RM0008 pages 223-224](../refs/stm32f103x8-reference.pdf#page=223) |
| Sample time and external triggers | [RM0008 pages 225-226](../refs/stm32f103x8-reference.pdf#page=225) |
| DMA and dual ADC mode | [RM0008 pages 227-228](../refs/stm32f103x8-reference.pdf#page=227) |
| ADC status and control registers | [RM0008 pages 237-243](../refs/stm32f103x8-reference.pdf#page=237) |
| ADC sample-time registers | [RM0008 pages 244-245](../refs/stm32f103x8-reference.pdf#page=244) |
| ADC sequence and data registers | [RM0008 pages 246-249](../refs/stm32f103x8-reference.pdf#page=246) |
