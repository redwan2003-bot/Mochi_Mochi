#include "diagnostics.h"
#include <Wire.h>
#include <esp_flash.h>

void diagnostics_init() {
    // Configure dock sense pin (GPIO5)
    // External 20k resistor to GND acts as pulldown:
    // HIGH (3.3V) = Attached / Docked (5V line active through 10k/20k divider)
    // LOW  (0.0V) = Detached / Undocked
    pinMode(PIN_DOCK_SENSE, INPUT);

    // Configure TP4056 charge status input (GPIO7)
    // Cathode of 1N4148 diode to TP4056 CHRG pin, Anode to GPIO7.
    // Internal pullup holds GPIO7 at 3.3V when diode is reverse-biased (Standby/Full/Disconnected).
    // TP4056 pulls LOW during active charging.
    pinMode(PIN_CHRG_STAT, INPUT_PULLUP);

    // Configure TTP223 touch switch pin (GPIO6) with internal pulldown
    // Active HIGH momentary: HIGH = Touched / Active, LOW = Released / Idle
    pinMode(PIN_TOUCH_SENSE, INPUT_PULLDOWN);

    // Initialize I2C with strict bus timeout to prevent lockup on floating/shorted bus
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_BUS_FREQ_HZ);
    Wire.setTimeOut(I2C_TIMEOUT_MS);
}

void diagnostics_print_banner() {
    Serial.println();
    Serial.println("==================================================");
    Serial.println("  MOCHI BOT V1 - FIRMWARE DIAGNOSTICS");
    Serial.printf("  Firmware : %s\n", FIRMWARE_VERSION);
    Serial.printf("  Target   : %s\n", BUILD_TARGET);
    Serial.println("==================================================");
}

const char* diagnostics_flash_mode_str(FlashMode_t mode) {
    switch (mode) {
        case FM_QIO:       return "QIO (Quad I/O)";
        case FM_QOUT:      return "QOUT (Quad Output)";
        case FM_DIO:       return "DIO (Dual I/O)";
        case FM_DOUT:      return "DOUT (Dual Output)";
        case FM_FAST_READ: return "FAST_READ";
        case FM_SLOW_READ: return "SLOW_READ";
        default:           return "UNKNOWN";
    }
}

const char* diagnostics_reset_reason_str(esp_reset_reason_t reason) {
    switch (reason) {
        case ESP_RST_POWERON:   return "Power-on (cold boot)";
        case ESP_RST_EXT:       return "External pin reset";
        case ESP_RST_SW:        return "Software reset (esp_restart)";
        case ESP_RST_PANIC:     return "Software panic / exception";
        case ESP_RST_INT_WDT:   return "Interrupt Watchdog";
        case ESP_RST_TASK_WDT:  return "Task Watchdog";
        case ESP_RST_WDT:       return "Other Watchdog";
        case ESP_RST_DEEPSLEEP: return "Deep-sleep wake-up";
        case ESP_RST_BROWNOUT:  return "Brownout reset (voltage dip)";
        case ESP_RST_SDIO:      return "SDIO reset";
        default:                return "Unknown / undefined";
    }
}

void diagnostics_print_chip_info() {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    Serial.println("\n[CHIP IDENTIFICATION]");
    Serial.printf("  Model           : %s\n", ESP.getChipModel());
    Serial.printf("  Silicon Rev     : %d\n", ESP.getChipRevision());
    Serial.printf("  CPU Cores       : %d\n", chip_info.cores);
    Serial.printf("  CPU Frequency   : %u MHz\n", getCpuFrequencyMhz());

    uint32_t flash_size = ESP.getFlashChipSize();
    uint32_t flash_speed = ESP.getFlashChipSpeed();
    FlashMode_t flash_mode = ESP.getFlashChipMode();

    Serial.printf("  Flash Size      : %u KB (%.2f MB)\n", flash_size / 1024, flash_size / (1024.0 * 1024.0));
    Serial.printf("  Flash Speed     : %u MHz\n", flash_speed / 1000000);
    Serial.printf("  Flash Bus Mode  : %s\n", diagnostics_flash_mode_str(flash_mode));

    // Runtime PSRAM verification
    bool has_psram = psramFound();
    Serial.printf("  PSRAM Found     : %s\n", has_psram ? "YES" : "NO");
    if (has_psram) {
        Serial.printf("  PSRAM Size      : %u KB (%.2f MB)\n", ESP.getPsramSize() / 1024, ESP.getPsramSize() / (1024.0 * 1024.0));
        Serial.printf("  PSRAM Free      : %u KB\n", ESP.getFreePsram() / 1024);
    } else {
        Serial.println("  PSRAM Note      : Running on internal DRAM only (expected for base S3/FH4)");
    }
}

