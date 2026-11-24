#include "ble_manager.h"
#include "ble_services.h"
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include "time/clock_manager.h"
#include "storage/settings_storage.h"
#include "display/notification_overlay.h"
#include "display/pairing_screen.h"
#include "display/display_manager.h"
#include "audio/audio_output.h"
#include "app_state.h"

static bool s_is_connected = false;
static uint8_t s_driving_speed_kmh = 0;
static uint32_t s_last_speed_ms = 0;
static NimBLECharacteristic* s_status_char = nullptr;
static uint32_t s_last_status_notify_ms = 0;

constexpr uint32_t DRIVING_MODE_TIMEOUT_MS = 15000; // Return to normal if no speed updates for 15s

// ----------------------------------------------------------------------------
// NimBLE Security Callbacks (MITM-Safe Display Only Passkey Bonding)
// ----------------------------------------------------------------------------
class MochiSecurityCallbacks : public NimBLESecurityCallbacks {
    uint32_t onPassKeyRequest() override {
        uint32_t passkey = esp_random() % 1000000;
        pairing_screen_show(passkey);
        return passkey;
    }

    void onPassKeyNotify(uint32_t pass_key) override {
        pairing_screen_show(pass_key);
    }

    bool onSecurityRequest() override {
        return true;
    }

    void onAuthenticationComplete(ble_gap_conn_desc* desc) override {
        if (desc->sec_state.encrypted) {
            pairing_screen_show_success();
            if (settings_get().sound_enabled) {
                audio_play_chirp();
            }
        } else {
            pairing_screen_hide();
        }
    }

    bool onConfirmPIN(uint32_t pin) override {
        pairing_screen_show(pin);
        return true;
    }
};

// ----------------------------------------------------------------------------
// NimBLE Server Callbacks
// ----------------------------------------------------------------------------
class MochiServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) override {
        s_is_connected = true;
        Serial.printf("[BLE] Android companion connected (Peer: %s)\n",
                      NimBLEAddress(desc->peer_ota_addr).toString().c_str());
        
        // Wake up state machine and display if bot was sleeping
        app_state_wake();
    }

    void onDisconnect(NimBLEServer* pServer) override {
        s_is_connected = false;
        s_driving_speed_kmh = 0;
        Serial.println("[BLE] Companion disconnected. Restarting advertising.");
        NimBLEDevice::startAdvertising();
    }
};

// ----------------------------------------------------------------------------
// Characteristic Callbacks: Time Sync
// ----------------------------------------------------------------------------
class TimeSyncCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::string val = pCharacteristic->getValue();
        if (val.length() > 0 && val[0] == '{') {
            // JSON string format: {"epoch": 1700000000, "tz": 360}
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, val);
            if (!err) {
                uint32_t epoch = doc["epoch"] | 0;
                int16_t tz = doc["tz"] | 0;
                if (epoch > 0) {
                    clock_set_epoch(epoch, tz);
                }
            }
        } else if (val.length() == 6) {
            // Binary 6-byte format: [uint32_t epoch, int16_t tz_offset_min]
            uint32_t epoch = (uint8_t)val[0] | ((uint8_t)val[1] << 8) | ((uint8_t)val[2] << 16) | ((uint8_t)val[3] << 24);
            int16_t tz = (int16_t)((uint8_t)val[4] | ((uint8_t)val[5] << 8));
            clock_set_epoch(epoch, tz);
        }
    }
};

// ----------------------------------------------------------------------------
// Characteristic Callbacks: Notifications
// ----------------------------------------------------------------------------
class NotificationCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::string val = pCharacteristic->getValue();
        if (val.length() > 0) {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, val);
            if (!err) {
                const char* sender = doc["sender"] | "Alert";
                const char* msg = doc["msg"] | "";
                uint32_t dur = doc["dur"] | 5000;
                notification_show(sender, msg, dur);

                // Wake state machine and display so user sees notification
                app_state_wake();
            }
        }
    }
};

// ----------------------------------------------------------------------------
// Characteristic Callbacks: Driving Speed
// ----------------------------------------------------------------------------
class SpeedCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::string val = pCharacteristic->getValue();
        if (val.length() == 1) {
            s_driving_speed_kmh = (uint8_t)val[0];
            s_last_speed_ms = millis();
        } else if (val.length() > 0) {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, val);
            if (!err) {
                s_driving_speed_kmh = doc["speed"] | 0;
                s_last_speed_ms = millis();
            }
        }
    }
};

