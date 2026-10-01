# Blue Pill Bare-Metal

The main objective of this project is to learn how create bare metal
applications for any kind of microcontroller. It includes create the necessary
code to initialize the processor, drivers, and applications. I won't use the
official IDE or tools, but we get some vendor dependencies to simplify the
driver generation.

To make it as a practical project, I've just selected the Blue Pill board with
the STM32F103C microcontroller. It's the one that I had at home.

![Blue Pill board](docs/assets/img-20260904-094011.png)

- Board: Blue Pill development board
- MCU: STM32F103C8T6
- Core: ARM Cortex-M3
- Flash: 64 KiB official C8T6 size
- RAM: 20 KiB
- Debug/programming: ST-Link V2 clone over SWD
- Onboard LED: usually `PC13`, active-low

## Dependencies

This project has some dependencies to simply the development process, included
as git submodules. This method allows you update or change versions quickly.

- [STMicroelectronics/STM32CubeF1](https://github.com/STMicroelectronics/STM32CubeF1):
  HAL + LL Drivers, CMSIS Core, CMSIS Device, and MW libraries

## Getting started

You can replicate this repository following these steps:

```bash
git clone https://github.com/ppalazon/bluepill-bm.git
cd bluepill-bm
git submodule update --init --recursive
```

Once you've cloned it and got all dependencies, you can start compiling and
flashing to the Blue Pill board using the ST-Link V2 connector.

```bash
make APP=cmsis-dma-mem2mem
make APP=cmsis-dma-mem2mem flash
```

## Objectives

The first part is to know how to initialize the board and initialize a useful
applications.

- [x] Get datasheets and references manuals for the microcontroller STM32F103C.
- [x] Get blue pill board manual.
- [x] Get the memory map of the microcontroller.
- [x] Describe what happens after the reset or power on.
- [x] Prepare the linker script map to Flash and RAM
- [x] Write a C runtime code to prepare `.data` and `.bss`
- [x] Declare the vector table and startup code reach `main()`

The second part is to write drivers for the peripherals and test applications
for each peripheral.

- [x] GPIO peripheral
- [x] System Tick (SysTick) Timer
- [x] General-Purpose Timers (TIM)
- [x] The Universal Asynchronous Receiver / Transmitter Protocol (UART)
- [x] Analog-to-Digital Converter (ADC)
- [x] Serial Peripheral Interface (SPI)
- [ ] Inter-Integrated Circuit (I2C)
- [x] External Interrupts and Events (EXTI)
- [x] The Real-Time Clock (RTC)
- [x] Independent Watchdog (IWDG)
- [x] Direct Memory Access (DMA)
- [x] Power Management and Energy Efficiency

The third part consists in generate these same drivers using higher level
libraries such as Hardware Abstract Layer (HAL) and Low Layer (LL).

You can read more about this project on
[the documentation](https://pablo.palazon.dev/bluepill-bm)

## License

The code and documentation in this repository use the
[BSD 3-Clause License](LICENSE). Reference documents and other third-party
material remain under the licenses and terms of their copyright holders.

## Trademark notice

STM32 is a trademark of STMicroelectronics. This independent project uses the
STM32 name only to identify the target device and compatible software. The
project is not affiliated with or endorsed by STMicroelectronics.
