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
| **Acoustic Mic** | INMP441 MEMS | `GPIO10` | `GPIO18` | I²S DIN (Left Slot, 24-bit). Clocks: `BCLK=2, WS=1` |
| **Audio Amp** | MAX98357A Amp | `GPIO11` | `GPIO15` | I²S DOUT (Mono Right). Shared clocks: `BCLK=2, WS=1` |
| **Dock Detection** | 2-Pin Magnetic Dock | `GPIO5` | `GPIO5` | 10k/20k divider off dock 5V (HIGH=Docked, LOW=Battery) |
| **Charging Monitor**| TP4056 Pin 7 (CHRG) | `GPIO7` | `GPIO7` | 1N4148 diode cathode to TP4056 LED, internal pull-up |
| **Battery Switch** | SPDT Micro Slide | Battery Line| Battery Line | True physical zero-leakage LiPo cut-off switch |

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

## 🖨️ Mechanical & 3D Printing

All mechanical parts are documented and scripted in OpenSCAD:
* **CAD Spec**: [docs/3D_CAD_AND_ENCLOSURE_SPEC.md](docs/3D_CAD_AND_ENCLOSURE_SPEC.md)
* **OpenSCAD Source**: [tools/cad/mochi_bot_enclosure.scad](tools/cad/mochi_bot_enclosure.scad)
* **Envelope**: $49.0\text{ mm (W)} \times 63.5\text{ mm (H)} \times 19.5 / 25.5\text{ mm (Z)}$ stepped clamshell.
* **Flush Lanyard Channel**: $\varnothing 3.5\text{ mm}$ curved pass-through tunnel with a $3.0\text{ mm}$ solid bridge at the top-right corner ($>15\text{ kg}$ tensile strength).
* **Minimalist Docks**:
  1. *River-Stone Pebble*: Organic curved desk pebble ($88 \times 68 \times 26\text{ mm}$, 15° lean).
  2. *Folded Ribbon Loop*: Ultra-minimalist cantilever ribbon ($55 \times 58 \times 54\text{ mm}$, 18° lean).
  3. *Bauhaus Plinth*: Geometric desk block with integrated valet tray ($98 \times 60 \times 30\text{ mm}$, 15° lean).

---

## 🚀 Getting Started & Developer Guide

### 1. Compile Firmware
```powershell
& .venv\Scripts\platformio.exe run
```

### 2. Flash via USB-C
```powershell
powershell -ExecutionPolicy Bypass -File tools\flash_and_monitor.ps1
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
| **2026-11-24** | **Repository Port & Verification** | Project ported to permanent repository, Git initialized, full conversation history preserved, and ESP32-C6 SuperMini feasibility validated. |

---

<div align="center">
Made with ❤️ for Mochi Bot
</div>
