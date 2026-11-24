# Mochi Bot V1 - Physical Hardware Test Log & Bench Validation Guide

This document defines the bench validation procedures, current consumption measurement steps, thermal profiles, and boundary testing required before field deployment of Mochi Bot V1.

---

## 1. Safety & Programming Rules

1. **Flashing Condition**:
   - Always program and flash firmware via Native USB CDC (GPIO19 D−, GPIO20 D+) **ONLY** while the bot is **undocked** and the **LiPo battery is disconnected**.
   - Simultaneous USB connection and dock attachment must be prevented by the 3D-printed enclosure or mechanical clearance.
2. **Charger Configuration**:
   - Verify TP4056 linear charger configuration on the ESP32-S3 board.
   - The `BOOST` jumper must remain **OPEN** (~100 mA charge rate) to match the 500 mAh 1S LiPo safely (0.2C charging).
3. **Polarity Check**:
   - Prior to attaching the battery pack to the B+ and B− solder pads, measure the pack open-circuit voltage with a digital multimeter (DMM) to confirm polarity (Red = B+, Black = B−).
4. **Dock Polarity**:
   - 2-Pin Magnetic Pogo Pin 1: Regulated +5.0 V DC.
   - 2-Pin Magnetic Pogo Pin 2: System Ground (GND).
   - Ensure the dock cradle cannot reverse polarity under magnetic attraction.

---

## 2. Power Consumption Bench Test Matrix

Use a bench power supply with current resolution <= 0.1 mA (or a dedicated power profiler like Nordic PPK2) connected in series with the battery rail (nominal 3.7 V):

| Test Mode | State / Activity | Expected Current (mA) | Measured Current (mA) | Pass / Fail |
|---|---|---|---|---|
| **P01** | `PORTABLE_SLEEP` (OLED Off, I2S Off, Wi-Fi Off, MPU6050 Armed, Touch Armed) | ~12 - 18 mA | | [ ] |
| **P02** | `PORTABLE_ACTIVE` (Awake Face, 100% Brightness, Mic RX Active, Wi-Fi Modem Sleep) | ~75 - 95 mA | | [ ] |
| **P03** | `PORTABLE_ACTIVE` (Clap Event + Dual-tone Chirp Playback Peak) | ~140 - 220 mA (Peak) | | [ ] |
| **P04** | `DOCK_HOME` (Clock Card, 70% Brightness, Wi-Fi Active, BLE Connected) | Powered from Dock | | [ ] |
| **P05** | `DOCK_HOME` (Battery Lead Current while Docked) | <= 100 mA (Charge) | | [ ] |
| **P06** | `DOCK_HOME` (Battery Lead Current when Charge Full / Red LED Off) | 0.0 mA (No trickle) | | [ ] |

> [!IMPORTANT]
> If the LiPo battery supplies system current while docked on the 5V pogo cradle (P05/P06), immediately disconnect and verify the dock 5V input rail, diode drop, and TP4056 board routing.

---

## 3. Thermal Profile & Temperature Logging

Perform an 8-hour continuous dock test in ambient 22°C - 25°C room temperature with full Wi-Fi telemetry active:

| Sensor / Component | Location | Max Allowed Temp | Measured 1h | Measured 4h | Measured 8h | Pass / Fail |
|---|---|---|---|---|---|---|
| **TP4056 IC** | Charger chip surface | <= 65°C | | | | [ ] |
| **ESP32-S3 SoC** | RF shield surface | <= 60°C | | | | [ ] |
| **MAX98357A** | Audio amplifier IC | <= 50°C | | | | [ ] |
| **LiPo Cell** | Battery pouch surface | <= 45°C | | | | [ ] |

---

## 4. Signal & Peripheral Functional Validation

