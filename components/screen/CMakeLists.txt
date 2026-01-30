idf_component_register(
    SRCS "src/screen.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES "gif_library" "st7789_driver" "utils"
    EMBED_FILES "${CMAKE_SOURCE_DIR}/img/sara.gif"
)
