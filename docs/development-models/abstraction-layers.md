# C development models

The hardware workflow stays the same when the C abstraction changes. The linker,
startup path, clocks, pins, registers, build, and verification still have to match
the target.

## Comparison

| Model | Register access | Main dependency | Hardware visibility | Best first use |
|---|---|---|---|---|
| Direct-register C | Project-owned addresses and masks | Local headers | Highest | Learn the hardware sequence |
| CMSIS | Vendor device structs and ARM core APIs | CMSIS-Core and device headers | High | Use standard names and core helpers |
| STM32 LL | Thin vendor peripheral functions | STM32 LL drivers | Medium | Reduce register boilerplate |
| STM32 HAL | Higher-level peripheral handles and APIs | STM32 HAL and configuration | Lower | Build features quickly |
| RTOS application | APIs provided by an operating system | RTOS plus a hardware layer | Depends on lower layers | Coordinate concurrent tasks |

These are software choices, not different hardware workflows. A HAL call still
depends on the correct clock, pin, reset, interrupt, and device configuration.

## Recommended learning order

Use the smallest model that answers the current question:

1. Direct-register C for reset, GPIO, clocks, and one simple peripheral.
2. CMSIS for standard Cortex-M core access and vendor device descriptions.
3. LL when repeated register sequences must become small vendor helpers.
4. HAL when application delivery matters more than exposing every register.
5. An RTOS after the interrupt, timing, memory, and driver behavior is understood.

The project currently implements the first two models. LL and HAL examples must
use separate application prefixes so their dependencies cannot leak into
`bare-*` applications.

See [direct-register C](direct-registers.md) and [CMSIS](cmsis.md) for the
implemented models.
