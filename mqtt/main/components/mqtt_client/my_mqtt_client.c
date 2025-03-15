#include "my_mqtt_client.h"

#include <stdio.h>

#include "esp_log.h"

static const char *TAG = "MQTT_CLIENT";
static esp_mqtt_client_handle_t client;

static const char *TOPIC = "controller/command";
static const char *URI = "mqtt://192.168.1.100:1883";

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client_local = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected");
            esp_mqtt_client_subscribe(client_local, TOPIC, 0);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Disconnected");
            // TODO Implement retry logic here if needed
            break;
        case MQTT_EVENT_SUBSCRIBED:
            if (event->topic != NULL) {
                ESP_LOGI(TAG, "Subscribed to topic: %s", event->topic);
            } else {
                ESP_LOGI(
                    TAG,
                    "Subscribed to topic, topic name not returned in event");
            }
            break;
        case MQTT_EVENT_DATA:
            if (event->topic != NULL && event->data != NULL) {
                ESP_LOGI(TAG, "Received message on topic: %.*s",
                         event->topic_len, event->topic);
                ESP_LOGI(TAG, "Message data: %.*s", event->data_len,
                         event->data);
                // TODO handle message
            } else {
                ESP_LOGE(TAG, "Received message, but topic or data was null");
            }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type ==
                MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGE(TAG, "TCP transport error");
            }
            break;
        default:
            break;
    }
}

esp_err_t mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt5_cfg = {
        .broker.address.uri = URI,
        .session.protocol_ver = MQTT_PROTOCOL_V_5,
        .credentials.username = "",
        .credentials.authentication.password = "",
    };

    client = esp_mqtt_client_init(&mqtt5_cfg);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return ESP_FAIL;
    }

    esp_err_t err = esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID,
                                                   mqtt_event_handler, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register MQTT event handler, error: %d", err);
        return err;
    }

    err = esp_mqtt_client_start(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start MQTT client, error: %d", err);
        return err;
    }

    ESP_LOGI(TAG, "MQTT client started successfully");
    return ESP_OK;
}

void mqtt_publish(const char *topic, const char *message) {
    if (client == NULL) {
        ESP_LOGE(TAG, "Cannot publish: MQTT client not initialized");
        return;
    }
    int msg_id = esp_mqtt_client_publish(client, topic, message, 0, 0, 0);
    ESP_LOGI(TAG, "Published message to %s, msg_id=%d", topic, msg_id);
}
