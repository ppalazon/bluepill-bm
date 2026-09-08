# STM32F103C8T6 Acronyms

This list covers the main acronyms used throughout the project documentation and
local STM32 headers.

## Bus And Address Space

These terms describe how the CPU reaches memory, peripherals, and internal core registers.

| Acronym | Meaning | Context |
|---|---|---|
| `AHB` | Advanced High-performance Bus | Higher-speed ARM bus used for core memory/peripheral access |
| `APB` | Advanced Peripheral Bus | ARM peripheral bus; STM32F103 has APB1 and APB2 |
| `PPB` | Private Peripheral Bus | Cortex-M internal peripheral region at `0xE000_0000` |
| `SCS` | System Control Space | Cortex-M core peripheral space near `0xE000_E000` |

## Memory And Boot

These terms describe where code/data live and how the chip chooses what to execute after reset.

| Acronym | Meaning | Context |
|---|---|---|
| `BOOT` | Boot selection | Pins used to select Flash, system memory, or SRAM boot |
| `FLITF` | Flash Interface | Flash memory interface, often used in RCC clock bits |
| `MiB` | Mebibyte | Binary memory unit: 1 MiB = 1024 KiB |
| `RAM` | Random-Access Memory | Writable memory used for runtime data |
| `ROM` | Read-Only Memory | Non-volatile read-only memory; system bootloader lives in system memory ROM |
| `SRAM` | Static Random-Access Memory | Internal RAM; `STM32F103C8T6` has 20 KiB |
| `UID` | Unique Identifier | Factory-programmed unique device ID |

## Cortex-M Core And Debug

These terms belong to the ARM Cortex-M3 core itself, especially interrupts, system control, and debugging.

| Acronym | Meaning | Context |
|---|---|---|
| `MPU` | Memory Protection Unit | Optional Cortex-M protection unit; not implemented on many STM32F103 parts |
| `NVIC` | Nested Vectored Interrupt Controller | Cortex-M interrupt controller |
| `SCB` | System Control Block | Cortex-M system control registers |
| `STIR` | Software Trigger Interrupt Register | Cortex-M register used to trigger interrupts in software |
| `SWD` | Serial Wire Debug | ARM two-pin debug interface |
| `SWJ` | Serial Wire/JTAG | Debug port configuration covering SWD and JTAG pins |
| `JTAG` | Joint Test Action Group | ARM debug interface sharing some GPIO pins with alternate functions |
| `SysTick` | System Tick Timer | Cortex-M 24-bit timer commonly used for OS ticks or delays |

## STM32 Peripherals

These are hardware blocks provided by the STM32F103 around the Cortex-M3 CPU.

| Acronym | Meaning | Context |
|---|---|---|
| `ADC` | Analog-to-Digital Converter | Converts analog input voltage to a digital value |
| `AFIO` | Alternate Function I/O | Controls alternate pin functions, remapping, and EXTI source selection |
| `BKP` | Backup Registers | Battery-backed backup register peripheral |
| `CAN` | Controller Area Network | CAN bus controller peripheral; STM32 docs often call it bxCAN |
| `CRC` | Cyclic Redundancy Check | Hardware CRC calculation peripheral |
| `DMA` | Direct Memory Access | Peripheral that transfers data without CPU copying each word |
| `EXTI` | External Interrupt/Event Controller | Handles external interrupt/event lines |
| `GPIO` | General-Purpose Input/Output | Digital I/O port peripheral |
| `I2C` | Inter-Integrated Circuit | Two-wire serial bus peripheral |
| `IWDG` | Independent Watchdog | Watchdog clocked independently from the main system clock |
| `PWR` | Power Control | Peripheral for power and low-power mode control |
| `RCC` | Reset and Clock Control | Peripheral that controls clocks and peripheral resets |
| `RTC` | Real-Time Clock | Low-power timekeeping peripheral |
| `SPI` | Serial Peripheral Interface | Synchronous serial bus peripheral |
| `TIM` | Timer | STM32 timer peripheral prefix |
| `USART` | Universal Synchronous/Asynchronous Receiver/Transmitter | Serial communication peripheral |
| `USB` | Universal Serial Bus | USB full-speed device peripheral |
| `WWDG` | Window Watchdog | Watchdog that must be refreshed inside a timing window |

