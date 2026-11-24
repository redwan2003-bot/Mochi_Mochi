#include "settings_storage.h"
#include <Preferences.h>

static Preferences s_prefs;
static MochiSettings s_active_settings = {};
static MochiSettings s_persisted_settings = {};

constexpr const char* NVS_NAMESPACE = "mochi_cfg";
constexpr uint8_t CURRENT_CONFIG_VERSION = 2;

static void set_defaults(MochiSettings& s) {
    s.dock_brightness = 180;
    s.portable_brightness = 255;
    s.sound_enabled = true;
    s.motion_reaction_enabled = true;
    s.time_format_24h = true;
    s.time_zone_offset_min = 0;
    s.auto_card_rotate_sec = 0; // Manual touch cycling by default
    s.wifi_enabled = true;
    strncpy(s.wifi_ssid, "MochiHome", sizeof(s.wifi_ssid));
    s.wifi_ssid[sizeof(s.wifi_ssid) - 1] = '\0';
    strncpy(s.wifi_pass, "", sizeof(s.wifi_pass));
    s.wifi_pass[sizeof(s.wifi_pass) - 1] = '\0';
    strncpy(s.auth_token, "mochi_secret_token", sizeof(s.auth_token));
    s.auth_token[sizeof(s.auth_token) - 1] = '\0';
}

void settings_storage_init() {
    set_defaults(s_active_settings);
    s_persisted_settings = s_active_settings;

    if (!s_prefs.begin(NVS_NAMESPACE, false)) {
        Serial.println("[NVS] WARN: Failed to initialize NVS namespace. Using volatile defaults.");
        return;
    }

    uint8_t stored_version = s_prefs.getUChar("cfg_ver", 0);
    s_prefs.end();

    if (stored_version != CURRENT_CONFIG_VERSION) {
        Serial.printf("[NVS] First boot or config version mismatch (found %u, expected %u). Initializing defaults.\n",
                      stored_version, CURRENT_CONFIG_VERSION);
        settings_reset_to_defaults();
        return;
    }

    if (!s_prefs.begin(NVS_NAMESPACE, true)) { // Reopen in read-only mode to load settings
        Serial.println("[NVS] WARN: Failed to reopen NVS to load settings.");
        return;
    }

    // Load persisted settings
    s_active_settings.dock_brightness = s_prefs.getUChar("dock_brt", s_active_settings.dock_brightness);
    s_active_settings.portable_brightness = s_prefs.getUChar("port_brt", s_active_settings.portable_brightness);
    s_active_settings.sound_enabled = s_prefs.getBool("sound_en", s_active_settings.sound_enabled);
    s_active_settings.motion_reaction_enabled = s_prefs.getBool("motion_en", s_active_settings.motion_reaction_enabled);
    s_active_settings.time_format_24h = s_prefs.getBool("time_24h", s_active_settings.time_format_24h);
    s_active_settings.time_zone_offset_min = s_prefs.getShort("tz_offset", s_active_settings.time_zone_offset_min);
    s_active_settings.auto_card_rotate_sec = s_prefs.getUChar("card_rot", s_active_settings.auto_card_rotate_sec);
    s_active_settings.wifi_enabled = s_prefs.getBool("wifi_en", s_active_settings.wifi_enabled);
    s_prefs.getString("wifi_ssid", s_active_settings.wifi_ssid, sizeof(s_active_settings.wifi_ssid));
    s_prefs.getString("wifi_pass", s_active_settings.wifi_pass, sizeof(s_active_settings.wifi_pass));
    s_prefs.getString("auth_tok", s_active_settings.auth_token, sizeof(s_active_settings.auth_token));

    s_persisted_settings = s_active_settings;
    s_prefs.end();

    Serial.printf("[NVS] Loaded settings: DockBrt=%u, PortBrt=%u, Sound=%s, Motion=%s, 24h=%s, TZ=%d min, WiFi=%s (%s)\n",
                  s_active_settings.dock_brightness,
                  s_active_settings.portable_brightness,
                  s_active_settings.sound_enabled ? "ON" : "OFF",
                  s_active_settings.motion_reaction_enabled ? "ON" : "OFF",
                  s_active_settings.time_format_24h ? "YES" : "NO",
                  s_active_settings.time_zone_offset_min,
                  s_active_settings.wifi_enabled ? "ENABLED" : "DISABLED",
                  s_active_settings.wifi_ssid);
}

