# Mochi Bot V1 - 100% Zero-Terminal & No-Code User Guide

This guide is designed for users who **cannot or do not want to use a command line, terminal, or PowerShell**. Everything here is done using **only your web browser (Chrome or Edge) and standard double-clicks with your mouse**.

---

## Step 1: Flashing the ESP32-S3 Firmware (100% in your Browser)

You do **not** need to install Python, PlatformIO, or any coding software. You can flash Mochi Bot directly inside Google Chrome or Microsoft Edge using **Web Serial**.

### What you need:
* ESP32-S3 SuperMini board.
* USB-C Data Cable (make sure it's a data cable, not power-only).
* The single pre-compiled firmware file: [`tools/binaries/mochi_bot_v1_factory_merged.bin`](../tools/binaries/mochi_bot_v1_factory_merged.bin).

### 3-Minute Flashing Steps:
1. **Safety First**:
   - Ensure the LiPo battery is **disconnected** (or switch is OFF).
   - Ensure Mochi Bot is **undocked** from the magnetic charging cradle.
2. **Connect Hardware**:
   - Plug the ESP32-S3 SuperMini into your computer using the USB-C cable.
   - *(If the board is fresh, hold the small **BOOT** button on the ESP32-S3, press the **RESET** button once, then release **BOOT** to enter programming mode).*
3. **Open the Web Flasher in Chrome or Edge**:
   - Go to: **[https://esp.huhn.me](https://esp.huhn.me)** (or [Adafruit WebSerial ESPTool](https://adafruit.github.io/Adafruit_WebSerial_ESPTool/)).
4. **Flash**:
   - Click the blue **Connect** button on the web page.
   - A browser pop-up will ask you to select your device (e.g., `USB JTAG/serial debug unit` or `CP210x / CH340`). Click **Connect**.
   - In the file selection field, choose:
     - **File**: `mochi_bot_v1_factory_merged.bin`
     - **Offset**: `0x0` *(leave at 0x0 because all bootloaders and partitions are pre-merged!)*
   - Click **Program** (or Flash).
   - Wait 15–20 seconds until it reaches 100%. Done!

---

## Step 2: Interacting with Mochi Bot (Web Bluetooth)

Once Mochi Bot boots up, you can configure and control everything directly from your browser—no app store downloads, no terminal, and no Python.

### How to Open:
* Double-click [`Open_Web_Dashboard.bat`](../Open_Web_Dashboard.bat) in the project folder.
* *(Or directly double-click [`tools/web_ble_companion/index.html`](../tools/web_ble_companion/index.html) to open in Chrome or Edge).*

### Features you can control with your mouse:
* **Connect**: Click **"Connect Mochi"** $\rightarrow$ select `MochiBot-V1` $\rightarrow$ pair with the 6-digit PIN displayed on Mochi's OLED screen.
* **Sync Time**: Click the **"Sync PC Clock"** button to automatically set Mochi's internal clock to your PC's exact time.
* **Expression Picker**: Click buttons to make Mochi Happy, Sleepy, Wink, or Curious.
* **Send Notifications**: Type a message (e.g., "Discord", "Game invite from Alex") and click **"Send to Mochi"**. It immediately displays as a cute notification card!
* **Driving Mode**: Click **"Enable Driving Speedometer"** to test vehicle speed display.
* **Settings**: Drag sliders for OLED Brightness (0–255), Sleep Timeout (10–300s), and Audio Volume.

---

## Step 3: Streaming PC Stats to Mochi (Standalone .EXE)

If you want Mochi to sit on your desk and display your PC's live **CPU %, RAM %, and GPU %**, you don't need to install Python or run any script:

### How to Run:
1. Double-click [`MochiCompanion.exe`](../MochiCompanion.exe) in the project folder.
2. The cute Mochi icon appears in your **Windows Taskbar System Tray** (bottom right near the clock).
3. Right-click or double-click the icon:
   - Enter your Mochi Bot's Wi-Fi IP address.
   - Click **Save**.
4. Mochi's Dock Card 4 will immediately start showing your live PC performance gauges!

---

## Summary of 1-Click Desktop Shortcuts

| Task | File to Double-Click | Terminal Needed? |
|---|---|---|
| **Flash Firmware** | Web Browser: [esp.huhn.me](https://esp.huhn.me) *(using `mochi_bot_v1_factory_merged.bin`)* | **NO** |
| **Control Mochi / Sync Time** | [`Open_Web_Dashboard.bat`](../Open_Web_Dashboard.bat) *(or `index.html`)* | **NO** |
| **Stream PC Stats** | [`MochiCompanion.exe`](../MochiCompanion.exe) | **NO** |
| **Silent Tray Launcher** | [`Launch_Companion_Silent.vbs`](../Launch_Companion_Silent.vbs) | **NO** |
