idf_component_register(SRCS "src/gif.c"
                       INCLUDE_DIRS "include")

include(FetchContent)

FetchContent_Declare(
    AnimatedGIF
    GIT_REPOSITORY https://github.com/bitbank2/AnimatedGIF.git
    GIT_TAG 2.2.0
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/AnimatedGIF"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)

FetchContent_GetProperties(AnimatedGIF)
if(NOT AnimatedGIF_POPULATED)
    FetchContent_MakeAvailable(AnimatedGIF)
endif()

# AnimatedGIF hasn't official support ESP-IDF.
# __LINUX__ need to hack internal macro conditions
target_compile_definitions(${COMPONENT_LIB} PRIVATE __LINUX__)

# ESP32-C3 allow unaligned access to SRAM
target_compile_definitions(${COMPONENT_LIB} PUBLIC ALLOWS_UNALIGNED)
