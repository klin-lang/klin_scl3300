# klin_scl3300

Klin chip driver for the **Murata SCL3300-D01** 3-axis inclinometer (32-bit
SPI, off-frame).

Not the Arduino `DavidArmstrong/SCL3300` class, not in the Klin stdlib. The
app owns the bus (`machine_*` `Spi` + `Pin`, or any other C/Klin hooks). This
package sends 32-bit frames and converts raw registers.

Default measurement mode: **4** (inclinometer, ±10°). Modes 1–2 are full
±90° / 360° accel-style ranges; 3 is also ±10°.

## Requirements

- [Klin](https://github.com/klin-lang/klin) compiler

## Install

```sh
klin get github/klin-lang/klin_scl3300@v0.3.0
```

Repo: https://github.com/klin-lang/klin_scl3300  
Local: `-I` this tree or a sibling `klin_scl3300/`.

```sh
klin test klin_scl3300
klin run -I. examples/host_smoke.kl
```

Board sketch (Arduino + Klin, WeAct F411CE, CS=PB12):
[`examples/blackpill_scl3300/`](examples/blackpill_scl3300/).

## API (`@v0.3.0`)

| Symbol | Meaning |
|---|---|
| `version(): i32` | `3` at `v0.3.0` |
| `Wire` | `xfer(i32) → i32` (32-bit word) + `delay_ms` + `ctx` |
| `attach(wire, mode): !Dev` | SW reset, mode, enable angles; `error(err_whoami())` if last WHOAMI ≠ `0xC1` |
| `read()` | one block: acc / STO / temp / angles / status / WHOAMI |
| `whoami()` / `connected()` | expect `0xC1` |
| `set_mode` / `reset(): !i32` | change mode; SW reset + re-init (WHOAMI or `err_whoami`) |
| `err_whoami()` | `1` — WHOAMI mismatch after `attach` / `reset` |
| `err_flag1` / `err_flag2` | error flag registers |
| `serial()` | `SERIAL2 << 16 \| SERIAL1` |
| `command_reg` / `cur_bank` | `RdCMD` / `RdCurBank` |
| `angle_mdeg` / `angle_mdeg_360` | datasheet `raw/2^14*90` in millidegrees |
| `accel_mg(raw, mode)` | milli-g; 6000 / 3000 / 12000 LSB/g |
| `temp_mC` / `temp_mF` | datasheet milli-°C; Fahrenheit is `(mC * 9 / 5) + 32000` |
| `crc8_frame` / `frame_*` | Murata CRC-8 and MISO parse |
| `power_down` / `wake` | chip commands |

No heap, no Arduino `SPIClass`, no Fast Read (keep-CS-low). SPI: **Mode 0**,
MSB first, **≤ 4 MHz**. CS low for the 32 clocks, high **≥ 10 µs** between
frames — that gap belongs in `xfer`.

### Wire

```klin
import klin_scl3300 scl

fn spi32(ctx: *mut u8, mos: i32): i32 {
    // CS low, write 32 bits, read 32 bits, CS high, wait ≥10 µs
    return 0
}

fn wait_ms(ctx: *mut u8, ms: i32) {
}

fn main() {
    let wire = scl.Wire{ ctx: cast(*mut u8, 0), xfer: spi32, delay_ms: wait_ms }
    let imu = scl.attach(wire, 4) or {
        printf("scl3300 whoami fail err=%d\n", err)
        return
    }
    let s = imu.read()
    if s.ok() {
        printf("tilt_x_mdeg %d temp_mF %d\n", scl.angle_mdeg(s.ang_x), scl.temp_mF(s.temp))
    }
}
```

`read()` is off-frame: each MISO word is the answer to the **previous** MOSI
command. The first response after reset is discarded inside `attach`.

## Protocol notes

Commands are the datasheet 32-bit words (CRC already in the low byte), same
hex as `SCL3300.h`. This is not a port of that library's C++ class.

## Layout

Directory `klin_scl3300/` is one module. `*_test.kl` stays out of
`import klin_scl3300`.
