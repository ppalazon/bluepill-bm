# Build Process

This project builds a bare-metal executable for an STM32F103C8T6 Blue Pill board.
The final program runs directly on a Cortex-M3 microcontroller, not on the host
computer, so the build uses an ARM cross-compilation toolchain.

The default application is `bare-blink`, so a normal build creates these files:

```text
build/bluepill-bare-blink.elf
build/bluepill-bare-blink.bin
build/bluepill-bare-blink.hex
build/bluepill-bare-blink.map
```

The most important output is the ELF file. It contains the machine code, debug
symbols, section information, and the memory layout needed by debuggers and
flashing tools.

## Toolchain

The `Makefile` chooses these tools:

```make
CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE := arm-none-eabi-size
GDB := arm-none-eabi-gdb
OPENOCD := openocd
```

`arm-none-eabi-gcc` is the compiler driver. It can run the preprocessor, compile
C into assembly, assemble assembly into object files, and link object files into
an ELF executable.

`arm-none-eabi-objcopy` converts the ELF file into other formats. This project
uses it to create raw binary and Intel HEX files.

`arm-none-eabi-size` prints how much flash and RAM the program uses.

`arm-none-eabi-gdb` is the debugger. It reads the ELF debug information and
connects to the target through OpenOCD.

`openocd` talks to the ST-Link probe and the STM32 debug/flash interface. It can
flash the ELF, reset the chip, halt the CPU, and provide a GDB server.

Useful inspection tools from the same toolchain are:

```text
arm-none-eabi-nm       list symbols
arm-none-eabi-objdump  inspect sections and disassemble code
arm-none-eabi-readelf  inspect ELF headers, sections, and program headers
```

## Source Selection

The default application is selected here:

```make
APP ?= bare-blink
PROJECT := bluepill-$(APP)
```

If no `APP` is passed, `bare-blink` is used. The project name becomes
`bluepill-bare-blink`.

The source directories are:

```make
SRC_DIR := src
APP_DIR := apps/$(APP)
```

The C sources are all shared C files plus all C files for the selected app:

```make
C_SOURCES := $(wildcard $(SRC_DIR)/*.c)
C_SOURCES += $(wildcard $(APP_DIR)/*.c)
```

Assembly sources are disabled by default. The active startup implementation is
the C file `src/startup_stm32f103.c`:

```make
ASM_SOURCES :=
```

For `APP=bare-blink`, the important inputs are:

```text
src/*.c
apps/bare-blink/*.c
linker/STM32F103C8TX_FLASH.ld
```

The `Makefile` also checks that the selected application directory exists:

```make
ifeq ($(wildcard $(APP_DIR)),)
$(error Unknown APP '$(APP)': expected directory $(APP_DIR))
endif
```

## Architecture Flags

The architecture flags are:

```make
CPU_FLAGS := -mcpu=cortex-m3 -mthumb
```

`-mcpu=cortex-m3` tells GCC to generate instructions for an ARM Cortex-M3 CPU.
The STM32F103C8T6 contains a Cortex-M3 core, so this must match the actual chip.

`-mthumb` tells GCC to generate Thumb instructions. Cortex-M microcontrollers run
Thumb code, not the older ARM instruction set. For Cortex-M3, this is not an
optional performance choice; it is the instruction set the core executes.

These flags are used during both compilation and linking. That keeps object files
and linked startup/runtime code compatible with the target CPU.

If these flags are wrong, the program may fail to link, contain instructions the
CPU cannot execute, or crash immediately after reset.

## Common Compile Flags

The common flags are:

```make
COMMON_FLAGS := $(CPU_FLAGS) -Wall -Wextra -Werror -ffunction-sections -fdata-sections -g3 -O0
```

`-Wall` enables a useful baseline set of compiler warnings.

`-Wextra` enables additional warnings beyond `-Wall`.

`-Werror` turns warnings into errors. This is useful in bare-metal code because a
warning can point to a real hardware bug, such as a wrong type, missing function
prototype, or unintended conversion.

