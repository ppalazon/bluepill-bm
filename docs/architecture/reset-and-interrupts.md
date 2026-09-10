# Reset and interrupts

The Cortex-M3 starts from a vector table. The first word supplies the initial
stack pointer and the second word supplies the reset handler address.

## Reset path

For this project the path is:

```text
reset
  -> vector table
  -> Reset_Handler
  -> SystemInit
  -> copy .data
  -> clear .bss
  -> main
```

The active implementation is `src/startup_stm32f103.c`. The processor manual
defines the vector-table and exception behavior. The STM32 reference manual
defines the device-specific interrupt order and names.

## Interrupt path

An interrupt requires all of these pieces:

1. The device vector table contains the correct handler slot.
2. The peripheral enables its interrupt source.
3. The peripheral sets its status flag.
4. The NVIC enables the corresponding IRQ.
5. The application defines the handler with the exact vector name.
6. The handler clears the flag according to the reference manual.

The startup file provides weak default handlers. An application can replace one by
defining a function with the same name, such as `SysTick_Handler` or
`TIM3_IRQHandler`.

## Core and device responsibilities

CMSIS-Core describes core peripherals such as the NVIC and SysTick. The vendor
device description supplies STM32 interrupt numbers and peripheral registers.
Direct-register applications can use the same hardware sequence with local
definitions, as long as the definitions are checked against the target documents.

See [linker script and startup file](linker-startup.md) for the section layout and
[CMSIS](../development-models/cmsis.md) for the software-layer comparison.
