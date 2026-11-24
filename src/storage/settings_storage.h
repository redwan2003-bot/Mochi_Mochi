#pragma once

#include <Arduino.h>

// ============================================================================
// Mochi Bot V1 - Milestone 4 NVS Settings Storage
// Manages non-volatile configuration parameters via ESP32 Preferences/NVS.
// Implements field-level change detection for wear-leveling protection.
// ============================================================================

struct MochiSettings {
    uint8_t dock_brightness;         // 0 - 255 (default: 180)
    uint8_t portable_brightness;     // 0 - 255 (default: 255)
    bool sound_enabled;              // Audio chirp responses (default: true)
    bool motion_reaction_enabled;    // Accelerometer nudge reactions (default: true)
    bool time_format_24h;            // true = 24h, false = 12h (default: true)
    int16_t time_zone_offset_min;    // UTC offset in minutes (default: 0)
    uint8_t auto_card_rotate_sec;    // Dock card auto-rotate (0 = manual touch, default: 0)
    bool wifi_enabled;               // Wi-Fi subsystem enable (default: true)
    char wifi_ssid[33];              // Local Wi-Fi SSID
    char wifi_pass[65];              // Local Wi-Fi Password
    char auth_token[33];             // Bearer token for local REST API
};

// Initializes Preferences and loads settings (or creates defaults if unformatted)
void settings_storage_init();

// Returns reference to active settings
MochiSettings& settings_get();

// Persists modified settings to NVS (only writes fields that actually changed)
bool settings_save();

// Resets settings to factory defaults and persists
void settings_reset_to_defaults();