## Timer Peripheral Terms

These terms are used by the STM32F103C8T6 timer peripherals and their
registers.

| Acronym | Meaning | Context |
|---|---|---|
| `ARR` | Auto-Reload Register | Holds the terminal count that produces an update event |
| `ARPE` | Auto-Reload Preload Enable | `CR1` bit that buffers `ARR` until an update event |
| `BDTR` | Break and Dead-Time Register | TIM1 register for break, dead-time, and output safety control |
| `BKIN` | Break Input | TIM1 safety input that can disable timer outputs |
| `CC` | Capture/Compare | Timer channel function for measuring an input or acting on a count match |
| `CCER` | Capture/Compare Enable Register | Enables channels and selects capture edge or output polarity |
| `CCMR` | Capture/Compare Mode Register | Selects input capture, output compare, or PWM mode |
| `CCR` | Capture/Compare Register | Stores a captured count or an output compare threshold |
| `CHx` | Timer channel x | One of the timer's numbered capture/compare channels |
| `CHxN` | Complementary output of timer channel x | TIM1 complementary output for channels 1 through 3 |
| `CEN` | Counter Enable | `CR1` bit that starts the timer counter |
| `CNT` | Counter Register | Holds the current timer count |
| `DIER` | DMA/Interrupt Enable Register | Enables timer interrupt and DMA request sources |
| `DTG` | Dead-Time Generator | `BDTR` field that delays complementary TIM1 output transitions |
| `EGR` | Event Generation Register | Generates update, capture/compare, or trigger events in software |
| `ETR` | External Trigger | Dedicated external timer trigger input |
| `HSI` | High-Speed Internal oscillator | Default 8 MHz system clock source in this project |
| `IC` | Input Capture | Captures `CNT` on a selected input edge |
| `ITR` | Internal Trigger | Internal timer-to-timer trigger connection |
| `MMS` | Master Mode Selection | `CR2` field selecting the timer master trigger output |
| `MOE` | Main Output Enable | TIM1 `BDTR` bit that permits outputs to drive their pins |
| `OC` | Output Compare | Changes or reports an output when `CNT` matches `CCR` |
| `OPM` | One-Pulse Mode | `CR1` bit that stops the timer after its next update event |
| `PCLK` | APB Peripheral Clock | APB clock feeding a peripheral; timer clock can be twice this value |
| `PSC` | Prescaler Register | Divides the timer input clock by `PSC + 1` |
| `PWM` | Pulse-Width Modulation | Timer output whose duty cycle is selected by a capture/compare register |
| `RCR` | Repetition Counter Register | TIM1 register that delays update events by a programmed number of cycles |
| `SMS` | Slave Mode Selection | `SMCR` field selecting synchronization, encoder, or external-clock mode |
| `SMCR` | Slave Mode Control Register | Selects timer trigger source and slave behavior |
| `SR` | Status Register | Timer register containing update, trigger, and capture/compare flags |
| `TI` | Timer Input | Internal name for a timer channel input, such as TI1 or TI2 |
| `TRGO` | Trigger Output | Timer event routed internally to synchronize or trigger another peripheral |
| `TS` | Trigger Selection | `SMCR` field selecting the timer trigger input |
| `UDE` | Update DMA Enable | `DIER` bit enabling DMA requests on update events |
| `UDIS` | Update Disable | `CR1` bit that suppresses update events and shadow-register transfers |
| `UEV` | Update Event | Timer overflow, underflow, trigger, or software event that updates buffered registers |
| `UG` | Update Generation | `EGR` bit that forces an update event in software |
| `UIE` | Update Interrupt Enable | `DIER` bit enabling interrupts on update events |
| `UIF` | Update Interrupt Flag | `SR` flag set when an update event occurs |
| `URS` | Update Request Source | `CR1` bit limiting update interrupt requests to counter overflow or underflow |

## RCC Registers And Bits

These terms are used to configure clocks and reset control for the chip and its peripherals.

