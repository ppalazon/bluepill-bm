/*
 * SPDX-FileCopyrightText: 2026 Pablo Palazon
 * SPDX-License-Identifier: BSD-3-Clause
*/

ENTRY(Reset_Handler)

/* STM32F103C8T6 memory map. Keep these two lines as the board-specific part. */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* The initial stack pointer is the first word in the Cortex-M vector table. */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Minimal heap/stack reservations. Increase these when the program needs it. */
_Min_Heap_Size = 0x200;
_Min_Stack_Size = 0x400;

SECTIONS
{
  /* Cortex-M vector table. This must be first in flash. */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } > FLASH

  /* Program code and read-only data live in flash. */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)

    KEEP(*(.init))
    KEEP(*(.fini))

    . = ALIGN(4);
  } > FLASH

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH

  /* ARM unwind/exception metadata. Mostly useful for C++ or backtraces. */
  .ARM.extab :
  {
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } > FLASH

  .ARM :
  {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } > FLASH

  /* Runtime constructor/destructor arrays. Useful if C++ is added later. */
  .preinit_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN(__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN(__preinit_array_end = .);
    . = ALIGN(4);
  } > FLASH

  .init_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN(__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN(__init_array_end = .);
    . = ALIGN(4);
  } > FLASH

  .fini_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN(__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN(__fini_array_end = .);
    . = ALIGN(4);
  } > FLASH

  /* End of flash-loaded read-only content. Startup uses this for .data copy. */
  _etext = .;
  _sidata = LOADADDR(.data);

  /* Initialized variables run from RAM but their initial values are in flash. */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > RAM AT > FLASH

  /* Optional functions/data that should execute from RAM. */
  .ramfunc :
  {
    . = ALIGN(4);
    *(.ramfunc)
    *(.ramfunc*)
    *(.RamFunc)
    *(.RamFunc*)
    . = ALIGN(4);
  } > RAM AT > FLASH

  /* Zero-initialized variables. Startup clears this before main(). */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } > RAM

  /* Symbols used by newlib _sbrk() as the start of the heap. */
  . = ALIGN(8);
  PROVIDE(end = .);
  PROVIDE(_end = .);

  /* Reserve space so the linker fails if RAM is already too full. */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } > RAM

  /* Keep metadata out of the loadable image. */
  /DISCARD/ :
  {
    *(.note*)
    *(.comment*)
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
