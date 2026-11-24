#pragma once

#include <Arduino.h>

// ============================================================================
// Mochi Bot V1 - Dual-Target Board Configuration & Pin Map
// Hardware Architecture Support:
// - ESP32-S3 SuperMini (Xtensa Dual-Core, Baseline Target)
// - ESP32-C6 SuperMini (RISC-V Single-Core, Alternate Target)
// ============================================================================

#if defined(CONFIG_IDF_TARGET_ESP32C6) || defined(MOCHI_TARGET_ESP32C6)

// ----------------------------------------------------------------------------
// Target: ESP32-C6 SuperMini (1.3" OLED, 500mAh)
// ----------------------------------------------------------------------------
constexpr const char* BUILD_TARGET = "ESP32-C6 SuperMini (1.3\" SH1106, 500mAh)";

// I2C Subsystem (Remapped away from strapping GPIO8/9 to clean GPIO20/19 on Right header)
constexpr uint8_t PIN_I2C_SDA = 20;  // Right header Pin 4
constexpr uint8_t PIN_I2C_SCL = 19;  // Right header Pin 5

// I2S Audio Subsystem (Shared Clocks: WS=1, BCLK=2; Mic DIN=3, Amp DOUT=18)
constexpr uint8_t PIN_I2S_BCLK = 2;  // Shared bit clock (INMP441 SCK & MAX98357A BCLK)
constexpr uint8_t PIN_I2S_WS   = 1;  // Shared word select (INMP441 WS & MAX98357A LRC)
constexpr uint8_t PIN_I2S_DIN  = 3;  // Mic data in (Left header Pin 6, adjacent to mic)
constexpr uint8_t PIN_I2S_DOUT = 18; // Audio out to amp (Right header Pin 6)

// Native USB Serial/JTAG (Dedicated to USB-C; must NEVER be repurposed)
constexpr uint8_t PIN_USB_DM   = 12; // USB D-
constexpr uint8_t PIN_USB_DP   = 13; // USB D+

#elif defined(CONFIG_IDF_TARGET_ESP32S3) || defined(MOCHI_TARGET_ESP32S3) || !defined(CONFIG_IDF_TARGET)

// ----------------------------------------------------------------------------
// Target: ESP32-S3 SuperMini (1.3" OLED, 500mAh) - Baseline Target
// ----------------------------------------------------------------------------
constexpr const char* BUILD_TARGET = "ESP32-S3 SuperMini (1.3\" SH1106, 500mAh)";

// I2C Subsystem
constexpr uint8_t PIN_I2C_SDA = 8;   // Right header Pin 9
constexpr uint8_t PIN_I2C_SCL = 9;   // Right header Pin 8

// I2S Audio Subsystem (Shared Clocks)
constexpr uint8_t PIN_I2S_BCLK = 2;  // Shared bit clock (INMP441 SCK & MAX98357A BCLK)
constexpr uint8_t PIN_I2S_WS   = 1;  // Shared word select (INMP441 WS & MAX98357A LRC)
constexpr uint8_t PIN_I2S_DIN  = 10; // Audio in from INMP441 SD (Right header Pin 7)
constexpr uint8_t PIN_I2S_DOUT = 11; // Audio out to MAX98357A DIN (Right header Pin 6)

// Native USB Serial/JTAG (Dedicated to USB-C; must NEVER be repurposed)
constexpr uint8_t PIN_USB_DM   = 19; // USB D-
constexpr uint8_t PIN_USB_DP   = 20; // USB D+

#else
#error "Unsupported target microcontroller! Mochi Bot requires ESP32-S3 or ESP32-C6."
#endif

// ----------------------------------------------------------------------------
// Common Pin Assignments (Identical across both ESP32-S3 and ESP32-C6)
// ----------------------------------------------------------------------------
constexpr uint8_t PIN_WAKE_INT    = 4; // INT from MPU6050 (Motion wake interrupt)
constexpr uint8_t PIN_DOCK_SENSE  = 5; // 10k/20k voltage divider from 5V dock (HIGH=Docked, LOW=Battery)
constexpr uint8_t PIN_TOUCH_SENSE = 6; // TTP223 capacitive touch input (Active HIGH)
constexpr uint8_t PIN_CHRG_STAT   = 7; // 1N4148 cathode to TP4056 CHRG pin (Active LOW, internal pull-up)

// ----------------------------------------------------------------------------
// Common Timing, Address, and Bus Constants
// ----------------------------------------------------------------------------
constexpr uint32_t I2C_BUS_FREQ_HZ = 100000;       // Standard 100 kHz
constexpr uint32_t I2C_TIMEOUT_MS  = 25;           // Bounded bus transaction timeout

constexpr uint8_t I2C_ADDR_MPU6050_DEFAULT = 0x68; // AD0 pulled to GND (GY-521 default)
constexpr uint8_t I2C_ADDR_MPU6050_ALT     = 0x69; // AD0 pulled to VCC
constexpr uint8_t I2C_ADDR_OLED_DEFAULT     = 0x3C; // Standard SH1106 / SSD1306
constexpr uint8_t I2C_ADDR_OLED_ALT         = 0x3D;

constexpr uint8_t MPU6050_REG_WHO_AM_I     = 0x75;
constexpr uint8_t MPU6050_EXPECTED_CHIP_ID = 0x68;

constexpr uint32_t DOCK_DEBOUNCE_MS  = 50;
constexpr uint32_t CHRG_DEBOUNCE_MS  = 50;
constexpr uint32_t TOUCH_DEBOUNCE_MS = 30;

constexpr uint32_t SERIAL_BOOT_WAIT_MS = 2000; // Bounded wait for USB host terminal
constexpr const char* FIRMWARE_VERSION  = "1.0.0-M7-RC1";
