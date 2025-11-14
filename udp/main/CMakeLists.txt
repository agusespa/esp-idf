idf_component_register(
    SRCS "udp_main.c" "components/udp_server/udp_server.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES nvs_flash esp_event esp_wifi lwip wifi_utils
)
