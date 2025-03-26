#include <stdio.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MOTOR_PIN1 GPIO_NUM_18  // Motor direction pin 1
#define MOTOR_PIN2 GPIO_NUM_19  // Motor direction pin 2
#define MOTOR_PWM GPIO_NUM_5    // Motor speed control (PWM)

#define PWM_FREQ 1000  // 1kHz frequency
#define PWM_RES LEDC_TIMER_10_BIT
#define PWM_CHANNEL LEDC_CHANNEL_0

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

void set_motor_speed(uint32_t duty) {
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL);
}

void app_main(void) {
    gpio_config_t motor_pins = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << MOTOR_PIN1) | (1ULL << MOTOR_PIN2)};
    gpio_config(&motor_pins);

    ledc_timer_config_t pwm_timer = {.duty_resolution = PWM_RES,
                                     .freq_hz = PWM_FREQ,
                                     .speed_mode = LEDC_HIGH_SPEED_MODE,
                                     .timer_num = LEDC_TIMER_0};
    ledc_timer_config(&pwm_timer);

    ledc_channel_config_t pwm_channel = {.channel = PWM_CHANNEL,
                                         .duty = 0,
                                         .gpio_num = MOTOR_PWM,
                                         .speed_mode = LEDC_HIGH_SPEED_MODE,
                                         .hpoint = 0,
                                         .timer_sel = LEDC_TIMER_0};
    ledc_channel_config(&pwm_channel);

    while (1) {
        printf("Motor Forward\n");
        set_motor_direction(1);
        set_motor_speed(512);  // 50% duty cycle
        vTaskDelay(pdMS_TO_TICKS(2000));

        printf("Motor Stop\n");
        set_motor_direction(0);
        set_motor_speed(0);
        vTaskDelay(pdMS_TO_TICKS(1000));

        printf("Motor Reverse\n");
        set_motor_direction(-1);
        set_motor_speed(512);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
