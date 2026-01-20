idf_component_register(SRCS "src/gif.c"
                       INCLUDE_DIRS "include")

# AnimatedGIF hasn't official support ESP-IDF.
# __LINUX__ need to hack internal macro conditions
target_compile_definitions(${COMPONENT_LIB} PRIVATE __LINUX__)

# ESP32-C3 allow unaligned access to SRAM
target_compile_definitions(${COMPONENT_LIB} PUBLIC ALLOWS_UNALIGNED)
