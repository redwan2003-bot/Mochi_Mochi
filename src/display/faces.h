#pragma once

#include <U8g2lib.h>
#include "app_state.h"

// ============================================================================
// Mochi Bot V1 - Milestone 1 Facial Expressions & Dock Layouts (128x64)
// ============================================================================

// Draws the dock home screen: sleepy face with breathing animation & compact clock
void faces_draw_dock_home(U8G2& u8g2, const AppContext& ctx);

// Draws the portable active screen: expressive awake face with natural blinking
void faces_draw_portable_active(U8G2& u8g2, const AppContext& ctx);

// Draws face according to current expression state (sleepy, awake, blink, happy)
void faces_draw_expression(U8G2& u8g2, MochiExpression expr, int16_t y_offset = 0);

// Draws real-time vehicle speed and animated road dashboard for Driving Mode
void faces_draw_driving_mode(U8G2& u8g2, uint8_t speed_kmh);
