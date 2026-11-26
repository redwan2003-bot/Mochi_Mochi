# ✨ Mochi Bot (V1) — Hardware Companion & Desk Pet

<div align="center">

```text
       ╭──────────────────────────────────────────────╮
       │   ╭──────────────────────────────────────╮   │
       │   │  ( • ᴗ • )  zzZ     [ 12:45 PM ]      │   │
       │   │  ~ ~ ~ ~ ~          CPU: 42°C  RAM: 38%│   │
       │   ╰──────────────────────────────────────╯   │
       │        [ TTP223 TOUCH ]       [ MIC ]        │
       ╰──────────────────────┬───────────────────────╯
                              │ (15° Tilt Magnetic Dock)
                    ╭─────────┴─────────╮
                    │   RIVER-STONE DOCK│
                    ╰───────────────────╯
```

**An open-source, expressive desktop companion, smart glance display, and portable bag charm.**  
*Powered by ESP32-S3 / C6 SuperMini, 1.3" OLED, 6-Axis IMU, Touch, and Privacy-Safe Acoustic Detection.*

---

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Core%20v6.6+-orange.svg?style=flat-square&logo=platformio)](https://platformio.org/)
[![Framework](https://img.shields.io/badge/Framework-Arduino%20ESP32-blue.svg?style=flat-square&logo=espressif)](https://github.com/espressif/arduino-esp32)
[![Architecture](https://img.shields.io/badge/MCU-ESP32--S3%20%2F%20ESP32--C6-red.svg?style=flat-square&logo=espressif)](https://www.espressif.com/)
[![Display](https://img.shields.io/badge/Display-1.3%22%20OLED%20SH1106-success.svg?style=flat-square)](https://github.com/olikraus/u8g2)
[![Audio](https://img.shields.io/badge/I2S-INMP441%20%2B%20MAX98357A-purple.svg?style=flat-square)](https://www.analog.com/)
[![License](https://img.shields.io/badge/License-MIT-green.svg?style=flat-square)](LICENSE)

[Interactive Wiring Diagram](docs/mochi_wiring_diagram.html) • [3D CAD Enclosure Specs](docs/3D_CAD_AND_ENCLOSURE_SPEC.md) • [Assembly Guide](docs/MECHANICAL_AND_ASSEMBLY_GUIDE.md) • [Changelog](CHANGELOG.md)

</div>

---

## 📖 Table of Contents
- [Overview & Use Cases](#-overview--use-cases)
- [System Architecture Diagram](#-system-architecture-diagram)
- [Hardware & Pinout Matrix](#-hardware--pinout-matrix)
- [Core Features](#-core-features)
- [Interactive Expressions & UI Glance Deck](#-interactive-expressions--ui-glance-deck)
- [Zero-Terminal Windows Desktop App](#-zero-terminal-windows-desktop-app)
- [Web Bluetooth Dashboard](#-web-bluetooth-dashboard)
- [Mechanical & 3D Printing](#-mechanical--3d-printing)
- [Getting Started & Developer Guide](#-getting-started--developer-guide)
- [Privacy Guarantee](#-privacy-guarantee)
- [Daily Project Log](#-daily-project-log)

---

## 🌟 Overview & Use Cases

Mochi Bot is engineered to fulfill three distinct operational roles seamlessly:

1. **Desk Pet & Smart Monitor (Docked Mode)**:
   * Magnetically seated in a minimalist 15° tilted pebble dock.
   * Powered continuously from 5V dock pins while simultaneously recharging the internal 500 mAh LiPo.
   * Cycles through 4 customizable glance cards: **Mochi Sleepy Face**, **Digital Clock**, **Hardware Telemetry**, and **Live PC Metrics (CPU, RAM, GPU, Temp)**.
2. **Interactive Keychain / Bag Charm (Portable Mode)**:
   * Detached from the dock and powered from the internal LiPo cell.
   * Carried using a **zero-protrusion flush lanyard channel** carved through the top-right corner.
   * Sleeps automatically after 30 seconds of inactivity to conserve power; wakes up instantly when nudged (MPU6050 motion interrupt) or petted (TTP223 capacitive touch sensor).
3. **Vehicle Dashboard Companion (Driving Mode)**:
   * Connects to Android over BLE and streams high-frequency GPS ground speed to display an auxiliary digital speedometer.

---

## 📐 System Architecture Diagram

```mermaid
flowchart TD
    subgraph Power["Power Subsystem"]
        DOCK["2-Pin Magnetic Dock (5V / GND)"]
        SW["SPDT Slide Switch"]
        LIPO["1S 3.7V 500mAh LiPo"]
        TP["TP4056 Linear Charger"]
        DIV["10k/20k Voltage Divider"]
        
        DOCK -->|5V Power| TP
        DOCK -->|5V Sense| DIV
        DIV -->|GPIO5 HIGH/LOW| MCU["ESP32-S3 / C6 SuperMini"]
        LIPO -->|BAT+| SW
        SW -->|Switched VCC| TP
        TP -->|Regulated 3.3V / 5V| MCU
    end

    subgraph Sensors["Sensors & Inputs"]
        TOUCH["TTP223 Touch Pad"] -->|GPIO6 (Momentary)| MCU
        MPU["MPU6050 6-Axis IMU"] -->|I2C: SDA=8, SCL=9| MCU
        MPU -->|INT: GPIO4 Motion Wake| MCU
        MIC["INMP441 MEMS Mic"] -->|I2S DIN: GPIO10| MCU
    end

    subgraph Outputs["Displays & Sound"]
        MCU -->|I2C: SDA=8, SCL=9| OLED["1.3'' Monochrome OLED (SH1106)"]
        MCU -->|I2S DOUT: GPIO11| AMP["MAX98357A Class-D Amp"]
        AMP -->|Differential Output| SPK["4cm 8Ω 0.5W Speaker"]
    end

    subgraph Connectivity["Companions & Radios"]
        MCU <-->|BLE 5.0 (Passkey 6-digit)| ANDROID["Android Phone (Notifications, Time, GPS)"]
        MCU <-->|Local Wi-Fi REST API (Port 80)| PC["Windows PC (MochiCompanion Tray App)"]
    end
```

---

## 🔌 Hardware & Pinout Matrix

Visual representation of the **SuperMini** pin mappings:

```text
                     ESP32-S3 / C6 SUPERMINI
                         ╭─────────╮
                [TX] ─── │ 1    16 │ ─── 5V (From TP4056 OUT+)
                [RX] ─── │ 2    15 │ ─── GND
    (I2S WS)   GPIO1 ─── │ 3    14 │ ─── 3V3 (To OLED, Sensors, Mic)
    (I2S BCLK) GPIO2 ─── │ 4    13 │ ─── GPIO13 / GPIO20
               GPIO3 ─── │ 5    12 │ ─── GPIO12 / GPIO19 (I2C SCL on C6)
   (MPU INT)   GPIO4 ─── │ 6    11 │ ─── GPIO11 / GPIO18 (I2S DOUT)
  (Dock Sense) GPIO5 ─── │ 7    10 │ ─── GPIO10 / GPIO15 (I2S DIN)
  (TTP223)     GPIO6 ─── │ 8     9 │ ─── GPIO9  / GPIO14 (I2C SCL on S3)
  (Chrg Stat)  GPIO7 ─── │ 9     8 │ ─── GPIO8  / GPIO8  (I2C SDA on S3)
                         ╰───┬─┬───╯
                             │ │
                        [ USB-C PORT ]
```

| Subsystem | Component | S3 Pin | C6 Pin (Alt) | Function & Electrical Notes |
|:---|:---|:---:|:---:|:---|
| **Display** | 1.3" OLED (SH1106) | `GPIO8 / 9` | `GPIO20 / 19` | Fast 400 kHz I²C, 128×64 resolution, 3.3V |
| **Motion** | MPU6050 (GY-521) | `GPIO8 / 9` | `GPIO20 / 19` | I²C 6-Axis Gyro/Accelerometer (`0x68`) |
| **Motion Wake** | MPU6050 INT | `GPIO4` | `GPIO4` | Active-HIGH threshold motion wake interrupt |
| **Capacitive Touch** | TTP223 Sensor | `GPIO6` | `GPIO6` | Active-HIGH expression cycling & petting trigger |
| **Acoustic Mic** | INMP441 MEMS | `GPIO10` | `GPIO3` | I²S DIN (Left Slot, 24-bit). Clocks: `BCLK=2, WS=1` |
| **Audio Amp** | MAX98357A Amp | `GPIO11` | `GPIO18` | I²S DOUT (Mono Right). Shared clocks: `BCLK=2, WS=1` |
| **Dock Detection** | 2-Pin Magnetic Dock | `GPIO5` | `GPIO5` | 10k/20k divider off dock 5V (HIGH=Docked, LOW=Battery) |
| **Charging Monitor**| TP4056 Pin 7 (CHRG) | `GPIO7` | `GPIO7` | 1N4148 diode cathode to TP4056 LED, internal pull-up |
| **Battery Switch** | SPDT Micro Slide | Battery Line| Battery Line | True physical zero-leakage LiPo cut-off switch |

> [!NOTE]
> **ESP32-C6 SuperMini Strapping Pin Isolation**:
> On the ESP32-C6 SuperMini, GPIO9 is the physical BOOT strapping pin and GPIO8 is connected to the onboard WS2812 RGB LED. The firmware automatically remaps the I2C bus to clean, unstrapped pins **GPIO20 (SDA)** and **GPIO19 (SCL)** when compiling for the C6 target, ensuring 100% boot stability. All system sensor pins (`GPIO4`, `GPIO5`, `GPIO6`, `GPIO7`) and I2S clock lines (`BCLK=2`, `WS=1`) remain completely identical across both targets.

---

## 🎨 Interactive Expressions & UI Glance Deck

Mochi features procedural facial animations and a modular 4-card carousel:

```text
┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
│  ( • ᴗ • ) zzZ  │ │    1 2 : 4 5    │ │ BATT: 4.12V 98% │ │ CPU  42°C [38%] │
│                 │ │  MON, NOV 24    │ │ WIFI: -58 dBm   │ │ RAM  12.4G/32G  │
│  SLEEPY DOCK    │ │  SMART CLOCK    │ │ HARDWARE TELEM  │ │ PC STATS GLANCE │
└─────────────────┘ └─────────────────┘ └─────────────────┘ └─────────────────┘
     Card 1              Card 2              Card 3              Card 4
```

* **Expression Engine**:
  * Natural procedural blinking (3–7 second stochastic interval).
  * Breathing eye shape animation during docked idle.
  * Happy eye smile when top capacitive touch sensor is stroked.
  * Shocked expression with audio chirp on clap detection.
* **Stack-Preserving Overlay**:
  * BLE alerts (Discord, Slack, Outlook, Steam) overlay on screen for 5 seconds.
  * Automatically restores the previous dock card when the alert dismisses.

---

## 🖥️ Zero-Terminal Windows Desktop App

No command line, no Python installation, and no PowerShell scripts are needed for everyday use on Windows:

<div align="center">

| File | Purpose | How to Use |
|:---|:---|:---|
| **`MochiCompanion.exe`** | **Standalone Windows System Tray App** | Double-click to launch. Runs near the clock, streams PC telemetry (CPU %, RAM %, GPU %, CPU Temp), and sends custom notifications. |
| **`Launch_Companion_Silent.vbs`** | **Background Autostart** | Place shortcut in `shell:startup` for invisible background start on boot. |
| **`Open_Web_Dashboard.bat`** | **Web BLE Dashboard Launcher** | Launches Google Chrome / Edge directly to the local Web Bluetooth dashboard. |
| **`Flash_Firmware.bat`** | **1-Click Firmware Flasher** | Verifies safety, compiles, flashes over USB-C, and opens the serial monitor. |

</div>

---

## 🌐 Web Bluetooth Dashboard

Visit the standalone Web Bluetooth companion in `tools/web_ble_companion/index.html`:
* **Zero Installation**: Runs entirely inside Chrome/Edge via standard Web Bluetooth API.
* **Features**:
  * 1-Click Passkey pairing (displays 6-digit numeric code on Mochi's OLED).
  * One-tap PC clock RTC synchronization.
  * Custom notification popup sender with preview.
  * Live GPS driving mode simulator (real-time speedometer gauge).
  * Direct OLED expression tester.

---

## 🖨️ Mechanical & 3D Printing (Bahadur Rev 4.1 Master)

All mechanical parts are scripted in a single, fully parametric, 100% 2-manifold OpenSCAD master model:
* **CAD Spec**: [docs/3D_CAD_AND_ENCLOSURE_SPEC.md](docs/3D_CAD_AND_ENCLOSURE_SPEC.md)
* **Master OpenSCAD Source**: [`cad/bahadur_rev4_1/bahadur_rev4_1.scad`](cad/bahadur_rev4_1/bahadur_rev4_1.scad)
* **Pre-Exported Binary STLs**: [`cad/bahadur_rev4_1/STLs/`](cad/bahadur_rev4_1/STLs/)
* **Audit & Regression Data**: [`cad/bahadur_rev4_1/audit_rev4_1.json`](cad/bahadur_rev4_1/audit_rev4_1.json)
* **Envelope**: $49.00\text{ mm (W)} \times 60.50\text{ mm (H)} \times 19.50 / 24.30\text{ mm (Z)}$ ultra-compact clamshell ($-3.0\text{ mm}$ height reduction from Rev 1).
* **Speaker Datum Lock**: `bot_h - speaker_y = 60.50 - 37.50 = 23.00 mm`, guaranteeing shared zero-error dock compatibility across all swappable desktop docks.
* **Flush Lanyard Channel**: $\varnothing 3.5\text{ mm}$ curved pass-through tunnel with a $3.0\text{ mm}$ solid bridge at the top-right corner ($>15\text{ kg}$ tensile strength).
* **Brand Identity**: Debossed 1:1 Bahadur mustache logo (`cad/bahadur_rev4_1/Brand_Logos/bahadur-mustache-cad-deboss.svg`) and matching 128x64 OLED boot splash (`include/bahadur_boot_splash.h`).

### Rev 4.1 Master 8-Part Solid Verification Matrix (All Watertight 2-Manifold)

| Part | Solid Component | Dimensions ($X \times Y \times Z\text{ mm}$) | Net Volume ($\text{mm}^3$) | Bad Edges | Shells | Watertight | Binary STL Checksum (SHA-256) |
|:---:|:---|:---:|:---:|:---:|:---:|:---:|:---|
| **01** | `Front_shell` | $49.00 \times 60.50 \times 13.90$ | 7,924.55 | **0** | 1 | **TRUE** | `265d6445a55858fc49ee9f1ecdd9c2c6...` |
| **02** | `Rear_shell` | $49.00 \times 60.50 \times 11.60$ | 6,047.11 | **0** | 1 | **TRUE** | `77d3f82cf0b784a0d8299103e67c8702...` |
| **03** | `Acoustic_baffle` | $42.10 \times 42.10 \times 0.60$ | 830.99 | **0** | 1 | **TRUE** | `8b4cbfec7d6a54ad5eb23b090623a31d...` |
| **04** | `Battery_isolation_floor` | $44.60 \times 56.10 \times 1.80$ | 2,059.95 | **0** | 1 | **TRUE** | `13a298be58cb53c69ee0e6df2ff977a4...` |
| **08** | `Flush_switch_collar` | $1.50 \times 3.40 \times 2.80$ | 9.69 | **0** | 1 | **TRUE** | `031e5bb8292f7c00e6dc8ee8cf8e11e3...` |
| **05** | `Dock_A_River_Stone` | $80.00 \times 60.00 \times 25.85$ | 67,899.78 | **0** | 1 | **TRUE** | `68e82ef6a858ff48206d2e960359873a...` |
| **06** | `Dock_B_Ribbon_Loop` | $55.00 \times 58.00 \times 54.00$ | 30,937.11 | **0** | 1 | **TRUE** | `6a43bfef5fce07a2744747eb48624128...` |
| **07** | `Dock_C_Plinth_and_Tray` | $98.00 \times 60.00 \times 30.00$ | 75,353.35 | **0** | 1 | **TRUE** | `1cce2a514d7a7d57a2f111e8a93a0bcf...` |

### Pairwise Zero-Interference Collision Matrix
All assembly and seated interfaces have been audited via discrete voxel regression at $\le 0.25\text{ mm}$ grid resolution:
* `01_Front` vs `02_Rear` (Tongue-and-groove seam): **$0.00\text{ mm}^3$ overlap** (Nominal $0.15\text{ mm}$ perimeter slip fit).
* `01_Front` vs `04_Floor` (Isolation perimeter shelf): **$0.00\text{ mm}^3$ overlap**.
* `02_Rear` vs `04_Floor` (Locating rim seat): **$0.00\text{ mm}^3$ overlap**.
* `02_Rear` vs `03_Baffle` (Acoustic disc seat): **$0.00\text{ mm}^3$ overlap**.
* `01_Front` vs `08_Collar` (Switch port flush bore): **$0.00\text{ mm}^3$ overlap**.
* `Bot Assembly` vs `Dock A (River Stone)`: **$0.00\text{ mm}^3$ interference**.
* `Bot Assembly` vs `Dock B (Ribbon Loop)`: **$0.00\text{ mm}^3$ interference**.
* `Bot Assembly` vs `Dock C (Plinth & Tray)`: **$0.00\text{ mm}^3$ interference**.

---

## 🚀 Getting Started & Developer Guide

### 1. Dual-Target Compilation
The workspace natively compiles for both **ESP32-S3** and **ESP32-C6** SuperMini modules:

```powershell
# Compile both targets simultaneously (dual build verification)
& .venv\Scripts\platformio.exe run

# Or compile a specific target individually:
& .venv\Scripts\platformio.exe run -e esp32-s3-supermini
& .venv\Scripts\platformio.exe run -e esp32-c6-supermini
```

#### Dual-Target Binary Footprints (Verified Clean Build)
| Target Environment | Microcontroller Architecture | RAM Utilization | Flash Utilization | Binary Size |
|:---|:---|:---:|:---:|:---:|
| **`esp32-s3-supermini`** | Xtensa LX7 Dual-Core @ 240 MHz | **17.0%** (55.6 KB / 327.6 KB) | **40.5%** (1.27 MB / 3.14 MB) | `firmware.bin` |
| **`esp32-c6-supermini`** | RISC-V Single-Core @ 160 MHz | **15.9%** (52.2 KB / 327.6 KB) | **47.2%** (1.48 MB / 3.14 MB) | `firmware.bin` |

### 2. Flash via USB-C
```powershell
# S3 Target Flasher:
powershell -ExecutionPolicy Bypass -File tools\flash_and_monitor.ps1

# C6 Target Flasher:
& .venv\Scripts\platformio.exe run -e esp32-c6-supermini -t upload
```

### 3. Flash via Web Browser (No Tools Required)
You can flash the pre-compiled binary [`tools/binaries/mochi_bot_v1_factory_merged.bin`](tools/binaries/mochi_bot_v1_factory_merged.bin) directly using Chrome or Edge at [https://esp.huhn.me](https://esp.huhn.me) (Baud: 921600, Offset: `0x0000`).

---

## 🔒 Privacy Guarantee

* **100% On-Device Acoustic Processing**: The INMP441 microphone acts solely as a transient energy detector.
* **Zero Audio Buffering**: Raw audio samples are evaluated in 10 ms ring frames and discarded immediately.
* **Zero Transmission**: Audio is never recorded, never stored in NVS/Flash, and never transmitted over Wi-Fi, BLE, or serial.
* **Local Network Only**: PC telemetry uses a strictly local HTTP REST server (`192.168.x.x`) secured with a pre-shared bearer token.

---

## 📅 Daily Project Log

| Date | Milestone / Progress | Highlights & Changes |
|:---:|:---|:---|
| **2026-11-21** | **Milestone 0–2** | PlatformIO project setup, I²C bus bringup, 1.3" OLED SH1106 display driver, and procedural expression engine. |
| **2026-11-22** | **Milestone 3–6** | MPU6050 6-axis IMU, INMP441 + MAX98357A half-duplex I²S clap detector, NimBLE passkey security, Wi-Fi REST telemetry server. |
| **2026-11-23** | **Milestone 7 RC1** | TTP223 touch sensor, 2-pin magnetic dock divider, SPDT slide switch, 3D CAD OpenSCAD model, zero-terminal Windows executables. |
| **2026-11-24** | **Dual-Target C6/S3 & Daily Push** | Ported 1,593 files to `E:\MoChi Bot`. Unified toolchain on Arduino 3.3.5 / ESP-IDF 5.x (`pioarduino`). Designed C6 strapping pin protection (I2C remapped to GPIO20/19). Implemented unified I2S channel driver & NimBLE 2.x callbacks. Verified clean dual-target build (S3: 40.5% Flash, C6: 47.2% Flash). Synced daily updates to GitHub remote. |
| **2026-11-27** | **Bahadur Rev 4.1 CAD & Docks** | Engineered 100% 2-manifold OpenSCAD master suite (`cad/bahadur_rev4_1/bahadur_rev4_1.scad`). Compact height reduction ($60.5\text{ mm}$, $-3.0\text{ mm}$). Fixed speaker datum ($23.00\text{ mm}$). Implemented monolithic fast-CSG lofts & ribbon profile extrusion. All 8 binary STLs generated with 0 bad edges, 1 connected component, and watertight geometry. Audited pairwise enclosure interfaces & all 3 swappable docks (River Stone, Ribbon Loop, Plinth & Tray) with verified $0.00\text{ mm}^3$ interference. Added Bahadur 128x64 debossed boot splash header (`include/bahadur_boot_splash.h`). |

---

<div align="center">
Made with ❤️ for Mochi Bot
</div>
