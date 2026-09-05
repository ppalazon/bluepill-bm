# Linker Script And Startup File

This document explains how the linker script and startup file work together to
turn object files into firmware that can boot on an STM32F103C8T6 Blue Pill.

The linker script decides where every output section lives in memory. The startup
file contains the first code that runs after reset and prepares RAM before
calling `main`.

## Understanding Load Memory On STM32F103C8T6

The STM32F103C8T6 has one Cortex-M3 address space, but not every address points
to the same kind of memory.

For this project, the important regions are documented in the [Memory Map](stm32f103c8t6-memory-map.md):

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

## What Lives Where

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

## VMA And LMA

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

## Why A Linker Script Is Needed

Object files are relocatable. They contain code, data, symbols, and relocation
records, but they do not yet have the final embedded memory layout.

For example, `build/apps/blink/main.o` can contain a call to `delay`, a reference
to `GPIOC_ODR`, and a `main` symbol. The object file does not decide where
`main` will sit in Flash.

The linker is the locator. It takes all object files, resolves symbols, applies
relocations, assigns final addresses, and writes the final ELF file.

The linker script tells the linker the target memory map and section layout:

```ld
FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
```

Without this information, the linker would not know that code belongs at
`0x08000000` and writable data belongs at `0x20000000`.

## Current Linker Script

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

## Linker Script Directives

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

### The Location Counter

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

This matters because the vector table may not look referenced by normal C code:

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

### AT And AT > region

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

## Section Definitions

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

### .ARM.extab And .ARM.exidx

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

Purpose: code/data that should run from RAM.

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

## Creating A Linker Script From Scratch

Start with the chip memory map.

From `docs/stm32f103c8t6-memory-map.md`:

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

Discard metadata that should not be loaded:

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
arm-none-eabi-readelf -S build/bluepill-blink.elf
arm-none-eabi-readelf -l build/bluepill-blink.elf
arm-none-eabi-objdump -h build/bluepill-blink.elf
arm-none-eabi-nm -n build/bluepill-blink.elf
```

## Why A Startup File Is Needed

There is no operating system on the STM32F103C8T6. After reset, nothing prepares
RAM, initializes variables, or calls `main` unless the firmware provides that
code.

The startup file does that minimal runtime setup.

The current startup file is:

```text
startup/startup_stm32f103c8tx.s
```

It provides:

1. CPU assembly mode declarations.
2. A vector table in `.isr_vector`.
3. `Reset_Handler`.
4. `Default_Handler`.
5. Weak aliases for interrupts.

## Why Cortex-M Needs A Vector Table

On reset, a Cortex-M CPU does not start by executing instruction zero. It reads
two words from the vector table:

| Vector table word | Meaning                     |
| ----------------- | --------------------------- |
| Word 0            | Initial stack pointer value |
| Word 1            | Reset handler address       |

For this project, those words are:

```asm
.section .isr_vector,"a",%progbits
g_pfnVectors:
  .word _estack
  .word Reset_Handler
```

The first word loads `SP`. The second word loads `PC` and execution starts at
`Reset_Handler`.

After that, the vector table contains exception and interrupt handler addresses:

```asm
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
```

Peripheral interrupts are also listed in the STM32-defined order. If an interrupt
fires, the CPU looks up the handler address in this table.

## Reset_Handler

The current reset handler performs the standard bare-metal sequence.

First it sets the stack pointer from the linker symbol:

```asm
ldr r0, =_estack
mov sp, r0
```

Then it calls system setup:

```asm
bl SystemInit
```

In this project, `SystemInit` keeps the default clock setup simple.

Then it copies `.data` from Flash LMA to RAM VMA:

```asm
ldr r0, =_sdata
ldr r1, =_edata
ldr r2, =_sidata
movs r3, #0
```

Conceptually this is:

```c
uint32_t *dst = &_sdata;
uint32_t *end = &_edata;
uint32_t *src = &_sidata;

while (dst < end) {
  *dst++ = *src++;
}
```

Then it clears `.bss`:

```asm
ldr r2, =_sbss
ldr r4, =_ebss
movs r3, #0
```

Conceptually this is:

```c
uint32_t *dst = &_sbss;
uint32_t *end = &_ebss;

while (dst < end) {
  *dst++ = 0;
}
```

Then it enters the C program:

```asm
bl main
```

If `main` returns, startup loops forever:

```asm
5:
  b 5b
```

Returning from `main` has nowhere useful to go in a bare-metal program.

## Weak Interrupt Handlers

The startup file gives every handler a weak default implementation:

```asm
.macro weak_alias name
  .weak \name
  .thumb_set \name, Default_Handler
.endm
```

For example:

```asm
weak_alias SysTick_Handler
weak_alias EXTI0_IRQHandler
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

## Creating A Startup File From Scratch

Start by selecting the assembly syntax, CPU, and instruction set:

```asm
.syntax unified
.cpu cortex-m3
.thumb
```

Export the vector table and default handler symbols:

```asm
.global g_pfnVectors
.global Default_Handler
```

Reference linker symbols that the startup code needs:

```asm
.word _sidata
.word _sdata
.word _edata
.word _sbss
.word _ebss
```

Create `Reset_Handler` in executable code:

```asm
.section .text.Reset_Handler
.weak Reset_Handler
.thumb_func
.type Reset_Handler, %function
Reset_Handler:
  ldr r0, =_estack
  mov sp, r0
  bl SystemInit
  /* copy .data */
  /* clear .bss */
  bl main
1:
  b 1b
```

Create `Default_Handler`:

```asm
.section .text.Default_Handler,"ax",%progbits
.thumb_func
Default_Handler:
  b Default_Handler
```

Create the vector table in its own section:

```asm
.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object
g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  /* more exception and interrupt handlers */
```

The section name `.isr_vector` must match the linker script:

```ld
KEEP(*(.isr_vector))
```

Then add weak aliases for handlers:

```asm
.macro weak_alias name
  .weak \name
  .thumb_set \name, Default_Handler
.endm

weak_alias NMI_Handler
weak_alias HardFault_Handler
```

The exact interrupt order must match the STM32F103 medium-density vector table
from the reference manual or startup file template.

## Relocation During Linking And Startup

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

The startup file consumes those symbols:

```asm
ldr r0, =_sdata
ldr r1, =_edata
ldr r2, =_sidata
```

The linker and startup file must agree. If the linker script names a symbol
differently, startup will fail to link. If the symbols are wrong, the program may
boot with corrupted global variables.

## Verifying The Result

Build the project:

```sh
make
```

Check section addresses:

```sh
arm-none-eabi-readelf -S build/bluepill-blink.elf
```

Check loadable segments and VMA/LMA behavior:

```sh
arm-none-eabi-readelf -l build/bluepill-blink.elf
```

Disassemble startup and `main`:

```sh
arm-none-eabi-objdump -d -S build/bluepill-blink.elf
```

List symbols by address:

```sh
arm-none-eabi-nm -n build/bluepill-blink.elf
```

Inspect the linker map:

```sh
less build/bluepill-blink.map
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
