idf_component_register(
    SRCS "http_main.c" "components/http_server/http_server.c" "components/wifi_utils/wifi_utils.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES nvs_flash esp_event esp_wifi esp_http_server
)