`-ffunction-sections` places each function into its own section, usually named
`.text.<function>`.

`-fdata-sections` places each data object into its own section, such as
`.data.<name>` or `.bss.<name>`.

Those two section flags are paired with the linker flag `--gc-sections`. Together
they allow the linker to remove unused functions and data from the final image.
That matters on small microcontrollers because flash and RAM are limited.

`-g3` includes debug information, including extra macro information. This makes
GDB source-level debugging better.

`-O0` disables optimization. This is chosen for learning and debugging because
the generated code follows the C source more closely. With optimization enabled,
variables may disappear, lines may be reordered, and stepping in GDB can be more
confusing.

## C Compile Flags

C files use:

```make
CFLAGS := $(COMMON_FLAGS) -std=c11 -Iinclude
```

`-std=c11` selects the C11 language standard.

`-Iinclude` adds the project `include/` directory to the compiler header search
path. That is how application code can include the local STM32 register header:

```c
#include "stm32f103c8t6.h"
```

Vendor CMSIS include paths are intentionally conditional. They are only added
when the selected application name starts with `cmsis-`:

```make
ifneq ($(filter cmsis-%,$(APP)),)
CFLAGS += -I$(CMSIS_CORE_INC) -I$(CMSIS_DEVICE_INC) -DUSE_CMSIS
endif
```

This keeps `bare-*` applications limited to the local `include/` directory while
allowing future `cmsis-*` applications to use vendored STM32CubeF1 CMSIS headers.

## Assembly Flags

Assembly files use:

```make
ASFLAGS := $(COMMON_FLAGS) -x assembler-with-cpp
```

`-x assembler-with-cpp` tells GCC to treat the input as assembly that should pass
through the C preprocessor first. This is useful for startup files because they
can use preprocessor features later if needed.

The startup file also declares the target directly:

```asm
.syntax unified
.cpu cortex-m3
.thumb
```

That keeps the assembly source aligned with the compiler architecture flags.

## Build Stages

The default target is:

```make
all: $(ELF) $(BIN) $(HEX) size
```

Running `make` builds the ELF, creates `.bin` and `.hex` copies, and prints the
program size.

### 1. Preprocessing

The preprocessor expands `#include`, `#define`, conditional compilation, and
macros. For example, in `apps/bare-blink/main.c`, register helper macros are expanded
from `include/stm32f103c8t6.h`.

The `Makefile` does not keep the preprocessed file by default, but you can create
one manually:

```sh
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -std=c11 -Iinclude -E apps/bare-blink/main.c -o build/main.i
```

The `.i` file is C after preprocessing, before compilation.

### 2. Compilation

Compilation turns preprocessed C into assembly for the target CPU.

The `Makefile` normally compiles and assembles in one command with `-c`, but you
can stop after assembly manually:

```sh
arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Wall -Wextra -Werror -ffunction-sections -fdata-sections -g3 -O0 -std=c11 -Iinclude -S apps/bare-blink/main.c -o build/main.s
```

The `.s` file is ARM Thumb assembly text.

### 3. Assembling And Object Files

The project compiles each C file into an object file:

```make
$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
```

For example:

```text
apps/bare-blink/main.c -> build/apps/bare-blink/main.o
src/system_stm32f103.c -> build/src/system_stm32f103.o
```

This project keeps an assembly startup file under `asm/` as a reference, but
the active build now uses the C startup file in `src/startup_stm32f103.c`. If an
assembly startup file is enabled later, it is assembled into an object file with:

```make
$(BUILD_DIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@
```

For example, if enabled:

```text
asm/startup_stm32f103c8tx.s -> build/asm/startup_stm32f103c8tx.o
```

An object file contains machine code and symbols, but it is not placed at final
flash/RAM addresses yet. Relocation still happens during linking.

### 4. Linking

The linker combines all object files into the final ELF:

```make
$(ELF): $(OBJECTS) $(LINKER_SCRIPT)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
```

The linker flags are:

