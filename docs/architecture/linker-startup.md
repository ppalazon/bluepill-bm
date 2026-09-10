# Linker script and startup file

This document explains how the linker script and startup file work together to
turn object files into firmware that can boot on an STM32F103C8T6 Blue Pill.

The linker script decides where every output section lives in memory. The startup
file contains the first code that runs after reset and prepares RAM before
calling `main`.

## Understanding load memory on STM32F103C8T6

The STM32F103C8T6 has one Cortex-M3 address space, but not every address points
to the same kind of memory.

For this project, the important regions are documented in the [memory map](../targets/stm32f103c8t6/memory-map.md):

| Region         |                 Address range |                Size | Used for                                                             |
| -------------- | ----------------------------: | ------------------: | -------------------------------------------------------------------- |
| Embedded Flash | `0x0800_0000` - `0x0800_FFFF` |              64 KiB | Program image, vector table, code, constants, initial `.data` values |
| Embedded SRAM  | `0x2000_0000` - `0x2000_4FFF` |              20 KiB | Stack, heap, `.data`, `.bss`, runtime variables                      |
| Boot alias     | `0x0000_0000` - `0x0000_FFFF` | 64 KiB alias window | Reset vector fetch, aliased to Flash when booting normally           |

Flash is non-volatile. The program remains there after power is removed. That is
where the executable image is programmed by OpenOCD.

SRAM is volatile. It is lost on reset or power-off, but it is writable and fast.
Variables that change at runtime must live in RAM.

Normal boot uses this mapping:

```text
BOOT0 = 0
0x00000000 aliases embedded Flash at 0x08000000
```

The Cortex-M3 initially reads the vector table from address `0x00000000`. Because
Flash is aliased there in normal boot mode, the vector table stored at
`0x08000000` is also visible at `0x00000000` during reset.

## What lives where

A small bare-metal firmware usually has these sections:

| Section        | Runtime memory | Stored in flash image | Why                                                                                                   |
| -------------- | -------------- | --------------------- | ----------------------------------------------------------------------------------------------------- |
| `.isr_vector`  | Flash          | Yes                   | CPU needs reset and interrupt vectors immediately after reset                                         |
| `.text`        | Flash          | Yes                   | Program instructions are executed from Flash                                                          |
| `.rodata`      | Flash          | Yes                   | Constants do not need writable RAM                                                                    |
| `.data`        | RAM            | Yes                   | Initialized variables must be writable, but their initial values must be saved somewhere non-volatile |
| `.bss`         | RAM            | No                    | Zero-initialized variables only need RAM space; startup clears them                                   |
| heap/stack     | RAM            | No                    | Runtime scratch memory; no initial contents need to be stored                                         |
| debug sections | Not allocated  | No                    | Used by debugger/tooling, not loaded onto the MCU                                                     |

This is the reason `.data` is special: it has two addresses. It runs from RAM,
but its initial bytes are stored in Flash.

## VMA and LMA

GNU ld uses two address concepts for sections.

VMA means Virtual Memory Address. In bare-metal firmware, this is the runtime
address where the CPU accesses the section.

LMA means Load Memory Address. This is where the section's initial contents are
stored in the load image.

For this project:

| Section                                        | VMA                    | LMA   | Loadable?        | Allocatable?       |
| ---------------------------------------------- | ---------------------- | ----- | ---------------- | ------------------ |
| `.isr_vector`                                  | Flash, `0x08000000...` | Flash | Yes              | Yes                |
| `.text`                                        | Flash                  | Flash | Yes              | Yes                |
| `.rodata`                                      | Flash                  | Flash | Yes              | Yes                |
| `.ARM.extab`, `.ARM.exidx`                     | Flash                  | Flash | Yes              | Yes                |
| `.preinit_array`, `.init_array`, `.fini_array` | Flash                  | Flash | Yes              | Yes                |
| `.data`                                        | RAM, `0x20000000...`   | Flash | Yes              | Yes                |
| `.ramfunc`                                     | RAM                    | Flash | Yes              | Yes                |
| `.bss`                                         | RAM                    | None  | No file contents | Yes                |
| `._user_heap_stack`                            | RAM                    | None  | No file contents | Yes                |
| `.debug_*`                                     | None                   | None  | No               | No                 |
| `.comment`, `.note*`                           | None                   | None  | No               | No, discarded here |

