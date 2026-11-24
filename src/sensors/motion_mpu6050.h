#pragma once

#include <Arduino.h>
#include "board_config.h"

// ============================================================================
// Mochi Bot V1 - MPU6050 6-Axis Motion Sensor Driver
// I2C Interface on GPIO8 (SDA) / GPIO9 (SCL)
// Hardware Wake Interrupt on GPIO4 (INT)
// ============================================================================

struct AccelData {
    int16_t x;
    int16_t y;
    int16_t z;
};

// Initializes the MPU6050, checks WHO_AM_I, wakes from sleep, and configures +-2g
bool mpu6050_init();

// Configures hardware motion detection interrupt routed to INT pin (GPIO4)
// threshold: 1 LSB = 32 mg at +-2g (default 15 = ~480 mg)
// duration: duration counter (default 1)
void mpu6050_configure_wakeup(uint8_t threshold = 15, uint8_t duration = 1);

// Reads latest X, Y, Z acceleration data (in milli-g)
bool mpu6050_read_accel(AccelData& data);

// Reads INT_STATUS register (0x3A) to clear latched interrupt and de-assert INT pin
bool mpu6050_clear_interrupt();

// Returns true if sensor was successfully detected and initialized
bool mpu6050_is_available();

// Returns the active 7-bit I2C address (0x68 or 0x69)
uint8_t mpu6050_get_address();

// Attaches the GPIO4 hardware interrupt service routine
void mpu6050_attach_interrupt();

// Detaches the interrupt
void mpu6050_detach_interrupt();

// Checks and resets the volatile ISR flag (called from cooperative loop context)
bool mpu6050_poll_interrupt_flag();
