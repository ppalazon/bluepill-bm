# AGENTS.md

Guidance for AI agents working in this STM32 Blue Pill bare-metal project.

## Project Goal

This repository is a learning project for bare-metal STM32F103C8T6 development
without STM32CubeIDE. Keep the project small, explicit, and educational. Prefer
code and docs that explain the hardware path from reset, linker script, startup
code, registers, and peripherals.

Target hardware:

- Board: Blue Pill development board
- MCU: STM32F103C8T6
- Core: ARM Cortex-M3
- Flash: 64 KiB official C8T6 size
- RAM: 20 KiB
- Debug/programming: ST-Link V2 clone over SWD
- Onboard LED: usually `PC13`, active-low

## Current Tooling

Use the existing Makefile and docs tooling.

Common build commands:

```sh
make
make APP=bare-blink
make APP=cmsis-blink
make clean
```

Flash and debug commands:

```sh
make flash
make openocd
make debug
```

Docs verification:

```sh
mise exec -- mkdocs build --strict
```

The strict MkDocs build currently emits an informational Material for MkDocs
2.0 warning. That warning is known and is not a project failure if the build
exits successfully.

## Repository Layout

Important paths:

- `apps/`: one application per directory
- `apps/bare-blink/`: direct-register LED blink example
- `apps/cmsis-blink/`: CMSIS-based LED blink example
- `src/`: shared runtime code linked into applications
- `src/startup_stm32f103.c`: active startup and vector table
- `src/system_stm32f103.c`: current minimal system initialization
- `src/newlib_stubs.c`: minimal Newlib syscall stubs
- `include/stm32f103c8t6.h`: local direct-register header for `bare-*` apps
- `drivers/cmsis/`: project CMSIS-oriented driver code
- `src/asm/startup_stm32f103c8tx.s`: reference assembly startup, not linked by
  default
- `linker/STM32F103C8TX_FLASH.ld`: active linker script
- `openocd/bluepill.cfg`: OpenOCD config for this board/probe
- `docs/`: MkDocs source files
- `docs/refs/`: local PDF datasheets and reference manuals
- `vendor/STM32CubeF1/`: pinned STM32CubeF1 Git submodule
- `build/`: generated build output; do not edit by hand
- `site/`: generated MkDocs output; do not edit by hand

## Documentation architecture

The documentation uses four boundaries. Keep each fact in its owning section
and link to it from summaries instead of copying it:

- `docs/workflow/`: reusable microcontroller-development workflow,
  source-document roles, target identification, hardware-fact extraction, and
  bring-up checks
- `docs/architecture/`: processor and runtime concepts such as reset,
  interrupts, clocks, the linker script, and startup code
- `docs/targets/stm32f103c8t6/`: STM32F103C8T6 and Blue Pill facts, including
  the memory map, package pinout, board wiring, and device restrictions
- `docs/development-models/`: direct-register C, CMSIS, and comparisons with
  LL, HAL, and RTOS development
- `docs/peripherals/`: peripheral concepts and STM32F1 configuration procedures
- `docs/applications/`: small experiments that prove one hardware concept
- `docs/project/`: repository-specific build, flash, and debug procedures
- `docs/reference/`: shared terminology and reference material

The workflow starts with the datasheet, microcontroller reference manual,
processor manual, errata, and board schematic. It then produces a target
profile, memory map, linker script, startup code, register definitions, a
minimal application, and verification steps. The STM32F103C8T6 project is the
worked example, not the definition of the general workflow.

When documenting a new peripheral, identify its bus and clock, reset state,
pins, alternate functions, registers, configuration order, clock assumption,
status flags, interrupts, and a minimal verification method. Use the existing
peripheral pages as examples.

## Documentation writing rules

Load the `simple-english` skill before writing or substantially revising
technical documentation. Use short sentences, active voice, defined terms,
clear commands, and simple headings. Load the `writing-for-agents` skill before
modifying `AGENTS.md`, a skill, or any other instruction document. Run the
strict MkDocs build after changing docs or navigation.

Prefer short conceptual pages over copied manual chapters. State whether a
claim comes from the processor manual, MCU reference manual, datasheet, errata,
board documentation, source code, or a verified command. Do not silently apply
an ST device fact to a compatible clone.

## Application Naming Rules

Application prefixes define the abstraction layer:

- `bare-*`: direct-register examples using only local project headers
- `cmsis-*`: examples allowed to use vendored CMSIS headers from STM32CubeF1
- `ll-*`: future examples for STM32 LL drivers, only add when explicitly needed
- `hal-*`: future examples for STM32 HAL, only add when explicitly needed

Keep these layers separate. Do not include CMSIS, HAL, LL, or vendor headers in
`bare-*` examples.

## Build Boundaries

The Makefile enforces the current boundary:

- Default `APP` is `bare-blink`.
- Project name is `bluepill-$(APP)`.
- `bare-*` apps compile with `-Iinclude` only.
- `cmsis-*` apps also get CMSIS include paths and `-DUSE_CMSIS -DSTM32F103xB`.
- `drivers/cmsis/*.c` is compiled only for `cmsis-*` apps.

Do not add global CMSIS include paths to all builds. If a new app needs a
layer, make the Makefile condition explicit and prefix the app accordingly.

## CMSIS And Vendor Policy

STM32CubeF1 is a Git submodule at:

```text
vendor/STM32CubeF1
```

Current intended state:

```text
d12e75247d5bcedc734f829b394517ab4c2726e3 vendor/STM32CubeF1 (v1.8.7)
```

The submodule is pinned to ST's official `v1.8.7` release tag. Do not move it
to a branch tip unless explicitly asked.

