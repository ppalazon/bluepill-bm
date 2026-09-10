# Extract hardware facts

Turn each hardware requirement into a short trace from the manual to the C code.
For a peripheral, the trace normally includes its clock, reset state, pins,
configuration registers, status flags, and interrupt number.

## Peripheral extraction procedure

1. Find the peripheral chapter in the reference manual.
2. Find its bus and clock-enable bit in the RCC chapter.
3. Find its reset values and register map.
4. Find the package-specific pins in the datasheet.
5. Find alternate-function or remapping rules.
6. Write the required configuration order.
7. Record assumptions about the system and peripheral clocks.
8. Implement the smallest polling example.
9. Add interrupts, DMA, or higher-level APIs only after polling works.

## Configuration record

Use a record like this for each new peripheral:

| Item | Value for this use |
|---|---|
| Instance | `USART2` |
| Bus | APB1 |
| Clock enable | `RCC_APB1ENR_USART2EN` |
| Pins | `PA2` TX, `PA3` RX |
| GPIO modes | Alternate push-pull TX, input RX |
| Clock assumption | `PCLK1 = 8 MHz` |
| First registers | `SR`, `DR`, `BRR`, `CR1` |
| Verification | Observe TX with a 3.3 V UART adapter |

The record is not a replacement for the reference manual. It is an implementation
checklist that makes hidden assumptions visible.

## Register access rules

Check the register description before writing code:

- Preserve reserved bits unless the manual permits a full-register write.
- Use `volatile` for memory-mapped registers.
- Use the documented write sequence for status flags.
- Enable the peripheral clock before accessing its registers.
- Configure GPIO and alternate functions before enabling a signal source.
- Recalculate timing values when the clock tree changes.

The [USART](../peripherals/usart.md) and [timers](../peripherals/timers.md) pages
show this procedure for concrete STM32F1 peripherals.
