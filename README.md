# Blue Pill Bare-Metal

Small bare-metal STM32F103C8T6 Blue Pill learning project built without
STM32CubeIDE.

The objective is to make the full firmware path explicit:

- What happens after reset
- How the vector table and startup code reach `main()`
- How the linker script maps Flash and RAM
- How C runtime sections like `.data` and `.bss` are prepared
- How peripheral registers are configured directly
- How CMSIS-based examples compare with direct-register code later

Target hardware:

- Board: Blue Pill development board
- MCU: STM32F103C8T6
- Core: ARM Cortex-M3
- Flash: 64 KiB official C8T6 size
- RAM: 20 KiB
- Debug/programming: ST-Link V2 clone over SWD
- Onboard LED: usually `PC13`, active-low

![Blue Pill board](docs/assets/img-20260904-094011.png)

The project intentionally stays small and educational. Early `bare-*` examples
use local register definitions only. `cmsis-*` examples can use the pinned
STM32CubeF1 CMSIS headers while keeping that abstraction layer separate.

Build, flash, debug, repository layout, and project workflow notes live in the
documentation site under `docs/`.
