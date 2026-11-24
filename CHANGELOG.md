# Changelog

All notable changes to the Mochi Bot V1 firmware project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [1.0.0-M7-RC1] - 2026-11-22

### Milestone 7: Integrated Power, Charging, and Thermal Validation / Production Polish

#### Added
- **Power Transition Matrix**:
  - Full end-to-end integration between `SYS_STATE_DOCK_HOME`, `SYS_STATE_PORTABLE_ACTIVE`, and `SYS_STATE_PORTABLE_SLEEP`.
  - Immediate Wi-Fi radio power-up and reconnection upon docking or waking from `PORTABLE_SLEEP`.
  - Automatic OLED display panel power-down (`u8g2.setPowerSave(1)`) after 30 seconds of inactivity in portable mode.
  - Automatic sequential I2S bus shutdown (`i2s_bus_stop()`) in portable sleep to eliminate DMA audio bus power drain.
  - Automatic Wi-Fi radio shutdown (`WiFi.mode(WIFI_OFF)`) in portable sleep to conserve battery power.
- **Hardware Wake Sources**:
  - TTP223 capacitive touch interrupt/poll on GPIO6.
  - MPU6050 6-axis motion wake interrupt on INT (GPIO4).
  - 2-pin magnetic dock attachment on GPIO5 (10k/20k divider).
  - BLE companion incoming alerts / connections.
- **MPU6050 6-Axis IMU Migration**:
  - Replaced legacy LIS2DH12 driver with high-performance `motion_mpu6050` driver.
  - Probes default I2C address `0x68` (AD0=GND) and alternate `0x69` (AD0=VCC), reading WHO_AM_I register `0x75`.
  - Configured +-2g accelerometer scale with low-power gyroscope standby to reduce power consumption by ~80%.
  - Routed motion detection interrupt to GPIO4 with latched active-HIGH signaling and non-blocking INT_STATUS clearing.
- **Physical SPDT Battery Isolation Switch**:
  - Integrated SPDT micro slide switch in series with LiPo `BAT+` (Pin 2: LiPo B+ in, Pin 3: TP4056 BAT+ switched out, Pin 1: NC).
  - Eliminates parasitic quiescent drain during long-term storage and provides true zero-power cold isolation when flashing firmware over USB-C.
  - Updated interactive photorealistic wiring diagram (`mochi_wiring_diagram.html`) with clickable toggle state and live power rail visualization.
- **Final 3D CAD Blueprint & Flush Lanyard Integration**:
  - Locked production envelope: $49.0\text{ mm (W)} \times 63.5\text{ mm (H)} \times 19.5/25.5\text{ mm (Z)}$ stepped clamshell.
  - Added flush Top-Right corner lanyard tunnel ($\varnothing 3.5\text{ mm}$ channel at $Z = 15.5\text{ mm}$, $3.0\text{ mm}$ internal bridge) for zero-protrusion bag charm carry.
  - Sized universal dock pocket ($49.8 \times 26.3\text{ mm}$, $12.0\text{ mm}$ lip, $10.5\text{ mm}$ pogo offset) across all 3 minimalist docks.
  - Created parametric OpenSCAD source in `tools/cad/mochi_bot_enclosure.scad` and full mechanical specification in `docs/3D_CAD_AND_ENCLOSURE_SPEC.md`.
- **Hardware Bench Test Protocols**:
  - Created `docs/HARDWARE_TEST_LOG.md` with multi-rail measurement procedures, thermal safety boundaries, charge curve logging, and acoustic noise tuning sheets.

#### Changed
- Upgraded `FIRMWARE_VERSION` to `1.0.0-M7-RC1`.
- Cleaned up diagnostic boot banners and memory usage reporting.
- Ensured zero floating pins and strict debouncing across GPIO4, GPIO5, GPIO6, and GPIO7.

---

## [0.6.0-M6] - 2026-11-22

### Milestone 6: Windows Wi-Fi Telemetry & REST API Service

