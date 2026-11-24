#include "app_state.h"
#include "display/display_manager.h"
#include "display/dock_cards.h"
#include "display/notification_overlay.h"
#include "audio/i2s_bus.h"
#include "audio/audio_output.h"
#include "storage/settings_storage.h"
#include "time/clock_manager.h"
#include "wifi/wifi_manager.h"

constexpr uint32_t INACTIVITY_SLEEP_TIMEOUT_MS = 30000; // 30 seconds in portable mode
constexpr uint32_t TOUCH_REACTION_DURATION_MS   = 2000;  // 2 seconds for touch reaction
constexpr uint32_t MOTION_REACTION_DURATION_MS  = 1200;  // 1.2 seconds for nudge reaction
constexpr uint32_t BLINK_INTERVAL_MS           = 4000;  // Blink every 4 seconds
constexpr uint32_t BLINK_DURATION_MS           = 150;   // 150 ms blink duration

static AppContext g_ctx = {};

void app_state_init() {
    settings_storage_init();
    clock_init();

    const MochiSettings& cfg = settings_get();

    g_ctx.current_state = SYS_STATE_BOOT;
    g_ctx.current_expression = EXPR_AWAKE;
    g_ctx.is_docked = false;
    g_ctx.is_charging = false;
    g_ctx.is_touched = false;
    g_ctx.motion_reaction_enabled = cfg.motion_reaction_enabled;
    g_ctx.last_activity_ms = millis();
    g_ctx.expression_start_ms = 0;
    g_ctx.last_blink_ms = millis();
    g_ctx.dock_brightness = cfg.dock_brightness;
    g_ctx.portable_brightness = cfg.portable_brightness;
    g_ctx.dock_card_index = 0;
    g_ctx.last_card_rotate_ms = millis();
}

void app_state_on_dock_change(bool docked) {
    g_ctx.is_docked = docked;
    g_ctx.last_activity_ms = millis();

    if (docked) {
        g_ctx.current_state = SYS_STATE_DOCK_HOME;
        g_ctx.current_expression = EXPR_SLEEPY;
        display_wake();
        display_set_brightness(g_ctx.dock_brightness);
        i2s_bus_start_rx();
        wifi_power_up();
        g_ctx.last_card_rotate_ms = millis();
        Serial.println("[STATE] Switched to DOCK_HOME (Glance Cards Active)");
    } else {
        g_ctx.current_state = SYS_STATE_PORTABLE_ACTIVE;
        g_ctx.current_expression = EXPR_AWAKE;
        display_wake();
        display_set_brightness(g_ctx.portable_brightness);
        i2s_bus_start_rx();
        wifi_power_up();
        Serial.println("[STATE] Switched to PORTABLE_ACTIVE (Expressive Face)");
    }
}

void app_state_on_charge_change(bool charging) {
    g_ctx.is_charging = charging;
    Serial.printf("[STATE] Charger state updated: %s\n", charging ? "CHARGING" : "STANDBY/FULL");
}

void app_state_on_touch(bool pressed) {
    g_ctx.is_touched = pressed;
    uint32_t now = millis();
    g_ctx.last_activity_ms = now;

    if (pressed) {
        // 0. If a notification alert is currently showing, touch dismisses it
        if (notification_is_active()) {
            notification_dismiss();
            return;
        }

        // 1. If waking from sleep in portable mode
        if (g_ctx.current_state == SYS_STATE_PORTABLE_SLEEP) {
            g_ctx.current_state = SYS_STATE_PORTABLE_ACTIVE;
            g_ctx.current_expression = EXPR_AWAKE;
            display_wake();
            i2s_bus_start_rx();
            wifi_power_up();
            Serial.println("[STATE] Woke from PORTABLE_SLEEP via Touch");
            return;
        }

        // 2. If docked in DOCK_HOME, touch cycles through glance cards
        if (g_ctx.current_state == SYS_STATE_DOCK_HOME) {
            dock_cards_next();
            g_ctx.dock_card_index = (uint8_t)dock_cards_get_current();
            g_ctx.last_card_rotate_ms = now;
            return;
        }

        // 3. Otherwise in PORTABLE_ACTIVE, trigger temporary Happy expression reaction
        g_ctx.current_expression = EXPR_HAPPY;
        g_ctx.expression_start_ms = now;
        Serial.println("[STATE] Touch reaction: HAPPY expression");
    }
}

void app_state_on_motion(int16_t x, int16_t y, int16_t z) {
    uint32_t now = millis();
    g_ctx.last_activity_ms = now;

    // 1. Motion Wake from Portable Sleep
    if (g_ctx.current_state == SYS_STATE_PORTABLE_SLEEP) {
        g_ctx.current_state = SYS_STATE_PORTABLE_ACTIVE;
        g_ctx.current_expression = EXPR_AWAKE;
        display_wake();
        i2s_bus_start_rx();
        wifi_power_up();
        Serial.printf("[STATE] Woke from PORTABLE_SLEEP via MPU6050 Motion Wake! Accel: X=%d Y=%d Z=%d mg\n", x, y, z);
        return;
    }

    // 2. Motion Nudge Reaction in Portable Active
    if (g_ctx.current_state == SYS_STATE_PORTABLE_ACTIVE && g_ctx.motion_reaction_enabled) {
        // If not already in an active touch expression, show curious wink reaction
        if (g_ctx.current_expression != EXPR_HAPPY) {
            g_ctx.current_expression = EXPR_WINK;
            g_ctx.expression_start_ms = now;
            Serial.printf("[STATE] Motion nudge reaction (WINK). Accel: X=%d Y=%d Z=%d mg\n", x, y, z);
        }
    } else if (g_ctx.current_state == SYS_STATE_DOCK_HOME && g_ctx.motion_reaction_enabled) {
        // In dock mode, a nudge triggers a gentle sleepy eye blink
        g_ctx.current_expression = EXPR_BLINK;
        g_ctx.last_blink_ms = now;
    }
}

