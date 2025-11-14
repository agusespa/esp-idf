#include "udp_server.h"

#include <string.h>
#include <sys/param.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

static const char* TAG = "udp_server";
static int sock = -1;
static TaskHandle_t udp_task_handle = NULL;

static void udp_server_task(void* pvParameters) {
    char rx_buffer[128];
    char addr_str[128];
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;
    struct sockaddr_in dest_addr;

    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(UDP_PORT);

    sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "Socket created");

    int err = bind(sock, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        close(sock);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", UDP_PORT);

    while (1) {
        struct sockaddr_storage source_addr;
        socklen_t socklen = sizeof(source_addr);
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                          (struct sockaddr*)&source_addr, &socklen);

        if (len < 0) {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            break;
        } else {
            rx_buffer[len] = 0;

            if (source_addr.ss_family == PF_INET) {
                inet_ntoa_r(((struct sockaddr_in*)&source_addr)->sin_addr,
                           addr_str, sizeof(addr_str) - 1);
            }

            ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
            ESP_LOGI(TAG, "%s", rx_buffer);

            // Echo back the received data
            int err = sendto(sock, rx_buffer, len, 0,
                           (struct sockaddr*)&source_addr, sizeof(source_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
        }
    }

    if (sock != -1) {
        ESP_LOGE(TAG, "Shutting down socket");
        shutdown(sock, 0);
        close(sock);
    }
    vTaskDelete(NULL);
}

esp_err_t udp_server_start(void) {
    BaseType_t xReturned = xTaskCreate(udp_server_task, "udp_server",
                                       4096, NULL, 5, &udp_task_handle);
    if (xReturned != pdPASS) {
        ESP_LOGE(TAG, "Failed to create UDP server task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "UDP server started on port %d", UDP_PORT);
    return ESP_OK;
}

void udp_server_stop(void) {
    if (udp_task_handle != NULL) {
        vTaskDelete(udp_task_handle);
        udp_task_handle = NULL;
    }
    if (sock != -1) {
        shutdown(sock, 0);
        close(sock);
        sock = -1;
    }
    ESP_LOGI(TAG, "UDP server stopped");
}