```make
LDFLAGS := $(CPU_FLAGS) -T$(LINKER_SCRIPT) -nostartfiles -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map --specs=nano.specs --specs=nosys.specs
```

`-T linker/STM32F103C8TX_FLASH.ld` tells the linker to use the project linker
script. Bare-metal firmware needs this because there is no operating system to
decide where code and data go.

`-nostartfiles` prevents GCC from linking the default C runtime startup files.
This project provides its own Cortex-M startup file in `src/startup_stm32f103.c`.

`-Wl,--gc-sections` passes `--gc-sections` to the linker. It removes unused input
sections. This works well with `-ffunction-sections` and `-fdata-sections`.

`-Wl,-Map=build/bluepill-bare-blink.map` asks the linker to write a map file. The map
file shows which object files and sections were placed into the final memory
layout.

`--specs=nano.specs` selects the smaller newlib-nano C library. It is useful for
microcontrollers because it reduces code size compared with full newlib.

`--specs=nosys.specs` supplies stub system calls for a bare-metal target. There
is no host operating system providing calls like `open`, `read`, or `write`.

### 5. Linker Script Layout

The linker script starts with:

```ld
ENTRY(Reset_Handler)
```

That tells the linker that `Reset_Handler` is the program entry point.

The memory regions are:

```ld
FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
```

Flash starts at `0x08000000`. This is where the STM32 runs user firmware from.
RAM starts at `0x20000000`.

The initial stack pointer is placed at the top of RAM:

```ld
_estack = ORIGIN(RAM) + LENGTH(RAM);
```

The important output sections are:

| Section | Location | Purpose |
| --- | --- | --- |
| `.isr_vector` | Flash | Cortex-M vector table, including initial stack pointer and reset handler |
| `.text` | Flash | Program instructions |
| `.rodata` | Flash | Constant data |
| `.data` | RAM, loaded from Flash | Initialized global/static variables |
| `.bss` | RAM | Zero-initialized global/static variables |

The startup code uses linker-provided symbols:

| Symbol | Purpose |
| --- | --- |
| `_estack` | Initial stack pointer |
| `_sidata` | Flash source address for initial `.data` values |
| `_sdata` | Start of `.data` in RAM |
| `_edata` | End of `.data` in RAM |
| `_sbss` | Start of `.bss` in RAM |
| `_ebss` | End of `.bss` in RAM |

At reset, `Reset_Handler` sets the stack pointer, calls `SystemInit`, copies
`.data` from flash to RAM, clears `.bss`, then calls `main`.

### 6. ELF Output

The linked ELF is:

```text
build/bluepill-bare-blink.elf
```

Use the ELF for debugging and for OpenOCD flashing. It contains more information
than the raw `.bin` or `.hex` files.

### 7. Binary And HEX Outputs

The raw binary is created with:

```make
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@
```

This produces:

```text
build/bluepill-bare-blink.bin
```

The binary is just the loadable bytes, without ELF metadata or debug symbols.

The Intel HEX file is created with:

```make
$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@
```

This produces:

```text
build/bluepill-bare-blink.hex
```

HEX is a text format that includes addresses and checksums. Some flashing tools
prefer HEX instead of ELF or BIN.

### 8. Size Report

The `size` target runs:

```make
$(SIZE) $<
```

Manual command:

```sh
arm-none-eabi-size build/bluepill-bare-blink.elf
```

Typical output has these columns:

```text
text    data     bss     dec     hex filename
```

`text` is code plus read-only data in flash.

`data` is initialized data that occupies RAM at runtime and also consumes flash
for its initial values.

`bss` is zero-initialized data that occupies RAM but does not consume flash image
space for initial values.

## Inspecting Build Outputs

Build first:

```sh
make
```

List symbols in the ELF:

```sh
arm-none-eabi-nm build/bluepill-bare-blink.elf
```

Useful variants:

```sh
arm-none-eabi-nm -n build/bluepill-bare-blink.elf
arm-none-eabi-nm -S --size-sort build/bluepill-bare-blink.elf
```

