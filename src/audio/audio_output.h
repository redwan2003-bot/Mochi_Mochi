#pragma once

#include <Arduino.h>

// ============================================================================
// Mochi Bot V1 - Milestone 3 Audio Output & Tone Synthesizer
// Controls audio playback through the MAX98357A Class-D amplifier.
// Implements safe sequential bus switching with the INMP441 microphone
// to guarantee zero acoustic self-triggering and bounded power draw.
// ============================================================================

// Initializes the audio output subsystem
void audio_output_init();

// Synthesizes and plays a cheerful dual-tone chirp response (988 Hz -> 1318 Hz)
// Sequentially switches I2S bus to TX, plays tone, allows DMA drain, and restores RX.
bool audio_play_chirp();

// Returns whether audio playback is currently active
bool audio_is_playing();
