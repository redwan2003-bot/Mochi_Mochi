#include "faces.h"
#include "dock_cards.h"
#include <math.h>

void faces_draw_expression(U8G2& u8g2, MochiExpression expr, int16_t y_offset) {
    switch (expr) {
        case EXPR_AWAKE: {
            // Left eye (center x=38, y=34+y_offset)
            u8g2.setDrawColor(1);
            u8g2.drawDisc(38, 34 + y_offset, 14);
            // Right eye (center x=90, y=34+y_offset)
            u8g2.drawDisc(90, 34 + y_offset, 14);

            // Catchlights / Pupil highlights (black cutouts)
            u8g2.setDrawColor(0);
            u8g2.drawDisc(35, 30 + y_offset, 4);
            u8g2.drawDisc(87, 30 + y_offset, 4);
            u8g2.drawDisc(42, 37 + y_offset, 2);
            u8g2.drawDisc(94, 37 + y_offset, 2);

            // Cheerful blush marks under eyes
            u8g2.setDrawColor(1);
            u8g2.drawBox(20, 48 + y_offset, 6, 2);
            u8g2.drawBox(102, 48 + y_offset, 6, 2);
            break;
        }

        case EXPR_BLINK: {
            u8g2.setDrawColor(1);
            // Closed eyes as sleek horizontal slits
            u8g2.drawBox(24, 34 + y_offset, 28, 3);
            u8g2.drawBox(76, 34 + y_offset, 28, 3);
            break;
        }

        case EXPR_SLEEPY: {
            u8g2.setDrawColor(1);
            // Gentle curved downward sleepy eye slits
            u8g2.drawBox(26, 36 + y_offset, 24, 2);
            u8g2.drawBox(28, 38 + y_offset, 20, 2);
            u8g2.drawBox(32, 40 + y_offset, 12, 2);

            u8g2.drawBox(78, 36 + y_offset, 24, 2);
            u8g2.drawBox(80, 38 + y_offset, 20, 2);
            u8g2.drawBox(84, 40 + y_offset, 12, 2);

            // Subtle mouth curve
            u8g2.drawBox(60, 48 + y_offset, 8, 2);
            break;
        }

        case EXPR_HAPPY: {
            u8g2.setDrawColor(1);
            // Inverted upward arcs for joy ( ^  ^ )
            for (int t = 0; t < 3; ++t) {
                u8g2.drawLine(24, 38 + y_offset - t, 38, 26 + y_offset - t);
                u8g2.drawLine(38, 26 + y_offset - t, 52, 38 + y_offset - t);

                u8g2.drawLine(76, 38 + y_offset - t, 90, 26 + y_offset - t);
                u8g2.drawLine(90, 26 + y_offset - t, 104, 38 + y_offset - t);
            }

            // Rosy cheeks
            u8g2.drawDisc(18, 44 + y_offset, 4);
            u8g2.drawDisc(110, 44 + y_offset, 4);

            // Happy open smile
            u8g2.drawBox(58, 46 + y_offset, 12, 2);
            u8g2.drawBox(60, 48 + y_offset, 8, 3);
            break;
        }

        case EXPR_WINK: {
            u8g2.setDrawColor(1);
            // Left eye open
            u8g2.drawDisc(38, 34 + y_offset, 14);
            u8g2.setDrawColor(0);
            u8g2.drawDisc(35, 30 + y_offset, 4);
            u8g2.drawDisc(42, 37 + y_offset, 2);
            u8g2.setDrawColor(1);

            // Right eye winking slit
            for (int t = 0; t < 3; ++t) {
                u8g2.drawLine(76, 38 + y_offset - t, 90, 26 + y_offset - t);
                u8g2.drawLine(90, 26 + y_offset - t, 104, 38 + y_offset - t);
            }
            break;
        }
    }
}

void faces_draw_dock_home(U8G2& u8g2, const AppContext& ctx) {
    if (ctx.current_expression == EXPR_HAPPY) {
        // Render top status header + happy reaction face on clap/event
        u8g2.setFont(u8g2_font_profont10_mr);
        u8g2.setDrawColor(1);
        u8g2.drawFrame(2, 1, 32, 10);
        u8g2.drawStr(6, 9, "DOCK");
        if (ctx.is_charging) {
            u8g2.drawFrame(38, 1, 38, 10);
            u8g2.drawStr(42, 9, "CHRG");
        }
        u8g2.drawLine(0, 13, 127, 13);
        faces_draw_expression(u8g2, ctx.current_expression, 4);
    } else {
        // Delegate to active glance card (Clock, Sleepy Face, or Telemetry)
        dock_cards_render(u8g2, ctx);
    }
}

void faces_draw_portable_active(U8G2& u8g2, const AppContext& ctx) {
    // Portable active face rendered full-screen without header clutter
    faces_draw_expression(u8g2, ctx.current_expression, 0);
}

void faces_draw_driving_mode(U8G2& u8g2, uint8_t speed_kmh) {
    u8g2.setDrawColor(1);

    // Top Header: [DRIVE MODE] and [BLE]
    u8g2.setFont(u8g2_font_profont10_mr);
    u8g2.drawFrame(2, 1, 64, 10);
    u8g2.drawStr(6, 9, "DRIVE MODE");

    u8g2.drawStr(100, 9, "[BLE]");
    u8g2.drawLine(0, 13, 127, 13);

    // Center: Large Speed Display in logisoso24_tn
    char speed_str[8];
    snprintf(speed_str, sizeof(speed_str), "%3u", speed_kmh);

    u8g2.setFont(u8g2_font_logisoso24_tn);
    int16_t sw = u8g2.getStrWidth(speed_str);
    int16_t total_w = sw + 30; // speed digits + spacing + "KM/H"
    int16_t start_x = (128 - total_w) / 2;

    u8g2.drawStr(start_x, 43, speed_str);

    u8g2.setFont(u8g2_font_profont11_mr);
    u8g2.drawStr(start_x + sw + 4, 38, "KM/H");

    // Bottom: Horizon line and animated road dashes
    uint32_t now = millis();
    int anim_offset = 16 - ((now / 40) % 16);
    u8g2.drawLine(0, 50, 127, 50);

    for (int x = -16 + anim_offset; x < 128; x += 16) {
        u8g2.drawLine(x, 58, x + 8, 58);
    }
}
