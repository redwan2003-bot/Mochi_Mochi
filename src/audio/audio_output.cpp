#include "audio_output.h"
#include "i2s_bus.h"
#include "sensors/clap_detector.h"
#include "storage/settings_storage.h"
#include "app_state.h"
#include <cmath>

static bool s_is_playing = false;

constexpr uint32_t SAMPLE_RATE = 16000;
// Peak safe digital amplitude (~ -14.7 dBFS). Capped strictly to protect 
// the 0.25-0.5W 8-ohm thin speaker (RBD-2694) from cone overdrive (CAD Blueprint Page 4).
constexpr int16_t PEAK_SAFE_AMPLITUDE = 6000;

void audio_output_init() {
    s_is_playing = false;
    Serial.println("[AUDIO] Audio Output subsystem initialized.");
}

bool audio_is_playing() {
    return s_is_playing;
}

bool audio_play_chirp() {
    if (s_is_playing || !settings_get().sound_enabled) {
        return false;
    }

    s_is_playing = true;

    // Suppress clap detector for tone duration (200 ms) + acoustic reverberation margin (550 ms)
    clap_detector_suppress(750);

    // 1. Release I2S bus from RX capture mode
    i2s_bus_stop();

    // 2. Initialize I2S bus for TX playback on MAX98357A (GPIO11 DIN)
    if (!i2s_bus_start_tx()) {
        Serial.println("[AUDIO] ERROR: Failed to switch I2S to TX mode.");
        s_is_playing = false;
        if (!app_state_is_display_sleeping()) {
            i2s_bus_start_rx();
        }
        return false;
    }

    // ------------------------------------------------------------------------
    // Tone 1: B5 (988 Hz) for 80 ms (1280 samples = 8 chunks of 160)
    // Tone 2: E6 (1318 Hz) for 120 ms (1920 samples = 12 chunks of 160)
    // Total duration: 200 ms (3200 samples = 20 chunks)
    // ------------------------------------------------------------------------
    constexpr size_t CHUNK_SIZE = 160;
    int16_t chunk_buf[CHUNK_SIZE];

    double phase = 0.0;
    double phase_inc1 = (2.0 * M_PI * 987.77) / SAMPLE_RATE;
    double phase_inc2 = (2.0 * M_PI * 1318.51) / SAMPLE_RATE;

    // Render Tone 1 (8 chunks)
    for (size_t chunk = 0; chunk < 8; ++chunk) {
        for (size_t i = 0; i < CHUNK_SIZE; ++i) {
            size_t sample_idx = chunk * CHUNK_SIZE + i;
            // Smooth attack on first 160 samples (10 ms)
            float env = (sample_idx < 160) ? ((float)sample_idx / 160.0f) : 1.0f;
            chunk_buf[i] = (int16_t)(std::sin(phase) * env * PEAK_SAFE_AMPLITUDE);
            phase += phase_inc1;
            if (phase >= 2.0 * M_PI) phase -= 2.0 * M_PI;
        }
        i2s_bus_write_pcm(chunk_buf, CHUNK_SIZE, 50);
    }

    // Render Tone 2 (12 chunks)
    for (size_t chunk = 0; chunk < 12; ++chunk) {
        for (size_t i = 0; i < CHUNK_SIZE; ++i) {
            size_t sample_idx = chunk * CHUNK_SIZE + i;
            // Smooth decay on last 240 samples (15 ms)
            float env = 1.0f;
            if (sample_idx >= (1920 - 240)) {
                env = (float)(1920 - sample_idx) / 240.0f;
            }
            chunk_buf[i] = (int16_t)(std::sin(phase) * env * PEAK_SAFE_AMPLITUDE);
            phase += phase_inc2;
            if (phase >= 2.0 * M_PI) phase -= 2.0 * M_PI;
        }
        i2s_bus_write_pcm(chunk_buf, CHUNK_SIZE, 50);
    }

    // 3. Allow DMA buffer to fully drain to speaker before stopping clocks
    delay(50);

    // 4. Release TX mode to conserve amplifier power
    i2s_bus_stop();

    s_is_playing = false;
    Serial.println("[AUDIO] Played chirp notification (988 Hz -> 1318 Hz).");

    // 5. Restore RX listening mode if bot is awake
    if (!app_state_is_display_sleeping()) {
        i2s_bus_start_rx();
    }

    return true;
}
