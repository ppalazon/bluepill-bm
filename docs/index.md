# Documentation map

This documentation has one purpose: show how to move from authoritative hardware
documents to a working C application. The STM32F103C8T6 Blue Pill is the worked
example. The workflow is intended to apply to other microcontrollers.

## Read the documentation in this order

```text
1. workflow
   identify the target and read the source documents
        |
        v
2. targets/stm32f103c8t6
   record the facts for this MCU and board
        |
        v
3. architecture
   connect the processor reset path to the MCU memory and clock model
        |
        v
4. development-models
   choose how C will access the hardware
        |
        v
5. peripherals
   implement one hardware block from its reference-manual procedure
        |
        v
6. applications
   prove the procedure with a small program
        |
        v
7. project
   build, flash, debug, and inspect the result
```

The [workflow overview](workflow/overview.md) is the first page to read. The
[bring-up checklist](workflow/bring-up-checklist.md) is the short verification
path after the target profile and linker setup exist.

## Directory ownership

Each directory answers a different question. Keep a fact in one owner and link to
it from other pages.

| Directory | Question | Example |
|---|---|---|
| `workflow/` | What process works for a new MCU? | [Source documents](workflow/source-documents.md) |
| `targets/` | What is true for this MCU, package, and board? | [Target profile](targets/stm32f103c8t6/target-profile.md) |
| `architecture/` | How does the processor boot and how is firmware placed in memory? | [Reset and interrupts](architecture/reset-and-interrupts.md) |
| `development-models/` | Which C software layer is being used? | [C development models](development-models/abstraction-layers.md) |
| `peripherals/` | How is an STM32F1 hardware block configured? | [USART](peripherals/usart.md) |
| `applications/` | Which small program proves the concept? | [Bare blink](applications/bare-blink.md) |
| `project/` | How does this repository build and debug it? | [Build process](project/build-process.md) |
| `reference/` | Where are shared terms defined? | [Acronyms](reference/acronyms.md) |
| `refs/` | Which primary documents support the claims? | Local PDF manuals and datasheets |

## Workflow outputs

The workflow produces these artifacts in order:

```text
source documents
  -> target profile
  -> memory map and pin plan
  -> clock assumptions
  -> linker script
  -> startup and vector table
  -> register definitions
  -> peripheral procedure
  -> minimal application
  -> verified firmware image
```

The project-specific outputs are implemented by these paths:

| Output | Implementation |
|---|---|
| Target memory layout | `linker/STM32F103C8TX_FLASH.ld` |
| Startup and vector table | `src/startup_stm32f103.c` |
| Minimal clock setup | `src/system_stm32f103.c` |
| Direct-register definitions | `include/stm32f103c8t6.h` |
| Direct-register application | `apps/bare-blink/main.c` |
| CMSIS drivers | `drivers/cmsis/` |
| Build selection | `Makefile` |

The source files are authoritative for current repository behavior. The
documentation explains the hardware reasoning, assumptions, and verification
method around those files.

## Choose an entry point

| If you need to... | Start with... |
|---|---|
| Learn how to approach a new MCU | [Workflow overview](workflow/overview.md) |
| Decide which manual to read | [Source documents](workflow/source-documents.md) |
| Identify the Blue Pill target | [Target profile](targets/stm32f103c8t6/target-profile.md) |
| Create a linker script | [Memory map](targets/stm32f103c8t6/memory-map.md), then [linker and startup](architecture/linker-startup.md) |
| Understand reset and `main` | [Reset and interrupts](architecture/reset-and-interrupts.md) |
| Add a peripheral | [Extract hardware facts](workflow/extracting-hardware-facts.md), then the relevant page in `peripherals/` |
| Compare C programming layers | [C development models](development-models/abstraction-layers.md) |
| Build or debug this repository | [Build process](project/build-process.md) |

## Current development layers

The application prefix identifies the software layer:

| Prefix | Layer | Headers |
|---|---|---|
| `bare-*` | Direct-register C | Local project headers only |
| `cmsis-*` | CMSIS | Pinned STM32CubeF1 CMSIS headers |
| `ll-*` | STM32 LL | Reserved for future examples |
| `hal-*` | STM32 HAL | Reserved for future examples |

The Makefile keeps these layers separate. Do not add vendor include paths to a
`bare-*` application.

## Repository commands

```sh
make
make APP=bare-blink
make APP=cmsis-blink
make flash
make openocd
make debug
```

After changing documentation or navigation, run:

```sh
mise exec -- mkdocs build --strict
```
