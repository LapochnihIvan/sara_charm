#include "server.h"

#include <stdbool.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <mdns.h>

#include "utils/embed_file.h"
#include "utils/esp_try.h"


#define ACCEPT_ENCODING_BUF_LEN (32)

typedef esp_err_t (*http_handler_t)(httpd_req_t*);

static void start_mdns_server(void);
static esp_err_t start_http_server(server_handle_t server);
static void add_get_handler(server_handle_t server,
                            const char* uri,
                            http_handler_t handler);
static esp_err_t get_index_html_handler(httpd_req_t* req);
static esp_err_t get_main_wasm_handler(httpd_req_t* req);
static esp_err_t get_main_js_handler(httpd_req_t* req);
static esp_err_t get_file_handler_impl(httpd_req_t* req,
                                       const char* content_type,
                                       const uint8_t* file,
                                       size_t file_len);
static bool is_client_accepts_gzip(httpd_req_t* req);

esp_err_t start_server(const server_handle_t server)
{
    start_mdns_server();

    return start_http_server(server);
}

void stop_server(const server_handle_t server)
{
    mdns_free();

    httpd_stop(server);
}

static void start_mdns_server(void)
{
    mdns_init();
    mdns_hostname_set("sara_charm");
    mdns_instance_name_set("Sara charm");
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
}

static esp_err_t start_http_server(const server_handle_t server)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_TRY(httpd_start(server, &config));

    add_get_handler(server, "/", get_index_html_handler);
    add_get_handler(
        server,
        "/sara_charm_frontend_bg.wasm",
        get_main_wasm_handler
    );
    add_get_handler(server, "/sara_charm_frontend.js", get_main_js_handler);

    return ESP_OK;
}

static void add_get_handler(const server_handle_t server,
                            const char* const uri,
                            const http_handler_t handler)
{
    const httpd_uri_t uri_handler = {
        .uri      = uri,
        .method   = HTTP_GET,
        .handler  = handler,
        .user_ctx = NULL
    };
    (void)httpd_register_uri_handler(server, &uri_handler);
}

static esp_err_t get_index_html_handler(httpd_req_t* const req)
{
    EXTERN_EMBED_ARCHIVED_FILE(index, html, gz);
    const embed_file_t index_html = GET_EMBED_ARCHIVED_FILE(index, html, gz);
    return get_file_handler_impl(
        req,
        "text/html",
        index_html.data,
        index_html.len
    );
}

static esp_err_t get_main_wasm_handler(httpd_req_t* const req)
{
    EXTERN_EMBED_ARCHIVED_FILE(sara_charm_frontend_bg, wasm, gz);
    const embed_file_t main_wasm =
        GET_EMBED_ARCHIVED_FILE(sara_charm_frontend_bg, wasm, gz);
    return get_file_handler_impl(
        req,
        "application/wasm",
        main_wasm.data,
        main_wasm.len
    );
}

static esp_err_t get_main_js_handler(httpd_req_t* const req)
{
    EXTERN_EMBED_ARCHIVED_FILE(sara_charm_frontend, js, gz);
    const embed_file_t main_js =
        GET_EMBED_ARCHIVED_FILE(sara_charm_frontend, js, gz);
    return get_file_handler_impl(
        req,
        "application/javascript",
        main_js.data,
        main_js.len
    );
}

static esp_err_t get_file_handler_impl(httpd_req_t* const req,
                                       const char* const content_type,
                                       const uint8_t* const file,
                                       const size_t file_len)
{
    if (!is_client_accepts_gzip(req))
    {
        httpd_resp_set_status(req, "406 Not Acceptable");

        return httpd_resp_send(req, NULL, 0);
    }
    else
    {
        httpd_resp_set_type(req, content_type);
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");

        return httpd_resp_send(req, (const char *)file, file_len);
    }
}

static bool is_client_accepts_gzip(httpd_req_t* const req)
{
    char hdr_buf[ACCEPT_ENCODING_BUF_LEN];

    httpd_req_get_hdr_value_str(
        req,
        "Accept-Encoding",
        hdr_buf,
        ACCEPT_ENCODING_BUF_LEN - 1
    );

    return strstr(hdr_buf, "gzip") != NULL || strchr(hdr_buf, '*') != NULL;
}
