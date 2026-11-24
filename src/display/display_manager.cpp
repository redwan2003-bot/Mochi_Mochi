#include "display_manager.h"
#include "faces.h"
#include "pairing_screen.h"
#include "notification_overlay.h"
#include "ble/ble_manager.h"
#include "app_state.h"
#include <Wire.h>

// U8g2 Hardware I2C constructor for 1.3" SH1106 (128x64) using full RAM buffer (1024 bytes)
static U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ PIN_I2C_SCL, /* data=*/ PIN_I2C_SDA);

static bool g_display_available = false;
static bool g_display_sleeping = false;
static uint32_t g_last_frame_ms = 0;
constexpr uint32_t FRAME_INTERVAL_MS = 33; // Target ~30 FPS refresh

bool display_init() {
    // 1. Check if OLED acknowledges on the I2C bus before starting driver
    uint8_t candidates[2] = {I2C_ADDR_OLED_DEFAULT, I2C_ADDR_OLED_ALT};
    uint8_t detected_addr = 0;

    for (uint8_t addr : candidates) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            detected_addr = addr;
            break;
        }
    }

    if (detected_addr == 0) {
        Serial.println("[DISPLAY] OLED not detected on I2C bus. Running degraded without display.");
        g_display_available = false;
        return false;
    }

    Serial.printf("[DISPLAY] Initializing 1.3\" SH1106 OLED at 0x%02X...\n", detected_addr);
    
    // U8g2 takes 8-bit I2C address (7-bit shifted left by 1)
    u8g2.setI2CAddress(detected_addr * 2);
    
    // Initialize U8g2
    if (!u8g2.begin()) {
        Serial.println("[DISPLAY] U8g2 begin failed.");
        g_display_available = false;
        return false;
    }

    // Set fast 400 kHz I2C clock for smooth UI updates
    u8g2.setBusClock(400000);

    // Set default initial contrast
    u8g2.setContrast(200);

    g_display_available = true;
    g_display_sleeping = false;

    // Show quick initial greeting frame
    u8g2.clearBuffer();
    faces_draw_expression(u8g2, EXPR_AWAKE, 0);
    u8g2.sendBuffer();

    Serial.println("[DISPLAY] 1.3\" OLED initialized successfully.");
    return true;
}

void display_set_brightness(uint8_t brightness) {
    if (!g_display_available) return;
    u8g2.setContrast(brightness);
}

void display_sleep() {
    if (!g_display_available || g_display_sleeping) return;
    u8g2.setPowerSave(1); // Turns off internal charge pump and display panel
    g_display_sleeping = true;
    Serial.println("[DISPLAY] Panel power save enabled (Display Sleep).");
}

void display_wake() {
    if (!g_display_available || !g_display_sleeping) return;
    u8g2.setPowerSave(0); // Restores panel power
    g_display_sleeping = false;
    Serial.println("[DISPLAY] Panel power save disabled (Display Wake).");
}

void display_render() {
    if (!g_display_available || g_display_sleeping) return;

    uint32_t now = millis();
    if (now - g_last_frame_ms < FRAME_INTERVAL_MS) {
        return; // Frame-rate throttling
    }
    g_last_frame_ms = now;

    AppContext& ctx = app_state_get_context();

    u8g2.clearBuffer();

    // 1. Highest priority: BLE Passkey Pairing Screen
    if (pairing_screen_is_active()) {
        pairing_screen_render(u8g2);
    }
    // 2. Second priority: Transient Notification Overlay
    else if (notification_is_active()) {
        notification_render(u8g2);
    }
    // 3. Third priority: Driving Mode Speed Dashboard (when speed > 0)
    else if (ble_is_driving_mode()) {
        faces_draw_driving_mode(u8g2, ble_get_driving_speed());
    }
    // 4. Base state views (Dock Home or Portable Active)
    else {
        switch (ctx.current_state) {
            case SYS_STATE_DOCK_HOME:
                faces_draw_dock_home(u8g2, ctx);
                break;

            case SYS_STATE_PORTABLE_ACTIVE:
                faces_draw_portable_active(u8g2, ctx);
                break;

            case SYS_STATE_BOOT:
            default:
                faces_draw_expression(u8g2, EXPR_AWAKE, 0);
                break;
        }
    }

    u8g2.sendBuffer();
}

bool display_is_available() {
    return g_display_available;
}

U8G2& display_get_u8g2() {
    return u8g2;
}
