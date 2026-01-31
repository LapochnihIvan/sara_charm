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
