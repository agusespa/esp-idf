#include <stdio.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MOTOR_PIN1 GPIO_NUM_16  // Motor 1 direction pin 1
#define MOTOR_PIN2 GPIO_NUM_17  // Motor 1 direction pin 2

void set_motor_direction(int direction) {
    if (direction == 1) {
        gpio_set_level(MOTOR_PIN1, 1);
        gpio_set_level(MOTOR_PIN2, 0);
    } else if (direction == -1) {
        gpio_set_level(MOTOR_PIN1, 0);
        gpio_set_level(MOTOR_PIN2, 1);
    } else {
        gpio_set_level(MOTOR_PIN1, 0);
        gpio_set_level(MOTOR_PIN2, 0);
    }
}

void app_main(void) {
    gpio_config_t motor_pins = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << MOTOR_PIN1) | (1ULL << MOTOR_PIN2)};
    gpio_config(&motor_pins);

    while (1) {
        printf("Motor Forward\n");
        set_motor_direction(1);
        vTaskDelay(pdMS_TO_TICKS(3000));

        printf("Motor Stop\n");
        set_motor_direction(0);
        vTaskDelay(pdMS_TO_TICKS(1000));

        printf("Motor Reverse\n");
        set_motor_direction(-1);
        vTaskDelay(pdMS_TO_TICKS(3000));

        printf("Motor Stop\n");
        set_motor_direction(0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
