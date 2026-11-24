#include "clock_manager.h"
#include <sys/time.h>
#include <cstdio>

static bool s_is_synced = false;
static int16_t s_tz_offset_minutes = 0;

static const char* const DAYS_OF_WEEK[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char* const MONTHS[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void clock_init() {
    s_is_synced = false;
    s_tz_offset_minutes = 0;
    Serial.println("[CLOCK] Software RTC initialized (Awaiting BLE synchronization).");
}

void clock_set_epoch(time_t epoch_seconds, int16_t tz_offset_minutes) {
    s_tz_offset_minutes = tz_offset_minutes;
    // Apply timezone offset to epoch
    time_t local_epoch = epoch_seconds + (tz_offset_minutes * 60);

    struct timeval tv;
    tv.tv_sec = local_epoch;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);

    s_is_synced = true;
    Serial.printf("[CLOCK] Time synchronized! Epoch: %ld, TZ Offset: %d min\n", (long)local_epoch, tz_offset_minutes);
}

bool clock_is_synced() {
    return s_is_synced;
}

void clock_get_time(MochiTime& out_time) {
    time_t now = time(nullptr);
    struct tm ti;
    localtime_r(&now, &ti);

    out_time.hour = ti.tm_hour;
    out_time.minute = ti.tm_min;
    out_time.second = ti.tm_sec;
    out_time.year = ti.tm_year + 1900;
    out_time.month = ti.tm_mon + 1;
    out_time.day = ti.tm_mday;
    out_time.day_of_week = ti.tm_wday;
    out_time.is_synced = s_is_synced;
}

void clock_format_time(char* buf, size_t max_len, bool format_24h, bool show_seconds) {
    if (!buf || max_len == 0) return;

    if (!s_is_synced) {
        // Blink colon on fallback time (12:00) using millis()
        bool blink = (millis() / 500) % 2 == 0;
        snprintf(buf, max_len, "12%c00", blink ? ':' : ' ');
        return;
    }

    MochiTime t;
    clock_get_time(t);

    if (format_24h) {
        if (show_seconds) {
            snprintf(buf, max_len, "%02u:%02u:%02u", t.hour, t.minute, t.second);
        } else {
            snprintf(buf, max_len, "%02u:%02u", t.hour, t.minute);
        }
    } else {
        uint8_t h12 = t.hour % 12;
        if (h12 == 0) h12 = 12;
        const char* ampm = (t.hour >= 12) ? "PM" : "AM";
        if (show_seconds) {
            snprintf(buf, max_len, "%2u:%02u:%02u %s", h12, t.minute, t.second, ampm);
        } else {
            snprintf(buf, max_len, "%2u:%02u %s", h12, t.minute, ampm);
        }
    }
}

void clock_format_time_parts(char* digits_buf, size_t digits_len, char* ampm_buf, size_t ampm_len, bool format_24h) {
    if (digits_buf && digits_len > 0) digits_buf[0] = '\0';
    if (ampm_buf && ampm_len > 0) ampm_buf[0] = '\0';

    if (!s_is_synced) {
        bool blink = (millis() / 500) % 2 == 0;
        if (digits_buf && digits_len > 0) {
            snprintf(digits_buf, digits_len, "12%c00", blink ? ':' : ' ');
        }
        return;
    }

    MochiTime t;
    clock_get_time(t);

    if (format_24h) {
        if (digits_buf && digits_len > 0) {
            snprintf(digits_buf, digits_len, "%02u:%02u", t.hour, t.minute);
        }
    } else {
        uint8_t h12 = t.hour % 12;
        if (h12 == 0) h12 = 12;
        if (digits_buf && digits_len > 0) {
            snprintf(digits_buf, digits_len, "%2u:%02u", h12, t.minute);
        }
        if (ampm_buf && ampm_len > 0) {
            snprintf(ampm_buf, ampm_len, "%s", (t.hour >= 12) ? "PM" : "AM");
        }
    }
}

void clock_format_date(char* buf, size_t max_len) {
    if (!buf || max_len == 0) return;

    if (!s_is_synced) {
        snprintf(buf, max_len, "Ready to Sync");
        return;
    }

    MochiTime t;
    clock_get_time(t);

    const char* dow = (t.day_of_week < 7) ? DAYS_OF_WEEK[t.day_of_week] : "";
    const char* mon = (t.month >= 1 && t.month <= 12) ? MONTHS[t.month - 1] : "";

    snprintf(buf, max_len, "%s, %s %u", dow, mon, t.day);
}

void clock_format_uptime(char* buf, size_t max_len) {
    if (!buf || max_len == 0) return;

    uint32_t total_sec = millis() / 1000;
    uint32_t hours = total_sec / 3600;
    uint32_t mins = (total_sec % 3600) / 60;
    uint32_t secs = total_sec % 60;

    snprintf(buf, max_len, "%02u:%02u:%02u", (unsigned int)hours, (unsigned int)mins, (unsigned int)secs);
}
