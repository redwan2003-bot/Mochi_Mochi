#pragma once

#include <Arduino.h>

// ============================================================================
// Mochi Bot V1 - Milestone 6 Wi-Fi Station Manager
// Manages local network connectivity in Station mode (WIFI_STA):
// - Non-blocking connection and exponential backoff retry state machine
// - Radio power management (modem sleep, radio down in PORTABLE_SLEEP)
// - Telemetry access (IP address, RSSI signal strength)
// ============================================================================

// Initializes Wi-Fi in Station mode using configured credentials
void wifi_manager_init();

// Non-blocking poll for connection maintenance and reconnect timers
void wifi_manager_poll();

// Query whether Wi-Fi is actively connected with valid IP
bool wifi_is_connected();

// Populates buffer with formatted IP address string (e.g. "192.168.1.120")
void wifi_get_ip_str(char* buf, size_t max_len);

// Returns signal strength in dBm (0 if not connected)
int8_t wifi_get_rssi();

// Powers down Wi-Fi radio to conserve battery (e.g. during PORTABLE_SLEEP)
void wifi_power_down();

// Restores Wi-Fi radio and initiates connection (e.g. on wake or dock)
void wifi_power_up();