CMSIS include paths:

```text
vendor/STM32CubeF1/Drivers/CMSIS/Core/Include
vendor/STM32CubeF1/Drivers/CMSIS/Device/ST/STM32F1xx/Include
```

For Blue Pill `STM32F103C8T6`, the STM32CubeF1 device define is:

```text
STM32F103xB
```

CMSIS source files should include the family entry header:

```c
#include "stm32f1xx.h"
```

Do not include `stm32f103xb.h` directly unless there is a concrete reason.

## clangd Policy

Keep clangd configuration layered:

- Root `.clangd`: common C and Cortex-M flags only
- `apps/.clangd`: local include paths, with CMSIS flags for `cmsis-*` paths
- `drivers/cmsis/.clangd`: CMSIS driver include paths and symbols

Do not use global `CPATH` for this project. Do not add CMSIS include paths to
the root `.clangd`.

Known header handling in `drivers/cmsis/.clangd`:

```yaml
---
If:
  PathMatch: .*\.h
CompileFlags:
  Add:
    - -x
    - c-header
```

This avoids clangd guessing Objective-C++ for standalone or empty `.h` files.

## Startup And Linker Rules

The active startup file is `src/startup_stm32f103.c`.

It is responsible for:

- Providing the interrupt vector table
- Loading the initial stack pointer from `_estack`
- Calling `SystemInit()`
- Copying `.data` from flash to RAM
- Clearing `.bss`
- Calling `main()`
- Routing unimplemented interrupts to `Default_Handler`

Interrupt handlers are weak aliases. Defining a function with the same handler
name in an app or driver overrides the default.

The assembly startup file in `asm/` is reference material only. Do not wire it
into the build unless explicitly asked.

The active linker script is `linker/STM32F103C8TX_FLASH.ld` and currently
models:

```text
FLASH: 0x08000000, 64K
RAM:   0x20000000, 20K
```

## Clock Assumptions

`src/system_stm32f103.c` currently keeps the chip on its simple default clock
setup. It does not yet configure HSE, PLL, 72 MHz system clock, flash wait
states, or bus prescalers.

When adding timing-sensitive code, state the assumed clock clearly. SysTick
docs and examples should not silently assume 72 MHz until clock setup exists.

## Documentation Style

Docs are written for learning. Prefer concise conceptual pages over exhaustive
manual rewrites.

For peripheral docs, follow the style of `docs/gpio.md` and `docs/systick.md`:

- Start with a short plain-English explanation
- Add a `Basic Idea` section
- Add a `Registers You Use Most` section
- Include a small practical C example when useful
- Add practical hardware or usage notes
- End with `Reference Manual Pages`
- Link to project-local PDFs in `docs/refs/`

Use direct PDF page anchors when referencing RM0008:

```markdown
[RM0008 page 159](refs/stm32f103x8-reference.pdf#page=159)
```

Known useful RM0008 anchors:

- GPIO and AFIO chapter start: `refs/stm32f103x8-reference.pdf#page=159`
- GPIO port structure and modes: `refs/stm32f103x8-reference.pdf#page=160`
- GPIO mode table: `refs/stm32f103x8-reference.pdf#page=161`
- GPIO behavior details: `refs/stm32f103x8-reference.pdf#page=163`
- GPIO registers: `refs/stm32f103x8-reference.pdf#page=171`
- `RCC_APB2ENR`: `refs/stm32f103x8-reference.pdf#page=112`
- AFIO remapping overview: `refs/stm32f103x8-reference.pdf#page=175`
- `AFIO_MAPR`: `refs/stm32f103x8-reference.pdf#page=184`
- SysTick clock source note: `refs/stm32f103x8-reference.pdf#page=93`
- SysTick calibration value: `refs/stm32f103x8-reference.pdf#page=197`
- SysTick vector-table entry: `refs/stm32f103x8-reference.pdf#page=204`

RM0008 contains STM32-specific SysTick information, but the bit-level SysTick
register definition belongs to ARM Cortex-M3 core documentation.

After changing docs or nav, run:

```sh
mise exec -- mkdocs build --strict
```

## OpenOCD Notes

Use `openocd/bluepill.cfg` for this board. It includes:

```tcl
set CPUTAPID 0x2ba01477
```

This was needed because the board/probe reported Cortex-M3 debug ID
`0x2ba01477`, while the default STM32F1 OpenOCD config expected a different
value.

## Code Style

Keep changes minimal and explicit.

- Prefer direct, readable C over abstractions in early learning examples
- Keep register examples close to the hardware being explained
- Use `volatile` for memory-mapped registers
- Avoid adding compatibility layers without a concrete need
- Avoid introducing HAL or LL into bare-register examples
- Preserve generated-output directories as generated only
- Do not edit vendored STM32CubeF1 files unless explicitly requested
- Prefer small comments that explain hardware intent, not obvious C syntax

## Verification Expectations

For firmware changes, run the smallest relevant build:

```sh
make APP=bare-blink
make APP=cmsis-blink
```

For changes that affect shared runtime, startup, linker, or Makefile behavior,
build both bare and CMSIS examples if available.

For docs changes, run the strict MkDocs build. If a command cannot be run
because the toolchain is missing or hardware is unavailable, report that
clearly.

## Current Useful Next Steps

Likely future project additions:

1. Add a SysTick interrupt example.
2. Add USART1 output and implement `board_putchar()` for `printf`.
3. Configure the STM32F103 clock tree for 72 MHz.
4. Add more direct-register peripheral examples before introducing higher
   layers.
5. Keep improving docs with local reference-manual links.
