#include <stdint.h>
#include <stdio.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *MOTOR_TAG = "MOTOR_CONTROL";
static const char *ENCODER_TAG = "ENCODER";

/* Motor control definitions */
#define MOTOR_PIN1 GPIO_NUM_14
#define MOTOR_PIN2 GPIO_NUM_12
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL_PIN1 LEDC_CHANNEL_0
#define LEDC_CHANNEL_PIN2 LEDC_CHANNEL_1
#define LEDC_DUTY_RES LEDC_TIMER_10_BIT
#define LEDC_FREQUENCY 1000
#define MAX_DUTY ((1 << LEDC_DUTY_RES) - 1)

/* Encoder definitions */
#define ENCODER_PIN GPIO_NUM_2
#define ENCODER_LOG_INTERVAL_MS 1000
#define ENCODER_MAX_COUNT INT32_MAX
#define ENCODER_DEBOUNCE_US 1000

/* Encoder state */
typedef struct {
    volatile int32_t pulse_count;
    gpio_num_t encoder_pin;
    bool initialized;
    portMUX_TYPE spinlock;
    volatile int64_t last_pulse_time_us;
} encoder_state_t;

static encoder_state_t encoder_state = {
    .pulse_count = 0,
    .encoder_pin = ENCODER_PIN,
    .initialized = false,
    .spinlock = portMUX_INITIALIZER_UNLOCKED,
    .last_pulse_time_us = 0,
};

/* Encoder ISR */
static void IRAM_ATTR encoder_isr_handler(void *arg) {
    int64_t now = esp_timer_get_time();

    if (now - encoder_state.last_pulse_time_us < ENCODER_DEBOUNCE_US) {
        return;  // debounce
    }

    encoder_state.last_pulse_time_us = now;

    portENTER_CRITICAL_ISR(&encoder_state.spinlock);
    encoder_state.pulse_count++;
    if (encoder_state.pulse_count >= ENCODER_MAX_COUNT) {
        ESP_EARLY_LOGW(ENCODER_TAG, "Pulse counter overflow, resetting");
        encoder_state.pulse_count = 0;
    }
    portEXIT_CRITICAL_ISR(&encoder_state.spinlock);
}

esp_err_t encoder_init(void) {
    static bool isr_service_installed = false;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << encoder_state.encoder_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE,
    };

    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(ENCODER_TAG, "Failed to configure encoder GPIO pin");
        return ret;
    }

    if (!isr_service_installed) {
        ret = gpio_install_isr_service(0);
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
            ESP_LOGE(ENCODER_TAG, "Failed to install GPIO ISR service");
            return ret;
        }
        isr_service_installed = true;
    }

    ret = gpio_isr_handler_add(encoder_state.encoder_pin, encoder_isr_handler,
                               NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(ENCODER_TAG, "Failed to add encoder ISR handler");
        return ret;
    }

    encoder_state.pulse_count = 0;
    encoder_state.last_pulse_time_us = esp_timer_get_time();
    encoder_state.initialized = true;

    ESP_LOGI(ENCODER_TAG, "Encoder initialized on GPIO %d",
             encoder_state.encoder_pin);
    return ESP_OK;
}

long encoder_get_count(void) {
    long count;
    portENTER_CRITICAL(&encoder_state.spinlock);
    count = encoder_state.pulse_count;
    portEXIT_CRITICAL(&encoder_state.spinlock);
    return count;
}

void encoder_print_status(void) {
    if (!encoder_state.initialized) {
        ESP_LOGW(ENCODER_TAG, "Encoder not initialized");
        return;
    }

    long count = encoder_get_count();
    ESP_LOGI(ENCODER_TAG, "Encoder Status:");
    ESP_LOGI(ENCODER_TAG, "  - GPIO Pin: %d", encoder_state.encoder_pin);
    ESP_LOGI(ENCODER_TAG, "  - Current Count: %ld", count);
    ESP_LOGI(ENCODER_TAG, "  - Status: Initialized");
}