| Acronym | Meaning | Context |
|---|---|---|
| `BDCR` | Backup Domain Control Register | RCC register for backup-domain clock control |
| `CFGR` | Configuration Register | RCC clock configuration register |
| `CIR` | Clock Interrupt Register | RCC register for clock interrupt flags and enables |
| `CR` | Control Register | Generic register name; in RCC, `CR` is Clock Control Register |
| `CSR` | Control/Status Register | RCC register containing reset flags and LSI control/status |
| `EN` | Enable | Common suffix for enable bits, such as `IOPCEN` |
| `ENR` | Enable Register | Common RCC suffix for clock-enable registers |
| `RSTR` | Reset Register | Common RCC suffix for peripheral reset registers |

## GPIO Registers And Fields

These terms are used to configure and control digital input/output pins.

| Acronym | Meaning | Context |
|---|---|---|
| `BRR` | Bit Reset Register | GPIO register used to reset output pins |
| `BSRR` | Bit Set/Reset Register | GPIO register used to atomically set or reset output pins |
| `CNF` | Configuration | GPIO pin configuration field paired with `MODE` |
| `CRH` | Configuration Register High | GPIO config register for pins 8-15 |
| `CRL` | Configuration Register Low | GPIO config register for pins 0-7 |
| `IDR` | Input Data Register | GPIO register used to read input pin states |
| `LCKR` | Lock Register | GPIO register used to lock pin configuration |
| `MODE` | Mode | GPIO field selecting input or output speed mode |
| `ODR` | Output Data Register | GPIO register used to read/write output pin states |
| `PP` | Push-Pull | GPIO output type where the pin is actively driven high and low |

## AFIO And EXTI Registers

These terms are used to route pin functions and configure external interrupt/event lines.

| Acronym | Meaning | Context |
|---|---|---|
| `EMR` | Event Mask Register | EXTI register controlling which lines generate events |
| `EXTICR` | External Interrupt Configuration Register | AFIO register selecting which GPIO port feeds an EXTI line |
| `FTSR` | Falling Trigger Selection Register | EXTI register selecting falling-edge trigger lines |
| `IMR` | Interrupt Mask Register | EXTI register controlling which lines generate interrupts |
| `MAPR` | Remap Register | AFIO register controlling alternate-function remapping and SWJ config |
| `PR` | Pending Register | EXTI register holding pending interrupt/event flags |
| `RTSR` | Rising Trigger Selection Register | EXTI register selecting rising-edge trigger lines |
| `SWIER` | Software Interrupt Event Register | EXTI register used to trigger EXTI lines from software |

## Board And Pin Labels

These terms are common labels on boards and schematics, not CPU register names.

| Acronym | Meaning | Context |
|---|---|---|
| `VCC` | Voltage Common Collector | Board label commonly used for positive supply voltage |
| `VSS` | Voltage Source Substrate | ST naming for ground pins |

## STM32 Naming Patterns

This section explains how long register and bit names are assembled from smaller pieces.

STM32 register and bit names are usually built from a peripheral name, register name, and field or bit name.

Example:

```text
RCC_APB2ENR_IOPCEN
```

This means:

| Part | Meaning |
|---|---|
| `RCC` | Reset and Clock Control peripheral |
| `APB2ENR` | APB2 peripheral clock enable register |
| `IOPCEN` | I/O port C clock enable bit |

Common suffixes:

| Suffix | Meaning | Example |
|---|---|---|
| `BASE` | Base address of a memory region or peripheral | `GPIOC_BASE` |
| `OFFSET` | Register offset from a peripheral base address | `GPIO_ODR_OFFSET` |
| `EN` | Enable bit | `RCC_APB2ENR_IOPCEN` |
| `RSTR` | Reset register | `RCC_APB2RSTR` |
| `MASK` | Bit mask used to clear or isolate fields | `GPIO_CFG_MASK(pin)` |
| `CFG` | Configuration value or helper | `GPIO_CFG(pin, mode, cnf)` |

GPIO configuration register names:

| Register | Meaning |
|---|---|
| `GPIOx_CRL` | Configures pins 0-7 of GPIO port x |
| `GPIOx_CRH` | Configures pins 8-15 of GPIO port x |
| `GPIOx_IDR` | Reads input state for GPIO port x |
| `GPIOx_ODR` | Reads/writes output state for GPIO port x |
| `GPIOx_BSRR` | Atomically sets or resets GPIO output bits |
| `GPIOx_BRR` | Resets GPIO output bits |
| `GPIOx_LCKR` | Locks GPIO configuration |
