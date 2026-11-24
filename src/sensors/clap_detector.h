#pragma once

#include <Arduino.h>

// ============================================================================
// Mochi Bot V1 - Milestone 3 Acoustic Clap Detector
// Privacy-first acoustic energy detector:
// - Evaluates 10 ms Mean Absolute Amplitude (MAA) frames at 16 kHz.
// - Zero audio recording, storage, transcription, or transmission.
// - Raw sample buffer is immediately scrubbed from memory after energy calculation.
// - Adaptive Exponential Moving Average (EMA) background noise floor tracking.
// - Enforces duration bounds (15-60 ms) and refractory cooldown (800 ms).
// ============================================================================

// Initializes the clap detector state and baseline thresholds
void clap_detector_init();

// Non-blocking poll invoked in main loop.
// Returns true when a valid transient clap event is detected.
bool clap_detector_poll();

// Suppresses detection for the specified duration (e.g. during speaker playback & reverb)
void clap_detector_suppress(uint32_t duration_ms);

// Returns current adaptive background noise floor estimate
float clap_detector_get_noise_floor();

// Returns last calculated frame energy (MAA)
uint32_t clap_detector_get_last_energy();
