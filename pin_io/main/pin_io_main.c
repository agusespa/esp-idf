#include <inttypes.h>
#include <stdio.h>

#include "driver/gpio.h"
#include "esp_chip_info.h"
#include "esp_err.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BUTTON_PIN GPIO_NUM_4
#define LED_PIN GPIO_NUM_5

void restart_sequence() {
    for (int i = 3; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}

void print_details() {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ", CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154)
               ? ", 802.15.4 (Zigbee/Thread)"
               : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);

    uint32_t flash_size;
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }
    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded"
                                                         : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n",
           esp_get_minimum_free_heap_size());
}

void app_main(void) {
    print_details();
    /* restart_sequence(); */

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

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

