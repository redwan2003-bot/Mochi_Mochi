#pragma once

#include <Arduino.h>
#include <driver/i2s.h>
#include "board_config.h"

// ============================================================================
// Mochi Bot V1 - Milestone 3 Sequential I2S Bus Arbiter
// Manages shared clocks: GPIO2 (BCLK) and GPIO1 (WS/LRC)
// Sequential switching: RX (INMP441 GPIO10) vs TX (MAX98357A GPIO11)
// ============================================================================

enum I2SBusMode {
    I2S_BUS_DISABLED,
    I2S_BUS_RX_ACTIVE,
    I2S_BUS_TX_ACTIVE
};

// Initializes the I2S driver in RX capture mode (16 kHz, 32-bit slot, Left channel)
bool i2s_bus_start_rx();

// Initializes the I2S driver in TX playback mode (16 kHz, 16-bit, Left channel)
bool i2s_bus_start_tx();

// Completely stops and uninstalls the I2S driver to release clocks and DMA
void i2s_bus_stop();

// Reads raw 32-bit audio samples from INMP441 via DMA
// sample_count: number of 32-bit sample words to read
// Returns actual number of samples read
size_t i2s_bus_read_raw(int32_t* buffer, size_t sample_count, uint32_t timeout_ms = 25);

// Writes 16-bit PCM audio samples to MAX98357A via DMA
// Returns actual number of samples written
size_t i2s_bus_write_pcm(const int16_t* buffer, size_t sample_count, uint32_t timeout_ms = 100);

// Returns current active bus mode
I2SBusMode i2s_bus_get_mode();
