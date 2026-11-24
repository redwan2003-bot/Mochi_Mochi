#pragma once

#include <Arduino.h>
#include <time.h>

// ============================================================================
// Mochi Bot V1 - Milestone 4 Software RTC & Clock Manager
// Manages system time with POSIX standard time integration,
// timezone offsets, and string formatting for dock glance cards.
// ============================================================================

struct MochiTime {
    uint8_t hour;        // 0 - 23
    uint8_t minute;      // 0 - 59
    uint8_t second;      // 0 - 59
    uint16_t year;       // e.g. 2026
    uint8_t month;       // 1 - 12
    uint8_t day;         // 1 - 31
    uint8_t day_of_week; // 0 = Sunday, 1 = Monday, ...
    bool is_synced;
};

// Initializes internal software RTC
void clock_init();

// Synchronizes clock with external epoch (e.g. from Android BLE companion in M5)
void clock_set_epoch(time_t epoch_seconds, int16_t tz_offset_minutes = 0);

// Returns true if clock has received a valid external synchronization
bool clock_is_synced();

// Populates MochiTime structure with current time
void clock_get_time(MochiTime& out_time);

// Formats time string (e.g. "14:32", "02:32 PM", or "14:32:05")
void clock_format_time(char* buf, size_t max_len, bool format_24h, bool show_seconds = false);

// Formats separated numeric digits and AM/PM suffix (ideal for custom/numeric-only fonts)
void clock_format_time_parts(char* digits_buf, size_t digits_len, char* ampm_buf, size_t ampm_len, bool format_24h);

// Formats date string (e.g. "Sun, Nov 22" or "Nov 22, 2026")
void clock_format_date(char* buf, size_t max_len);

// Formats system uptime string (e.g. "02:15:43")
void clock_format_uptime(char* buf, size_t max_len);
