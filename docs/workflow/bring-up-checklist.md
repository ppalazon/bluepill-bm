# Bring-up checklist

Bring up a new microcontroller in small tests. Each test must prove one layer
before the next layer depends on it.

## Target and tools

- Confirm the MCU marking, package, and board wiring.
- Confirm the compiler target flags.
- Confirm the linker Flash and SRAM regions.
- Confirm the debug probe and target voltage.
- Confirm the boot pins select the intended memory.

## Boot test

- Place the vector table at the boot address.
- Confirm the initial stack pointer is inside SRAM.
- Confirm the reset handler address has the required Thumb bit.
- Break at `Reset_Handler` with GDB.
- Break at `main` after startup initializes `.data` and `.bss`.

## Peripheral test

- Enable one peripheral clock.
- Configure one pin or register group.
- Read back status where the hardware supports it.
- Use a visible or measurable result.
- Record the clock assumption and expected timing.

## Image inspection

```sh
arm-none-eabi-size build/bluepill-bare-blink.elf
arm-none-eabi-readelf -S build/bluepill-bare-blink.elf
arm-none-eabi-readelf -l build/bluepill-bare-blink.elf
arm-none-eabi-nm -n build/bluepill-bare-blink.elf
```

The ELF file is the primary debug artifact. The map file explains why sections
and symbols occupy their final addresses.

## Failure order

When firmware does not run, check in this order:

1. Power and ground
2. Boot pins
3. Debug connection
4. Flash address and image verification
5. Vector table and stack address
6. Reset handler and `main` breakpoints
7. Clock configuration
8. Peripheral clock and pin configuration
9. External wiring and signal polarity
