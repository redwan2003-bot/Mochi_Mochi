# Product and V1 scope

## Product vision

Mochi is an expressive companion carried as a keychain/bag charm, docked as a desk buddy, or used as an auxiliary display in an older car. Dock mode is Mochi's home state. It should feel like a calm little desk pet and show useful information at a glance.

## Confirmed V1 platform

- ESP32-S3 SuperMini
- MPU6050 6-axis motion sensor (GY-521)
- Protected 1-cell 3.7 V 300 mAh LiPo
- Existing OLED, INMP441 microphone, MAX98357A amp, speaker, and three-contact dock from the confirmed PDF
- Android phone and Windows PC

## V1 features

1. Android notifications forwarded over BLE with user-controlled allowlist/privacy.
2. Windows CPU, memory, storage, GPU, and temperature telemetry when exposed by that PC, over local Wi-Fi.
3. Optional driving view using Android phone GPS speed over BLE. Mochi is an auxiliary display and does not attach to car wiring.
4. MPU6050 motion wake in portable mode.
5. Clap detection only. A clap causes a visual expression and short sound. No voice assistant, speech recognition, recording, audio storage, or audio transmission.
6. Three-pin pogo dock with MAX40200AUK+ reverse blocker. Dock input is intended to power system operation while the onboard charger charges the battery. Undocked operation uses the battery.
7. Always-visible dock display. Default is sleepy Mochi plus a compact clock. Companion software selects supported layouts and cards.

## Dock experience

- Dock is the ambient home state. It stays visible at owner-set brightness.
- Built-in 128x64 monochrome layouts: sleepy face + time, large digital clock, and compact glance cards.
- Owner can order up to three cards. Candidate cards are Mochi home, PC stats, and phone glance.
- Notification/event overlays return to the selected home screen after a short interval.
- Single clap can prompt a playful response. Optional double clap can cycle cards, disabled until acoustic tests pass.
- Motion may briefly animate/brighten the current screen, configurable off. Acceleration is not presence detection.
- Use preset layouts in V1, not a general screen editor.
- Wiring has no charger-status or battery monitor connection. Show docked state only, not battery percentage or charge completion.
- Monochrome OLED has no red night mode. No light sensor is fitted, so brightness is manual or scheduled in software.

## Out of scope

- Voice commands, wake word, speech recognition, audio recording/storage/upload
- iPhone-specific integration
- OBD-II/CAN or direct car wiring
- Health/medical sensing
- Guaranteed Windows GPU/temperature metrics on every PC
- User-uploaded animation editor
- New electrical parts or changed GPIO wiring without owner's approval

## UX priorities

1. Calm, friendly, expressive default dock face.
2. Useful glance information without overcrowding the small OLED.
3. Explicit privacy controls for notifications, location, and PC telemetry.
4. Clear stale/disconnected data states.
5. Preserve portable battery runtime.