MochiSettings& settings_get() {
    return s_active_settings;
}

bool settings_save() {
    if (!s_prefs.begin(NVS_NAMESPACE, false)) {
        Serial.println("[NVS] ERROR: Failed to open NVS for writing.");
        return false;
    }

    bool changed = false;

    if (s_active_settings.dock_brightness != s_persisted_settings.dock_brightness) {
        s_prefs.putUChar("dock_brt", s_active_settings.dock_brightness);
        changed = true;
    }
    if (s_active_settings.portable_brightness != s_persisted_settings.portable_brightness) {
        s_prefs.putUChar("port_brt", s_active_settings.portable_brightness);
        changed = true;
    }
    if (s_active_settings.sound_enabled != s_persisted_settings.sound_enabled) {
        s_prefs.putBool("sound_en", s_active_settings.sound_enabled);
        changed = true;
    }
    if (s_active_settings.motion_reaction_enabled != s_persisted_settings.motion_reaction_enabled) {
        s_prefs.putBool("motion_en", s_active_settings.motion_reaction_enabled);
        changed = true;
    }
    if (s_active_settings.time_format_24h != s_persisted_settings.time_format_24h) {
        s_prefs.putBool("time_24h", s_active_settings.time_format_24h);
        changed = true;
    }
    if (s_active_settings.time_zone_offset_min != s_persisted_settings.time_zone_offset_min) {
        s_prefs.putShort("tz_offset", s_active_settings.time_zone_offset_min);
        changed = true;
    }
    if (s_active_settings.auto_card_rotate_sec != s_persisted_settings.auto_card_rotate_sec) {
        s_prefs.putUChar("card_rot", s_active_settings.auto_card_rotate_sec);
        changed = true;
    }
    if (s_active_settings.wifi_enabled != s_persisted_settings.wifi_enabled) {
        s_prefs.putBool("wifi_en", s_active_settings.wifi_enabled);
        changed = true;
    }
    if (strncmp(s_active_settings.wifi_ssid, s_persisted_settings.wifi_ssid, sizeof(s_active_settings.wifi_ssid)) != 0) {
        s_prefs.putString("wifi_ssid", s_active_settings.wifi_ssid);
        changed = true;
    }
    if (strncmp(s_active_settings.wifi_pass, s_persisted_settings.wifi_pass, sizeof(s_active_settings.wifi_pass)) != 0) {
        s_prefs.putString("wifi_pass", s_active_settings.wifi_pass);
        changed = true;
    }
    if (strncmp(s_active_settings.auth_token, s_persisted_settings.auth_token, sizeof(s_active_settings.auth_token)) != 0) {
        s_prefs.putString("auth_tok", s_active_settings.auth_token);
        changed = true;
    }

    if (changed) {
        s_persisted_settings = s_active_settings;
        Serial.println("[NVS] Settings successfully persisted to flash.");
    }

    s_prefs.end();
    return true;
}

void settings_reset_to_defaults() {
    s_prefs.end(); // Ensure clean state before opening for clear/write
    if (!s_prefs.begin(NVS_NAMESPACE, false)) {
        return;
    }
    s_prefs.clear();
    set_defaults(s_active_settings);
    s_prefs.putUChar("cfg_ver", CURRENT_CONFIG_VERSION);
    s_prefs.putUChar("dock_brt", s_active_settings.dock_brightness);
    s_prefs.putUChar("port_brt", s_active_settings.portable_brightness);
    s_prefs.putBool("sound_en", s_active_settings.sound_enabled);
    s_prefs.putBool("motion_en", s_active_settings.motion_reaction_enabled);
    s_prefs.putBool("time_24h", s_active_settings.time_format_24h);
    s_prefs.putShort("tz_offset", s_active_settings.time_zone_offset_min);
    s_prefs.putUChar("card_rot", s_active_settings.auto_card_rotate_sec);
    s_prefs.putBool("wifi_en", s_active_settings.wifi_enabled);
    s_prefs.putString("wifi_ssid", s_active_settings.wifi_ssid);
    s_prefs.putString("wifi_pass", s_active_settings.wifi_pass);
    s_prefs.putString("auth_tok", s_active_settings.auth_token);

    s_persisted_settings = s_active_settings;
    s_prefs.end();
    Serial.println("[NVS] Factory defaults restored and persisted.");
}
