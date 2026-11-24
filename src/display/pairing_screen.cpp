#include "pairing_screen.h"
#include <cstdio>

static bool s_is_active = false;
static bool s_is_success = false;
static uint32_t s_passkey = 0;
static uint32_t s_success_start_ms = 0;

void pairing_screen_show(uint32_t passkey) {
    s_passkey = passkey;
    s_is_active = true;
    s_is_success = false;
    Serial.printf("[BLE] Displaying 6-digit pairing code on OLED: %06u\n", (unsigned int)passkey);
}

void pairing_screen_show_success() {
    s_is_success = true;
    s_success_start_ms = millis();
    Serial.println("[BLE] Pairing success! Displaying confirmation on OLED.");
}

void pairing_screen_hide() {
    s_is_active = false;
    s_is_success = false;
}

bool pairing_screen_is_active() {
    if (s_is_success) {
        if (millis() - s_success_start_ms >= 2000) {
            s_is_active = false;
            s_is_success = false;
            return false;
        }
        return true;
    }
    return s_is_active;
}

void pairing_screen_render(U8G2& u8g2) {
    if (!pairing_screen_is_active()) return;

    u8g2.setDrawColor(1);

    if (s_is_success) {
        // Pairing Success Screen
        u8g2.setFont(u8g2_font_profont10_mr);
        u8g2.drawStr(2, 9, "[BLE SECURITY]");
        u8g2.drawLine(0, 13, 127, 13);

        u8g2.setFont(u8g2_font_helvB14_tr);
        const char* msg = "PAIRED!";
        int16_t w = u8g2.getStrWidth(msg);
        u8g2.drawStr((128 - w) / 2, 40, msg);

        u8g2.setFont(u8g2_font_profont11_mr);
        const char* sub = "Companion Linked";
        int16_t sw = u8g2.getStrWidth(sub);
        u8g2.drawStr((128 - sw) / 2, 58, sub);
        return;
    }

    // Active Passkey Screen
    u8g2.setFont(u8g2_font_profont10_mr);
    u8g2.drawStr(2, 9, "[BLE PAIRING]");
    u8g2.drawLine(0, 13, 127, 13);

    // Format passkey with a space between groups of 3 for readability
    char code_str[12];
    snprintf(code_str, sizeof(code_str), "%03u %03u", 
             (unsigned int)(s_passkey / 1000), 
             (unsigned int)(s_passkey % 1000));

    u8g2.setFont(u8g2_font_logisoso24_tn);
    int16_t code_w = u8g2.getStrWidth(code_str);
    int16_t code_x = (128 - code_w) / 2;
    u8g2.drawStr(code_x, 43, code_str);

    u8g2.setFont(u8g2_font_profont10_mr);
    const char* hint = "Enter code on phone";
    int16_t hint_w = u8g2.getStrWidth(hint);
    u8g2.drawStr((128 - hint_w) / 2, 59, hint);
}
