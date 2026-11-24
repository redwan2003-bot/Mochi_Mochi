#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include "board_config.h"

// ============================================================================
// Mochi Bot V1 - Milestone 1 Display Manager (1.3" Monochrome I2C OLED)
// ============================================================================

// Initializes U8g2 driver for SH1106 / SSD1306 (128x64) with non-blocking probe
bool display_init();

// Main rendering dispatch (renders based on current app state & expressions)
void display_render();

// Sets display contrast / brightness level (0 - 255)
void display_set_brightness(uint8_t brightness);

// Powers down OLED (DCDC converter off) for battery sleep
void display_sleep();

// Restores OLED power and re-arms display
void display_wake();

// Returns true if physical display acknowledged during init
bool display_is_available();

// Direct access to U8g2 instance for custom drawing
U8G2& display_get_u8g2();
