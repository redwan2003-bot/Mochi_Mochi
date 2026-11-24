#include <Arduino.h>
#include "board_config.h"
#include "diagnostics.h"
#include "app_state.h"
#include "display/display_manager.h"
#include "sensors/motion_mpu6050.h"
#include "sensors/clap_detector.h"
#include "audio/i2s_bus.h"
#include "audio/audio_output.h"
#include "storage/settings_storage.h"
#include "time/clock_manager.h"
#include "display/dock_cards.h"
#include "ble/ble_manager.h"
#include "wifi/wifi_manager.h"
#include "server/rest_server.h"

// Tracking variables for periodic heartbeat
static uint32_t last_heartbeat_ms = 0;

// Non-blocking debounce state variables for 2-pin dock detection (GPIO5)
static int last_raw_dock_pin = LOW;
static uint32_t last_dock_debounce_ms = 0;
static bool debounced_docked_state = false;

// Non-blocking debounce state variables for TP4056 charge status (GPIO7)
static int last_raw_chrg_pin = HIGH;
static uint32_t last_chrg_debounce_ms = 0;
static bool debounced_charging_state = false;

// Non-blocking debounce state variables for TTP223 touch detection (GPIO6)
static int last_raw_touch_pin = LOW;
static uint32_t last_touch_debounce_ms = 0;
static bool debounced_touch_state = false;

void setup() {
    // ------------------------------------------------------------------------
    // 1. Initialize Native USB CDC Serial Console
    // ------------------------------------------------------------------------
    Serial.begin(115200);

    // Non-blocking TX timeout: drops characters if host buffer is full/disconnected
    Serial.setTxTimeoutMs(0);

    // Bounded wait for host terminal to enumerate (up to 2000 ms)
    uint32_t wait_start = millis();
    while (!Serial && (millis() - wait_start < SERIAL_BOOT_WAIT_MS)) {
        delay(10);
    }

    // ------------------------------------------------------------------------
    // 2. Initialize Diagnostics Subsystems & Bounded I2C
    // ------------------------------------------------------------------------
    diagnostics_init();

    // ------------------------------------------------------------------------
    // 3. Output Boot Banner & Environment Health
    // ------------------------------------------------------------------------
    diagnostics_print_banner();
    diagnostics_print_chip_info();
    diagnostics_print_reset_reason();
    diagnostics_print_memory_stats();

    // ------------------------------------------------------------------------
    // 4. Run Hardware Probes (I2C scan, GPIO5 dock, GPIO7 charge, GPIO6 touch)
    // ------------------------------------------------------------------------
    M0HardwareInventory inv = diagnostics_run_inventory();

    // ------------------------------------------------------------------------
    // 5. Initialize Application State & 1.3" OLED Display Manager
    // ------------------------------------------------------------------------
    app_state_init();
    display_init();

    // ------------------------------------------------------------------------
    // 6. Initialize MPU6050 Motion Sensor & Wake Interrupt (GPIO4)
    // ------------------------------------------------------------------------
    mpu6050_init();

    // Set initial hardware states into the application context
    debounced_docked_state = inv.is_docked;
    last_raw_dock_pin = digitalRead(PIN_DOCK_SENSE);
    app_state_on_dock_change(debounced_docked_state);

    debounced_charging_state = inv.is_charging;
    last_raw_chrg_pin = digitalRead(PIN_CHRG_STAT);
    app_state_on_charge_change(debounced_charging_state);

    debounced_touch_state = inv.touch_active;
    last_raw_touch_pin = digitalRead(PIN_TOUCH_SENSE);
    if (debounced_touch_state) {
        app_state_on_touch(true);
    }

    // ------------------------------------------------------------------------
    // 7. Initialize Audio Subsystem & Sequential I2S Arbiter (Milestone 3)
    // ------------------------------------------------------------------------
    audio_output_init();
    clap_detector_init();
    i2s_bus_start_rx();

    // ------------------------------------------------------------------------
    // 8. Initialize Android BLE Companion Service (Milestone 5)
    // ------------------------------------------------------------------------
    ble_manager_init();

    // ------------------------------------------------------------------------
    // 9. Initialize Wi-Fi Station & Local REST Server (Milestone 6)
    // ------------------------------------------------------------------------
    wifi_manager_init();
    rest_server_init();

    Serial.println("\n[BOOT] Mochi Bot V1 (Milestone 7 RC) initialization complete. Entering cooperative loop.\n");
}

