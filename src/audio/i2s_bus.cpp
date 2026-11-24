#include "i2s_bus.h"

// ============================================================================
// Mochi Bot V1 - Milestone 3 Sequential I2S Bus Implementation
// Shared BCLK and WS between INMP441 and MAX98357A.
// Sequential half-duplex operation guarantees acoustic isolation and
// releases DMA and clocks during low power sleep.
//
// Dual-Target Architecture:
// - ESP-IDF 4.x (Arduino Core 2.x, ESP32-S3): driver/i2s.h legacy API
// - ESP-IDF 5.x (Arduino Core 3.x, ESP32-C6): driver/i2s_std.h channel API
// ============================================================================

static I2SBusMode s_current_mode = I2S_BUS_DISABLED;

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
// ============================================================================
// ESP-IDF 5.x / Arduino Core 3.x Channel-based Driver (ESP32-C6)
// ============================================================================
#include <driver/i2s_std.h>

static i2s_chan_handle_t s_rx_handle = nullptr;
static i2s_chan_handle_t s_tx_handle = nullptr;

bool i2s_bus_start_rx() {
    if (s_current_mode == I2S_BUS_RX_ACTIVE) {
        return true;
    }
    i2s_bus_stop();

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num = 4;
    chan_cfg.dma_frame_num = 160;

    esp_err_t err = i2s_new_channel(&chan_cfg, nullptr, &s_rx_handle);
    if (err != ESP_OK) {
        Serial.printf("[I2S] RX new channel failed: 0x%X\n", err);
        return false;
    }

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = (gpio_num_t)PIN_I2S_BCLK,
            .ws   = (gpio_num_t)PIN_I2S_WS,
            .dout = I2S_GPIO_UNUSED,
            .din  = (gpio_num_t)PIN_I2S_DIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;

    err = i2s_channel_init_std_mode(s_rx_handle, &std_cfg);
    if (err != ESP_OK) {
        Serial.printf("[I2S] RX init std mode failed: 0x%X\n", err);
        i2s_del_channel(s_rx_handle);
        s_rx_handle = nullptr;
        return false;
    }

    err = i2s_channel_enable(s_rx_handle);
    if (err != ESP_OK) {
        Serial.printf("[I2S] RX enable failed: 0x%X\n", err);
        i2s_del_channel(s_rx_handle);
        s_rx_handle = nullptr;
        return false;
    }

    s_current_mode = I2S_BUS_RX_ACTIVE;
    Serial.println("[I2S] Bus initialized in RX mode (IDF5 channel API, 16kHz, 32-bit Left)");
    return true;
}

bool i2s_bus_start_tx() {
    if (s_current_mode == I2S_BUS_TX_ACTIVE) {
        return true;
    }
    i2s_bus_stop();

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.dma_desc_num = 4;
    chan_cfg.dma_frame_num = 160;
    chan_cfg.auto_clear = true;

    esp_err_t err = i2s_new_channel(&chan_cfg, &s_tx_handle, nullptr);
    if (err != ESP_OK) {
        Serial.printf("[I2S] TX new channel failed: 0x%X\n", err);
        return false;
    }

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = (gpio_num_t)PIN_I2S_BCLK,
            .ws   = (gpio_num_t)PIN_I2S_WS,
            .dout = (gpio_num_t)PIN_I2S_DOUT,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;

    err = i2s_channel_init_std_mode(s_tx_handle, &std_cfg);
    if (err != ESP_OK) {
        Serial.printf("[I2S] TX init std mode failed: 0x%X\n", err);
        i2s_del_channel(s_tx_handle);
        s_tx_handle = nullptr;
        return false;
    }

    err = i2s_channel_enable(s_tx_handle);
    if (err != ESP_OK) {
        Serial.printf("[I2S] TX enable failed: 0x%X\n", err);
        i2s_del_channel(s_tx_handle);
        s_tx_handle = nullptr;
        return false;
    }

    s_current_mode = I2S_BUS_TX_ACTIVE;
    Serial.println("[I2S] Bus initialized in TX mode (IDF5 channel API, 16kHz, 16-bit Left)");
    return true;
}

void i2s_bus_stop() {
    if (s_rx_handle) {
        i2s_channel_disable(s_rx_handle);
        i2s_del_channel(s_rx_handle);
        s_rx_handle = nullptr;
    }
    if (s_tx_handle) {
        i2s_channel_disable(s_tx_handle);
        i2s_del_channel(s_tx_handle);
        s_tx_handle = nullptr;
    }
    s_current_mode = I2S_BUS_DISABLED;
}

