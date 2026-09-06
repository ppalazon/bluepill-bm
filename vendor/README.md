# Vendored Dependencies

This directory is reserved for external source code pinned for reproducible
builds.

The intended CMSIS source is:

```text
https://github.com/STMicroelectronics/STM32CubeF1
```

When added, keep it under:

```text
vendor/STM32CubeF1/
```

The Makefile only exposes STM32CubeF1 CMSIS include paths to applications whose
name starts with `cmsis-`.

`bare-*` applications must keep using only the local register definitions in:

```text
include/stm32f103c8t6.h
```
