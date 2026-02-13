idf_component_register(SRCS "src/screen.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "st7789_driver"
                       PRIV_REQUIRES "gif_library" "utils"
                       EMBED_FILES "${CMAKE_SOURCE_DIR}/img/sara.gif")
