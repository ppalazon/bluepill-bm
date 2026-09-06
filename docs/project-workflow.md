# Project Workflow

This page describes the path followed to build this STM32F103C8T6 bare-metal
project from scratch. It is intentionally schematic: the goal is to show the
order of decisions and the way of thinking, not every register detail.

## Mental Model

A bare-metal project has no operating system between the program and the chip.
The firmware must provide everything needed to boot, place code in memory,
initialize RAM, configure peripherals, and enter `main`.

The basic questions are always the same:

1. What chip am I targeting?
2. Where are Flash, RAM, and peripheral registers mapped?
3. What is the first instruction that runs after reset?
4. Which files are target-independent application code, and which files are
   target startup/runtime code?
5. Which hardware block must be configured before the application can use it?

For this project, the answers are:

| Question | Project answer |
| --- | --- |
| MCU | `STM32F103C8T6` |
| CPU | ARM Cortex-M3 |
| Flash | `0x08000000`, 64 KiB |
| SRAM | `0x20000000`, 20 KiB |
| First code | `Reset_Handler` from `src/startup_stm32f103.c` |
| First app | `apps/bare-blink/main.c` |
| First peripheral | GPIOC, pin `PC13`, the onboard LED |

## Build From The Outside In

The workflow is to build the project from the fixed hardware facts toward the
application code.

```text
chip facts
  -> memory map
  -> linker script
  -> startup code
  -> register definitions
  -> build system
  -> first peripheral driver/use
  -> application
  -> flash/debug workflow
```

Each layer depends on the layer before it. If the memory map is wrong, the
linker script is wrong. If the linker script is wrong, startup cannot prepare
RAM correctly. If startup is wrong, `main` may never run.

## Step 1: Identify The Target

Start with the exact board and MCU, not with generic ARM code.

For the Blue Pill:

```text
Board: Blue Pill
MCU:   STM32F103C8T6
CPU:   Cortex-M3
Flash: 64 KiB
RAM:   20 KiB
LED:   PC13, usually active-low
Debug: SWD through ST-Link
```

This decides the compiler flags, linker script memory sizes, OpenOCD target,
startup vector table, and peripheral register addresses.

## Step 2: Create The Memory Map

The linker needs to know where code and data can live.

For this project:

```ld
FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
```

The rule of thumb is simple:

| Data kind | Runtime location | Why |
| --- | --- | --- |
| Vector table | Flash | CPU reads it immediately after reset. |
| Code | Flash | Non-volatile program storage. |
| Constants | Flash | They do not need writable RAM. |
| Initialized variables | RAM | They change at runtime, but initial values are stored in Flash. |
| Zeroed variables | RAM | Startup clears them before `main`. |
| Stack | RAM | Function calls and local runtime state need writable memory. |

See [Memory Map](stm32f103c8t6-memory-map.md) and
[Linker And Startup](linker-startup.md) for the detailed version.

## Step 3: Write The Linker Script

The linker script places program sections into Flash and RAM.

The important jobs are:

1. Put `.isr_vector`, `.text`, and `.rodata` in Flash.
2. Put `.data`, `.bss`, heap, and stack in RAM.
3. Record where the initial `.data` values are stored in Flash.
4. Expose symbols that startup code can use, such as `_sidata`, `_sdata`,
   `_edata`, `_sbss`, `_ebss`, and `_estack`.

The linker script is the bridge between the ELF file and the physical memory of
the microcontroller.

## Step 4: Write Startup Code

On Cortex-M, the CPU starts from the vector table:

```text
word 0: initial stack pointer
word 1: reset handler address
```

The project startup file provides that vector table and the reset handler.

`Reset_Handler` does the minimum runtime setup:

1. Copy `.data` initial values from Flash to RAM.
2. Clear `.bss` in RAM to zero.
3. Call `SystemInit()` for early chip setup.
4. Call `main()`.
5. Loop forever if `main()` returns.

It also provides weak default interrupt handlers. That allows an application to
override only the interrupts it needs later.

## Step 5: Add Minimal System Code

`SystemInit()` exists because many embedded projects expect one early chip setup
function before `main`.

