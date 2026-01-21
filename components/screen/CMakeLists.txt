idf_component_register(SRCS "src/screen.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES esp_driver_gpio esp_driver_spi gif_library
                       EMBED_FILES "${CMAKE_SOURCE_DIR}/img/sara.gif")
