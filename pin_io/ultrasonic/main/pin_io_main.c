#include <stdio.h>

#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BUTTON_PIN GPIO_NUM_4
#define LED_PIN GPIO_NUM_5

void app_main(void) {
    // Configure the button pin as input
    gpio_config_t button_config = {.mode = GPIO_MODE_INPUT,
                                   .pull_up_en = GPIO_PULLUP_DISABLE,
                                   .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                   .intr_type = GPIO_INTR_DISABLE,
                                   .pin_bit_mask = (1ULL << BUTTON_PIN)};
    esp_err_t ret = gpio_config(&button_config);
    if (ret != ESP_OK) {
        printf("Failed to configure button GPIO: %s\n", esp_err_to_name(ret));
        return;
    }
    printf("Button GPIO configured successfully!\n");

    // Configure the LED pin as output
    gpio_config_t led_config = {.mode = GPIO_MODE_OUTPUT,
                                .pull_up_en = GPIO_PULLUP_DISABLE,
                                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                .intr_type = GPIO_INTR_DISABLE,
                                .pin_bit_mask = (1ULL << LED_PIN)};
    ret = gpio_config(&led_config);
    if (ret != ESP_OK) {
        printf("Failed to configure led GPIO: %s\n", esp_err_to_name(ret));
        return;
    }
    printf("Led GPIO configured successfully!\n");

    while (1) {
        int buttonState = gpio_get_level(BUTTON_PIN);
        printf("Button State: %d\n", buttonState);

        if (buttonState == 1) {
            gpio_set_level(LED_PIN, 1);
        } else {
            gpio_set_level(LED_PIN, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

