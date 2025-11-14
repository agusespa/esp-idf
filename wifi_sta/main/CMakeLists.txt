idf_component_register(SRCS "wifi_sta_main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_event nvs_flash esp_wifi wifi_utils
)
