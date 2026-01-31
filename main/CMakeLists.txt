idf_component_register(
    SRCS "src/main.c"
    INCLUDE_DIRS ""
    PRIV_REQUIRES "esp_driver_gpio" "nvs_flash" "screen" "wifi_point"
)
