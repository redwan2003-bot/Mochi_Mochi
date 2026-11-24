#pragma once

#include <Arduino.h>

// ============================================================================
// Mochi Bot V1 - Milestone 5 BLE Companion Manager
// Manages Android companion communication via NimBLE:
// - MITM-safe passkey security (displays 6-digit PIN on OLED)
// - Epoch time synchronization to Software RTC
// - Privacy-preserving transient notifications overlay
// - Real-time vehicle speed telemetry for Driving Mode
// - Companion settings read/write synchronization
// ============================================================================

// Initializes NimBLE device, security, services, and begins advertising
void ble_manager_init();

// Non-blocking loop update for BLE timeouts, status, and telemetry notifications
void ble_manager_poll();

// Query whether an Android companion is actively connected
bool ble_is_connected();

// Query whether bot is in Driving Mode (speed > 0 and recent)
bool ble_is_driving_mode();

// Returns current vehicle speed in km/h
uint8_t ble_get_driving_speed();