void loop() {
    uint32_t now = millis();

    // ------------------------------------------------------------------------
    // 1. Process MPU6050 Motion Wake / Nudge Interrupt (GPIO4)
    // ------------------------------------------------------------------------
    if (mpu6050_poll_interrupt_flag()) {
        AccelData accel = {0, 0, 0};
        mpu6050_read_accel(accel);
        mpu6050_clear_interrupt(); // Clear hardware latch on INT pin
        app_state_on_motion(accel.x, accel.y, accel.z);
    }

    // ------------------------------------------------------------------------
    // 2. Process Acoustic Clap Detector (INMP441 on GPIO10)
    // ------------------------------------------------------------------------
    if (clap_detector_poll()) {
        app_state_on_clap();
    }

    // ------------------------------------------------------------------------
    // 3. Process BLE Companion Telemetry & Driving Mode
    // ------------------------------------------------------------------------
    ble_manager_poll();

    // ------------------------------------------------------------------------
    // 4. Process Wi-Fi Station & Local REST Server
    // ------------------------------------------------------------------------
    wifi_manager_poll();
    rest_server_poll();

    // ------------------------------------------------------------------------
    // 2. Non-blocking 2-pin dock debounce (using DOCK_DEBOUNCE_MS)
    //    HIGH = Docked (3.3V from 10k/20k divider), LOW = Undocked
    // ------------------------------------------------------------------------
    int current_raw_dock = digitalRead(PIN_DOCK_SENSE);
    if (current_raw_dock != last_raw_dock_pin) {
        last_dock_debounce_ms = now;
        last_raw_dock_pin = current_raw_dock;
    }

    if ((now - last_dock_debounce_ms) >= DOCK_DEBOUNCE_MS) {
        bool candidate_docked = (current_raw_dock == HIGH);
        if (candidate_docked != debounced_docked_state) {
            debounced_docked_state = candidate_docked;
            Serial.printf("[EVENT] Dock transition: %s (GPIO5 = %s)\n",
                          debounced_docked_state ? "DOCKED / ATTACHED" : "UNDOCKED / PORTABLE",
                          debounced_docked_state ? "HIGH (3.3V)" : "LOW (0.0V)");
            app_state_on_dock_change(debounced_docked_state);
        }
    }

    // ------------------------------------------------------------------------
    // 3. Non-blocking TP4056 charge status debounce (using CHRG_DEBOUNCE_MS)
    //    LOW = Actively Charging (Red LED on), HIGH = Full / Off
    // ------------------------------------------------------------------------
    int current_raw_chrg = digitalRead(PIN_CHRG_STAT);
    if (current_raw_chrg != last_raw_chrg_pin) {
        last_chrg_debounce_ms = now;
        last_raw_chrg_pin = current_raw_chrg;
    }

    if ((now - last_chrg_debounce_ms) >= CHRG_DEBOUNCE_MS) {
        bool candidate_charging = (current_raw_chrg == LOW);
        if (candidate_charging != debounced_charging_state) {
            debounced_charging_state = candidate_charging;
            Serial.printf("[EVENT] Charge status: %s (GPIO7 = %s)\n",
                          debounced_charging_state ? "ACTIVELY CHARGING" : "STANDBY / FULL / OFF",
                          debounced_charging_state ? "LOW (GND)" : "HIGH (3.3V)");
            app_state_on_charge_change(debounced_charging_state);
        }
    }

    // ------------------------------------------------------------------------
    // 4. Non-blocking TTP223 touch sensor debounce (using TOUCH_DEBOUNCE_MS)
    //    Acts as Mochi Bot's expression controller
    // ------------------------------------------------------------------------
    int current_raw_touch = digitalRead(PIN_TOUCH_SENSE);
    if (current_raw_touch != last_raw_touch_pin) {
        last_touch_debounce_ms = now;
        last_raw_touch_pin = current_raw_touch;
    }

    if ((now - last_touch_debounce_ms) >= TOUCH_DEBOUNCE_MS) {
        bool candidate_touched = (current_raw_touch == HIGH);
        if (candidate_touched != debounced_touch_state) {
            debounced_touch_state = candidate_touched;
            Serial.printf("[EVENT] Expression Touch: %s (GPIO6 = %s)\n",
                          debounced_touch_state ? "TOUCHED / PRESSED" : "RELEASED / IDLE",
                          debounced_touch_state ? "HIGH" : "LOW");
            app_state_on_touch(debounced_touch_state);
        }
    }

    // ------------------------------------------------------------------------
    // 5. Update Application State Machine (Blinks, timeouts, sleep policy)
    // ------------------------------------------------------------------------
    app_state_update();

    // ------------------------------------------------------------------------
    // 6. Render Display (Throttled to ~30 FPS, sleep-aware)
    // ------------------------------------------------------------------------
    display_render();

    // ------------------------------------------------------------------------
    // 7. Periodic heartbeat (every 5 seconds)
    // ------------------------------------------------------------------------
    if (now - last_heartbeat_ms >= 5000) {
        last_heartbeat_ms = now;
        AppContext& ctx = app_state_get_context();
        const char* state_str = "BOOT";
        switch (ctx.current_state) {
            case SYS_STATE_DOCK_HOME: state_str = "DOCK_HOME"; break;
            case SYS_STATE_PORTABLE_ACTIVE: state_str = "PORTABLE_ACTIVE"; break;
            case SYS_STATE_PORTABLE_SLEEP: state_str = "PORTABLE_SLEEP"; break;
            default: break;
        }

        char time_str[16];
        clock_format_time(time_str, sizeof(time_str), true, true);

        char ip_str[16];
        wifi_get_ip_str(ip_str, sizeof(ip_str));

        Serial.printf("[HEARTBEAT] Up: %lu s | Time: %s | State: %s | Card: %d | BLE: %s | Spd: %u | WiFi: %s | PC: %s | Heap: %u B | Dock: %s\n",
                      now / 1000,
                      time_str,
                      state_str,
                      (int)dock_cards_get_current(),
                      ble_is_connected() ? "CONN" : "ADV",
                      ble_get_driving_speed(),
                      ip_str,
                      rest_server_is_pc_telemetry_active() ? "ONLINE" : "OFFLINE",
                      ESP.getFreeHeap(),
                      debounced_docked_state ? "YES" : "NO");
    }

    // Cooperative yield to feed FreeRTOS Core 1 idle watchdog
    delay(5);
}