Loadable means the section has bytes that must be present in the firmware image.

Allocatable means the section occupies memory when the program runs.

`.bss` is allocatable but not loadable. The ELF says how much RAM it needs, but
the firmware image does not store thousands of zero bytes. Startup code clears it
instead.

`.data` is both loadable and allocatable, but its VMA and LMA are different. The
ELF stores the initial bytes in Flash and records that the section must live in
RAM at runtime.

## Why a linker script is needed

Object files are relocatable. They contain code, data, symbols, and relocation
records, but they do not yet have the final embedded memory layout.

For example, `build/apps/bare-blink/main.o` can contain a call to `delay`, a reference
to `GPIOC_ODR`, and a `main` symbol. The object file does not decide where
`main` will sit in Flash.

The linker is the locator. It takes all object files, resolves symbols, applies
relocations, assigns final addresses, and writes the final ELF file.

The linker script tells the linker the target memory map and section layout:

```ld
FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
```

Without this information, the linker cannot place code at `0x08000000` or
writable data at `0x20000000`.

## Current linker script

The project linker script is:

```text
linker/STM32F103C8TX_FLASH.ld
```

It starts with the entry point:

```ld
ENTRY(Reset_Handler)
```

This marks `Reset_Handler` as the program entry symbol. It is also useful for
debuggers and map files. On Cortex-M, the hardware still starts by reading the
vector table, not by jumping directly to the ELF entry field.

The memory regions come directly from the STM32F103C8T6 memory map:

```ld
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}
```

The initial stack pointer is defined as the first address after RAM:

```ld
_estack = ORIGIN(RAM) + LENGTH(RAM);
```

For this chip:

```text
RAM starts: 0x20000000
RAM size:   0x00005000
_estack:    0x20005000
```

The stack grows downward, so the initial stack pointer starts at the top of RAM.

## Linker script directives

### ENTRY

`ENTRY(symbol)` sets the ELF entry point symbol.

In this project:

```ld
ENTRY(Reset_Handler)
```

### MEMORY

`MEMORY` declares named memory regions available on the target.

```ld
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}
```

The attributes are documentation and consistency hints:

| Attribute | Meaning    |
| --------- | ---------- |
| `r`       | Readable   |
| `w`       | Writable   |
| `x`       | Executable |

### SECTIONS

`SECTIONS` describes the output sections in the final ELF.

```ld
SECTIONS
{
  .isr_vector : { KEEP(*(.isr_vector)) } > FLASH
  .text : { *(.text*) } > FLASH
  .data : { *(.data*) } > RAM AT > FLASH
  .bss : { *(.bss*) *(COMMON) } > RAM
}
```

The order matters. The first output section placed in Flash starts at the Flash
origin unless the script moves the location counter elsewhere.

### The location counter

The symbol `.` is the linker location counter. It means the current output
address inside the current section.

This aligns the current address to a 4-byte boundary:

```ld
. = ALIGN(4);
```

This records the current address in a symbol:

```ld
_sdata = .;
```

This reserves bytes by moving the location counter forward:

```ld
. = . + _Min_Heap_Size;
. = . + _Min_Stack_Size;
```

### KEEP

`KEEP()` prevents sections from being removed by linker garbage collection.

The vector table does not look referenced by normal C code:

```ld
KEEP(*(.isr_vector))
```

The CPU references the vector table by address at reset. The linker does not know
that unless the script keeps it.

### > region

`> FLASH` or `> RAM` sets the VMA region for an output section.

```ld
.text : { *(.text*) } > FLASH
.bss  : { *(.bss*) } > RAM
```

### AT and AT > region

`AT` controls the LMA of a section when it differs from the VMA.

This project uses:

```ld
.data :
{
  _sdata = .;
  *(.data*)
  _edata = .;
} > RAM AT > FLASH
```

That means `.data` runs from RAM, but its initial contents are stored in Flash.

### ALIGN

`ALIGN(n)` rounds the location counter up to the next `n`-byte boundary.

The script uses 4-byte alignment for normal ARM data/code and 8-byte alignment
for heap/stack boundaries.

