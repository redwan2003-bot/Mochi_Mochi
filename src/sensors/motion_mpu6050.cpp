#include "motion_mpu6050.h"
#include <Wire.h>

// MPU6050 Register Map
constexpr uint8_t REG_SMPLRT_DIV    = 0x19;
constexpr uint8_t REG_CONFIG        = 0x1A;
constexpr uint8_t REG_GYRO_CONFIG   = 0x1B;
constexpr uint8_t REG_ACCEL_CONFIG  = 0x1C;
constexpr uint8_t REG_MOT_THR       = 0x1F;
constexpr uint8_t REG_MOT_DUR       = 0x20;
constexpr uint8_t REG_INT_PIN_CFG   = 0x37;
constexpr uint8_t REG_INT_ENABLE    = 0x38;
constexpr uint8_t REG_INT_STATUS    = 0x3A;
constexpr uint8_t REG_ACCEL_XOUT_H  = 0x3B;
constexpr uint8_t REG_PWR_MGMT_1    = 0x6B;
constexpr uint8_t REG_PWR_MGMT_2    = 0x6C;
constexpr uint8_t REG_WHO_AM_I      = 0x75;

static uint8_t g_sensor_addr = 0x00;
static bool g_sensor_available = false;
static volatile bool g_isr_motion_flag = false;

// Hardware ISR for GPIO4 (INT from MPU6050)
static void IRAM_ATTR mpu6050_gpio_isr() {
    g_isr_motion_flag = true;
}

// Low-level I2C register helpers with bounded timeouts
static bool write_reg(uint8_t reg, uint8_t value) {
    if (g_sensor_addr == 0) return false;
    Wire.beginTransmission(g_sensor_addr);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

static bool read_reg(uint8_t reg, uint8_t& value) {
    if (g_sensor_addr == 0) return false;
    Wire.beginTransmission(g_sensor_addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(g_sensor_addr, (uint8_t)1) != 1) return false;
    value = Wire.read();
    return true;
}

bool mpu6050_init() {
    g_sensor_available = false;
    g_sensor_addr = 0x00;

    // Probe default (0x68) and alternate (0x69) I2C addresses
    uint8_t candidates[2] = {I2C_ADDR_MPU6050_DEFAULT, I2C_ADDR_MPU6050_ALT};
    for (uint8_t addr : candidates) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Wire.beginTransmission(addr);
            Wire.write(REG_WHO_AM_I);
            if (Wire.endTransmission(false) == 0) {
                if (Wire.requestFrom(addr, (uint8_t)1) == 1) {
                    uint8_t id = Wire.read();
                    if (id == MPU6050_EXPECTED_CHIP_ID || id == 0x70 || id == 0x72) {
                        g_sensor_addr = addr;
                        g_sensor_available = true;
                        break;
                    }
                }
            }
        }
    }

    if (!g_sensor_available) {
        Serial.println("[MOTION] MPU6050 not detected on I2C bus. Running degraded without motion.");
        return false;
    }

    Serial.printf("[MOTION] Initializing MPU6050 at address 0x%02X...\n", g_sensor_addr);

    // 1. Wake device from sleep (clear SLEEP bit, select PLL with X-axis gyro reference) -> 0x01
    write_reg(REG_PWR_MGMT_1, 0x01);
    delay(10);

    // 2. Sample rate divider: 1 kHz / (1 + 19) = 50 Hz sample rate
    write_reg(REG_SMPLRT_DIV, 19);

    // 3. Digital Low-Pass Filter (DLPF): 44 Hz bandwidth -> 0x03
    write_reg(REG_CONFIG, 0x03);

    // 4. Accelerometer configuration: +-2g scale (AFS_SEL = 0) -> 0x00
    write_reg(REG_ACCEL_CONFIG, 0x00);

    // 5. Gyroscope configuration: +-250 dps -> 0x00
    write_reg(REG_GYRO_CONFIG, 0x00);

    // 6. Power optimization: Put gyros in standby (STBY_XG, STBY_YG, STBY_ZG = 1),
    // keep accelerometer running active. Reduces MPU6050 current by ~80% (~0.5 mA)!
    write_reg(REG_PWR_MGMT_2, 0x07);

    // 7. Configure INT pin: Active HIGH, push-pull, latched until INT_STATUS read -> 0x30
    write_reg(REG_INT_PIN_CFG, 0x30);

    // 8. Configure default motion detection wake-up threshold (15 * 32 mg = ~480 mg)
    mpu6050_configure_wakeup(15, 1);

    // 9. Clear any pending interrupt latch from power-up
    mpu6050_clear_interrupt();

    // 10. Attach hardware interrupt to GPIO4
    mpu6050_attach_interrupt();

    Serial.println("[MOTION] MPU6050 configured: +-2g Accel, Low-Power Gyro Standby, Motion Interrupt latched to GPIO4.");
    return true;
}

void mpu6050_configure_wakeup(uint8_t threshold, uint8_t duration) {
    if (!g_sensor_available) return;

    // MOT_THR: 1 LSB = 32 mg at +-2g scale
    write_reg(REG_MOT_THR, threshold);

    // MOT_DUR: Duration counter
    write_reg(REG_MOT_DUR, duration);

    // INT_ENABLE: Enable motion detection interrupt (MOT_EN = bit 6) -> 0x40
    write_reg(REG_INT_ENABLE, 0x40);
}

bool mpu6050_read_accel(AccelData& data) {
    if (!g_sensor_available) return false;

    // Read 6 bytes starting from ACCEL_XOUT_H (0x3B)
    Wire.beginTransmission(g_sensor_addr);
    Wire.write(REG_ACCEL_XOUT_H);
    if (Wire.endTransmission(false) != 0) return false;

    if (Wire.requestFrom(g_sensor_addr, (uint8_t)6) != 6) return false;

    uint8_t xh = Wire.read();
    uint8_t xl = Wire.read();
    uint8_t yh = Wire.read();
    uint8_t yl = Wire.read();
    uint8_t zh = Wire.read();
    uint8_t zl = Wire.read();

    int16_t raw_x = (int16_t)((xh << 8) | xl);
    int16_t raw_y = (int16_t)((yh << 8) | yl);
    int16_t raw_z = (int16_t)((zh << 8) | zl);

    // At +-2g scale, sensitivity is 16384 LSB/g (1000 mg)
    data.x = (int16_t)(((int32_t)raw_x * 1000) / 16384);
    data.y = (int16_t)(((int32_t)raw_y * 1000) / 16384);
    data.z = (int16_t)(((int32_t)raw_z * 1000) / 16384);

    return true;
}

bool mpu6050_clear_interrupt() {
    if (!g_sensor_available) return false;
    uint8_t status = 0;
    // Reading INT_STATUS (0x3A) clears the MOT_INT bit and de-asserts the INT pin
    return read_reg(REG_INT_STATUS, status);
}

bool mpu6050_is_available() {
    return g_sensor_available;
}

uint8_t mpu6050_get_address() {
    return g_sensor_addr;
}

void mpu6050_attach_interrupt() {
    pinMode(PIN_WAKE_INT, INPUT); // MPU6050 INT pin actively drives HIGH when triggered
    attachInterrupt(digitalPinToInterrupt(PIN_WAKE_INT), mpu6050_gpio_isr, RISING);
}

void mpu6050_detach_interrupt() {
    detachInterrupt(digitalPinToInterrupt(PIN_WAKE_INT));
}

bool mpu6050_poll_interrupt_flag() {
    if (g_isr_motion_flag) {
        g_isr_motion_flag = false;
        return true;
    }
    return false;
}