### 4.1 2-Pin Dock & Voltage Divider (GPIO5)
- [ ] Measure voltage at GPIO5 pad when undocked: Must be **0.0 V**.
- [ ] Measure voltage at GPIO5 pad when docked (+5.0 V on pogo): Must be **3.3 V ± 0.1 V** (via 10k/20k divider).
- [ ] Confirm debounced dock event logs in serial console (`[EVENT] Dock transition: DOCKED / ATTACHED`).

### 4.2 TP4056 Charge Monitoring (GPIO7)
- [ ] Connect depleted battery (< 3.8 V) and attach dock.
- [ ] Verify TP4056 Red LED lights up.
- [ ] Measure voltage at GPIO7 pad: Must be **LOW (< 0.6 V)** due to 1N4148 diode conduction.
- [ ] Disconnect dock or wait until charge complete (Red LED off).
- [ ] Measure voltage at GPIO7 pad: Must be pulled **HIGH (3.3 V)** by internal pullup.
- [ ] Confirm serial event logs: `[EVENT] Charge status: ACTIVELY CHARGING` -> `STANDBY / FULL / OFF`.

### 4.3 TTP223 Capacitive Touch Sensor (GPIO6)
- [ ] Tap touch sensor: Verifies `app_state_on_touch(true)` trigger.
- [ ] In `PORTABLE_SLEEP`: Wakes bot to `PORTABLE_ACTIVE` and shows awake face.
- [ ] In `PORTABLE_ACTIVE`: Displays Happy expression reaction.
- [ ] In `DOCK_HOME`: Cycles through 4 glance cards (Clock -> Sleepy Face -> Telemetry -> PC Stats).
- [ ] When notification is visible: Dismisses notification alert immediately.

### 4.4 MPU6050 6-Axis Motion Sensor & Interrupt (GPIO4)
- [ ] Verify I2C acknowledge at address `0x68` (or `0x69`).
- [ ] Verify `WHO_AM_I` register reads `0x68` (or `0x70`/`0x72`).
- [ ] In `PORTABLE_SLEEP`: Gentle tap/movement triggers INT on GPIO4, waking the bot immediately.
- [ ] In `PORTABLE_ACTIVE`: Nudge triggers curious Wink expression reaction.

### 4.5 Sequential I2S Audio & Clap Detection (GPIO2, GPIO1, GPIO10, GPIO11)
- [ ] In quiet room: Measure baseline mic noise floor (typically ~100 to 300).
- [ ] Sharp hand clap at 0.5m: Triggers `app_state_on_clap()`, shows Happy face, and plays chirp.
- [ ] Continuous talking / background music: Confirms zero false triggers (adaptive EMA noise rejection).
- [ ] Chirp playback: Confirms speaker audio does **not** trigger the microphone detector (750 ms acoustic suppression).

### 4.6 BLE Companion Connectivity
- [ ] Android companion connects to `Mochi-Bot`.
- [ ] Verify 6-digit numeric passkey displays clearly on OLED.
- [ ] Verify Time Sync updates POSIX clock.
- [ ] Verify incoming notification displays banner with progress bar.
- [ ] In Driving Mode (phone moving > 0 km/h): Verify speedometer dashboard with animated road dashes.

### 4.7 Windows Wi-Fi Telemetry & REST API
- [ ] Wi-Fi connects to configured AP and obtains IP address.
- [ ] Send `POST /api/v1/telemetry` with Bearer Token from Windows script.
- [ ] Dock Card 4 (`PC TELEMETRY`) displays CPU %, RAM %, GPU %, CPU temp bars.
- [ ] Stop Windows telemetry script: After 15 seconds, displays `"PC Disconnected / Sleep"`.

---

## 5. Architectural & Privacy Compliance

- [x] **Zero Audio Storage / Transmission**: No audio buffers are retained or transmitted over BLE/Wi-Fi.
- [x] **Honest Telemetry**: No simulated battery percentage or invented hardware metrics.
- [x] **Isolated Local Network**: No third-party cloud servers or external telemetry endpoints.
- [x] **Non-blocking Loop**: Zero `delay()` inside event handlers; cooperative scheduling throughout.
