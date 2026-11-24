# Confirmed hardware and pin map

The user's confirmed two-sheet circuit drawing is `hardware_reference/mochi-bot-s3-wiring-diagram.pdf`. Follow the drawing if this transcription is incomplete.

---

## Dual-Target Pin Matrix (ESP32-S3 vs ESP32-C6)

The firmware natively supports two hardware targets configurable via `platformio.ini`:
1. **Target 1: ESP32-S3 SuperMini** (Nologo-style, Xtensa LX7 Dual-Core @ 240MHz)
2. **Target 2: ESP32-C6 SuperMini** (RISC-V Single-Core @ 160MHz)

| Function / Signal | ESP32-S3 SuperMini | ESP32-C6 SuperMini | Electrical Constraints & Notes |
|---|---|---|---|
| **I2C SDA** (OLED + MPU6050) | **GPIO8** | **GPIO20** | C6 remapped from GPIO8 (onboard WS2812 LED conflict) |
| **I2C SCL** (OLED + MPU6050) | **GPIO9** | **GPIO19** | C6 remapped from GPIO9 (physical BOOT strapping pin conflict) |
| **I2S Bit Clock (BCLK)** | **GPIO2** | **GPIO2** | Shared bit clock to INMP441 & MAX98357A |
| **I2S Word Select (WS/LRCK)**| **GPIO1** | **GPIO1** | Shared word select / frame clock |
| **I2S Mic Data (DIN)** | **GPIO10** | **GPIO3** | Audio in from INMP441 (transient energy analysis only) |
| **I2S Amp Data (DOUT)** | **GPIO11** | **GPIO18** | Audio out to MAX98357A I2S DAC/Amp |
| **Motion Interrupt (INT)** | **GPIO4** | **GPIO4** | Active HIGH motion/wake interrupt from MPU6050 |
| **Dock Sense** | **GPIO5** | **GPIO5** | 10kΩ/20kΩ divider from 5V dock rail (3.3V docked, 0V undocked) |
| **Capacitive Touch** | **GPIO6** | **GPIO6** | TTP223 momentary touch input (active HIGH) |
| **TP4056 Charge Sense** | **GPIO7** | **GPIO7** | Cathode of TP4056 Red CHRG LED via 1N4148 diode (active LOW) |
| **Native USB D−** | **GPIO19** | **GPIO12** | S3 uses native USB CDC; C6 uses USB Serial/JTAG |
| **Native USB D+** | **GPIO20** | **GPIO13** | S3 uses native USB CDC; C6 uses USB Serial/JTAG |

---

## ESP32-C6 Strapping Pin & Peripheral Isolation Rationale

When porting to the ESP32-C6 SuperMini, direct pin-for-pin copying of S3 GPIOs 8/9 would cause hardware failure:
1. **GPIO9 on ESP32-C6** is the **BOOT Strapping Pin**. Pulling GPIO9 low at power-up enters ROM bootloader mode. Connecting an I2C clock (SCL) with strong pull-up or slave pull-down creates boot lockups and bus contention.
2. **GPIO8 on ESP32-C6** is hardwired on the SuperMini PCB to an **onboard WS2812 Addressable RGB LED**. Driving I2C data on GPIO8 produces flickering and data distortion.
3. **Resolution**: On ESP32-C6, the native USB Serial/JTAG peripheral is on GPIO12 (D-) and GPIO13 (D+). This frees up **GPIO20** and **GPIO19** as completely clean, unstrapped general-purpose I/O pins, making them the optimal choices for I2C SDA and SCL.
4. **All other pins (1, 2, 4, 5, 6, 7)** remain identical between both platforms, allowing 80% shared wiring harness compatibility.

---

## Detailed Pin-to-Pin Connection Map (ESP32-S3 Baseline)

