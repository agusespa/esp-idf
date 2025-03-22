#include <stdio.h>
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"

#define TRIG_PIN GPIO_NUM_14
#define ECHO_PIN GPIO_NUM_15
#define SOUND_SPEED 0.034

void send_pulse() {
    gpio_set_level(TRIG_PIN, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIG_PIN, 0);
}

uint32_t measure_pulse() {
    // Wait for echo pin to go HIGH
    while (gpio_get_level(ECHO_PIN) == 0);
    uint32_t start = esp_timer_get_time();

    // Wait for echo pin to go LOW
    while (gpio_get_level(ECHO_PIN) == 1);
    uint32_t end = esp_timer_get_time();

    return end - start;
}

void app_main(void) {
    gpio_config_t trig_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << TRIG_PIN)
    };
    gpio_config(&trig_config);

    gpio_config_t echo_config = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << ECHO_PIN)
    };
    gpio_config(&echo_config);

    while (1) {
        send_pulse();
        uint32_t pulse_duration = measure_pulse();
        float distance = (pulse_duration * SOUND_SPEED) / 2.0;
        printf("Distance: %.2f cm\n", distance);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
