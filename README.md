# Instrumentation Control System

Control system for the Husky Robotics Science Box. A Raspberry Pi runs C programs that control actuators and read sensors; an Efinix Ti60F225 FPGA handles real-time tasks (encoder counting, PWM generation, Hall sensor aggregation) and communicates with the Pi over a custom 32-bit SPI interface.

---

## System Architecture

```
+---------------------------------+        SPI (10 MHz)        +----------------------------------+
|         Raspberry Pi            | <------------------------> |      Efinix Ti60F225 FPGA        |
|                                 |                            |                                  |
|  C programs (this repo)         |                            |  - Quadrature encoder counters   |
|  +- High-level automation       |                            |  - PWM generators (motor/servo)  |
|  +- Subsystem controllers       |                            |  - PWM-in decoder (abs. encoder) |
|  +- functions.c SPI library     |                            |  - Hall sensor aggregation       |
+---------------------------------+                            +----------------------------------+
         |
         |  GPIO / wiringPi
         v
  Pumps, relays, fans, heating element, I2C sensors, 1-wire temperature, ToF, color sensor
```

---

## SPI Command Protocol

Every transaction is a 32-bit word. The Pi is always the SPI leader.

| Bits  | Field           | Description                                      |
|-------|-----------------|--------------------------------------------------|
| 31:26 | Command         | Selects the FPGA operation                       |
| 25:21 | Address/Index   | Motor channel, encoder index, or data address    |
| 20:0  | Data            | PWM period/uptime (µs), or data request address  |

**Command codes**

| Code | Name              | Description                                              |
|------|-------------------|----------------------------------------------------------|
| 0    | Set PWM uptime    | Set on-time (µs) for PWM channel [25:21]                 |
| 1    | Get data          | Request FPGA to return data at data address [7:0]        |
| 2    | Set PWM period    | Set period (µs) for PWM channel [25:21]                  |
| 3    | Reset encoder     | Zero the count for encoder [25:21]                       |

**Data addresses (used with command 1)**

| Address | Returns                                            |
|---------|----------------------------------------------------|
| 0       | Debug echo                                         |
| 2       | Encoder 0 count (32-bit signed)                    |
| 3       | Encoder 1 count                                    |
| 4       | Encoder 2 count                                    |
| 5       | Hall sensor inputs (bits 0–2)                      |
| 6       | Centrifuge absolute encoder PWM uptime             |

The C-side API is in [C_Code/functions.c](C_Code/functions.c) and [C_Code/functions.h](C_Code/functions.h). The FPGA-side implementation is in [verilog/SPI.sv](verilog/SPI.sv) and [verilog/top_level.sv](verilog/top_level.sv).

---

## Hardware Pin Mapping

Pin constants are defined in [C_Code/pins.h](C_Code/pins.h) using wiringPi numbering.

| Alias               | wPi Pin | Connected To                   |
|---------------------|---------|--------------------------------|
| `HEAT_ELEMENT_PIN`  | 25      | 12 V heating element           |
| `NINHYDRIN_PIN`     | 23      | Ninhydrin reagent pump         |
| `KCL_PIN`           | 22      | KCl reagent pump               |
| `SPEC_PIN`          | 21      | Spectrometer pump              |
| `DISPOSAL_PIN`      | 30      | Disposal pump                  |
| `CENTRIFUGE_PIN`    | 0       | Centrifuge motor relay         |
| `FLUIDSPUMP_IN/OUT` | 5 / 4   | Fluids pump H-bridge           |
| `SPIN_COLUMN_PIN1/2`| 1 / 16  | Column rotation H-bridge       |
| `COLUMN_RL_PIN1/2`  | 29 / 27 | Column raise/lower H-bridge    |
| `LOWER/RAISE_PUMP`  | 11 / 31 | Pump raise/lower H-bridge      |
| `SDA/SCL`           | 8 / 9   | I²C bus                        |
| `MOSI/MISO/SCLK/CE` | 12/13/14/10 | SPI bus to FPGA           |
| `ONEWIRE`           | 7       | 1-wire temperature sensor      |

FPGA encoder and PWM channel assignments:

| Constant                | FPGA Channel | Sensor/Actuator            |
|-------------------------|-------------|----------------------------|
| `ENC_CENTRIFUGE_INC`    | 2           | Centrifuge incremental enc.|
| `ENC_CENTRIFUGE_ABS`    | 6           | Centrifuge absolute enc.   |
| `ENC_RAISE_LOWER`       | 3           | Column raise/lower encoder |
| `ENC_COLUMN_RL_INDEX`   | 1           | Column index encoder       |
| `ENC_COLUMN_ROTATE`     | 4           | Column rotation encoder    |
| `CENTRIFUGE_SERVO_CHANNEL` | 0        | Centrifuge servo PWM out   |
| `AUGUR_CHANNEL`         | 1           | Augur drill PWM out        |
| `HALL_CHANNEL`          | 5           | Hall sensor data           |

---

## File Structure