void diagnostics_print_reset_reason() {
    esp_reset_reason_t reason = esp_reset_reason();
    Serial.println("\n[SYSTEM HEALTH & BOOT REASON]");
    Serial.printf("  Reset Reason    : (%d) %s\n", (int)reason, diagnostics_reset_reason_str(reason));
}

void diagnostics_print_memory_stats() {
    Serial.println("\n[MEMORY ALLOCATION]");
    Serial.printf("  Internal Heap   : %u bytes free\n", ESP.getFreeHeap());
    Serial.printf("  Min Free Heap   : %u bytes (watermark)\n", ESP.getMinFreeHeap());
    Serial.printf("  Max Alloc Block : %u bytes contiguous\n", ESP.getMaxAllocHeap());
}

uint8_t diagnostics_scan_i2c() {
    Serial.println("\n[I2C BUS SCANNER]");
    Serial.printf("  Scanning GPIO%d (SDA) / GPIO%d (SCL)...\n", PIN_I2C_SDA, PIN_I2C_SCL);

    uint8_t count = 0;
    for (uint8_t addr = 0x08; addr < 0x78; ++addr) {
        Wire.beginTransmission(addr);
        uint8_t err = Wire.endTransmission();
        if (err == 0) {
            Serial.printf("  -> Device responding at address 0x%02X", addr);
            if (addr == I2C_ADDR_MPU6050_DEFAULT) {
                Serial.print(" (MPU6050 Default / AD0=GND)");
            } else if (addr == I2C_ADDR_MPU6050_ALT) {
                Serial.print(" (MPU6050 Alternate / AD0=VCC)");
            } else if (addr == I2C_ADDR_OLED_DEFAULT) {
                Serial.print(" (1.3\" OLED 0x3C)");
            } else if (addr == I2C_ADDR_OLED_ALT) {
                Serial.print(" (1.3\" OLED 0x3D)");
            }
            Serial.println();
            count++;
        }
    }

    if (count == 0) {
        Serial.println("  -> No I2C devices responded on the bus.");
        Serial.println("     Check: 3.3V power, GND, GPIO8/GPIO9 wiring, and pull-up resistors.");
    } else {
        Serial.printf("  Total devices detected: %d\n", count);
    }
    return count;
}

bool diagnostics_probe_mpu6050(uint8_t& detected_addr, uint8_t& chip_id) {
    uint8_t candidates[2] = {I2C_ADDR_MPU6050_DEFAULT, I2C_ADDR_MPU6050_ALT};
    for (uint8_t addr : candidates) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            // Address responded; read WHO_AM_I register (0x75)
            Wire.beginTransmission(addr);
            Wire.write(MPU6050_REG_WHO_AM_I);
            if (Wire.endTransmission(false) == 0) { // Repeated start
                if (Wire.requestFrom(addr, (uint8_t)1) == 1) {
                    uint8_t id = Wire.read();
                    // MPU6050 standard WHO_AM_I is 0x68 (some variants return 0x70 or 0x72)
                    if (id == MPU6050_EXPECTED_CHIP_ID || id == 0x70 || id == 0x72) {
                        detected_addr = addr;
                        chip_id = id;
                        return true; // Valid MPU6050 confirmed
                    }
                }
            }
        }
    }
    detected_addr = 0x00;
    chip_id = 0x00;
    return false;
}

bool diagnostics_probe_oled(uint8_t& detected_addr) {
    uint8_t candidates[2] = {I2C_ADDR_OLED_DEFAULT, I2C_ADDR_OLED_ALT};
    for (uint8_t addr : candidates) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            detected_addr = addr;
            return true;
        }
    }
    detected_addr = 0x00;
    return false;
}

