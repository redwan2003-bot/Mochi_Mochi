#include "dock_cards.h"
#include "faces.h"
#include "time/clock_manager.h"
#include "storage/settings_storage.h"
#include "sensors/clap_detector.h"
#include "server/rest_server.h"
#include "wifi/wifi_manager.h"
#include <cmath>

static DockCardType s_active_card = DOCK_CARD_CLOCK;

void dock_cards_next() {
    s_active_card = (DockCardType)((s_active_card + 1) % DOCK_CARD_COUNT);
    Serial.printf("[DOCK] Switched to glance card %d\n", s_active_card);
}

void dock_cards_set(DockCardType card) {
    if (card < DOCK_CARD_COUNT) {
        s_active_card = card;
    }
}

DockCardType dock_cards_get_current() {
    return s_active_card;
}

static void draw_card_indicator(U8G2& u8g2, uint8_t active_idx) {
    // Renders 4 small indicator dots at top-right (X: 104..124, Y: 4..8)
    constexpr int start_x = 104;
    constexpr int y = 6;
    for (int i = 0; i < DOCK_CARD_COUNT; ++i) {
        int cx = start_x + (i * 6);
        if (i == active_idx) {
            u8g2.drawDisc(cx, y, 2);
        } else {
            u8g2.drawCircle(cx, y, 2);
        }
    }
}

static void render_clock_card(U8G2& u8g2, const AppContext& ctx) {
    // ------------------------------------------------------------------------
    // Top Status Header (Height 12 px)
    // ------------------------------------------------------------------------
    u8g2.setFont(u8g2_font_profont10_mr);
    u8g2.setDrawColor(1);

    // Left: Dock indicator icon
    u8g2.drawFrame(2, 1, 32, 10);
    u8g2.drawStr(6, 9, "DOCK");

    // Center-Left: Charging indicator if active
    if (ctx.is_charging) {
        u8g2.drawFrame(38, 1, 38, 10);
        u8g2.drawStr(42, 9, "CHRG");
    }

    // Top-Right: Card indicator dots
    draw_card_indicator(u8g2, 0);

    // Divider line
    u8g2.drawLine(0, 13, 127, 13);

    // ------------------------------------------------------------------------
    // Big Digital Clock (Center, Y=14..48)
    // ------------------------------------------------------------------------
    char digits_str[12];
    char ampm_str[6];
    bool is_24h = settings_get().time_format_24h;
    clock_format_time_parts(digits_str, sizeof(digits_str), ampm_str, sizeof(ampm_str), is_24h);

    u8g2.setFont(u8g2_font_logisoso24_tn);
    int16_t time_w = u8g2.getStrWidth(digits_str);
    int16_t ampm_w = (ampm_str[0] != '\0') ? 16 : 0;
    int16_t total_w = time_w + (ampm_w > 0 ? (ampm_w + 3) : 0);
    int16_t time_x = (128 - total_w) / 2;
    u8g2.drawStr(time_x, 43, digits_str);

    if (ampm_str[0] != '\0') {
        u8g2.setFont(u8g2_font_profont11_mr);
        u8g2.drawStr(time_x + time_w + 3, 42, ampm_str);
    }

    // ------------------------------------------------------------------------
    // Date & Sync Status Bar (Bottom, Y=50..63)
    // ------------------------------------------------------------------------
    char date_str[24];
    clock_format_date(date_str, sizeof(date_str));

    u8g2.setFont(u8g2_font_profont11_mr);
    int16_t date_w = u8g2.getStrWidth(date_str);
    int16_t date_x = (128 - date_w) / 2;
    u8g2.drawStr(date_x, 59, date_str);
}

static void render_sleepy_face_card(U8G2& u8g2, const AppContext& ctx) {
    // ------------------------------------------------------------------------
    // Top Status Header (Height 12 px)
    // ------------------------------------------------------------------------
    u8g2.setFont(u8g2_font_profont10_mr);
    u8g2.setDrawColor(1);

    u8g2.drawFrame(2, 1, 32, 10);
    u8g2.drawStr(6, 9, "DOCK");

    if (ctx.is_charging) {
        u8g2.drawFrame(38, 1, 38, 10);
        u8g2.drawStr(42, 9, "CHRG");
    }

    // Compact 5-char digital clock placeholder or synced time (fits in 32px header slot)
    char compact_time[8];
    clock_format_time_parts(compact_time, sizeof(compact_time), nullptr, 0, true);
    u8g2.drawStr(78, 9, compact_time);

    // Top-Right: Card indicator dots
    draw_card_indicator(u8g2, 1);

    // Divider line
    u8g2.drawLine(0, 13, 127, 13);

    // ------------------------------------------------------------------------
    // Sleepy Face with subtle sinusoidal breathing animation
    // ------------------------------------------------------------------------
    uint32_t now = millis();
    int16_t breath_y = (int16_t)(sin((now % 3000) * (2.0 * M_PI / 3000.0)) * 2.0);

    faces_draw_expression(u8g2, ctx.current_expression, breath_y + 4);
}

