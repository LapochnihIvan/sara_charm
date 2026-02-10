#include "server.h"

#include <stdbool.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <mdns.h>

#include "esp_http_server.h"
#include "utils/embed_file.h"


#define ACCEPT_ENCODING_BUF_LEN (32)

#define GET_URI_HANDLER(uri_path, handler_func)  \
    {                                            \
        .uri      = (uri_path),                  \
        .method   = HTTP_GET,                    \
        .handler  = (handler_func),              \
        .user_ctx = NULL                         \
    }

static void start_mdns_server(void);
static httpd_handle_t start_http_server(void);
static esp_err_t get_index_html_handler(httpd_req_t* req);
static esp_err_t get_main_wasm_handler(httpd_req_t* req);
static esp_err_t get_main_js_handler(httpd_req_t* req);
static esp_err_t get_file_handler_impl(httpd_req_t* req,
                                       const char* content_type,
                                       const uint8_t* file,
                                       size_t file_len);
static bool is_client_accepts_gzip(httpd_req_t* req);

server_handle_t start_server(void)
{
    start_mdns_server();

    return start_http_server();
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

static httpd_handle_t start_http_server(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    const httpd_uri_t index_html_uri =
        GET_URI_HANDLER("/", get_index_html_handler);
    const httpd_uri_t main_wasm_uri =
        GET_URI_HANDLER("/sara_charm_frontend_bg.wasm", get_main_wasm_handler);
    const httpd_uri_t main_js_uri =
        GET_URI_HANDLER("/sara_charm_frontend.js", get_main_js_handler);
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &index_html_uri);
        httpd_register_uri_handler(server, &main_wasm_uri);
        httpd_register_uri_handler(server, &main_js_uri);
        return server;
    }

    return NULL;
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