void app_state_on_clap() {
    uint32_t now = millis();
    g_ctx.last_activity_ms = now;

    // Trigger Happy reaction face
    g_ctx.current_expression = EXPR_HAPPY;
    g_ctx.expression_start_ms = now;
    Serial.println("[STATE] Acoustic clap reaction: HAPPY expression + Audio Chirp");

    // Play dual-tone chirp audio response (if sound is enabled)
    if (settings_get().sound_enabled) {
        audio_play_chirp();
    }
}

void app_state_update() {
    uint32_t now = millis();

    // ------------------------------------------------------------------------
    // 1. Temporary Expression Timer (Happy/Wink reactions expire back to base)
    // ------------------------------------------------------------------------
    if (g_ctx.current_expression == EXPR_HAPPY) {
        if (now - g_ctx.expression_start_ms >= TOUCH_REACTION_DURATION_MS) {
            g_ctx.current_expression = (g_ctx.current_state == SYS_STATE_DOCK_HOME) ? EXPR_SLEEPY : EXPR_AWAKE;
        }
    } else if (g_ctx.current_expression == EXPR_WINK) {
        if (now - g_ctx.expression_start_ms >= MOTION_REACTION_DURATION_MS) {
            g_ctx.current_expression = (g_ctx.current_state == SYS_STATE_DOCK_HOME) ? EXPR_SLEEPY : EXPR_AWAKE;
        }
    }

    // ------------------------------------------------------------------------
    // 2. Natural Blinking Logic (Portable Active Mode)
    // ------------------------------------------------------------------------
    if (g_ctx.current_state == SYS_STATE_PORTABLE_ACTIVE && g_ctx.current_expression == EXPR_AWAKE) {
        if (now - g_ctx.last_blink_ms >= BLINK_INTERVAL_MS) {
            g_ctx.current_expression = EXPR_BLINK;
            g_ctx.last_blink_ms = now;
        }
    } else if (g_ctx.current_expression == EXPR_BLINK) {
        if (now - g_ctx.last_blink_ms >= BLINK_DURATION_MS) {
            g_ctx.current_expression = (g_ctx.current_state == SYS_STATE_DOCK_HOME) ? EXPR_SLEEPY : EXPR_AWAKE;
            g_ctx.last_blink_ms = now;
        }
    }

    // ------------------------------------------------------------------------
    // 3. Inactivity Timeout (Portable Sleep Entry)
    // ------------------------------------------------------------------------
    if (g_ctx.current_state == SYS_STATE_PORTABLE_ACTIVE) {
        if (now - g_ctx.last_activity_ms >= INACTIVITY_SLEEP_TIMEOUT_MS) {
            g_ctx.current_state = SYS_STATE_PORTABLE_SLEEP;
            display_sleep();
            i2s_bus_stop();     // Stop audio subsystem to conserve battery in sleep
            wifi_power_down();  // Power down Wi-Fi radio to conserve battery in sleep
            Serial.println("[STATE] Inactivity timeout reached (30s). Entering PORTABLE_SLEEP (OLED OFF, Mic OFF, WiFi OFF, MPU6050 armed).");
        }
    }

    // ------------------------------------------------------------------------
    // 4. Dock Glance Card Auto-Rotation (if configured in settings)
    // ------------------------------------------------------------------------
    if (g_ctx.current_state == SYS_STATE_DOCK_HOME && settings_get().auto_card_rotate_sec > 0) {
        if (now - g_ctx.last_card_rotate_ms >= (settings_get().auto_card_rotate_sec * 1000UL)) {
            dock_cards_next();
            g_ctx.dock_card_index = (uint8_t)dock_cards_get_current();
            g_ctx.last_card_rotate_ms = now;
        }
    }
}

void app_state_wake() {
    uint32_t now = millis();
    g_ctx.last_activity_ms = now;

    if (g_ctx.current_state == SYS_STATE_PORTABLE_SLEEP) {
        g_ctx.current_state = SYS_STATE_PORTABLE_ACTIVE;
        g_ctx.current_expression = EXPR_AWAKE;
        display_wake();
        i2s_bus_start_rx();
        wifi_power_up();
        Serial.println("[STATE] Woke from PORTABLE_SLEEP via BLE/WiFi event");
    } else {
        display_wake();
    }
}

void app_state_sync_settings() {
    const MochiSettings& s = settings_get();
    g_ctx.dock_brightness = s.dock_brightness;
    g_ctx.portable_brightness = s.portable_brightness;
    g_ctx.motion_reaction_enabled = s.motion_reaction_enabled;

    uint8_t target_brt = (g_ctx.current_state == SYS_STATE_DOCK_HOME) ? g_ctx.dock_brightness : g_ctx.portable_brightness;
    display_set_brightness(target_brt);
    Serial.printf("[STATE] Settings synchronized to runtime context. Active brightness: %u\n", target_brt);
}

SystemState app_state_get_current() {
    return g_ctx.current_state;
}

MochiExpression app_state_get_expression() {
    return g_ctx.current_expression;
}

bool app_state_is_display_sleeping() {
    return (g_ctx.current_state == SYS_STATE_PORTABLE_SLEEP);
}

AppContext& app_state_get_context() {
    return g_ctx;
}