static void render_telemetry_card(U8G2& u8g2, const AppContext& ctx) {
    // ------------------------------------------------------------------------
    // Top Status Header (Height 12 px)
    // ------------------------------------------------------------------------
    u8g2.setFont(u8g2_font_profont10_mr);
    u8g2.setDrawColor(1);

    u8g2.drawStr(2, 9, "SYS TELEMETRY");

    // Top-Right: Card indicator dots
    draw_card_indicator(u8g2, 2);

    // Divider line
    u8g2.drawLine(0, 13, 127, 13);

    // ------------------------------------------------------------------------
    // Hardware Telemetry Lines (ProFont 10)
    // ------------------------------------------------------------------------
    char buf[32];

    // Line 1: Dock Pogo state
    snprintf(buf, sizeof(buf), "Dock: 5V POGO (HIGH)");
    u8g2.drawStr(2, 24, buf);

    // Line 2: Charge state
    snprintf(buf, sizeof(buf), "Chrg: TP4056 (%s)", ctx.is_charging ? "CHARGING" : "STANDBY/FULL");
    u8g2.drawStr(2, 36, buf);

    // Line 3: Free Heap & Mic Noise Floor
    snprintf(buf, sizeof(buf), "Heap: %lu B | Mic: %.0f", (unsigned long)ESP.getFreeHeap(), clap_detector_get_noise_floor());
    u8g2.drawStr(2, 48, buf);

    // Line 4: System Uptime
    char up_str[16];
    clock_format_uptime(up_str, sizeof(up_str));
    snprintf(buf, sizeof(buf), "Uptime: %s", up_str);
    u8g2.drawStr(2, 60, buf);
}

static void draw_stat_bar(U8G2& u8g2, int x, int y, int w, int h, uint8_t pct) {
    u8g2.drawFrame(x, y, w, h);
    int fill = (pct * (w - 4)) / 100;
    if (fill > 0) {
        u8g2.drawBox(x + 2, y + 2, fill, h - 4);
    }
}

static void render_pc_stats_card(U8G2& u8g2, const AppContext& ctx) {
    u8g2.setFont(u8g2_font_profont10_mr);
    u8g2.setDrawColor(1);

    u8g2.drawStr(2, 9, "PC TELEMETRY");
    draw_card_indicator(u8g2, 3);
    u8g2.drawLine(0, 13, 127, 13);

    if (!rest_server_is_pc_telemetry_active()) {
        u8g2.setFont(u8g2_font_profont11_mr);
        const char* msg1 = "PC Disconnected / Sleep";
        int16_t w1 = u8g2.getStrWidth(msg1);
        u8g2.drawStr((128 - w1) / 2, 34, msg1);

        char ip_str[20];
        wifi_get_ip_str(ip_str, sizeof(ip_str));
        char msg2[32];
        snprintf(msg2, sizeof(msg2), "IP: %s", ip_str);
        int16_t w2 = u8g2.getStrWidth(msg2);
        u8g2.drawStr((128 - w2) / 2, 48, msg2);

        u8g2.setFont(u8g2_font_profont10_mr);
        const char* msg3 = "Port 80: POST /api/v1/telemetry";
        int16_t w3 = u8g2.getStrWidth(msg3);
        u8g2.drawStr((128 - w3) / 2, 60, msg3);
        return;
    }

    const PCTelemetryData& pc = rest_server_get_pc_telemetry();
    char buf[24];

    // Line 1: CPU
    u8g2.drawStr(2, 23, "CPU");
    draw_stat_bar(u8g2, 24, 16, 48, 8, pc.cpu_usage);
    snprintf(buf, sizeof(buf), "%2u%% %2uC", pc.cpu_usage, pc.cpu_temp);
    u8g2.drawStr(76, 23, buf);

    // Line 2: RAM
    u8g2.drawStr(2, 35, "RAM");
    draw_stat_bar(u8g2, 24, 28, 48, 8, pc.ram_usage);
    snprintf(buf, sizeof(buf), "%2u%%", pc.ram_usage);
    u8g2.drawStr(76, 35, buf);

    // Line 3: GPU
    u8g2.drawStr(2, 47, "GPU");
    draw_stat_bar(u8g2, 24, 40, 48, 8, pc.gpu_usage);
    snprintf(buf, sizeof(buf), "%2u%%", pc.gpu_usage);
    u8g2.drawStr(76, 47, buf);

    // Bottom Line: Wi-Fi RSSI & Link info
    char ip_str[16];
    wifi_get_ip_str(ip_str, sizeof(ip_str));
    snprintf(buf, sizeof(buf), "%s (%d dBm)", ip_str, wifi_get_rssi());
    u8g2.drawStr(2, 60, buf);
}

void dock_cards_render(U8G2& u8g2, const AppContext& ctx) {
    switch (s_active_card) {
        case DOCK_CARD_CLOCK:
            render_clock_card(u8g2, ctx);
            break;
        case DOCK_CARD_SLEEPY_FACE:
            render_sleepy_face_card(u8g2, ctx);
            break;
        case DOCK_CARD_TELEMETRY:
            render_telemetry_card(u8g2, ctx);
            break;
        case DOCK_CARD_PC_STATS:
            render_pc_stats_card(u8g2, ctx);
            break;
        default:
            render_clock_card(u8g2, ctx);
            break;
    }
}
