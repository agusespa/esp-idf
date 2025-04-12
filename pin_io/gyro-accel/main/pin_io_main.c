#include <stdio.h>

#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_SCL_IO 25
#define I2C_MASTER_SDA_IO 26
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000
#define MPU6050_ADDR 0x68

#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_CONFIG 0x1A
#define MPU6050_ACCEL_XOUT_H 0x3B
#define TAG "MPU6050"

static esp_err_t mpu6050_write_byte(uint8_t reg, uint8_t data) {
    return i2c_master_write_to_device(I2C_MASTER_NUM, MPU6050_ADDR,
                                      (uint8_t[]){reg, data}, 2,
                                      1000 / portTICK_PERIOD_MS);
}

static esp_err_t mpu6050_read_bytes(uint8_t reg, uint8_t *buf, size_t len) {
    return i2c_master_write_read_device(I2C_MASTER_NUM, MPU6050_ADDR, &reg, 1,
                                        buf, len, 1000 / portTICK_PERIOD_MS);
}

static void mpu6050_init() {
    mpu6050_write_byte(MPU6050_PWR_MGMT_1, 0x00);    // Wake up
    mpu6050_write_byte(MPU6050_ACCEL_CONFIG, 0x10);  // ±8g → bits 3:4 = 10
    mpu6050_write_byte(MPU6050_GYRO_CONFIG, 0x08);   // ±500°/s → bits 3:4 = 01
    mpu6050_write_byte(MPU6050_CONFIG, 0x04);        // DLPF = 21 Hz → value = 4
}

static void i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

static int16_t read_word(uint8_t *data, int idx) {
    return (int16_t)((data[idx] << 8) | data[idx + 1]);
}

void app_main() {
    i2c_master_init();
    mpu6050_init();

    ESP_LOGI(TAG, "MPU6050 initialized");

    while (1) {
        uint8_t data[14];
        if (mpu6050_read_bytes(MPU6050_ACCEL_XOUT_H, data, 14) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read sensor data");
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        // Raw sensor readings
        int16_t ax = read_word(data, 0);
        int16_t ay = read_word(data, 2);
        int16_t az = read_word(data, 4);
        int16_t temp_raw = read_word(data, 6);
        int16_t gx = read_word(data, 8);
        int16_t gy = read_word(data, 10);
        int16_t gz = read_word(data, 12);

        // Scaling factors
        float accel_scale = 9.81f / 4096.0f;  // ±8g → 4096 LSB/g
        float gyro_scale =
            (3.1415926f / 180.0f) / 65.5f;  // ±500°/s → 65.5 LSB/(°/s) → rad/s
        float temp = (temp_raw / 340.0f) + 36.53f;

        printf("Accel: X=%.2f Y=%.2f Z=%.2f m/s²\n", ax * accel_scale,
               ay * accel_scale, az * accel_scale);

        printf("Gyro:  X=%.2f Y=%.2f Z=%.2f rad/s\n", gx * gyro_scale,
               gy * gyro_scale, gz * gyro_scale);

        printf("Temp:  %.2f °C\n\n", temp);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
