idf_component_register(
    SRCS "wifi_utils.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi esp_event freertos
)
