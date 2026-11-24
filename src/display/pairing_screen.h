#pragma once

#include <U8g2lib.h>
#include <Arduino.h>

// ============================================================================
// Mochi Bot V1 - Milestone 5 BLE Pairing Screen
// Displays 6-digit numeric passkey on the 1.3" OLED for MITM-safe BLE bonding
// ============================================================================

// Displays the 6-digit passkey on screen
void pairing_screen_show(uint32_t passkey);

// Displays transient success message after bonding completion
void pairing_screen_show_success();

// Hides/dismisses the pairing screen
void pairing_screen_hide();

// Checks if pairing screen is currently active
bool pairing_screen_is_active();

// Renders the pairing screen onto the U8g2 buffer
void pairing_screen_render(U8G2& u8g2);
