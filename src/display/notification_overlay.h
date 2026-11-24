#pragma once

#include <U8g2lib.h>
#include <Arduino.h>

// ============================================================================
// Mochi Bot V1 - Milestone 5 Notification Overlay
// Displays transient, privacy-preserving notification alerts (sender + preview)
// Automatically dismisses after countdown or upon touch sensor tap.
// ============================================================================

struct NotificationItem {
    char sender[24];
    char message[72];
    uint32_t received_ms;
    uint32_t duration_ms;
    bool is_active;
};

// Shows a transient notification overlay for the specified duration (ms)
void notification_show(const char* sender, const char* message, uint32_t duration_ms = 5000);

// Dismisses the active notification immediately
void notification_dismiss();

// Checks if a notification overlay is currently active and unexpired
bool notification_is_active();

// Renders the notification overlay onto the U8g2 buffer
void notification_render(U8G2& u8g2);
