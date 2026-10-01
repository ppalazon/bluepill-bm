# Blue Pill Bare-Metal

The main objective of this project is to learn how create bare metal
applications for any kind of microcontroller. In this case, I've just selected
one that I have at home, and more specifically the Blue Pill board with the
STM32F103C microcontroller.

![Blue Pill board](docs/assets/img-20260904-094011.png)

- Board: Blue Pill development board
- MCU: STM32F103C8T6
- Core: ARM Cortex-M3
- Flash: 64 KiB official C8T6 size
- RAM: 20 KiB
- Debug/programming: ST-Link V2 clone over SWD
- Onboard LED: usually `PC13`, active-low

But, to make it a little bit harder, it's a clone microcontroller by the
Chinese CKS. So, I can't use the official IDE (STM32CubeIDE), and I have to
create my own workflow and build system to work with this board. I use
well-known open source application such as gcc and make.

## Bare metal applications

The idea behind the bare metal applications is that they are executed directly
by the microprocessor without a operating system that manages the resources.
Once the application starts it must be executed forever in a super-loop with no
end on it. A very common of this loop is the following snippet:

```c
while (1) { ... }
```

## Initialization

To reach to super-loop of an useful application we need to answer the following
questions:

- What happens after reset
- How the vector table and startup code reach `main()`
- How the linker script maps Flash and RAM
- How C runtime sections like `.data` and `.bss` are prepared
- How peripheral registers are configured directly

Once, we've got a common environment for this board, creating applications
would be easier.

Target hardware:

The project intentionally stays small and educational. Early `bare-*` examples
use local register definitions only. `cmsis-*` examples can use the pinned
STM32CubeF1 CMSIS headers while keeping that abstraction layer separate.

Build, flash, debug, repository layout, and project workflow notes live in the
documentation site under `docs/`.

## License

The code and documentation in this repository use the
[BSD 3-Clause License](LICENSE). Reference documents and other third-party
material remain under the licenses and terms of their copyright holders.

## Trademark notice

STM32 is a trademark of STMicroelectronics. This independent project uses the
STM32 name only to identify the target device and compatible software. The
project is not affiliated with or endorsed by STMicroelectronics.
