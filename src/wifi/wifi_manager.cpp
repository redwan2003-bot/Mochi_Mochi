#include "wifi_manager.h"
#include "storage/settings_storage.h"
#include <WiFi.h>

static bool s_is_connected = false;
static uint32_t s_last_reconnect_ms = 0;
constexpr uint32_t RECONNECT_INTERVAL_MS = 10000;

void wifi_manager_init() {
    s_is_connected = false;
    const MochiSettings& s = settings_get();

    if (!s.wifi_enabled) {
        Serial.println("[WIFI] Wi-Fi is disabled in settings. Skipping initialization.");
        WiFi.mode(WIFI_OFF);
        return;
    }

    if (strlen(s.wifi_ssid) == 0) {
        Serial.println("[WIFI] No SSID configured. Awaiting provisioning.");
        WiFi.mode(WIFI_OFF);
        return;
    }

    Serial.printf("[WIFI] Initializing Station mode for SSID: '%s'...\n", s.wifi_ssid);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(true); // Enable 802.11 modem sleep for reduced current draw
    WiFi.setHostname("mochi-bot");
    WiFi.begin(s.wifi_ssid, s.wifi_pass);
    s_last_reconnect_ms = millis();
}

void wifi_manager_poll() {
    const MochiSettings& s = settings_get();
    if (!s.wifi_enabled || strlen(s.wifi_ssid) == 0) {
        return;
    }

    wl_status_t status = WiFi.status();

    if (status == WL_CONNECTED) {
        if (!s_is_connected) {
            s_is_connected = true;
            Serial.printf("[WIFI] Connected! IP: %s | RSSI: %d dBm\n",
                          WiFi.localIP().toString().c_str(), WiFi.RSSI());
        }
    } else {
        if (s_is_connected) {
            s_is_connected = false;
            Serial.println("[WIFI] Disconnected from AP.");
        }

        uint32_t now = millis();
        if (now - s_last_reconnect_ms >= RECONNECT_INTERVAL_MS) {
            s_last_reconnect_ms = now;
            Serial.printf("[WIFI] Retrying connection to '%s'...\n", s.wifi_ssid);
            WiFi.disconnect();
            WiFi.begin(s.wifi_ssid, s.wifi_pass);
        }
    }
}

bool wifi_is_connected() {
    return s_is_connected;
}

void wifi_get_ip_str(char* buf, size_t max_len) {
    if (!buf || max_len == 0) return;
    if (!s_is_connected) {
        snprintf(buf, max_len, "No IP");
        return;
    }
    snprintf(buf, max_len, "%s", WiFi.localIP().toString().c_str());
}

int8_t wifi_get_rssi() {
    if (!s_is_connected) return 0;
    return WiFi.RSSI();
}

void wifi_power_down() {
    if (WiFi.getMode() != WIFI_OFF) {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        s_is_connected = false;
        Serial.println("[WIFI] Radio powered down (WIFI_OFF).");
    }
}

void wifi_power_up() {
    if (WiFi.getMode() != WIFI_OFF) {
        return; // Radio is already active
    }
    const MochiSettings& s = settings_get();
    if (s.wifi_enabled && strlen(s.wifi_ssid) > 0) {
        Serial.println("[WIFI] Powering up radio and reconnecting...");
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(true);
        WiFi.begin(s.wifi_ssid, s.wifi_pass);
        s_last_reconnect_ms = millis();
    }
}