| Module / signal | Connect to | Firmware use / notes |
|---|---|---|
| OLED VCC/GND | S3 3V3 / common GND | 3.3 V-compatible I2C OLED (SSD1306 / SH1106) |
| OLED SDA/SCL | GPIO8 / GPIO9 | Shared I2C bus (replaces to GPIO20/GPIO19 on C6) |
| MPU6050 VCC/GND | 3V3 / common GND | GY-521 module on 3.3V rail |
| MPU6050 SDA/SCL | GPIO8 / GPIO9 | Shared I2C bus (replaces to GPIO20/GPIO19 on C6) |
| MPU6050 AD0 | GND | Expected address 0x68 (pulled low on GY-521) |
| MPU6050 INT | GPIO4 | Motion/wake interrupt |
| INMP441 VDD/GND | 3V3 / common GND | Confirm module bypass and SD pulldown |
| INMP441 SCK/BCLK | GPIO2 | Shared I2S bit clock |
| INMP441 WS/LRCK | GPIO1 | Shared I2S word select |
| INMP441 SD/DOUT | GPIO10 | Mic data into S3 (GPIO3 on C6) |
| INMP441 L/R | GND | Left channel |
| MAX98357A VIN/GND | SYS / common GND | SYS is board 5V header downstream of dock blocker |
| MAX98357A BCLK, LRC/WS | GPIO2, GPIO1 | Shared I2S clocks |
| MAX98357A DIN | GPIO11 | Audio data from S3 (GPIO18 on C6) |
| MAX98357A SD/MODE | 3V3 | Left channel per drawing; verify actual module straps |
| MAX98357A GAIN | Module default | No added connection |
| TTP223 VCC/GND | 3V3 / common GND | Capacitive touch expression switch |
| TTP223 I/O | GPIO6 | Active HIGH momentary touch input |
| Speaker | MAX98357A OUT+ and OUT− | Bridge output. Neither terminal to GND. |
| 2-Pin Dock P1 | Regulated +5.0 V DC | Connected to TP4056 IN+ and 10k/20k divider to GPIO5 |
| 2-Pin Dock P2 | Common GND | Supply negative / system ground return |
| Dock Sense (GPIO5) | 10kΩ/20kΩ divider center | 3.3V when docked (HIGH), 0.0V when undocked (LOW) |
| Charge Sense (GPIO7)| 1N4148 Diode Anode | Cathode to TP4056 Red CHRG LED. Active LOW with internal pullup |
| SPDT Slide Switch | Series w/ BAT+ | Pin 2: LiPo B+ (in), Pin 3: TP4056 BAT+ (switched out), Pin 1: NC |
| Protected battery (−)| TP4056 BAT− pad | Protected 1S 3.7V 500 mAh LiPo ground return |
| System 5V / SYS | TP4056 OUT+ | Powers ESP32 5V header and MAX98357A VIN |

GPIO19/20 on ESP32-S3 are native USB D−/D+. Do not repurpose on S3. Cradle must cover/exclude USB-C to prevent simultaneous dock and USB connection. Program with unit undocked and cell disconnected, as specified in the PDF.

## MAX40200AUK+ reverse blocker

SOT-23-5 pin mapping from the PDF:

- Pin 1 VDD and pin 3 EN to dock P1 / protected 5 V input.
- Pin 5 OUT to S3 5V/SYS header and MAX98357A VIN.
- Pin 2 GND to common ground.
- Pin 4 NC left open.
- C1 4.7 uF from input to GND. C2 1 uF from output to GND. X7R, at least 10 V, close to U6. Verify effective capacitance under 5 V bias per drawing.
- R1 100 kOhm from input to ground to discharge the input pad after undocking.

Use regulated, current-limited 5 V. Vehicle use requires a regulated automotive USB adapter, never raw vehicle voltage.

## Power constraints and validation

- Protected 1S LiPo, nominal 3.7 V, 500 mAh, 4.2 V full-charge endpoint with integrated PCM protection.
- TP4056 linear charger handles cell charging via the 2-pin magnetic dock (+5V, GND).
- Dock sensing on GPIO5 via 10kΩ/20kΩ voltage divider (3.3V when docked, 0.0V undocked).
- Charge status on GPIO7 via 1N4148 diode (LOW when actively charging, HIGH when full/standby).
- Dock 5 V powers SYS and charges the battery when docked. When undocked, the 500 mAh cell powers the system.
- Mechanical design must exclude USB-C while docked.
- Flashing safety: program only via native USB CDC while undocked and battery disconnected.

## Check actual modules before power

- Verify S3 board variant/marking, B+/B− labels, BOOST state, USB clearance, and U6 orientation.
- Verify MPU6050 (GY-521) AD0 strap and I2C address (0x68).
- Verify OLED controller/address and voltage tolerance.
- Verify INMP441 carrier local bypass capacitance and SD pulldown.
- Verify amp SD/MODE and GAIN straps, speaker impedance/rating, audio noise/clipping.
- Verify sequential I2S RX/TX mode switch on chosen S3 framework.
