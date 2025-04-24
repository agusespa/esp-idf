#include <stdio.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MOTOR_PIN1 GPIO_NUM_14
#define MOTOR_PIN2 GPIO_NUM_12
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL_PIN1 LEDC_CHANNEL_0
#define LEDC_CHANNEL_PIN2 LEDC_CHANNEL_1
#define LEDC_DUTY_RES LEDC_TIMER_10_BIT
#define LEDC_FREQUENCY 1000
#define MAX_DUTY ((1 << LEDC_DUTY_RES) - 1)

static const char *TAG = "MOTOR_CONTROL";

esp_err_t drive_motor(int direction, int speed) {
    if (speed < 0 || speed > MAX_DUTY) {
        ESP_LOGE(TAG, "Invalid speed value: %d. Must be between 0 and %d",
                 speed, MAX_DUTY);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = ESP_OK;

    if (direction == 0) {
        // Stop the motor
        ret |= ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PIN1, 0);
        ret |= ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PIN1);
        ret |= ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PIN2, 0);
        ret |= ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PIN2);
        ESP_LOGI(TAG, "Motor Stop");
    } else if (direction == 1) {
        // Forward
        ret |= ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PIN1, speed);
        ret |= ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PIN1);
        ret |= ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PIN2, 0);
        ret |= ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PIN2);
        ESP_LOGI(TAG, "Motor Forward at speed %d", speed);
    } else if (direction == -1) {
        // Reverse
        ret |= ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PIN1, 0);
        ret |= ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PIN1);
        ret |= ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PIN2, speed);
        ret |= ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PIN2);
        ESP_LOGI(TAG, "Motor Reverse at speed %d", speed);
    }

    return ret;
}

void app_main(void) {
    // Initialize LEDC timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Configure LEDC channels for both pins
    ledc_channel_config_t ledc_pin1_channel = {
        .channel = LEDC_CHANNEL_PIN1,
        .duty = 0,
        .gpio_num = MOTOR_PIN1,
        .speed_mode = LEDC_MODE,
        .timer_sel = LEDC_TIMER,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_pin1_channel));

    ledc_channel_config_t ledc_pin2_channel = {
        .channel = LEDC_CHANNEL_PIN2,
        .duty = 0,
        .gpio_num = MOTOR_PIN2,
        .speed_mode = LEDC_MODE,
        .timer_sel = LEDC_TIMER,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_pin2_channel));

    while (1) {
        printf("Forward at slow speed\n");
        ESP_ERROR_CHECK(drive_motor(1, 256));
        vTaskDelay(pdMS_TO_TICKS(3000));
        printf("Forward at medium speed\n");
        ESP_ERROR_CHECK(drive_motor(1, 512));
        vTaskDelay(pdMS_TO_TICKS(3000));
        printf("Forward at full speed\n");
        ESP_ERROR_CHECK(drive_motor(1, 1023));
        vTaskDelay(pdMS_TO_TICKS(3000));

        printf("Stop\n");
        ESP_ERROR_CHECK(drive_motor(0, 0));
        vTaskDelay(pdMS_TO_TICKS(1000));

        printf("Reverse at slow speed\n");
        ESP_ERROR_CHECK(drive_motor(-1, 256));
        vTaskDelay(pdMS_TO_TICKS(3000));
        printf("Reverse at medium speed\n");
        ESP_ERROR_CHECK(drive_motor(-1, 512));
        vTaskDelay(pdMS_TO_TICKS(3000));
        printf("Reverse at full speed\n");
        ESP_ERROR_CHECK(drive_motor(-1, 1023));
        vTaskDelay(pdMS_TO_TICKS(3000));

        printf("Stop\n");
        ESP_ERROR_CHECK(drive_motor(0, 0));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
