idf_component_register(
    SRCS "mqtt_main.c" "components/wifi_utils/wifi_utils.c" "components/mqtt_client/my_mqtt_client.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES nvs_flash esp_event esp_wifi mqtt
)
