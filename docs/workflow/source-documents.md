# Source documents

Use the document that owns a fact. Do not use a vendor example, a header file, or
an online tutorial as the primary authority for hardware behavior.

## Document roles

| Document | Answers | Typical implementation use |
|---|---|---|
| Datasheet | Which part, package, pins, limits, and memories are present? | Target profile, linker memory, pin selection |
| Reference manual | How do the MCU buses, clocks, resets, and peripherals work? | Register definitions and driver procedures |
| Processor manual | How does the Cortex-M core reset, handle exceptions, and access core peripherals? | Startup, vector table, NVIC, SysTick |
| Errata | Which documented device behaviors need workarounds? | Restrictions and validation tests |
| Board schematic | How is the physical board wired? | LED, buttons, power, oscillator, and debug connections |
| Vendor headers and SVD | How are the documented facts represented in software tools? | CMSIS headers, debugger register views, cross-checks |

The datasheet and reference manual describe different layers. The datasheet is
the authority for the exact part and package. The reference manual is the
authority for peripheral operation. The processor manual is the authority for
Cortex-M behavior that is common to many MCUs.

## Reading order

Read the documents in this order:

1. Identify the exact MCU ordering code and package in the datasheet.
2. Record Flash, SRAM, boot modes, clocks, power limits, and package pins.
3. Use the reference manual to trace the clock, reset, bus, and peripheral paths.
4. Use the processor manual to understand reset, exceptions, and core registers.
5. Check errata before relying on timing, analog, low-power, or peripheral details.
6. Check the board schematic before assigning a board component to an MCU pin.
7. Compare vendor headers and SVD data with the documents. The documents remain
   authoritative when the software representation differs.

## Evidence table

Keep extracted facts in a target profile or a focused hardware page. Each fact
must record its source and its implementation consequence.

| Fact | Source | Consequence |
|---|---|---|
| Flash origin and size | Datasheet | Linker `MEMORY` entry |
| Initial stack behavior | Processor manual | Vector table word zero |
| GPIO clock bit | Reference manual | Peripheral initialization order |
| LED connection | Board schematic | Application pin and polarity |

This prevents one document from becoming an unverified collection of copied facts.

## Project sources

The local copies used by this project are listed in the MkDocs reference section:

- [STM32F103x8 datasheet](../refs/stm32f103x8-datasheet.pdf)
- [STM32F103x8 reference manual](../refs/stm32f103x8-reference.pdf)
- [ARM Cortex-M3 Generic User Guide](../refs/arm-cortex-m3-generic-user-guide.pdf)
- [CKS32F103x datasheet](../refs/CKS32F103x.pdf)
- [CKS32F103x8 reference manual](../refs/CKS32F103x8-reference-manual.pdf)

The CKS documents are relevant when the board contains a clone rather than an ST
STM32F103. Do not silently substitute one device's electrical limits for another's.