size_t i2s_bus_read_raw(int32_t* buffer, size_t sample_count, uint32_t timeout_ms) {
    if (s_current_mode != I2S_BUS_RX_ACTIVE || !s_rx_handle || !buffer || sample_count == 0) {
        return 0;
    }
    size_t bytes_read = 0;
    esp_err_t err = i2s_channel_read(s_rx_handle, buffer, sample_count * sizeof(int32_t), &bytes_read, pdMS_TO_TICKS(timeout_ms));
    if (err != ESP_OK) {
        return 0;
    }
    return bytes_read / sizeof(int32_t);
}

size_t i2s_bus_write_pcm(const int16_t* buffer, size_t sample_count, uint32_t timeout_ms) {
    if (s_current_mode != I2S_BUS_TX_ACTIVE || !s_tx_handle || !buffer || sample_count == 0) {
        return 0;
    }
    size_t bytes_written = 0;
    esp_err_t err = i2s_channel_write(s_tx_handle, buffer, sample_count * sizeof(int16_t), &bytes_written, pdMS_TO_TICKS(timeout_ms));
    if (err != ESP_OK) {
        return 0;
    }
    return bytes_written / sizeof(int16_t);
}

#else
// ============================================================================
// ESP-IDF 4.x / Arduino Core 2.x Legacy Driver (ESP32-S3 Baseline)
// ============================================================================
#include <driver/i2s.h>

bool i2s_bus_start_rx() {
    if (s_current_mode == I2S_BUS_RX_ACTIVE) {
        return true;
    }
    i2s_bus_stop();

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 160,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("[I2S] RX driver install failed: 0x%X\n", err);
        return false;
    }

    i2s_pin_config_t pin_config = {
        .bck_io_num = PIN_I2S_BCLK,
        .ws_io_num = PIN_I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PIN_I2S_DIN
    };

    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("[I2S] RX set pin failed: 0x%X\n", err);
        i2s_driver_uninstall(I2S_NUM_0);
        return false;
    }

    s_current_mode = I2S_BUS_RX_ACTIVE;
    Serial.println("[I2S] Bus initialized in RX mode (INMP441, 16kHz, 32-bit Left)");
    return true;
}

bool i2s_bus_start_tx() {
    if (s_current_mode == I2S_BUS_TX_ACTIVE) {
        return true;
    }
    i2s_bus_stop();

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 160,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("[I2S] TX driver install failed: 0x%X\n", err);
        return false;
    }

    i2s_pin_config_t pin_config = {
        .bck_io_num = PIN_I2S_BCLK,
        .ws_io_num = PIN_I2S_WS,
        .data_out_num = PIN_I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("[I2S] TX set pin failed: 0x%X\n", err);
        i2s_driver_uninstall(I2S_NUM_0);
        return false;
    }

    s_current_mode = I2S_BUS_TX_ACTIVE;
    Serial.println("[I2S] Bus initialized in TX mode (MAX98357A, 16kHz, 16-bit Left)");
    return true;
}

void i2s_bus_stop() {
    if (s_current_mode != I2S_BUS_DISABLED) {
        i2s_driver_uninstall(I2S_NUM_0);
        s_current_mode = I2S_BUS_DISABLED;
    }
}

size_t i2s_bus_read_raw(int32_t* buffer, size_t sample_count, uint32_t timeout_ms) {
    if (s_current_mode != I2S_BUS_RX_ACTIVE || !buffer || sample_count == 0) {
        return 0;
    }
    size_t bytes_read = 0;
    esp_err_t err = i2s_read(I2S_NUM_0, buffer, sample_count * sizeof(int32_t), &bytes_read, pdMS_TO_TICKS(timeout_ms));
    if (err != ESP_OK) {
        return 0;
    }
    return bytes_read / sizeof(int32_t);
}

size_t i2s_bus_write_pcm(const int16_t* buffer, size_t sample_count, uint32_t timeout_ms) {
    if (s_current_mode != I2S_BUS_TX_ACTIVE || !buffer || sample_count == 0) {
        return 0;
    }
    size_t bytes_written = 0;
    esp_err_t err = i2s_write(I2S_NUM_0, buffer, sample_count * sizeof(int16_t), &bytes_written, pdMS_TO_TICKS(timeout_ms));
    if (err != ESP_OK) {
        return 0;
    }
    return bytes_written / sizeof(int16_t);
}

#endif

I2SBusMode i2s_bus_get_mode() {
    return s_current_mode;
}
