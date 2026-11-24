#include "notification_overlay.h"
#include <cstring>
#include <cstdio>

static NotificationItem s_item = {"", "", 0, 0, false};

void notification_show(const char* sender, const char* message, uint32_t duration_ms) {
    if (!sender && !message) return;

    snprintf(s_item.sender, sizeof(s_item.sender), "%s", sender ? sender : "Notification");
    snprintf(s_item.message, sizeof(s_item.message), "%s", message ? message : "");
    s_item.received_ms = millis();
    s_item.duration_ms = (duration_ms > 0) ? duration_ms : 5000;
    s_item.is_active = true;

    Serial.printf("[NOTIF] Displaying alert from '%s' (%u bytes)\n", s_item.sender, (unsigned)strlen(s_item.message));
}

void notification_dismiss() {
    if (s_item.is_active) {
        s_item.is_active = false;
        Serial.println("[NOTIF] Notification dismissed.");
    }
}

bool notification_is_active() {
    if (!s_item.is_active) return false;

    if (millis() - s_item.received_ms >= s_item.duration_ms) {
        s_item.is_active = false;
        return false;
    }
    return true;
}

void notification_render(U8G2& u8g2) {
    if (!notification_is_active()) return;

    uint32_t now = millis();
    uint32_t elapsed = now - s_item.received_ms;

    // ------------------------------------------------------------------------
    // Header Bar: [ALERT] <Sender>
    // ------------------------------------------------------------------------
    u8g2.setFont(u8g2_font_profont10_mr);
    u8g2.setDrawColor(1);

    char header_buf[32];
    snprintf(header_buf, sizeof(header_buf), "[ALERT] %s", s_item.sender);
    u8g2.drawStr(2, 9, header_buf);

    u8g2.drawLine(0, 13, 127, 13);

    // ------------------------------------------------------------------------
    // Message Body: Wrapped across up to 3 lines (Y = 25, 37, 49)
    // ------------------------------------------------------------------------
    const char* p = s_item.message;
    size_t total_len = strlen(p);
    size_t offset = 0;
    int line_y = 25;

    for (int line = 0; line < 3 && offset < total_len; ++line) {
        char line_buf[24];
        size_t chunk_len = 0;
        size_t max_chunk = sizeof(line_buf) - 1;

        // Try to break at word boundary if possible
        while (chunk_len < max_chunk && (offset + chunk_len) < total_len) {
            chunk_len++;
        }

        // If not at end of string and not ending on space, backtrack to last space
        if ((offset + chunk_len) < total_len) {
            size_t last_space = chunk_len;
            while (last_space > 0 && p[offset + last_space] != ' ') {
                last_space--;
            }
            if (last_space > 0) {
                chunk_len = last_space;
            }
        }

        strncpy(line_buf, p + offset, chunk_len);
        line_buf[chunk_len] = '\0';

        u8g2.drawStr(2, line_y, line_buf);
        line_y += 12;

        offset += chunk_len;
        while (offset < total_len && p[offset] == ' ') {
            offset++; // Skip leading spaces on next line
        }
    }

    // ------------------------------------------------------------------------
    // Bottom Countdown Progress Bar (Y = 57..62)
    // ------------------------------------------------------------------------
    float progress = 1.0f - ((float)elapsed / (float)s_item.duration_ms);
    if (progress < 0.0f) progress = 0.0f;
    int bar_w = (int)(progress * 124.0f);

    u8g2.drawFrame(1, 57, 126, 5);
    if (bar_w > 0) {
        u8g2.drawBox(2, 58, bar_w, 3);
    }
}
