#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"

#define TRIG_PIN GPIO_NUM_18
#define ECHO_PIN GPIO_NUM_19
#define SOUND_SPEED 0.034
#define TIMEOUT_US 50000
#define FILTER_SIZE 5

typedef struct {
    float buffer[FILTER_SIZE];
    int index;
} DistanceFilter;

DistanceFilter filter = {{0}, 0};

static float get_median(float *buffer, int size) {
    float temp[FILTER_SIZE];
    memcpy(temp, buffer, sizeof(float) * size);

    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (temp[j] > temp[j + 1]) {
                float t = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = t;
            }
        }
    }
    return temp[size / 2];
}

static float update_filter(DistanceFilter *f, float new_val) {
    f->buffer[f->index] = new_val;
    f->index = (f->index + 1) % FILTER_SIZE;
    return get_median(f->buffer, FILTER_SIZE);
}

void send_pulse() {
    gpio_set_level(TRIG_PIN, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIG_PIN, 0);
}

uint32_t measure_pulse() {
    uint32_t start_time = esp_timer_get_time();
    uint32_t timeout_time = start_time + TIMEOUT_US;
    while (gpio_get_level(ECHO_PIN) == 0) {
        if (esp_timer_get_time() > timeout_time) {
            return 0;
        }
    }

    start_time = esp_timer_get_time();
    timeout_time = start_time + TIMEOUT_US;
    while (gpio_get_level(ECHO_PIN) == 1) {
        if (esp_timer_get_time() > timeout_time) {
            return 0;
        }
    }

    return esp_timer_get_time() - start_time;
}

void app_main(void) {
    gpio_config_t trig_config = {.mode = GPIO_MODE_OUTPUT,
                                 .pull_up_en = GPIO_PULLUP_DISABLE,
                                 .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                 .intr_type = GPIO_INTR_DISABLE,
                                 .pin_bit_mask = (1ULL << TRIG_PIN)};
    gpio_config(&trig_config);

    gpio_config_t echo_config = {.mode = GPIO_MODE_INPUT,
                                 .pull_up_en = GPIO_PULLUP_DISABLE,
                                 .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                 .intr_type = GPIO_INTR_DISABLE,
                                 .pin_bit_mask = (1ULL << ECHO_PIN)};
    gpio_config(&echo_config);

    while (1) {
        send_pulse();
        uint32_t pulse_duration = measure_pulse();
        float distance = (pulse_duration * SOUND_SPEED) / 2.0;

        float filtered_distance = update_filter(&filter, distance);
        printf("Filtered Distance: %.2f cm\n", filtered_distance);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