At the current learning stage it stays minimal. The project does not configure a
PLL or complex clock tree yet. The first goal is to boot reliably and run a
simple GPIO program.

The principle is: make the smallest hardware configuration that lets the next
step work.

## Step 6: Define Registers

For `bare-*` applications, the project uses a small local header:

```text
include/stm32f103c8t6.h
```

This header defines only the register addresses and bit masks currently needed.
That keeps the first examples readable and forces us to understand which
registers are being touched.

For example, using the LED requires:

| Need | Register |
| --- | --- |
| Enable GPIOC clock | `RCC_APB2ENR` |
| Configure PC13 | `GPIOC_CRH` |
| Toggle or drive PC13 | `GPIOC_ODR` or `GPIOC_BSRR` |

Later `cmsis-*` applications can use the vendored STM32CubeF1 CMSIS headers, but
the direct-register examples remain independent.

## Step 7: Build The Firmware

The `Makefile` turns source files into firmware outputs.

The build pipeline is:

```text
.c files
  -> .o object files
  -> .elf linked firmware with symbols
  -> .bin raw flash image
  -> .hex Intel HEX image
```

The key target flags are:

```text
-mcpu=cortex-m3
-mthumb
```

Those tell GCC to generate code for the actual CPU inside the STM32F103.

Application selection is done with `APP`:

```sh
make APP=bare-blink
make APP=cmsis-blink
```

The naming convention keeps responsibilities clear:

| Prefix | Meaning |
| --- | --- |
| `bare-*` | Uses local direct-register definitions only. |
| `cmsis-*` | Uses CMSIS headers from the pinned STM32CubeF1 submodule. |
| `ll-*` | Future STM32 LL-based examples. |
| `hal-*` | Future STM32 HAL-based examples. |

See [Build Process](build-process.md) for the detailed build explanation.

## Step 8: Make The First App Small

The first application should prove the complete boot-to-hardware chain with the
least possible peripheral complexity.

Blinking `PC13` is useful because it proves:

1. The linker placed the image correctly.
2. The vector table is valid.
3. Startup reached `main`.
4. The compiler generated valid Cortex-M3 code.
5. Register writes reach the RCC and GPIO peripherals.
6. The flashing/debug path works.

The first app does only this:

```text
enable GPIOC clock
configure PC13 as output
toggle PC13 forever
```

That is enough to validate the whole skeleton.

## Step 9: Flash And Debug

OpenOCD connects the host tools to the MCU through ST-Link and SWD.

The basic workflow is:

```text
make
make flash
```

For debugging:

```text
make openocd
make debug
```

The ELF file is the important debug artifact because it contains symbols and
source mapping. The `.bin` and `.hex` files are useful flash image formats, but
they do not contain the same debug information.

## Step 10: Add Complexity One Layer At A Time

After the first blink works, the project can grow safely.

The pattern is:

```text
one new hardware concept
  -> one small register/API addition
  -> one tiny application or driver use
  -> build
  -> flash or inspect
  -> document what changed
```

Good next layers are:

| Layer | What it teaches |
| --- | --- |
| GPIO input button | Pull-ups, input reads, debounce. |
| SysTick delay | CPU timer, interrupts or polling. |
| USART | Alternate functions, baud rate, serial debugging. |
| EXTI | Interrupt lines and NVIC. |
| Timers | Hardware timing and PWM. |
| CMSIS version | Same hardware, standard vendor register names. |
| LL/HAL versions | Higher-level vendor libraries and their tradeoffs. |

## Working Rules

Use these rules when building bare-metal projects:

1. Trust the reference manual more than examples found online.
2. Change one hardware assumption at a time.
3. Keep startup and linker code boring and explicit.
4. Prefer a working minimal program before adding abstractions.
5. Separate board facts, chip facts, runtime code, drivers, and applications.
6. Keep generated files out of source control.
7. Pin vendor dependencies so examples remain reproducible.
8. Build often, inspect the map file when memory layout matters, and debug from
   the ELF file.

The main idea is to make every layer explainable. If the LED blinks, we should
be able to say which register enabled the clock, which register configured the
pin, where the code lives in Flash, how RAM was initialized, and how the CPU
reached `main`.