### PROVIDE

`PROVIDE(symbol = expression)` defines a symbol only if it was not already
defined elsewhere.

This project uses:

```ld
PROVIDE(end = .);
PROVIDE(_end = .);
```

These symbols are commonly used by `_sbrk()` as the start of heap memory.

### LOADADDR

`LOADADDR(section)` returns the LMA of an output section.

This project uses:

```ld
_sidata = LOADADDR(.data);
```

Startup uses `_sidata` as the Flash source address when copying `.data` into RAM.

### /DISCARD/

`/DISCARD/` removes matching input sections from the output file.

```ld
/DISCARD/ :
{
  *(.note*)
  *(.comment*)
}
```

This keeps unnecessary metadata out of the loadable image.

## Section definitions

### .isr_vector

Purpose: Cortex-M vector table.

Location: Flash, first output section, starting at `0x08000000`.

Definition:

```ld
.isr_vector :
{
  . = ALIGN(4);
  KEEP(*(.isr_vector))
  . = ALIGN(4);
} > FLASH
```

This section must be first in Flash because the boot alias exposes Flash at
`0x00000000`, and the Cortex-M reset sequence reads the first two words from the
vector table.

### .text

Purpose: executable code.

Location: Flash.

Definition:

```ld
.text :
{
  *(.text)
  *(.text*)
  KEEP(*(.init))
  KEEP(*(.fini))
} > FLASH
```

The wildcard `*(.text*)` collects normal `.text` plus per-function sections
created by `-ffunction-sections`.

### .rodata

Purpose: read-only constants.

Location: Flash.

Definition:

```ld
.rodata :
{
  *(.rodata)
  *(.rodata*)
} > FLASH
```

Constants do not need RAM unless code explicitly copies them there.

### .ARM.extab and .ARM.exidx

Purpose: ARM exception unwind metadata.

Location: Flash.

These sections are mostly useful for C++, exceptions, or stack unwinding. Keeping
them makes the linker script more compatible with code that expects ARM EABI
metadata.

### .preinit_array, .init_array, .fini_array

Purpose: constructor/destructor arrays.

Location: Flash.

These are more important for C++ than for the current C-only blink application.
They are included so the runtime layout remains conventional.

### .data

Purpose: initialized writable variables.

Runtime location: RAM.

Load location: Flash.

Definition:

```ld
_sidata = LOADADDR(.data);

.data :
{
  _sdata = .;
  *(.data)
  *(.data*)
  _edata = .;
} > RAM AT > FLASH
```

Startup copies bytes from `_sidata` to `_sdata` until `_edata`.

### .ramfunc

Purpose: code and data that run from RAM.

Runtime location: RAM.

Load location: Flash.

This is useful for functions that must execute while Flash is busy, such as Flash
erase/write routines. The current blink app does not need it, but the section is
available.

### .bss

Purpose: zero-initialized variables.

Runtime location: RAM.

Load location: none.

Definition:

```ld
.bss :
{
  _sbss = .;
  __bss_start__ = _sbss;
  *(.bss)
  *(.bss*)
  *(COMMON)
  _ebss = .;
  __bss_end__ = _ebss;
} > RAM
```

Startup writes zeroes from `_sbss` to `_ebss`.

### .\_user_heap_stack

Purpose: reserve minimum heap and stack space.

Location: RAM.

Definition:

```ld
._user_heap_stack :
{
  . = ALIGN(8);
  . = . + _Min_Heap_Size;
  . = . + _Min_Stack_Size;
  . = ALIGN(8);
} > RAM
```

This does not initialize memory. It makes the linker fail if static RAM usage plus
the reserved heap/stack minimum does not fit into the 20 KiB RAM region.

## Creating a linker script from scratch

Start with the chip memory map.

From `docs/targets/stm32f103c8t6/memory-map.md`:

```text
Embedded Flash: 0x08000000 - 0x0800FFFF, 64 KiB
Embedded SRAM:  0x20000000 - 0x20004FFF, 20 KiB
```

Create the memory regions:

```ld
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}
```

Set the entry point and stack top:

```ld
ENTRY(Reset_Handler)
_estack = ORIGIN(RAM) + LENGTH(RAM);
```

