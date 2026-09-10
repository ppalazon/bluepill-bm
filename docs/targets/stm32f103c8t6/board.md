# Blue Pill board

The Blue Pill is a board, not an MCU model. Board-level facts must be verified
against the particular board revision and schematic.

## Connections used by this project

| Board function | MCU connection |
|---|---|
| Onboard LED | Usually `PC13`, active-low |
| SWDIO | `PA13` |
| SWCLK | `PA14` |
| Ground | `GND` |
| Target reference | `3.3V` |

The LED is not a substitute for a measurement instrument. Its polarity, resistor,
and brightness depend on the board wiring. The STM32F1 datasheet limits the output
drive and speed of `PC13`, `PC14`, and `PC15`. Keep the LED example within those
limits.

## Programming

The project uses OpenOCD with `openocd/bluepill.cfg`:

```sh
make APP=bare-blink
make flash
```

For debugging, run `make openocd` in one terminal and `make debug` in another.
The complete repository commands are documented in [build process](../../project/build-process.md).

Do not connect 5 V to a 3.3 V supply pin. A 5 V-tolerant input specification does
not mean that the MCU supply or every pin can be driven from 5 V.
