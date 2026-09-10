# Identify the target

Start with the exact MCU and board. A Cortex-M3 compiler flag does not identify an
MCU, and a board name does not identify its silicon, package, or memory size.

## Record the target

Create a target profile with these fields before writing startup code:

| Field | Example |
|---|---|
| Board | Blue Pill |
| MCU ordering code | `STM32F103C8T6` |
| Silicon family | STM32F1 medium density |
| Core | ARM Cortex-M3 |
| Package | LQFP48 |
| Flash | 64 KiB |
| SRAM | 20 KiB |
| Debug | SWD through ST-Link |
| Board LED | Usually `PC13`, active-low |

Verify each value against the correct source. The board can contain a compatible
clone, so record the marking and the source documents used for that device.

## Resolve variant differences

Do not copy a linker script or startup file from a nearby part until these values
match:

- Flash size and origin
- SRAM size and origin
- Package pin count
- Interrupt vector list
- Peripheral instances
- Alternate-function mappings
- Device header symbol and compiler define

The project uses `STM32F103xB` for the vendor CMSIS device family because the C8T6
belongs to the 64 KiB to 128 KiB medium-density range. The linker still models the
actual 64 KiB C8T6 target.

## Output

The result of this step is a target profile, a memory map, a pinout, and a list of
known restrictions. The rest of the workflow must refer to those pages instead
of repeating target facts.
