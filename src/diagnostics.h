#pragma once

#include <Arduino.h>
#include <esp_chip_info.h>
#include <esp_system.h>
#include "board_config.h"

// ============================================================================
// Mochi Bot V1 - Milestone 0 Diagnostics Subsystem
// Safe startup, hardware inventory, and runtime environment verification.
// Updated for 2-pin magnetic dock, TP4056 charge status, and TTP223 touch.
// ============================================================================

struct M0HardwareInventory {
    bool mpu6050_found;
    uint8_t mpu6050_addr;
    uint8_t mpu6050_chip_id;
    
    bool oled_found;
    uint8_t oled_addr;

    bool is_docked;
    bool is_charging;
    bool touch_active;
    uint8_t i2c_device_count;
};

// Initializes diagnostic subsystems (GPIO5 divider input, GPIO7 pullup, GPIO6 pulldown, I2C bus)
void diagnostics_init();

// Output banner and firmware metadata
void diagnostics_print_banner();

// Probes and prints silicon revision, core count, flash geometry, and PSRAM
void diagnostics_print_chip_info();

// Decodes and prints previous reset reason (brownout, WDT, panic, power-on, etc.)
void diagnostics_print_reset_reason();

// Prints DRAM, PSRAM (if present), and heap watermarks
void diagnostics_print_memory_stats();

// Executes a non-blocking 7-bit I2C bus scan across GPIO8/GPIO9
uint8_t diagnostics_scan_i2c();

// Probes MPU6050 WHO_AM_I register (0x75) expecting 0x68
bool diagnostics_probe_mpu6050(uint8_t& detected_addr, uint8_t& chip_id);

// Probes OLED address ACK (0x3C / 0x3D)
bool diagnostics_probe_oled(uint8_t& detected_addr);

// Reads dock sense pin (GPIO5 via 10k/20k divider; HIGH = Docked, LOW = Undocked)
bool diagnostics_read_dock_state();

// Reads TP4056 charging status (GPIO7 via diode isolation; LOW = Charging, HIGH = Full/Off)
bool diagnostics_read_charge_state();

// Reads TTP223 touch sensor pin (GPIO6; HIGH = Touched, LOW = Idle)
bool diagnostics_read_touch_state();

// Runs the full inventory sweep and fills the inventory struct
M0HardwareInventory diagnostics_run_inventory();

// String decoders for human-readable logs
const char* diagnostics_flash_mode_str(FlashMode_t mode);
const char* diagnostics_reset_reason_str(esp_reset_reason_t reason);
