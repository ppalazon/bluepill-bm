# STM32F103C8T6 target profile

This project uses a Blue Pill board with an STM32F103C8T6-compatible target. The
profile below is the boundary between general Cortex-M workflow and STM32F1
implementation details.

## Device facts

| Item | Value |
|---|---|
| MCU | `STM32F103C8T6` |
| Family | STM32F1, medium density |
| Core | ARM Cortex-M3 |
| Package | LQFP48 |
| Flash | 64 KiB at `0x08000000` |
| SRAM | 20 KiB at `0x20000000` |
| Normal boot | Main Flash, `BOOT0 = 0` |
| Debug | SWD on `PA13` and `PA14` |
| Board LED | Usually `PC13`, active-low |

The exact memory layout is documented in the [memory map](memory-map.md). Pin and
package restrictions are documented in the [pinout](pinout.md).

## Software target

The compiler target is:

```text
-mcpu=cortex-m3 -mthumb
```

The CMSIS device family define used by this project is `STM32F103xB`. The linker
script uses the actual C8T6 capacity of 64 KiB Flash and 20 KiB SRAM.

## Board facts

The board is programmed and debugged through an ST-Link clone using SWD. Keep
`SWDIO`, `SWCLK`, ground, and target reference voltage connected. Do not treat the
Blue Pill board name as an electrical specification. Verify power, oscillator,
LED, and connector wiring on the board being used.

## Source boundary

Use the ST documents for STM32 behavior and the CKS documents when the installed
silicon is a CKS32-compatible clone. The [source-document guide](../../workflow/source-documents.md)
explains how to resolve differences.