Place the vector table first:

```ld
SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } > FLASH
}
```

Add code and constants in Flash:

```ld
.text :
{
  . = ALIGN(4);
  *(.text)
  *(.text*)
  . = ALIGN(4);
} > FLASH

.rodata :
{
  . = ALIGN(4);
  *(.rodata)
  *(.rodata*)
  . = ALIGN(4);
} > FLASH
```

Record the end of Flash-loaded read-only content and the load address of
`.data`:

```ld
_etext = .;
_sidata = LOADADDR(.data);
```

Add initialized RAM data with a Flash load address:

```ld
.data :
{
  . = ALIGN(4);
  _sdata = .;
  *(.data)
  *(.data*)
  . = ALIGN(4);
  _edata = .;
} > RAM AT > FLASH
```

Add zero-initialized RAM data:

```ld
.bss :
{
  . = ALIGN(4);
  _sbss = .;
  *(.bss)
  *(.bss*)
  *(COMMON)
  . = ALIGN(4);
  _ebss = .;
} > RAM
```

Reserve heap and stack room:

```ld
_Min_Heap_Size = 0x200;
_Min_Stack_Size = 0x400;

. = ALIGN(8);
PROVIDE(end = .);
PROVIDE(_end = .);

._user_heap_stack :
{
  . = ALIGN(8);
  . = . + _Min_Heap_Size;
  . = . + _Min_Stack_Size;
  . = ALIGN(8);
} > RAM
```

Discard metadata that the image does not load:

```ld
/DISCARD/ :
{
  *(.note*)
  *(.comment*)
}
```

Then inspect the result:

```sh
make
arm-none-eabi-readelf -S build/bluepill-bare-blink.elf
arm-none-eabi-readelf -l build/bluepill-bare-blink.elf
arm-none-eabi-objdump -h build/bluepill-bare-blink.elf
arm-none-eabi-nm -n build/bluepill-bare-blink.elf
```

## Why a startup file is needed

There is no operating system on the STM32F103C8T6. After reset, nothing prepares
RAM, initializes variables, or calls `main` unless the firmware provides that
code.

The startup file does that minimal runtime setup. In this project, the active
startup implementation is written in C:

```text
src/startup_stm32f103.c
```

This is the file that the current `Makefile` compiles and links into the final
firmware image.

The project also keeps an assembly version here:

```text
asm/startup_stm32f103c8tx.s
```

That assembly file is only for education and reference. It shows the same startup
ideas using explicit Cortex-M assembly, but it is not the current way this
project starts the microcontroller and it is not linked by the current Makefile.

The active C startup provides:

1. Linker symbol declarations for `.data`, `.bss`, and the stack top.
2. A vector table in `.isr_vector`.
3. Weak default handlers for exceptions and interrupts.
4. `Default_Handler`.
5. `Reset_Handler`.

## Why C startup works here

Many embedded projects write startup code in assembly because startup runs before
the normal C runtime is initialized. That is still true here, so the C startup
must be simple and careful.

For this STM32F103C8T6/Cortex-M3 project, C startup is practical because the
Cortex-M reset sequence already does the most important CPU setup before calling
`Reset_Handler`: it loads the initial stack pointer from vector table word 0.

The vector table starts like this in `src/startup_stm32f103.c`:

```c
const uint32_t g_pfnVectors[] __attribute__((section(".isr_vector"), used)) = {
    (uint32_t)&_estack,
    (uint32_t)&Reset_Handler,
    /* more exception and interrupt handlers */
};
```

At reset, the CPU reads `_estack` into `SP`, then branches to `Reset_Handler`.
That means the C function `Reset_Handler` can run with a valid stack.

This is why the active C `Reset_Handler` does not manually write `SP`. The
hardware has already done it.

This approach depends on a few conditions:

1. The CPU architecture must load the initial stack pointer from the vector table.
2. The vector table must be placed at the boot address, which this linker script does with `.isr_vector` first in Flash.
3. The C `Reset_Handler` must not rely on initialized global/static variables before it copies `.data` and clears `.bss`.
4. The compiler must not insert runtime assumptions that require initialized C library state before startup has prepared RAM.

