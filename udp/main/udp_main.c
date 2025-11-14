#include <stdio.h>

#include "components/udp_server/udp_server.h"
#include "wifi_utils.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

static const char* TAG = "UDP_MAIN";

void app_main() {
    esp_err_t nvs_err = nvs_flash_init();
    if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES ||
        nvs_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_err);

    ESP_LOGI(TAG, "Connecting to wi-fi ESP_WIFI_MODE_STA...");
    wifi_status_t status = wifi_init_sta();
    if (status == WIFI_STATUS_FAIL) {
        ESP_LOGE(TAG, "Wi-Fi connection failed. Stopping execution.");
        return;
    }

    ESP_LOGI(TAG, "Starting UDP server...");
    esp_err_t udp_err = udp_server_start();
    if (udp_err != ESP_OK) {
        ESP_LOGE(TAG, "UDP server initialization failed. Stopping execution.");
        return;
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