// ----------------------------------------------------------------------------
// Characteristic Callbacks: Settings Sync
// ----------------------------------------------------------------------------
class SettingsCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic) override {
        MochiSettings& s = settings_get();
        JsonDocument doc;
        doc["dock_brt"] = s.dock_brightness;
        doc["port_brt"] = s.portable_brightness;
        doc["sound"] = s.sound_enabled;
        doc["motion"] = s.motion_reaction_enabled;
        doc["time_24h"] = s.time_format_24h;
        doc["tz"] = s.time_zone_offset_min;
        doc["card_rot"] = s.auto_card_rotate_sec;
        String json;
        serializeJson(doc, json);
        pCharacteristic->setValue(json.c_str());
    }

    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::string val = pCharacteristic->getValue();
        if (val.length() > 0) {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, val);
            if (!err) {
                MochiSettings& s = settings_get();
                if (doc.containsKey("dock_brt")) s.dock_brightness = doc["dock_brt"];
                if (doc.containsKey("port_brt")) s.portable_brightness = doc["port_brt"];
                if (doc.containsKey("sound")) s.sound_enabled = doc["sound"];
                if (doc.containsKey("motion")) s.motion_reaction_enabled = doc["motion"];
                if (doc.containsKey("time_24h")) s.time_format_24h = doc["time_24h"];
                if (doc.containsKey("tz")) s.time_zone_offset_min = doc["tz"];
                if (doc.containsKey("card_rot")) s.auto_card_rotate_sec = doc["card_rot"];
                settings_save();
                app_state_sync_settings();
            }
        }
    }
};

void ble_manager_init() {
    Serial.println("[BLE] Initializing NimBLE companion service (Mochi-Bot)...");

    NimBLEDevice::init("Mochi-Bot");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); // Max TX power for reliable range

    // Security Configuration: MITM-safe Passkey bonding
    NimBLEDevice::setSecurityAuth(true, true, true); // Bonding, MITM, Secure Connections
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY); // Bot displays code, phone inputs it
    NimBLEDevice::setSecurityCallbacks(new MochiSecurityCallbacks());

    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MochiServerCallbacks());

    // Create Mochi Service
    NimBLEService* pService = pServer->createService(MOCHI_SERVICE_UUID);

    // Time Sync Characteristic (Write)
    NimBLECharacteristic* pTimeChar = pService->createCharacteristic(
        MOCHI_CHAR_TIME_SYNC_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    pTimeChar->setCallbacks(new TimeSyncCallbacks());

    // Notification Characteristic (Write)
    NimBLECharacteristic* pNotifChar = pService->createCharacteristic(
        MOCHI_CHAR_NOTIF_UUID,
        NIMBLE_PROPERTY::WRITE
    );
    pNotifChar->setCallbacks(new NotificationCallbacks());

    // Driving Speed Characteristic (Write)
    NimBLECharacteristic* pSpeedChar = pService->createCharacteristic(
        MOCHI_CHAR_SPEED_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    pSpeedChar->setCallbacks(new SpeedCallbacks());

    // Settings Sync Characteristic (Read / Write)
    NimBLECharacteristic* pSettingsChar = pService->createCharacteristic(
        MOCHI_CHAR_SETTINGS_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    pSettingsChar->setCallbacks(new SettingsCallbacks());

    // Status Characteristic (Read / Notify)
    s_status_char = pService->createCharacteristic(
        MOCHI_CHAR_STATUS_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );

    pService->start();

    // Start Advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(MOCHI_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // 7.5ms
    pAdvertising->setMaxPreferred(0x12); // 22.5ms
    NimBLEDevice::startAdvertising();

    Serial.println("[BLE] Advertising started. Waiting for companion connection.");
}

void ble_manager_poll() {
    uint32_t now = millis();

    // Driving mode timeout
    if (s_driving_speed_kmh > 0 && (now - s_last_speed_ms >= DRIVING_MODE_TIMEOUT_MS)) {
        s_driving_speed_kmh = 0;
        Serial.println("[BLE] Driving mode timed out (speed 0). Returning to normal view.");
    }

    // Periodic status notification to connected companion (every 5s)
    if (s_is_connected && s_status_char && (now - s_last_status_notify_ms >= 5000)) {
        s_last_status_notify_ms = now;

        AppContext& ctx = app_state_get_context();
        JsonDocument doc;
        doc["dock"] = ctx.is_docked;
        doc["chrg"] = ctx.is_charging;
        doc["heap"] = ESP.getFreeHeap();
        doc["uptime"] = now / 1000;
        String json;
        serializeJson(doc, json);
        s_status_char->setValue(json.c_str());
        s_status_char->notify();
    }
}

bool ble_is_connected() {
    return s_is_connected;
}

bool ble_is_driving_mode() {
    return (s_driving_speed_kmh > 0 && (millis() - s_last_speed_ms < DRIVING_MODE_TIMEOUT_MS));
}

uint8_t ble_get_driving_speed() {
    return s_driving_speed_kmh;
}