`-n` sorts symbols by address. `-S --size-sort` shows symbol sizes and sorts by
size.

Disassemble executable code:

```sh
arm-none-eabi-objdump -d build/bluepill-bare-blink.elf
```

Disassemble with source mixed in:

```sh
arm-none-eabi-objdump -d -S build/bluepill-bare-blink.elf
```

Show section headers:

```sh
arm-none-eabi-objdump -h build/bluepill-bare-blink.elf
```

Read ELF headers and metadata:

```sh
arm-none-eabi-readelf -h build/bluepill-bare-blink.elf
arm-none-eabi-readelf -S build/bluepill-bare-blink.elf
arm-none-eabi-readelf -l build/bluepill-bare-blink.elf
arm-none-eabi-readelf -a build/bluepill-bare-blink.elf
```

`-h` shows the ELF header.

`-S` shows sections such as `.isr_vector`, `.text`, `.data`, and `.bss`.

`-l` shows program headers, which describe loadable memory regions.

`-a` shows all available ELF information.

Inspect the linker map:

```sh
less build/bluepill-bare-blink.map
```

The map file is often the best place to answer these questions:

1. Which object file contributed this symbol?
2. Why is this function in the firmware?
3. How much flash/RAM does this section use?
4. Where did the linker place this section in memory?

## Build Commands

Build the default app:

```sh
make
```

Build the blink app explicitly:

```sh
make APP=bare-blink
```

Clean generated files:

```sh
make clean
```

Print the size again:

```sh
make size
```

## Flashing With OpenOCD

The `flash` target is:

```make
flash: $(ELF)
	$(OPENOCD) -f $(OPENOCD_CFG) -c "program $(ELF) verify reset exit"
```

For the default app, this expands to roughly:

```sh
openocd -f openocd/bluepill.cfg -c "program build/bluepill-bare-blink.elf verify reset exit"
```

The OpenOCD config selects the ST-Link adapter and STM32F1 target:

```tcl
source [find interface/stlink.cfg]
transport select hla_swd
set CPUTAPID 0x2ba01477
source [find target/stm32f1x.cfg]
reset_config none
```

The `program` command writes the ELF loadable sections to flash.

`verify` reads flash back and checks that programming succeeded.

`reset` resets the target after programming.

`exit` closes OpenOCD when flashing is complete.

## Manual Flashing Steps

Connect the ST-Link to the Blue Pill using SWD:

| ST-Link | Blue Pill | Purpose |
| --- | --- | --- |
| `SWDIO` | `SWDIO` | Debug data |
| `SWCLK` | `SWCLK` | Debug clock |
| `GND` | `GND` | Common ground |
| `3.3V` | `3.3V` | Target reference/power, depending on probe/board setup |

Do not connect 5 V to a 3.3 V pin. The STM32F103 is a 3.3 V microcontroller.

Build the firmware:

```sh
make APP=bare-blink
```

Flash it with the Makefile target:

```sh
make flash
```

Or run OpenOCD manually:

```sh
openocd -f openocd/bluepill.cfg -c "program build/bluepill-bare-blink.elf verify reset exit"
```

If you want to keep OpenOCD running for debugging:

```sh
make openocd
```

Then connect GDB from another terminal:

```sh
arm-none-eabi-gdb build/bluepill-bare-blink.elf
```

Inside GDB:

```gdb
target extended-remote localhost:3333
monitor reset halt
break main
continue
```

## Common Flashing Problems

If OpenOCD cannot find the ST-Link, check USB permissions and udev rules.

If OpenOCD connects but cannot talk to the target, check `SWDIO`, `SWCLK`, `GND`,
and target power.

If reset fails, remember that this project uses:

```tcl
reset_config none
```

That is intentional for simple ST-Link clone wiring where `NRST` is not
connected.

If flashing succeeds but the program does not run, check that the ELF was linked
for flash at `0x08000000`, the vector table is first in flash, and the startup
file provides `Reset_Handler`.