Those conditions are true enough for this minimal Cortex-M3 firmware.

This is not valid on every system. Some CPUs start executing from a reset
address without automatically setting up a stack. Some systems need assembly to
select a CPU mode, initialize stack pointers for several modes, configure memory
controllers, set exception state, or perform low-level ABI setup before any C
function can run. On those systems, assembly startup is not optional.

For this project, C keeps the startup easier to read while still showing the real
bare-metal responsibilities.

## Why Cortex-M needs a vector table

On reset, a Cortex-M CPU does not start by executing instruction zero. It reads
two words from the vector table:

| Vector table word | Meaning                     |
| ----------------- | --------------------------- |
| Word 0            | Initial stack pointer value |
| Word 1            | Reset handler address       |

For this project, those words are defined in C in `src/startup_stm32f103.c`:

```c
const uint32_t g_pfnVectors[] __attribute__((section(".isr_vector"), used)) = {
    (uint32_t)&_estack,
    (uint32_t)&Reset_Handler,
    /* more exception and interrupt handlers */
};
```

The first word loads `SP`. The second word loads `PC` and execution starts at
`Reset_Handler`.

After that, the vector table contains exception and interrupt handler addresses:

```c
    (uint32_t)&NMI_Handler,
    (uint32_t)&HardFault_Handler,
    (uint32_t)&MemManage_Handler,
    (uint32_t)&BusFault_Handler,
    (uint32_t)&UsageFault_Handler,
```

Peripheral interrupts are also listed in the STM32-defined order. If an interrupt
fires, the CPU looks up the handler address in this table.

## Reset_Handler

The active reset handler is the C function `Reset_Handler` in
`src/startup_stm32f103.c`. It performs the standard bare-metal sequence.

First it calls system setup:

```c
SystemInit();
```

In this project, `SystemInit` keeps the default clock setup simple.

Then it copies `.data` from Flash LMA to RAM VMA:

```c
uint32_t *p_src_mem = &_sidata;
uint32_t *p_dest_mem = &_sdata;

while (p_dest_mem < &_edata) {
  *p_dest_mem++ = *p_src_mem++;
}
```

`_sidata` is the Flash load address of `.data`. `_sdata` and `_edata` describe
the RAM address range where `.data` must live while the program runs.

Then it clears `.bss`:

```c
p_dest_mem = &_sbss;

while (p_dest_mem < &_ebss) {
  *p_dest_mem++ = 0;
}
```

`_sbss` and `_ebss` describe the RAM address range occupied by zero-initialized
global/static variables.

Then it enters the application:

```c
main();
```

If `main` returns, startup loops forever:

```c
while (1) {
}
```

Returning from `main` has nowhere useful to go in a bare-metal program.

## Weak interrupt handlers

The active C startup gives every handler a weak default implementation with this
macro:

```c
#define DEFAULT_HANDLER(name) \
  void name(void) __attribute__((weak, alias("Default_Handler")))
```

For example:

```c
DEFAULT_HANDLER(SysTick_Handler);
DEFAULT_HANDLER(EXTI0_IRQHandler);
```

Weak means application code can override the handler by defining a real function
with the same name:

```c
void SysTick_Handler(void) {
  /* real handler */
}
```

If no real handler exists, the vector table points to `Default_Handler`, which
loops forever. That is safer than jumping to an undefined address.

## Creating a startup file from scratch

For this project, create the startup file in C unless you have a specific reason
to write assembly.

Start by declaring the linker symbols that the startup code needs:

```c
extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
```

Declare the startup functions:

```c
void Reset_Handler(void);
void Default_Handler(void);
void SystemInit(void);
int main(void);
```

Create a weak-handler macro:

```c
#define DEFAULT_HANDLER(name) \
  void name(void) __attribute__((weak, alias("Default_Handler")))
```

Use it for core exceptions and peripheral interrupts:

```c
DEFAULT_HANDLER(NMI_Handler);
DEFAULT_HANDLER(HardFault_Handler);
DEFAULT_HANDLER(SysTick_Handler);
DEFAULT_HANDLER(WWDG_IRQHandler);
/* add the rest of the STM32F103 handlers in vector-table order */
```

Create the vector table in `.isr_vector`:

