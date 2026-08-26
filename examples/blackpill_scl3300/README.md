# WeAct Black Pill + SCL3300 (Arduino ↔ Klin)

Side-by-side sketches: Arduino (`accel_loop.ino`, DavidArmstrong/SCL3300)
and Klin (`accel_loop.kl` + `wire.kl`, `klin_scl3300`).

This folder is an **app** example. The chip driver stays MCU-agnostic; the
bus is yours.

## Hardware

| Piece | Notes |
|---|---|
| MCU | WeAct **STM32F411CE** Black Pill (e.g. [Elektroweb J-094](https://elektroweb.pl/pl/stm32/778-mikrokontroler-stm32f411ceu6-stm32-blackpill.html)) |
| Sensor | Murata **SCL3300-D01** on SPI (Mode 0, ≤ 4 MHz, 32-bit words) |

### Wiring (SPI1 + soft CS)

| SCL3300 | Black Pill |
|---|---|
| VDD / VDDIO | 3V3 |
| GND | GND |
| SCK | PA5 (SPI1 SCK, AF5) |
| MISO | PA6 (SPI1 MISO, AF5) |
| MOSI | PA7 (SPI1 MOSI, AF5) |
| CS | **PB12** (GPIO, idle HIGH) |

Same SPI1 pins as the G-176 TFT example in
[`klin_st7735`](https://github.com/klin-lang/klin_st7735/tree/main/examples/g176_blackpill).
If both share the bus, give each device its own CS and keep unused CS HIGH.

## Arduino

Needs STM32duino (WeAct / Generic F411CE) and
[DavidArmstrong/SCL3300](https://github.com/DavidArmstrong/SCL3300).

Arduino IDE wants a sketch folder named like the `.ino`. Open
`accel_loop.ino` and let the IDE move it, or copy to
`accel_loop/accel_loop.ino`.

`Serial` on this board is usually **USB CDC**. `SPI.begin()` + library
`begin()` hide Mode 0, CS timing, and the off-frame protocol.

`readX` / `readY` / `readZ` return **float g** in the library’s current
mode. Prefer accelerometer mode (library `setMode(1)` / `MODE1`) if you
want “g”; inclinometer mode is for angles.

## Klin

```sh
klin get github/klin-lang/machine_stm32@v0.5.0
klin get github/klin-lang/klin_scl3300@v0.3.0
# from the klin_scl3300 repo root:
#   dart run path/to/klin/bin/klin.dart --emit-c -I. examples/blackpill_scl3300/accel_loop.kl
```

`wire.kl` is the adapter (CS + 32-bit SPI + ≥10 µs gap) — the cost Arduino
hides in `SPI` + `SCL3300.cpp`. `accel_loop.kl` is the app.

| Arduino | Klin |
|---|---|
| `setup` / `loop` | `fn main()` + `while true` |
| `SCL3300(CS)` + `begin()` | `scl.attach(wire, mode): !Dev` |
| `readX/Y/Z` → `float` g | `read()` block + `accel_mg(raw, mode)` → **milli-g** (`i32`) |
| `SPI.begin()` | `machine.spi_out` (≤ 4 MHz, mode 0) |
| USB `Serial.print` | USART1 PA9/PA10 (`write_u8`) — not USB CDC |
| `delay(500)` | busy-wait (or a timer later) |

Default package mode is **4** (inclinometer ±10°). This sketch uses
**mode 1** so `accel_mg` matches “acceleration in g” (printed as milli-g:
1000 ≈ 1.000 g). No float API — that is intentional.

Flash: `klin init weact-f411`, keep `board/`, drop in these `.kl` files,
then `klin get && make && make flash` (BOOT0 + NRST). See Klin
[147](https://github.com/klin-lang/klin/blob/main/issues/147-board-weact-f411.md)
/ [148](https://github.com/klin-lang/klin/blob/main/issues/148-klin-flash.md).

## License

Same as the package (MIT). Arduino library is separate (check upstream).
