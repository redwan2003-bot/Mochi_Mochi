#pragma once

#include <Arduino.h>

// ============================================================================
// Mochi Bot V1 - Milestone 6 REST Server & PC Telemetry Service
// Lightweight local HTTP REST server on port 80:
// - Bearer Token authentication on all protected endpoints
// - POST /api/v1/telemetry: Ingests PC CPU/RAM/GPU/Temp stats
// - GET  /api/v1/status: Reports bot health and telemetry
// - POST /api/v1/notify: Direct LAN push notifications from PC
// - 15-second stale watchdog for PC sleep / disconnect detection
// ============================================================================

struct PCTelemetryData {
    uint8_t cpu_usage;     // 0 - 100 %
    uint8_t ram_usage;     // 0 - 100 %
    uint8_t gpu_usage;     // 0 - 100 %
    uint8_t cpu_temp;      // Celsius (e.g. 52)
    uint32_t last_update_ms;
    bool is_valid;
};

// Initializes the WebServer on port 80 with endpoints and auth header collection
void rest_server_init();

// Non-blocking poll for incoming HTTP client requests and stale watchdog
void rest_server_poll();

// Query whether PC telemetry data is recent and valid (< 15s stale timeout)
bool rest_server_is_pc_telemetry_active();

// Returns reference to active PC telemetry data
const PCTelemetryData& rest_server_get_pc_telemetry();