#### Added
- Station mode 802.11 b/g/n Wi-Fi manager with non-blocking 10-second reconnect backoff and modem sleep (`WiFi.setSleep(true)`).
- Local REST API server on port 80:
  - `POST /api/v1/telemetry`: CPU %, RAM %, GPU %, CPU temperature with 15-second stale timeout.
  - `GET /api/v1/status`: Device health, heap, uptime, Wi-Fi RSSI, clock status.
  - `POST /api/v1/notify`: Windows PC notification popups with title, body, and duration.
  - HTTP `Authorization: Bearer <token>` security enforcement.
- 4th Glance Card (`DOCK_CARD_PC_STATS`) with dynamic bar meters and Wi-Fi RSSI badge.

---

## [0.5.0-M5] - 2026-11-22

### Milestone 5: Android BLE Companion Service & Driving Mode

#### Added
- NimBLE server implementation with custom 128-bit UUIDs for Mochi Bot Service.
- Characteristics for Time Sync, Notifications, Phone Speed, Settings Sync, and Hardware Status.
- Passkey security pairing display (`BLE_HS_IO_DISPLAY_ONLY`) rendering 6-digit numeric codes.
- Transient notification overlay with auto-wrapping text, progress bar, and touch dismissal.
- Real-time Driving Mode speed dashboard (>0 km/h) with animated scrolling road dashes.

---

## [0.4.0-M4] - 2026-11-22

### Milestone 4: NVS Settings Persistence, Clock Engine & Dock Glance Cards

#### Added
- Versioned NVS flash storage (`mochi_cfg`, version 2) with wear-leveling delta protection.
- POSIX software RTC clock engine (`settimeofday`, `localtime_r`) supporting 12h/24h time formatting.
- 3-card dock carousel: Smart Clock, Sleepy Breathing Face, Hardware Telemetry.
- Touch sensor (GPIO6) card cycling and automatic timer rotation.

---

## [0.3.0-M3] - 2026-11-22

### Milestone 3: Sequential I2S Audio Bus & Acoustic Clap Detector

#### Added
- Half-duplex sequential I2S bus arbiter sharing GPIO2 (BCLK) and GPIO1 (WS/LRC).
- INMP441 MEMS microphone driver on GPIO10 (RX) running at 16 kHz 32-bit.
- Bounded 10 ms MAA energy detector with adaptive EMA background noise tracking.
- MAX98357A I2S Class-D amplifier driver on GPIO11 (TX) with dual-tone chirp audio synthesis.
- Strict 750 ms acoustic suppression tail preventing speaker self-triggering.
- Absolute zero audio recording, storage, transcription, or transmission guarantee.

---

## [0.2.0-M2] - 2026-11-22

### Milestone 2: LIS2DH12 Accelerometer & Motion Wake Interrupt

#### Added
- Register-level LIS2DH12 3-axis accelerometer driver over shared I2C bus (GPIO8/9).
- Low-power 25 Hz normal mode configuration with latched INT1 threshold interrupt on GPIO4.
- Atomic `IRAM_ATTR` ISR and non-blocking latch clearing in cooperative loop.
- Motion wake from portable sleep and curious wink expression reaction in portable active mode.

---

## [0.1.0-M1] - 2026-11-22

### Milestone 1: 1.3" SH1106 OLED Driver & Procedural Expressions

#### Added
- U8g2 monochrome 128x64 SH1106 driver at 400 kHz I2C bus speed.
- Procedural vector expressions: Sleepy face with sine-wave breathing, Awake, Blink, Happy, Wink.
- Non-blocking 50 ms debounce for 2-pin magnetic dock sense on GPIO5 (10k/20k divider).
- Non-blocking 50 ms debounce for TP4056 charge status on GPIO7 (1N4148 diode).
- Non-blocking 30 ms debounce for TTP223 capacitive touch switch on GPIO6.

---

## [0.0.1-M0] - 2026-11-21

### Milestone 0: Identity, Centralized Pin Map & Runtime Diagnostics

#### Added
- PlatformIO project scaffolding targeting ESP32-S3 SuperMini (`espressif32 @ 6.6.0`, Arduino core 2.0.14).
- Centralized hardware definitions and pin map in `include/board_config.h`.
- Native USB CDC serial console diagnostics (GPIO19 D-, GPIO20 D+).
- Hardware inventory scanner: I2C bus scan, WHO_AM_I probe, GPIO pin sanity checks.
- Bounded wait loops, bus timeout protections, and watchdog-safe execution.