```
HR-pi/
│
├── C_Code/                     # All Raspberry Pi C code
│   ├── functions.c / functions.h   # Core SPI library: fpga_command, fpga_datatran,
│   │                               #   fpga_pwm_uptime/period, fpga_reset_encoder, etc.
│   ├── pins.h                  # wiringPi pin and FPGA channel constants
│   ├── Makefile                # Build system (see Building below)
│   ├── mains.txt               # List of source files that become standalone binaries
│   ├── Compilation_instructions.txt
│   │
│   ├── Automation/             # High-level automated sequences
│   │   ├── dirtSample.c        # Full dirt-sample pickup and deposit sequence
│   │   ├── centrifugeFluids.c  # Centrifuge a fluid sample
│   │   ├── mixingChamberAutomation.c  # Reagent mixing sequence
│   │   └── burnerController.c  # Heating element control loop
│   │
│   ├── Augur/                  # Augur drill control
│   │   └── PWM_Map.c           # Maps commanded speed to FPGA PWM uptime
│   │
│   ├── Centrifuge/             # Centrifuge subsystem
│   │   ├── centrifugeSpin.c    # Spin to target RPM and hold
│   │   ├── rotateServoTo.c     # Move centrifuge servo to absolute position
│   │   └── old/                # Superseded centrifuge implementations
│   │
│   ├── Column/                 # Sample column subsystem
│   │   ├── raise_lower_column.c      # Move column to absolute encoder position
│   │   ├── raise_lower_column_acc.c  # Acceleration-ramped version
│   │   └── rotateTo_column.c         # Rotate column to target angle
│   │
│   ├── Pump/                   # Fluid pump subsystem
│   │   ├── raiseLower_Pump.c   # Move pump head to height via encoder feedback
│   │   ├── pumpToHeight.c      # High-level pump-to-height wrapper
│   │   └── flood_spectrometer.c # Fill spectrometer cuvette
│   │
│   ├── Sensors/                # Individual sensor drivers
│   │   ├── colorRead.c / colorReadTest.c  # TCS color sensor read/log
│   │   ├── tempSensor.c / tempSensorRead.c  # I²C temperature sensor
│   │   ├── 1wTempSensor.c      # DS18B20 1-wire temperature sensor
│   │   └── tof.c               # VL53L0X time-of-flight distance sensor
│   │
│   ├── FPGAtesting/            # Low-level FPGA diagnostic programs
│   │   ├── Get_data.c          # Read a data address and print result
│   │   ├── GetRaw.c            # Raw 32-bit SPI transaction
│   │   ├── fpwm.c              # Set PWM period and uptime interactively
│   │   ├── centriAbs.c         # Read centrifuge absolute encoder
│   │   └── stresstest.c / fasttest.c / sigintTest.c  # Throughput and interrupt tests
│   │
│   ├── SystemsTesting/         # Integration and subsystem tests
│   │   ├── softPWM.c           # Software PWM test
│   │   ├── resetEnc.c          # Reset and verify encoder
│   │   ├── toggle.c / all_low.c  # GPIO state tests
│   │   ├── tofTest.c           # ToF sensor integration test
│   │   ├── tempSensorRead.c / 1wTempSensor.c  # Sensor read tests
│   │   ├── waterLevelRead.c    # ADS1015-based water level test
│   │   └── pumpToHeight.c / flood_spectrometer.c  # Pump integration tests
│   │
│   └── archive/                # Unused code kept for reference
│       ├── sdp_i2c/            # SDP pressure sensor driver (Sensirion)
│       └── *.c                 # Older motor, servo, and sensor implementations
│
├── verilog/                    # FPGA source (Efinix Ti60F225, synthesized with Efinity)
│   ├── top_level.sv            # Top-level module; wires all submodules together
│   ├── SPI.sv                  # SPI follower: latches 32-bit commands, drives 32-bit replies
│   ├── encoder.sv              # Quadrature encoder counter
│   ├── pwm.sv                  # PWM generator (period + uptime configurable)
│   ├── pwm_in.sv               # PWM input decoder (reads absolute encoder pulse width)
│   ├── posedge_trigger.sv      # Edge-detection utility
│   ├── README.md               # FPGA module hierarchy
│   ├── outflow/                # Efinity place-and-route outputs (bitfile, reports)
│   ├── work_pnr/               # Efinity PnR working files
│   ├── work_syn/               # Efinity synthesis working files
│   └── archive/                # Earlier spectrometer and ultrasonic sensor modules
│
├── calibration_data/           # CSV calibration files
│   ├── calibration.csv         # General sensor calibration
│   ├── dark_calibration_final.csv   # Spectrometer dark reference
│   ├── light_calibration_final.csv  # Spectrometer light reference
│   └── peaks_colors.csv        # Color sensor peak mapping
│
├── output_data/                # Raw sensor output logs
│   ├── colorReadTest.csv       # Color sensor test data
│   ├── TempAndHumidity.csv     # Temperature and humidity log
│   └── peaks_colors.csv        # Color peak output
│
└── wiringpi_setup.sh           # Script to build and install wiringPi from source
```

---

## Building

### Prerequisites

- GCC cross-compiler (or build directly on the Pi)
- [wiringPi](https://github.com/WiringPi/WiringPi) — run `wiringpi_setup.sh` to install from source if it is not already present

### Compiling

1. Add the relative path to each `.c` file you want as a binary to `C_Code/mains.txt` (one per line; lines starting with `#` are comments).
2. From `C_Code/`, run:

```bash
make
```

Each entry in `mains.txt` is compiled with `-DIS_MAIN`, which enables its `main()` function. All other `.c` files are compiled into a shared static library (`libcommon.a`) and linked into every binary automatically. Binaries are placed in `C_Code/`.

```bash
make clean   # remove build artifacts and binaries
```

---

## FPGA

The FPGA project targets the **Efinix Ti60F225** development board and is synthesized with **Efinity**. The bitfile (`verilog/outflow/test_project.bit`) can be programmed with:

```bash
bash verilog/work_pnr/run_efx_pgm.sh
```

To re-synthesize after editing HDL, open the project in Efinity or run the synthesis and PnR scripts in `verilog/work_syn/` and `verilog/work_pnr/`.

**FPGA module hierarchy:**

```
top_level
├── SPI          — 32-bit SPI follower receiver/transmitter
├── encoder (×3) — Quadrature encoder counters
├── pwm (×3)     — Configurable PWM output generators
└── pwm_in       — PWM pulse-width decoder (centrifuge absolute encoder)
```
