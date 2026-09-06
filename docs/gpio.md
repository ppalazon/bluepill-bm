# GPIO Peripheral

GPIO means General-Purpose Input/Output. A GPIO pin is a digital connection
between the STM32 and an external circuit. Each pin belongs to a port, such as
`GPIOA`, `GPIOB`, or `GPIOC`, and each port has up to 16 pins: `PA0..PA15`,
`PB0..PB15`, `PC0..PC15`, and so on.

The Blue Pill onboard LED is normally connected to `PC13`. It is usually
active-low, so driving `PC13` low turns the LED on and driving it high turns the
LED off.

## Basic Idea

A GPIO pin can be configured as one of these common modes:

| Mode | Meaning |
| --- | --- |
| Input floating | The MCU reads the external voltage; nothing inside pulls it high or low. |
| Input pull-up / pull-down | The MCU weakly pulls the pin to `VDD` or `GND` when nothing else drives it. |
| Analog | The digital input/output logic is disconnected; used for ADC or unused pins. |
| Output push-pull | The MCU actively drives the pin high or low. |
| Output open-drain | The MCU can drive low, but needs a pull-up resistor for high. |
| Alternate function | A peripheral such as USART, SPI, I2C, or TIM owns the pin. |

On STM32F1, each pin has a 4-bit configuration field:

```text
CNF[1:0]   configuration type
MODE[1:0]  input/output mode or output speed
```

Pins `0..7` are configured in `GPIOx_CRL`; pins `8..15` are configured in
`GPIOx_CRH`.

## Registers You Use Most

GPIO is controlled by memory-mapped registers. Writing specific bits changes how
the hardware behaves.

| Register | Purpose |
| --- | --- |
| `RCC_APB2ENR` | Enables the clock for GPIO ports and AFIO. A GPIO port must be clocked before use. |
| `GPIOx_CRL` | Configures pins `0..7` of one GPIO port. |
| `GPIOx_CRH` | Configures pins `8..15` of one GPIO port. |
| `GPIOx_IDR` | Reads the current input level on the pins. |
| `GPIOx_ODR` | Stores the output value for the pins. |
| `GPIOx_BSRR` | Atomically sets or resets output bits; safer than read-modify-write on `ODR`. |
| `AFIO_MAPR` | Remaps some peripheral functions to alternate pins. |

For example, configuring and driving the Blue Pill LED uses three ideas:

```c
RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;  /* Enable GPIOC clock. */

GPIOC->CRH &= ~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13);
GPIOC->CRH |= GPIO_CRH_MODE13_1;     /* PC13 output, 2 MHz push-pull. */

GPIOC->BSRR = GPIO_BSRR_BR13;        /* LED on: PC13 low. */
GPIOC->BSRR = GPIO_BSRR_BS13;        /* LED off: PC13 high. */
```

## External Connections

GPIO pins are not just software bits. They are electrical connections, so the
external circuit matters.

| Connection | Typical GPIO mode | Notes |
| --- | --- | --- |
| LED | Push-pull output | Use a current-limiting resistor unless the board already has one. |
| Button to GND | Input pull-up | Pin reads high when released, low when pressed. |
| Button to `VDD` | Input pull-down | Pin reads low when released, high when pressed. |
| I2C `SCL` / `SDA` | Alternate-function open-drain | Needs pull-up resistors. External pull-ups are preferred. |
| USART TX / SPI SCK / SPI MOSI | Alternate-function push-pull | The peripheral drives the output signal. |
| ADC input | Analog | Keep the voltage within the allowed input range. |

Avoid connecting two push-pull outputs together. If one output drives high while
the other drives low, they fight electrically and can damage the devices.

Also check the STM32 datasheet for pin limits, 5 V tolerance, maximum current,
and which alternate functions are available on each physical package pin.

## Reference Manual Pages

Use the project-local STM32F103 reference manual for the full register details:

| Topic | Direct link |
| --- | --- |
| GPIO and AFIO chapter start | [RM0008 page 159](refs/stm32f103x8-reference.pdf#page=159) |
| GPIO port structure and modes | [RM0008 page 160](refs/stm32f103x8-reference.pdf#page=160) |
| GPIO mode table | [RM0008 page 161](refs/stm32f103x8-reference.pdf#page=161) |
| Input, output, alternate, and analog behavior | [RM0008 page 163](refs/stm32f103x8-reference.pdf#page=163) |
| GPIO registers: `CRL`, `CRH`, `IDR`, `ODR`, `BSRR` | [RM0008 page 171](refs/stm32f103x8-reference.pdf#page=171) |
| `RCC_APB2ENR`: GPIO and AFIO clock enables | [RM0008 page 112](refs/stm32f103x8-reference.pdf#page=112) |
| AFIO remapping overview | [RM0008 page 175](refs/stm32f103x8-reference.pdf#page=175) |
| `AFIO_MAPR` register | [RM0008 page 184](refs/stm32f103x8-reference.pdf#page=184) |
