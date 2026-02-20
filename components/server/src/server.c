#include "server.h"

#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <mdns.h>

#include <pb_encode.h>
#include <pb_decode.h>

#include "wifi_point.h"
#include "utils/embed_file.h"
#include "utils/esp_try.h"

#include "messages.pb.h"


#define ACCEPT_ENCODING_BUF_LEN (32)

typedef esp_err_t (*http_handler_t)(httpd_req_t*);

static esp_err_t start_mdns_server(void);
static esp_err_t start_http_server(server_handle_t* server);
static void add_get_handler(server_handle_t server,
                            const char* uri,
                            http_handler_t handler);
static void add_post_handler(server_handle_t server,
                             const char* uri,
                             http_handler_t handler);
static esp_err_t get_index_html_handler(httpd_req_t* req);
static esp_err_t get_main_wasm_handler(httpd_req_t* req);
static esp_err_t get_main_js_handler(httpd_req_t* req);
static esp_err_t get_favicon_ico_handler(httpd_req_t* req);
static esp_err_t get_wifi_settings_handler(httpd_req_t* req);
static esp_err_t change_wifi_settings_handler(httpd_req_t* req);
static esp_err_t send_proto(const void* msg,
                            uint8_t* msg_buf,
                            size_t msg_len,
                            const pb_msgdesc_t* msg_info,
                            httpd_req_t* req);
static esp_err_t receive_proto(void* msg,
                               uint8_t* msg_buf,
                               size_t msg_len,
                               const pb_msgdesc_t* msg_info,
                               httpd_req_t* req);
static void add_handler_impl(server_handle_t server,
                             const char* uri,
                             httpd_method_t method,
                             http_handler_t handler);
static esp_err_t get_gzip_file_handler_impl(httpd_req_t* req,
                                            const char* content_type,
                                            const embed_file_t* file);
static esp_err_t get_file_handler_impl(httpd_req_t* req,
                                       const char* content_type,
                                       const embed_file_t* file);
static bool is_client_accepts_gzip(httpd_req_t* req);

esp_err_t start_server(server_handle_t* const server)
{
    ESP_TRY(start_mdns_server());

    const esp_err_t res = start_http_server(server);
    if (res != ESP_OK)
    {
        mdns_free();
    }

    return res;
}

void stop_server(const server_handle_t server)
{
    mdns_free();

    httpd_stop(server);
}

static esp_err_t start_mdns_server(void)
{
    ESP_TRY(mdns_init());
    (void)mdns_hostname_set("sara_charm");
    (void)mdns_instance_name_set("Sara charm");

    return mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
}

static esp_err_t start_http_server(server_handle_t* const server)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
    ESP_TRY(httpd_start(server, &config));

    add_get_handler(*server, "/", get_index_html_handler);
    add_get_handler(
        *server,
        "/sara_charm_frontend_bg.wasm",
        get_main_wasm_handler
    );
    add_get_handler(*server, "/sara_charm_frontend.js", get_main_js_handler);
    add_get_handler(*server, "/favicon.ico", get_favicon_ico_handler);

    add_get_handler(*server, "/api/wifi_settings", get_wifi_settings_handler);
    add_post_handler(
        *server,
        "/api/wifi_settings",
        change_wifi_settings_handler
    );

    return ESP_OK;
}

static void add_get_handler(const server_handle_t server,
                            const char* const uri,
                            const http_handler_t handler)
{
    add_handler_impl(server, uri, HTTP_GET, handler);
}

static void add_post_handler(const server_handle_t server,
                             const char* const uri,
                             const http_handler_t handler)
{
    add_handler_impl(server, uri, HTTP_POST, handler);
}

static esp_err_t get_index_html_handler(httpd_req_t* const req)
{
    EXTERN_EMBED_ARCHIVED_FILE(index, html, gz);
    const embed_file_t index_html = GET_EMBED_ARCHIVED_FILE(index, html, gz);
    return get_gzip_file_handler_impl(
        req,
        "text/html",
        &index_html
    );
}

static esp_err_t get_main_wasm_handler(httpd_req_t* const req)
{
    EXTERN_EMBED_ARCHIVED_FILE(sara_charm_frontend_bg, wasm, gz);
    const embed_file_t main_wasm =
        GET_EMBED_ARCHIVED_FILE(sara_charm_frontend_bg, wasm, gz);
    return get_gzip_file_handler_impl(
        req,
        "application/wasm",
        &main_wasm
    );
}

static esp_err_t get_main_js_handler(httpd_req_t* const req)
{
    EXTERN_EMBED_ARCHIVED_FILE(sara_charm_frontend, js, gz);
    const embed_file_t main_js =
        GET_EMBED_ARCHIVED_FILE(sara_charm_frontend, js, gz);
    return get_gzip_file_handler_impl(
        req,
        "application/javascript",
        &main_js
    );
}

