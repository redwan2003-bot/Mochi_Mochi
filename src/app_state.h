#pragma once

#include <Arduino.h>

// ============================================================================
// Mochi Bot V1 - Milestone 2 Application State Machine with Motion Wake
// ============================================================================

enum SystemState {
    SYS_STATE_BOOT,
    SYS_STATE_PORTABLE_ACTIVE,
    SYS_STATE_PORTABLE_SLEEP,
    SYS_STATE_DOCK_HOME
};

enum MochiExpression {
    EXPR_SLEEPY,
    EXPR_AWAKE,
    EXPR_BLINK,
    EXPR_HAPPY,
    EXPR_WINK
};

struct AppContext {
    SystemState current_state;
    MochiExpression current_expression;
    
    bool is_docked;
    bool is_charging;
    bool is_touched;
    bool motion_reaction_enabled;
    
    uint32_t last_activity_ms;
    uint32_t expression_start_ms;
    uint32_t last_blink_ms;
    
    uint8_t dock_brightness;      // 0 - 255
    uint8_t portable_brightness;  // 0 - 255
    uint8_t dock_card_index;      // Active dock glance card (0: Clock, 1: Sleepy, 2: Telemetry)
    uint32_t last_card_rotate_ms;
};

// State machine lifecycle
void app_state_init();
void app_state_update();

// Event dispatchers
void app_state_on_dock_change(bool docked);
void app_state_on_charge_change(bool charging);
void app_state_on_touch(bool pressed);
void app_state_on_motion(int16_t x, int16_t y, int16_t z);
void app_state_on_clap();
void app_state_wake();
void app_state_sync_settings();

// State query helpers
SystemState app_state_get_current();
MochiExpression app_state_get_expression();
bool app_state_is_display_sleeping();
AppContext& app_state_get_context();
