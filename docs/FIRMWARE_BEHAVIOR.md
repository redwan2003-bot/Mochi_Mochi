# Firmware behavior requirements

## Boot and diagnostics

- Initialize without long blocking animations that delay sensors/radios.
- Serial diagnostics: firmware version, reset reason, free heap, I2C scan, sensor presence, dock state, and radio state.
- Missing OLED/MPU6050/mic/amp must not cause a boot loop. Show degraded status where possible.
- Bound bus timeouts and keep watchdog-safe service loops.

## Display and dock

- Undocked active: expressive Mochi home. After inactivity, OLED off; arm MPU6050 motion wake.
- Debounce GPIO5 dock transitions.
- Dock mode: powered by dock as designed, always visible, default sleepy face + compact clock.
- Built-in larger clock and glance cards. Persist screen choice, brightness cap, time format, and card order in versioned NVS.
- Android BLE or Windows Wi-Fi syncs time. Maintain while powered and indicate unsynchronized/stale time if uncertain.
- Notifications respect user privacy and return to the selected screen after timeout/dismissal.
- Motion reaction is configurable and rate-limited.
- No unsupported battery/charger status.

## Motion

- Configure MPU6050 I2C and low-power interrupt using a documented driver/register map.
- INT/GPIO4 wakes. ISR only captures a flag/timestamp; sensor servicing happens outside ISR.
- Handle interrupt latching/clear, sensor absence, and false wake events.
- Do not describe acceleration as presence detection.

## Clap and speaker

- I2S RX from GPIO2 BCLK, GPIO1 WS, GPIO10 data. Compute short-window energy/envelope.
- Detector uses adaptive noise floor, configurable threshold, transient duration, and refractory period.
- Detect clap-like energy only. No speech recognition, wake word, audio recording, persistent audio, or audio transport.
- Clap causes OLED reaction and short local sound through I2S TX GPIO2/GPIO1/GPIO11.
- Gate detection during playback and a short post-playback tail to avoid self-trigger.
- Optional double clap cycles a dock card, disabled until false-trigger testing passes.
- Shared clocks require RX and TX to match GPIO2/1. Start with sequential capture/playback and prove on hardware before simultaneous full duplex.
- Use bounded buffers and stop I2S in battery-saving sleep where possible. Measure current before listening in portable sleep.

## Connectivity and data

- Android bonded BLE: settings, clock sync, notifications, optional phone GPS speed.
- Windows local Wi-Fi: selected telemetry and clock sync. Enable receiver only when PC telemetry is enabled.
- BLE/Wi-Fi may coexist, but test coexistence, heap, reconnects, and current.
- Messages are versioned/timestamped, length-limited, range-checked, and reject stale/unknown/malformed data.
- No notification, location, Wi-Fi password, or token in logs.
- Mark disconnected/stale data instead of showing old values as live.

## Power states

- GPIO5 low: dock mode, dock input powers system as designed, charger remains board hardware.
- GPIO5 high: battery portable mode.
- Firmware does not control charge circuit.
- Use deep/light sleep only after proving wake sources and radio behavior.
- Close I2S/Wi-Fi when not used where possible.
- Do not promise runtime before measurement.
