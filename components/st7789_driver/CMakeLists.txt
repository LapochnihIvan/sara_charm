idf_component_register(
    SRCS "src/st7789_driver.c"
    INCLUDE_DIRS "include"
    REQUIRES "esp_driver_spi"
    PRIV_REQUIRES "esp_driver_gpio" "utils"
)
