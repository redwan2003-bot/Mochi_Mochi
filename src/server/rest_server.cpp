#include "rest_server.h"
#include "storage/settings_storage.h"
#include "wifi/wifi_manager.h"
#include "time/clock_manager.h"
#include "display/notification_overlay.h"
#include "app_state.h"
#include <WebServer.h>
#include <ArduinoJson.h>

static WebServer s_server(80);
static PCTelemetryData s_pc_data = {0, 0, 0, 0, 0, false};
static bool s_server_started = false;

constexpr uint32_t PC_TELEMETRY_STALE_TIMEOUT_MS = 15000;

static bool authenticate_request() {
    const MochiSettings& s = settings_get();

    // If no auth token is set (empty string), allow requests
    if (strlen(s.auth_token) == 0) {
        return true;
    }

    String token = "";
    if (s_server.hasHeader("Authorization")) {
        String auth_hdr = s_server.header("Authorization");
        if (auth_hdr.startsWith("Bearer ")) {
            token = auth_hdr.substring(7);
            token.trim();
        }
    } else if (s_server.hasArg("token")) {
        token = s_server.arg("token");
        token.trim();
    }

    if (token.equals(s.auth_token)) {
        return true;
    }

    s_server.send(401, "application/json", "{\"error\":\"unauthorized\",\"message\":\"Invalid or missing Bearer token\"}");
    return false;
}

static void handle_post_telemetry() {
    if (!authenticate_request()) return;

    if (!s_server.hasArg("plain")) {
        s_server.send(400, "application/json", "{\"error\":\"missing_body\"}");
        return;
    }

    String body = s_server.arg("plain");
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);

    if (err) {
        s_server.send(400, "application/json", "{\"error\":\"invalid_json\"}");
        return;
    }

    s_pc_data.cpu_usage = constrain((int)(doc["cpu_usage"] | 0), 0, 100);
    s_pc_data.ram_usage = constrain((int)(doc["ram_usage"] | 0), 0, 100);
    s_pc_data.gpu_usage = constrain((int)(doc["gpu_usage"] | 0), 0, 100);
    s_pc_data.cpu_temp  = constrain((int)(doc["cpu_temp"] | 0), 0, 125);
    s_pc_data.last_update_ms = millis();
    s_pc_data.is_valid = true;

    s_server.send(200, "application/json", "{\"status\":\"ok\"}");
}

static void handle_get_status() {
    if (!authenticate_request()) return;

    AppContext& ctx = app_state_get_context();
    char time_str[16];
    clock_format_time(time_str, sizeof(time_str), true, true);

    JsonDocument doc;
    doc["device"] = "Mochi-Bot";
    doc["docked"] = ctx.is_docked;
    doc["charging"] = ctx.is_charging;
    doc["free_heap"] = ESP.getFreeHeap();
    doc["uptime_sec"] = millis() / 1000;
    doc["wifi_rssi"] = wifi_get_rssi();
    doc["time"] = time_str;

    String resp;
    serializeJson(doc, resp);
    s_server.send(200, "application/json", resp);
}

static void handle_post_notify() {
    if (!authenticate_request()) return;

    if (!s_server.hasArg("plain")) {
        s_server.send(400, "application/json", "{\"error\":\"missing_body\"}");
        return;
    }

    String body = s_server.arg("plain");
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);

    if (err) {
        s_server.send(400, "application/json", "{\"error\":\"invalid_json\"}");
        return;
    }

    const char* sender = doc["sender"] | "Windows PC";
    const char* msg = doc["msg"] | "";
    uint32_t dur = doc["dur"] | 5000;

    notification_show(sender, msg, dur);
    app_state_wake();

    s_server.send(200, "application/json", "{\"status\":\"ok\"}");
}

static void handle_not_found() {
    s_server.send(404, "application/json", "{\"error\":\"not_found\"}");
}

void rest_server_init() {
    // Collect Authorization header for Bearer token validation
    const char* header_keys[] = {"Authorization"};
    s_server.collectHeaders(header_keys, 1);

    // Register endpoints
    s_server.on("/api/v1/telemetry", HTTP_POST, handle_post_telemetry);
    s_server.on("/api/v1/status", HTTP_GET, handle_get_status);
    s_server.on("/api/v1/notify", HTTP_POST, handle_post_notify);
    s_server.onNotFound(handle_not_found);

    s_server.begin();
    s_server_started = true;
    Serial.println("[HTTP] REST API Server started on port 80 (Bearer Auth enabled).");
}

void rest_server_poll() {
    if (!s_server_started) return;

    if (wifi_is_connected()) {
        s_server.handleClient();
    }

    // Check 15-second stale watchdog for PC telemetry
    if (s_pc_data.is_valid && (millis() - s_pc_data.last_update_ms >= PC_TELEMETRY_STALE_TIMEOUT_MS)) {
        s_pc_data.is_valid = false;
        Serial.println("[HTTP] PC telemetry stale (>15s). Marked offline / sleep.");
    }
}

bool rest_server_is_pc_telemetry_active() {
    if (!s_pc_data.is_valid) return false;
    return (millis() - s_pc_data.last_update_ms < PC_TELEMETRY_STALE_TIMEOUT_MS);
}

const PCTelemetryData& rest_server_get_pc_telemetry() {
    return s_pc_data;
}