static esp_err_t get_favicon_ico_handler(httpd_req_t* const req)
{
    EXTERN_EMBED_FILE(favicon, ico);
    const embed_file_t favicon_ico = GET_EMBED_FILE(favicon, ico);
    return get_file_handler_impl(
        req,
        "image/x-icon",
        &favicon_ico
    );
}

static esp_err_t get_wifi_settings_handler(httpd_req_t* const req)
{
    messages_WiFiSettings settings;
    wifi_point_get_settings(settings.ssid, settings.password);

    uint8_t msg_buf[messages_WiFiSettings_size];
    return send_proto(
        (void*)&settings,
        msg_buf,
        messages_WiFiSettings_size,
        &messages_WiFiSettings_msg,
        req
    );
}

static esp_err_t change_wifi_settings_handler(httpd_req_t* const req)
{
    if (req->content_len > messages_WiFiSettings_size)
    {
        httpd_resp_send_err(req, HTTPD_413_CONTENT_TOO_LARGE, NULL);
    }

    messages_WiFiSettings settings;
    uint8_t msg_buf[messages_WiFiSettings_size];
    esp_err_t res = receive_proto(
        (void*)&settings, msg_buf,
        req->content_len,
        &messages_WiFiSettings_msg,
        req
    );
    if (res != ESP_OK)
    {
        return ESP_OK;
    }

    res = wifi_point_change_settings(
        settings.ssid,
        strlen(settings.ssid),
        settings.password,
        strlen(settings.password)
    );

    if (res != ESP_OK)
    {
        return httpd_resp_send_err(
            req,
            res == ESP_ERR_WIFI_PASSWORD ? 
                HTTPD_400_BAD_REQUEST :
                HTTPD_500_INTERNAL_SERVER_ERROR, 
            esp_err_to_name(res)
        );
    }

    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t send_proto(const void* const msg,
                            uint8_t* const msg_buf,
                            const size_t msg_len,
                            const pb_msgdesc_t* const msg_info,
                            httpd_req_t* const req)
{
    pb_ostream_t proto_encoder = pb_ostream_from_buffer(msg_buf, msg_len);
    (void)pb_encode(&proto_encoder, msg_info, msg);

    return httpd_resp_send(req, (char*)msg_buf, proto_encoder.bytes_written);
}

static esp_err_t receive_proto(void* const msg,
                               uint8_t* const msg_buf,
                               const size_t msg_len,
                               const pb_msgdesc_t* const msg_info,
                               httpd_req_t* const req)
{
    const int receive_res = httpd_req_recv(req, (char*)msg_buf, msg_len);
    if (receive_res == HTTPD_SOCK_ERR_TIMEOUT)
    {
        ESP_TRY(httpd_resp_send_408(req));

        return ESP_ERR_TIMEOUT;
    }
    else if (receive_res == HTTPD_SOCK_ERR_FAIL)
    {
        ESP_TRY(httpd_resp_send_500(req));

        return ESP_FAIL;
    }

    pb_istream_t proto_decoder = pb_istream_from_buffer(msg_buf, msg_len);
    if (!pb_decode(&proto_decoder, msg_info, msg))
    {
        ESP_TRY(httpd_resp_send_err(
            req,
            HTTPD_400_BAD_REQUEST,
            PB_GET_ERROR(&proto_decoder)
        ));

        return ESP_ERR_INVALID_ARG;
    }

    return ESP_OK;
}

static void add_handler_impl(const server_handle_t server,
                             const char* const uri,
                             const httpd_method_t method,
                             const http_handler_t handler)
{
    const httpd_uri_t uri_handler = {
        .uri      = uri,
        .method   = method,
        .handler  = handler,
        .user_ctx = NULL
    };
    (void)httpd_register_uri_handler(server, &uri_handler);
}

static esp_err_t get_gzip_file_handler_impl(httpd_req_t* const req,
                                            const char* const content_type,
                                            const embed_file_t* const file)
{
    if (!is_client_accepts_gzip(req))
    {
        return httpd_resp_send_custom_err(
            req,
            "406 Not Acceptable",
            "Only gzip encoding supported"
        );
    }

    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");

    return get_file_handler_impl(req, content_type, file);
}

static esp_err_t get_file_handler_impl(httpd_req_t* const req,
                                       const char* const content_type,
                                       const embed_file_t* const file)
{
    (void)httpd_resp_set_type(req, content_type);

    return httpd_resp_send(req, (const char *)file->data, file->len);
}

static bool is_client_accepts_gzip(httpd_req_t* const req)
{
    char hdr_buf[ACCEPT_ENCODING_BUF_LEN];

    (void)httpd_req_get_hdr_value_str(
        req,
        "Accept-Encoding",
        hdr_buf,
        ACCEPT_ENCODING_BUF_LEN
    );

    return strstr(hdr_buf, "gzip") != NULL || strchr(hdr_buf, '*') != NULL;
}
