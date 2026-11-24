#pragma once

#include <Arduino.h>

// ============================================================================
// Mochi Bot V1 - Centralized Board Configuration & Pin Map
// Hardware Architecture:
// - ESP32-S3 SuperMini (Nologo-style)
// - 1.3" 128x64 Monochrome I2C OLED (SH1106 / SSD1306)
// - MPU6050 6-Axis Motion Sensor (I2C, GY-521)
// - TTP223 Capacitive Touch Switch (Expression Controller)
// - INMP441 MEMS Microphone & MAX98357A Class-D Amp (Shared I2S Clocks)
// - 2-Pin Magnetic Charging Dock with TP4056 Linear Charger
// - Protected 1S 3.7V 500 mAh LiPo Battery
// ============================================================================

// ----------------------------------------------------------------------------
// I2C Subsystem (1.3" OLED & MPU6050 Motion Sensor)
// ----------------------------------------------------------------------------
constexpr uint8_t PIN_I2C_SDA = 8;
constexpr uint8_t PIN_I2C_SCL = 9;
constexpr uint32_t I2C_BUS_FREQ_HZ = 100000;       // Standard 100 kHz for initial bringup
constexpr uint32_t I2C_TIMEOUT_MS = 25;            // Bounded bus transaction timeout

// Expected I2C 7-bit addresses
constexpr uint8_t I2C_ADDR_MPU6050_DEFAULT = 0x68; // AD0 pulled to GND (GY-521 default)
constexpr uint8_t I2C_ADDR_MPU6050_ALT     = 0x69; // AD0 pulled to VCC
constexpr uint8_t I2C_ADDR_OLED_DEFAULT     = 0x3C; // Standard SH1106 / SSD1306
constexpr uint8_t I2C_ADDR_OLED_ALT         = 0x3D;

// MPU6050 Register definitions for M0 probe
constexpr uint8_t MPU6050_REG_WHO_AM_I     = 0x75;
constexpr uint8_t MPU6050_EXPECTED_CHIP_ID = 0x68;

// ----------------------------------------------------------------------------
// Motion Wake Interrupt
// ----------------------------------------------------------------------------
constexpr uint8_t PIN_WAKE_INT = 4; // INT from MPU6050

// ----------------------------------------------------------------------------
// 2-Pin Magnetic Dock Sense & TP4056 Charge Monitoring
// ----------------------------------------------------------------------------
// 2-Pin Magnetic Pogo (+5V and GND) feeds TP4056 IN+ and IN-.
// Voltage divider (10k from 5V line to GPIO5, 20k from GPIO5 to GND) steps 5V down to 3.3V:
// HIGH (3.3V) = Attached / Docked (stays active from 0% to 100% full charge)
// LOW  (0.0V) = Detached / Undocked (Portable battery mode)
constexpr uint8_t PIN_DOCK_SENSE = 5;
constexpr uint32_t DOCK_DEBOUNCE_MS = 50;

// Optional Charge Status: Cathode of 1N4148 diode to TP4056 Pin 7 (CHRG Red LED cathode),
// Anode to GPIO7 with internal pull-up enabled:
// LOW  = Actively Charging (Red LED glowing)
// HIGH = Charge Complete / Standby / Disconnected (Red LED off)
constexpr uint8_t PIN_CHRG_STAT = 7;
constexpr uint32_t CHRG_DEBOUNCE_MS = 50;

// ----------------------------------------------------------------------------
// TTP223 Capacitive Touch Sensor (Expression Controller)
// ----------------------------------------------------------------------------
constexpr uint8_t PIN_TOUCH_SENSE = 6; // TTP223 digital output (RTC_GPIO6)
                                       // Default: Momentary, Active HIGH
                                       // HIGH = Touched / Active
                                       // LOW  = Released / Idle
constexpr uint32_t TOUCH_DEBOUNCE_MS = 30;

// ----------------------------------------------------------------------------
// I2S Audio Subsystem (Shared Clocks)
// ----------------------------------------------------------------------------
constexpr uint8_t PIN_I2S_BCLK = 2;   // Shared bit clock (INMP441 SCK & MAX98357A BCLK)
constexpr uint8_t PIN_I2S_WS   = 1;   // Shared word select (INMP441 WS & MAX98357A LRC)
constexpr uint8_t PIN_I2S_DIN  = 10;  // Audio in from INMP441 SD (Data Out)
constexpr uint8_t PIN_I2S_DOUT = 11;  // Audio out to MAX98357A DIN (Data In)

// ----------------------------------------------------------------------------
// Native USB Pins (Hardware USB Serial/JTAG)
// ----------------------------------------------------------------------------
// Dedicated to native USB CDC. Must NEVER be repurposed in firmware.
constexpr uint8_t PIN_USB_DM = 19;    // USB D-
constexpr uint8_t PIN_USB_DP = 20;    // USB D+

// ----------------------------------------------------------------------------
// Startup Diagnostics & Safety Constants
// ----------------------------------------------------------------------------
constexpr uint32_t SERIAL_BOOT_WAIT_MS = 2000; // Bounded wait for USB host terminal
constexpr const char* FIRMWARE_VERSION  = "1.0.0-M7-RC1";
constexpr const char* BUILD_TARGET      = "ESP32-S3 SuperMini (1.3\" SH1106, 500mAh)";
