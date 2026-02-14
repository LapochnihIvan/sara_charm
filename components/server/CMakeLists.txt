idf_component_register(
    SRCS "src/server.c"
    INCLUDE_DIRS "include"
    REQUIRES "esp_http_server"
    PRIV_REQUIRES "esp_wifi" "esp_netif" "utils"
    EMBED_FILES
        "${CMAKE_SOURCE_DIR}/frontend/dist/index.html.gz"
        "${CMAKE_SOURCE_DIR}/frontend/dist/sara_charm_frontend.js.gz"
        "${CMAKE_SOURCE_DIR}/frontend/dist/sara_charm_frontend_bg.wasm.gz"
)

include(FetchContent)

FetchContent_Populate(
    mydep
    GIT_REPOSITORY https://github.com/nanopb/nanopb.git
    GIT_TAG        nanopb-0.4.9.1
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/nanopb"
    QUIET
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/nanopb/extra")
find_package(Nanopb REQUIRED)

nanopb_generate_cpp(TARGET proto ${CMAKE_SOURCE_DIR}/proto/requests.proto)

target_link_libraries(${COMPONENT_LIB} PRIVATE proto)
