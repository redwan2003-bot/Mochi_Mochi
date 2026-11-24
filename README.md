# Mochi Bot V1 Firmware

Mochi Bot V1 is an open-source, expressive companion bot designed for the **ESP32-S3 SuperMini** microcontroller. Mochi can be carried as a portable bag charm/keychain, docked as a desk pet and smart glance display, or used as an auxiliary dashboard in a vehicle.

---

## Hardware Architecture & Pin Map

Interactive visual wiring diagram: [docs/mochi_wiring_diagram.html](docs/mochi_wiring_diagram.html)

| Subsystem | Hardware Component | ESP32-S3 Pin | Notes |
|---|---|---|---|
| **MCU** | ESP32-S3 SuperMini | — | 240 MHz Dual-Core, 4MB Flash, USB-CDC |
| **Display** | 1.3" Monochrome OLED (SH1106) | GPIO8 (SDA), GPIO9 (SCL) | Fast 400 kHz I²C, 128×64 resolution |
| **Motion** | MPU6050 6-Axis Motion Sensor (GY-521) | GPIO8 (SDA), GPIO9 (SCL), GPIO4 (INT) | +-2g accel, low-power gyro standby, motion wake interrupt |
| **Touch** | TTP223 Capacitive Touch Switch | GPIO6 | Momentary active HIGH with internal pulldown |
| **Acoustic Mic** | INMP441 MEMS Microphone | GPIO2 (BCLK), GPIO1 (WS), GPIO10 (DIN) | Sequential 16 kHz I²S RX clap energy detector |
| **Audio Amp** | MAX98357A Class-D I²S Amp | GPIO2 (BCLK), GPIO1 (WS), GPIO11 (DOUT) | Sequential 16 kHz I²S TX dual-tone chirp |
| **Dock Sense** | 2-Pin Magnetic Dock (+5V, GND) | GPIO5 | 10k/20k voltage divider (HIGH=Docked, LOW=Undocked) |
| **Charge Monitor**| TP4056 Linear Charger | GPIO7 | Cathode of 1N4148 diode (LOW=Charging, HIGH=Standby/Full) |
| **Power Switch** | SPDT Micro Slide Switch | Series w/ BAT+ | Physical battery isolation, cold storage & safe USB flashing |
| **USB CDC** | Native USB D− / D+ | GPIO19 (D−), GPIO20 (D+) | Dedicated serial console / flashing port |
| **Battery** | Protected 1S 3.7V 500 mAh LiPo | B+ / B− Pads | Onboard charger (`BOOST` jumper open ~100 mA) |

---

## Key Features

1. **Procedural Expressions & Glance Cards**:
   - Breathing sleepy face, natural eye blinking, happy touch reaction, curious wink nudge reaction.
   - 4-card dock carousel: **Smart Clock**, **Sleepy Breathing Face**, **Hardware Telemetry**, and **PC Stats**.
2. **Sequential Half-Duplex I²S Audio**:
   - INMP441 and MAX98357A share I²S bit clock and word select pins without bus contention.
   - Clap detector uses 10 ms MAA energy frames with adaptive EMA background noise tracking.
   - **Zero Audio Storage / Transmission**: No audio recordings or transcripts are ever stored or sent over radios.
3. **Android BLE Companion**:
   - Passkey pairing screen with 6-digit numeric security code (`BLE_HS_IO_DISPLAY_ONLY`).
   - Time synchronization, transient notification popups, and GPS Driving Mode speedometer.
4. **Windows Wi-Fi Telemetry & REST API**:
   - Local WebServer on port 80 with `Authorization: Bearer <token>` protection.
   - Streams live CPU %, RAM %, GPU %, and CPU temperature to Dock Card 4 with 15-second stale timeout.
5. **Power Management & Wake**:
   - Inactivity sleep after 30 seconds in portable mode (OLED off, I2S uninstalled, Wi-Fi radio off).
   - Instant wake via TTP223 touch (GPIO6), MPU6050 motion (GPIO4), docking (GPIO5), or BLE alert.

---

## Windows 1-Click Launchers (Zero Terminal & Zero PowerShell Required)

For everyday Windows users, Mochi Bot includes standalone 1-click tools that run directly by double-clicking without opening PowerShell, modifying execution policies, or typing commands:

| Action | 1-Click File to Double-Click | Description |
|---|---|---|
| **Run Desktop Companion** | `MochiCompanion.exe` *(or `Launch_Companion_Silent.vbs`)* | **Standalone GUI & System Tray app** (no Python required!). Sits silently near the Windows clock and streams CPU/RAM/GPU telemetry over Wi-Fi. |
| **Open Web Dashboard** | `Open_Web_Dashboard.bat` | Launches the **Web Bluetooth Companion** in Chrome/Edge. Pair over BLE, control faces, sync PC clock, send notifications, test driving speedometer. |
| **Flash Firmware & Monitor**| `Flash_Firmware.bat` | Built-in safety pre-checks, compiles and flashes ESP32-S3 via USB-C, and immediately opens the live 115200 baud serial monitor. |

---

## Getting Started (Developer CLI)

### 1. Build Firmware
From the project directory:
```powershell
& .venv\Scripts\platformio.exe run
```

### 2. Flash to ESP32-S3
> [!CAUTION]
> Always flash via USB while the bot is **undocked** and the **LiPo battery is disconnected**.

```powershell
powershell -ExecutionPolicy Bypass -File tools\flash_and_monitor.ps1
```

### 3. Windows Telemetry Companion

#### Graphical Tray Companion (Recommended)
Run the desktop GUI & Windows System Tray companion application:
```powershell
python tools\windows_companion_tray.py
```
- Built with standard Python 3 Tkinter (zero heavy Qt dependencies) and system tray integration via `pystray`.
- Real-time CPU %, RAM %, GPU %, and CPU temperature gauges with customizable send toggles.
- Auto-persists target IP, token, and polling interval to `tools\mochi_pc_config.json`.
- Live connection indicator (Online, Reconnecting, Stale watchdog).
- Quick notification popup sender for Discord, Outlook, Steam, etc.
- Uses `psutil` if available with native Windows `ctypes` (`GetSystemTimes`, `GlobalMemoryStatusEx`) zero-dependency fallback.

#### Headless CLI Streaming
Run the lightweight console client:
```powershell
python tools\windows_telemetry_client.py --host <MOCHI_IP> --token mochi_secret_token
```

To send a test notification popup to Mochi's screen via CLI:
```powershell
python tools\windows_companion_tray.py --notify "Discord" "Hey, ready to game?"
```

### 4. Web Bluetooth Companion Dashboard
A zero-dependency, browser-based Web Bluetooth Companion dashboard is available for Chrome, Edge, and Chrome on Android at `tools/web_ble_companion/index.html`.

To run locally with Web Bluetooth enabled:
```powershell
python -m http.server 8080 --directory tools\web_ble_companion
```
Then visit [http://localhost:8080](http://localhost:8080) to pair, sync RTC time, push notifications, stream live GPS speedometer data, adjust device settings, and monitor real-time battery/dock telemetry.

---

## Documentation

- [CHANGELOG.md](CHANGELOG.md): Version history from Milestone 0 through Milestone 7 Release Candidate.
- [docs/HARDWARE_TEST_LOG.md](docs/HARDWARE_TEST_LOG.md): Physical bench test protocols, power measurement steps, and thermal checklist.
