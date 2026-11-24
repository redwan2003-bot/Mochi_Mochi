# Companion apps and data contract

Android and Windows companion apps are separate products. Implement stable device-side interfaces first.

## Android

- Pair/bond with Mochi over BLE.
- Request Notification Listener access and forward user-allowed notification fields only.
- Configure dock home, clock style, card order, brightness, motion response, clap settings, and privacy.
- Send time over BLE.
- In explicit driving mode only, send phone GPS speed and timestamp. Stop reading/sending when disabled.
- Show link/firmware/device status and update entry point.
- Explain why notification and location permissions are requested. Allowlist apps and hide previews. Do not forward private data while feature is off. Do not log location or message text.

## Windows

- Read CPU, memory, storage usage/capacity, GPU and temperature when exposed by the specific machine.
- Send selected, rate-limited telemetry to Mochi over trusted local Wi-Fi.
- Configure metrics/update rate; show unavailable, stale, or live status; include clear stop-sharing.
- Temperature/GPU provider support varies. Do not fabricate values.

## Transports

- BLE is Android V1 transport.
- Local Wi-Fi is Windows telemetry V1 transport.
- USB serial is development diagnostics/programming only, while undocked and cell disconnected per diagram.
- Keep schema/parser independent of BLE/Wi-Fi framing.
- Exact BLE UUIDs and Wi-Fi port/path are unchosen implementation details. Document before app integration.

## Logical JSON examples

These are logical payloads before BLE fragmentation/network framing.

~~~json
{"version":1,"type":"time_sync","timestamp_ms":1795000000000,"utc_offset_min":360}
{"version":1,"type":"speed","timestamp_ms":1795000000000,"speed_kmh":42.6}
{"version":1,"type":"telemetry","timestamp_ms":1795000000000,"cpu_pct":18.0,"memory_pct":63.0,"gpu_pct":24.0,"cpu_temp_c":54.0,"ssd_used_pct":71.0}
{"version":1,"type":"notification","timestamp_ms":1795000000000,"app":"Messages","title":"Alex","body":"On my way","privacy":"preview"}
{"version":1,"type":"settings","dock_screen":"sleepy_clock","clock_format":"24h","brightness":20,"cards":["home","pc","phone"],"motion_reaction":true}
{"version":1,"type":"status","docked":true,"ble_connected":true,"wifi_connected":true,"sensor_motion":true,"time_synced":true}
~~~

## Validation requirements

- Validate version/type/length/numeric range/timestamp before use.
- Bound payload size. BLE notification chunks include message ID, index, count, and timeout.
- Telemetry includes source and units. Unavailable values are null/omitted, not zero.
- Apply stale timeouts and visibly mark stale.
- Do not log notification body, location, credentials, or pairing token.
- Authenticate local Wi-Fi and protect configuration changes. Do not expose an unauthenticated control server.

Expose firmware version, dock state, sensor health, UI mode, BLE/Wi-Fi link, clock sync, and last error code without secrets.