void encoder_print_pulse_rate(uint32_t time_period_ms) {
    static int32_t last_count = 0;
    static uint32_t last_time = 0;
    uint32_t current_time = xTaskGetTickCount();

    if (!encoder_state.initialized) return;

    if (current_time - last_time >= pdMS_TO_TICKS(time_period_ms)) {
        int32_t current_count = encoder_get_count();
        long count_diff = current_count - last_count;
        float time_diff_sec =
            (float)(current_time - last_time) / configTICK_RATE_HZ;
        float pulse_rate = count_diff / time_diff_sec;

        ESP_LOGI(ENCODER_TAG,
                 "Pulse Rate: %.2f pulses/sec (Count: %ld, Time: %.2fs)",
                 pulse_rate, count_diff, time_diff_sec);

        last_count = current_count;
        last_time = current_time;
    }
}

esp_err_t drive_motor(int direction, int speed) {
    if (speed < 0 || speed > MAX_DUTY) {
        ESP_LOGE(MOTOR_TAG, "Invalid speed: %d", speed);
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = ESP_OK;

    if (direction == 0) {
        ret |= ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PIN1, 0);
        ret |= ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PIN1);
        ret |= ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PIN2, 0);
        ret |= ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PIN2);
        ESP_LOGI(MOTOR_TAG, "Motor Stop");
    } else if (direction == 1) {
        ret |= ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PIN1, speed);
        ret |= ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PIN1);
        ret |= ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PIN2, 0);
        ret |= ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PIN2);
        ESP_LOGI(MOTOR_TAG, "Motor Forward at speed %d", speed);
    } else if (direction == -1) {
        ret |= ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PIN1, 0);
        ret |= ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PIN1);
        ret |= ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_PIN2, speed);
        ret |= ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_PIN2);
        ESP_LOGI(MOTOR_TAG, "Motor Reverse at speed %d", speed);
    }

    return ret;
}

void motor_task(void *param) {
    while (1) {
        drive_motor(1, 512);
        vTaskDelay(pdMS_TO_TICKS(3000));

        drive_motor(1, 768);
        vTaskDelay(pdMS_TO_TICKS(3000));

        drive_motor(1, 1023);
        vTaskDelay(pdMS_TO_TICKS(3000));

        drive_motor(0, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));

        drive_motor(-1, 512);
        vTaskDelay(pdMS_TO_TICKS(3000));

        drive_motor(-1, 768);
        vTaskDelay(pdMS_TO_TICKS(3000));

        drive_motor(-1, 1023);
        vTaskDelay(pdMS_TO_TICKS(3000));

        drive_motor(0, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void encoder_logger_task(void *param) {
    long last_encoder_count = 0;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(ENCODER_LOG_INTERVAL_MS));

        if (encoder_state.initialized) {
            long current_encoder_count = encoder_get_count();
            long diff = current_encoder_count - last_encoder_count;

            ESP_LOGI(ENCODER_TAG, "Encoder count: %ld (change: %ld)",
                     current_encoder_count, diff);
            encoder_print_pulse_rate(ENCODER_LOG_INTERVAL_MS);

            last_encoder_count = current_encoder_count;
        }
    }
}

void app_main(void) {
    // PWM timer config
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .duty_resolution = LEDC_DUTY_RES,
        .timer_num = LEDC_TIMER,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // PWM channels config
    ledc_channel_config_t ledc_pin1_channel = {
        .channel = LEDC_CHANNEL_PIN1,
        .duty = 0,
        .gpio_num = MOTOR_PIN1,
        .speed_mode = LEDC_MODE,
        .timer_sel = LEDC_TIMER,
        .hpoint = 0,
    };
    ledc_channel_config_t ledc_pin2_channel = {
        .channel = LEDC_CHANNEL_PIN2,
        .duty = 0,
        .gpio_num = MOTOR_PIN2,
        .speed_mode = LEDC_MODE,
        .timer_sel = LEDC_TIMER,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_pin1_channel));
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_pin2_channel));

    if (encoder_init() != ESP_OK) {
        ESP_LOGE(ENCODER_TAG, "Encoder init failed");
    } else {
        encoder_print_status();
    }

    xTaskCreate(motor_task, "motor_task", 2048, NULL, 5, NULL);
    xTaskCreate(encoder_logger_task, "encoder_logger_task", 2048, NULL, 5,
                NULL);
}