```c
const uint32_t g_pfnVectors[] __attribute__((section(".isr_vector"), used)) = {
    (uint32_t)&_estack,
    (uint32_t)&Reset_Handler,
    (uint32_t)&NMI_Handler,
    (uint32_t)&HardFault_Handler,
    /* reserved entries and the rest of the handlers */
};
```

The section name `.isr_vector` must match the linker script:

```ld
KEEP(*(.isr_vector))
```

Create `Default_Handler`:

```c
void Default_Handler(void) {
  while (1) {
  }
}
```

Create `Reset_Handler`:

```c
void Reset_Handler(void) {
  SystemInit();

  uint32_t *p_src_mem = &_sidata;
  uint32_t *p_dest_mem = &_sdata;

  while (p_dest_mem < &_edata) {
    *p_dest_mem++ = *p_src_mem++;
  }

  p_dest_mem = &_sbss;
  while (p_dest_mem < &_ebss) {
    *p_dest_mem++ = 0;
  }

  main();

  while (1) {
  }
}
```

The exact interrupt order must match the STM32F103 medium-density vector table
from the reference manual or startup file template.

Use `asm/startup_stm32f103c8tx.s` only as a reference to compare the same ideas in
assembly. Do not enable it in the build unless you intentionally replace the C
startup implementation.

## Relocation during linking and startup

There are two related relocation ideas.

Link-time relocation happens when the linker combines object files. It resolves
symbol references and writes final addresses into the ELF. For example, a branch
to `main` in the startup object is resolved to the final Flash address of `main`.

Runtime data relocation happens after reset. The linker arranged `.data` with VMA
in RAM and LMA in Flash. Startup copies the initial values from Flash to RAM so C
code sees initialized writable variables at their RAM addresses.

The linker script creates the symbols used for runtime relocation:

```ld
_sidata = LOADADDR(.data);
_sdata = .;
_edata = .;
```

The active C startup file consumes those symbols:

```c
uint32_t *p_src_mem = &_sidata;
uint32_t *p_dest_mem = &_sdata;

while (p_dest_mem < &_edata) {
  *p_dest_mem++ = *p_src_mem++;
}
```

The linker and startup file must agree. If the linker script names a symbol
differently, startup will fail to link. If the symbols are wrong, the program can
boot with corrupted global variables.

## Verifying the result

Build the project:

```sh
make
```

Check section addresses:

```sh
arm-none-eabi-readelf -S build/bluepill-bare-blink.elf
```

Check loadable segments and VMA/LMA behavior:

```sh
arm-none-eabi-readelf -l build/bluepill-bare-blink.elf
```

Disassemble startup and `main`:

```sh
arm-none-eabi-objdump -d -S build/bluepill-bare-blink.elf
```

List symbols by address:

```sh
arm-none-eabi-nm -n build/bluepill-bare-blink.elf
```

Inspect the linker map:

```sh
less build/bluepill-bare-blink.map
```

Useful symbols to look for:

```text
_estack
g_pfnVectors
Reset_Handler
main
_sidata
_sdata
_edata
_sbss
_ebss
```

## References

GNU ld linker scripts:
<https://sourceware.org/binutils/docs/ld/Scripts.html>

GNU ld `MEMORY` command:
<https://sourceware.org/binutils/docs/ld/MEMORY.html>

GNU ld `SECTIONS` command:
<https://sourceware.org/binutils/docs/ld/SECTIONS.html>

GNU ld location counter:
<https://sourceware.org/binutils/docs/ld/Location-Counter.html>

GNU ld output section LMA:
<https://sourceware.org/binutils/docs/ld/Output-Section-LMA.html>

GNU ld `PROVIDE`:
<https://sourceware.org/binutils/docs/ld/PROVIDE.html>

GNU assembler ARM directives:
<https://sourceware.org/binutils/docs/as/ARM-Directives.html>

ARM Cortex-M3 Generic User Guide:
<https://developer.arm.com/documentation/dui0552/latest/>

ST RM0008 STM32F10xxx reference manual:
<https://www.st.com/resource/en/reference_manual/cd00171190.pdf>

STM32F103x8 datasheet:
<https://www.st.com/resource/en/datasheet/stm32f103c8.pdf>
