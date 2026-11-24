#include "clap_detector.h"
#include "audio/i2s_bus.h"

// ----------------------------------------------------------------------------
// Acoustic Detection Constants & Tuning Parameters
// ----------------------------------------------------------------------------
constexpr size_t FRAME_SAMPLES          = 160;    // 10 ms at 16 kHz
constexpr uint32_t MIN_CLAP_ENERGY      = 2200;   // Minimum MAA spike amplitude
constexpr float SPIKE_RATIO             = 3.5f;   // Ratio of frame energy to noise floor
constexpr uint32_t MIN_CLAP_DURATION_MS = 15;     // Minimum pulse duration
constexpr uint32_t MAX_CLAP_DURATION_MS = 70;     // Maximum pulse duration (rejects speech/coughs)
constexpr uint32_t REFRACTORY_COOLDOWN_MS = 800;  // Post-clap refractory window
constexpr float EMA_ALPHA               = 0.02f;  // Noise floor adaptation rate
constexpr float INITIAL_NOISE_FLOOR     = 150.0f;
constexpr float MIN_NOISE_FLOOR         = 40.0f;
constexpr float MAX_NOISE_FLOOR         = 8000.0f;

enum ClapDetectorState {
    CLAP_STATE_IDLE,
    CLAP_STATE_IN_SPIKE,
    CLAP_STATE_REJECTED,
    CLAP_STATE_COOLDOWN
};

static ClapDetectorState s_state = CLAP_STATE_IDLE;
static float s_noise_floor = INITIAL_NOISE_FLOOR;
static uint32_t s_last_frame_energy = 0;
static uint32_t s_spike_start_ms = 0;
static uint32_t s_peak_energy = 0;
static uint32_t s_cooldown_until_ms = 0;
static uint32_t s_suppressed_until_ms = 0;

void clap_detector_init() {
    s_state = CLAP_STATE_IDLE;
    s_noise_floor = INITIAL_NOISE_FLOOR;
    s_last_frame_energy = 0;
    s_spike_start_ms = 0;
    s_peak_energy = 0;
    s_cooldown_until_ms = 0;
    s_suppressed_until_ms = 0;
    Serial.println("[CLAP] Acoustic Clap Detector initialized (Privacy: zero audio retention).");
}

void clap_detector_suppress(uint32_t duration_ms) {
    uint32_t target_ms = millis() + duration_ms;
    if (target_ms > s_suppressed_until_ms) {
        s_suppressed_until_ms = target_ms;
    }
    s_state = CLAP_STATE_IDLE;
}

float clap_detector_get_noise_floor() {
    return s_noise_floor;
}

uint32_t clap_detector_get_last_energy() {
    return s_last_frame_energy;
}

bool clap_detector_poll() {
    // Only process if I2S bus is actively capturing RX audio
    if (i2s_bus_get_mode() != I2S_BUS_RX_ACTIVE) {
        return false;
    }

    uint32_t now = millis();

    // Check external acoustic suppression window (e.g. during speaker playback)
    if (now < s_suppressed_until_ms) {
        s_state = CLAP_STATE_IDLE;
        return false;
    }

    // Check refractory cooldown
    if (s_state == CLAP_STATE_COOLDOWN) {
        if (now >= s_cooldown_until_ms) {
            s_state = CLAP_STATE_IDLE;
        } else {
            return false;
        }
    }

    // Read 10 ms frame (160 samples of 32-bit words) non-blockingly (timeout 0)
    int32_t sample_buf[FRAME_SAMPLES];
    size_t samples_read = i2s_bus_read_raw(sample_buf, FRAME_SAMPLES, 0);
    if (samples_read < FRAME_SAMPLES) {
        return false; // Not enough samples available yet in DMA ring
    }

    // ------------------------------------------------------------------------
    // Privacy-Preserving Energy Calculation: Mean Absolute Amplitude (MAA)
    // ------------------------------------------------------------------------
    uint64_t sum = 0;
    for (size_t i = 0; i < samples_read; ++i) {
        // INMP441 outputs 24-bit audio in upper bits of 32-bit slot
        int32_t sample = sample_buf[i] >> 14;
        sum += (sample < 0) ? -sample : sample;
    }

    // Zero out memory buffer immediately to prevent any audio data retention
    memset(sample_buf, 0, sizeof(sample_buf));

    uint32_t frame_energy = (uint32_t)(sum / samples_read);
    s_last_frame_energy = frame_energy;

    // ------------------------------------------------------------------------
    // Transient Clap State Machine
    // ------------------------------------------------------------------------
    bool is_spike = (frame_energy >= MIN_CLAP_ENERGY) && 
                    (frame_energy >= (uint32_t)(s_noise_floor * SPIKE_RATIO));

    bool clap_detected = false;

    switch (s_state) {
        case CLAP_STATE_IDLE:
            if (is_spike) {
                s_state = CLAP_STATE_IN_SPIKE;
                s_spike_start_ms = now;
                s_peak_energy = frame_energy;
            } else {
                // Slowly adapt background noise floor estimate during quiet/steady periods
                s_noise_floor = s_noise_floor * (1.0f - EMA_ALPHA) + (float)frame_energy * EMA_ALPHA;
                if (s_noise_floor < MIN_NOISE_FLOOR) s_noise_floor = MIN_NOISE_FLOOR;
                if (s_noise_floor > MAX_NOISE_FLOOR) s_noise_floor = MAX_NOISE_FLOOR;
            }
            break;

        case CLAP_STATE_IN_SPIKE: {
            if (frame_energy > s_peak_energy) {
                s_peak_energy = frame_energy;
            }
            uint32_t duration = now - s_spike_start_ms;

            if (duration > MAX_CLAP_DURATION_MS) {
                // Sustained noise (speech, coughing, fan gust, whistle) -> Reject
                s_state = CLAP_STATE_REJECTED;
            } else if (!is_spike || frame_energy < (uint32_t)(s_noise_floor * 2.0f)) {
                // Pulse ended sharply
                if (duration >= MIN_CLAP_DURATION_MS) {
                    // Valid impulsive transient detected!
                    clap_detected = true;
                    s_state = CLAP_STATE_COOLDOWN;
                    s_cooldown_until_ms = now + REFRACTORY_COOLDOWN_MS;
                    Serial.printf("[CLAP] *CLAP DETECTED* Peak=%u, Floor=%.1f, Dur=%u ms\n",
                                  s_peak_energy, s_noise_floor, duration);
                } else {
                    // Too short or electrical glitch
                    s_state = CLAP_STATE_IDLE;
                }
            }
            break;
        }

        case CLAP_STATE_REJECTED:
            // Wait for sound energy to drop back down to ambient levels
            if (frame_energy < (uint32_t)(s_noise_floor * 2.0f)) {
                s_state = CLAP_STATE_IDLE;
            }
            break;

        case CLAP_STATE_COOLDOWN:
            break;
    }

    return clap_detected;
}
