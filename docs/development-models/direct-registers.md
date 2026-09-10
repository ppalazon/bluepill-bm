# Direct-register C

Direct-register code accesses the MCU registers through definitions owned by the
project. It is the smallest software layer in this repository and is the first
implementation model to use when learning a new peripheral.

## What the project owns

The `bare-*` applications use:

- `include/stm32f103c8t6.h` for the register definitions used by an example
- `src/startup_stm32f103.c` for startup and vectors
- `src/system_stm32f103.c` for minimal system setup
- The project linker script

They do not include CMSIS, HAL, LL, or vendor headers.

## Advantages and cost

This model makes addresses, masks, access width, and configuration order visible.
It also requires the developer to maintain every definition and verify it against
the reference manual. A small local header is suitable for teaching and focused
experiments, but it is not a complete device description.

## Minimal pattern

```c
RCC_APB2ENR |= RCC_APB2ENR_IOPCEN;
GPIOC_CRH &= ~GPIO_CFG_MASK(13u);
GPIOC_CRH |= GPIO_CFG(13u, GPIO_MODE_OUTPUT_2MHZ, GPIO_CNF_OUTPUT_PP);
GPIOC_BSRR = GPIO_PIN(13u) << 16;
```

The exact definitions must come from the target reference manual. The
[bare blink application](../applications/bare-blink.md) shows the complete path.
