idf_component_register(SRCS "src/screen.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_driver_gpio esp_driver_spi gif_library)
