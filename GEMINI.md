# Project instructions for Gemini

You are helping implement Mochi Bot V1 firmware for an ESP32-S3 companion. Read this file and the linked project documents before suggesting code.

## Source of truth

1. The user's confirmed hardware_reference/mochi-bot-s3-wiring-diagram.pdf is authoritative for all physical wiring. Follow it exactly.
2. docs/HARDWARE_AND_PINMAP.md is a transcription for convenience. If it conflicts with the PDF, flag the conflict and follow the PDF pending owner review.
3. docs/PRODUCT_AND_SCOPE.md and docs/FIRMWARE_REQUIREMENTS.md define V1 features.
4. Separate confirmed facts, implementation choices, assumptions, and open decisions.
5. Do not change parts, wiring, power design, privacy behavior, or core UX without clearly explaining why and getting the owner's decision.

## Fixed V1 constraints

- ESP32-S3 SuperMini matching the Nologo-style board in the supplied diagram.
- MPU6050 (GY-521), OLED, INMP441 microphone, MAX98357A amplifier, speaker, 2-pin magnetic dock, and protected 1S 3.7 V 500 mAh LiPo.
- Preserve exact GPIO map. Do not repurpose USB GPIO19/20.
- Use the board's charger as drawn, BOOST open. The dock has MAX40200AUK+ reverse blocking inside the pod.
- Android phone and Windows PC are the target companions.
- The mic detects clap-like sound energy only. Do not record, store, transmit, or recognize speech/audio.
- Clap response is a playful OLED reaction plus a brief local speaker sound.
- Docked, the dock supplies system power and the screen stays visible. Undocked, the system runs on battery and may turn off the display during sleep.
- Default dock home screen is sleepy Mochi with a compact clock. Owner can select built-in clock layouts and glance cards in the companion software.
- There is no battery monitor, charger-status signal, ambient-light sensor, or touchscreen in the confirmed wiring.

## Development rules

- Recommend PlatformIO with Espressif Arduino for the first prototype unless a concrete blocker favors a different stack.
- Before writing code, provide a project tree, dependency/version plan, state model, power plan, milestones, and bench tests.
- Implement only one milestone at a time. Build it before adding the next.
- Keep GPIOs centralized and drivers/services separated by function.
- Use bounded buffers and queues, timeouts, reconnect backoff, safe defaults, and versioned NVS settings.
- Keep the main loop responsive. Sensor ISRs should set a flag/timestamp only.
- Do not introduce libraries without explaining ESP32-S3 compatibility and memory impact.
- Never claim battery percentage, charger completion, precise thermal data, or runtime without an actual measurement.
- Keep notification text, location, Wi-Fi credentials, and tokens out of logs.

## Required workflow

1. Inspect this file, every docs file, and the confirmed PDF.
2. List only unresolved questions that cannot be answered from the kit or actual hardware facts.
3. Produce the implementation plan before code.
4. Implement and compile one milestone.
5. Report changed files, build command/result, physical checks still needed, and next milestone.
6. Continue only when asked, preserving the fixed wiring and privacy rules.

Compilation cannot verify board revision, LiPo polarity, reverse-current behavior, charge current, thermal behavior, I2S signal compatibility, radio coexistence, or runtime. Do not claim physical testing unless measurements are provided.
