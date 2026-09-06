# Vendored Dependencies

This directory contains external dependencies pinned through Git submodules.

## STM32CubeF1

`vendor/STM32CubeF1/` is a Git submodule for ST's STM32CubeF1 repository:

```text
https://github.com/STMicroelectronics/STM32CubeF1
```

Current checked submodule state:

```text
d12e75247d5bcedc734f829b394517ab4c2726e3 vendor/STM32CubeF1 (v1.8.7)
```

The submodule is intentionally pinned to the official `v1.8.7` release tag, not
to an arbitrary branch tip.

Initialize it after cloning the parent repository:

```sh
git submodule update --init --recursive
```

Update it to the commit recorded by the parent repository:

```sh
git submodule update --recursive
```

Check its current state:

```sh
git submodule status
```

## Usage Rule

STM32CubeF1 is only for applications whose name starts with `cmsis-`.

`bare-*` applications must keep using only the local register definitions in:

```text
include/stm32f103c8t6.h
```

The Makefile implements that boundary with:

```make
ifneq ($(filter cmsis-%,$(APP)),)
CFLAGS += -I$(CMSIS_CORE_INC) -I$(CMSIS_DEVICE_INC) -DUSE_CMSIS
endif
```

The expected CMSIS include paths are:

```text
vendor/STM32CubeF1/Drivers/CMSIS/Core/Include
vendor/STM32CubeF1/Drivers/CMSIS/Device/ST/STM32F1xx/Include
```

Do not add these include paths globally through `CPATH` or the root `.clangd`.
Each future `apps/cmsis-*` application should have its own `.clangd` fragment if
clangd needs to see CMSIS headers for editor diagnostics.

## Scope

For now, this project should use STM32CubeF1 only for CMSIS examples.

Do not use STM32 HAL or LL drivers in `bare-*` examples. Add HAL or LL only if a
future application explicitly needs to demonstrate those layers.