bool diagnostics_read_dock_state() {
    // Read GPIO5: HIGH (3.3V) via 10k/20k divider = Docked, LOW (0V) = Undocked
    int read1 = digitalRead(PIN_DOCK_SENSE);
    delay(DOCK_DEBOUNCE_MS);
    int read2 = digitalRead(PIN_DOCK_SENSE);
    if (read1 == read2) {
        return (read1 == HIGH);
    }
    delay(DOCK_DEBOUNCE_MS);
    return (digitalRead(PIN_DOCK_SENSE) == HIGH);
}

bool diagnostics_read_charge_state() {
    // Read GPIO7: LOW = TP4056 actively charging (Red LED glowing), HIGH = Standby/Full/Disconnected
    int read1 = digitalRead(PIN_CHRG_STAT);
    delay(CHRG_DEBOUNCE_MS);
    int read2 = digitalRead(PIN_CHRG_STAT);
    if (read1 == read2) {
        return (read1 == LOW);
    }
    delay(CHRG_DEBOUNCE_MS);
    return (digitalRead(PIN_CHRG_STAT) == LOW);
}

bool diagnostics_read_touch_state() {
    // Read GPIO6: HIGH = Touched / Active, LOW = Released / Idle
    int read1 = digitalRead(PIN_TOUCH_SENSE);
    delay(TOUCH_DEBOUNCE_MS);
    int read2 = digitalRead(PIN_TOUCH_SENSE);
    if (read1 == read2) {
        return (read1 == HIGH);
    }
    delay(TOUCH_DEBOUNCE_MS);
    return (digitalRead(PIN_TOUCH_SENSE) == HIGH);
}

M0HardwareInventory diagnostics_run_inventory() {
    M0HardwareInventory inv = {};

    inv.i2c_device_count = diagnostics_scan_i2c();
    inv.mpu6050_found = diagnostics_probe_mpu6050(inv.mpu6050_addr, inv.mpu6050_chip_id);
    inv.oled_found = diagnostics_probe_oled(inv.oled_addr);
    inv.is_docked = diagnostics_read_dock_state();
    inv.is_charging = diagnostics_read_charge_state();
    inv.touch_active = diagnostics_read_touch_state();

    Serial.println("\n[HARDWARE MODULE INVENTORY]");

    // Motion Sensor
    Serial.printf("  MPU6050 Sensor  : ");
    if (inv.mpu6050_found) {
        Serial.printf("PRESENT at 0x%02X (WHO_AM_I: 0x%02X - OK)\n", inv.mpu6050_addr, inv.mpu6050_chip_id);
    } else {
        Serial.println("NOT DETECTED (or WHO_AM_I mismatch)");
    }

    // OLED
    Serial.printf("  1.3\" OLED Display: ");
    if (inv.oled_found) {
        Serial.printf("PRESENT at 0x%02X (ACK OK)\n", inv.oled_addr);
    } else {
        Serial.println("NOT DETECTED");
    }

    // 2-Pin Dock Sense
    Serial.printf("  2-Pin Dock State: %s (GPIO5 = %s)\n", 
                  inv.is_docked ? "DOCKED / ATTACHED" : "UNDOCKED / PORTABLE",
                  inv.is_docked ? "HIGH (3.3V)" : "LOW (0.0V)");

    // TP4056 Charge Monitoring
    Serial.printf("  TP4056 Charger  : %s (GPIO7 = %s)\n",
                  inv.is_charging ? "ACTIVELY CHARGING" : "STANDBY / FULL / OFF",
                  inv.is_charging ? "LOW (GND)" : "HIGH (3.3V)");

    // TTP223 Touch Sensor (Expression Controller)
    Serial.printf("  TTP223 Touch    : %s (GPIO6 = %s)\n",
                  inv.touch_active ? "TOUCHED / ACTIVE" : "RELEASED / IDLE",
                  inv.touch_active ? "HIGH (3.3V)" : "LOW (0.0V)");

    Serial.println("==================================================");
    return inv;
}
