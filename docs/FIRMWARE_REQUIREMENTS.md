# Firmware architecture and staged build

## Starting stack

- PlatformIO, Espressif Arduino framework, C++.
- Pin exact framework/platform/library versions.
- Verify ESP32-S3 target and selected core's I2S API. Check library compatibility before adding dependencies.
- Use fixed-size buffers and clear ownership.

## Suggested tree

~~~text
platformio.ini
src/
  main.cpp
  board_config.h
  app_state.h/.cpp
  diagnostics.h/.cpp
  display/display.h/.cpp
  display/faces.h
  sensors/motion_mpu6050.h/.cpp
  sensors/clap_detector.h/.cpp
  audio/audio_output.h/.cpp
  connectivity/ble_service.h/.cpp
  connectivity/wifi_service.h/.cpp
  connectivity/protocol.h/.cpp
  storage/settings.h/.cpp
test/native/
~~~

Keep it lean. Avoid layers that only forward calls.

## State/event model

States: BOOT, PORTABLE_ACTIVE, PORTABLE_SLEEP, DOCK_HOME, DOCK_CARD, NOTIFICATION_OVERLAY, ERROR_DEGRADED.

Events: EV_BOOT_COMPLETE, EV_DOCKED, EV_UNDOCKED, EV_MOTION, EV_CLAP, EV_NOTIFICATION, EV_TELEMETRY_UPDATE, EV_LINK_LOST, EV_SETTINGS, EV_TIMEOUT.

ISR only sets a flag/timestamp. Service I2C/display/audio/BLE/Wi-Fi in loop/task context. Avoid blocking waits and unbounded retries. Give each subsystem bounded work.

## Milestones

### M0: Board identity and clean build

Create PlatformIO project. Choose exact board/upload profile after reading board marking. Centralize PDF GPIO map. Add serial diagnostics and safe startup. Record toolchain/build command.

### M1: OLED and dock state

Initialize OLED on GPIO8/9; confirm controller/address. Draw static face/clock placeholder. Debounce GPIO5. Select dock UI vs portable mode.

### M2: MPU6050 Motion Sensor
Probe identity/address; configure low-power INT GPIO4. Test wake, interrupt clear, thresholds, missing/stuck bus.

### M3: Clap/audio

I2S RX on GPIO2/1/10 and energy detector. Tune noise/threshold/duration/cooldown on real mic/enclosure. I2S TX on GPIO2/1/11 for brief sound. Prove sequential RX/TX and self-trigger suppression.

### M4: Settings and clock

Versioned NVS settings and migrations. Dock layout/time format/brightness/card order. Sync time using test messages and verify behavior through sleep/reboot.

### M5: Android BLE

Bonding; status/settings/time/notification/speed messages; payload validation/privacy; reconnect and bounded notification fragmentation.

### M6: Windows Wi-Fi telemetry

User-enabled local network connection, authentication, parser, stale timer, reconnect behavior. Test BLE/Wi-Fi coexistence and disable Wi-Fi when unused.

### M7: Integrated power validation

Measure portable and dock current at idle/radio/display/audio, battery lead current while docked, thermals, charging, audio peaks, wake and runtime. Report only measured conclusions.

## Gemini build rules

- Never implement all milestones in one code dump.
- At each step report files, assumptions, build command/results, bench checks, limitations, and next milestone.
- Fix compile failures before adding features.
- If hardware is unavailable, use mocks/host checks where useful and explicitly leave bench behavior unverified.
- Add CHANGELOG.md and docs/HARDWARE_TEST_LOG.md to the eventual firmware repository.
