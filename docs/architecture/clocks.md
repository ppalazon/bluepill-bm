# Clock assumptions

Timing code is correct only when it uses the actual clock that feeds the relevant
peripheral. The current project keeps the MCU on the default 8 MHz HSI clock.

## Current clock tree

`src/system_stm32f103.c` enables HSI, selects HSI as `SYSCLK`, clears the clock
configuration register, and leaves the APB prescalers at their reset values. The
current examples therefore assume:

```text
HSI     = 8 MHz
SYSCLK  = 8 MHz
HCLK    = 8 MHz
PCLK1   = 8 MHz
PCLK2   = 8 MHz
```

The project does not currently configure HSE, PLL, 72 MHz operation, or flash wait
states. Do not use 72 MHz in an example until the clock setup and its flash timing
are implemented.

## Timing procedure

Before calculating a divider:

1. Identify the clock source.
2. Identify the bus clock.
3. Check the peripheral-specific clock multiplier or divider.
4. Calculate the register value.
5. State the clock assumption beside the example.

For STM32F1 timers, the timer clock equals `PCLK` when the APB prescaler is one.
When the APB prescaler is greater than one, the timer clock is twice `PCLK`. For
USART, use the relevant `PCLK` directly.

See [SysTick](../peripherals/systick.md), [USART](../peripherals/usart.md), and
[timers](../peripherals/timers.md) for examples.
