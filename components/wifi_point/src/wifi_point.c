#include "wifi_point.h"

#include <string.h>

#include <nvs.h>

#include "utils/esp_try.h"


#define CHANNEL_NUM (6)

#define MIN_SSID_LEN       (1)
#define MIN_PASSPHRASE_LEN (8)

#define NVS_NAMESPACE ("wifi")
#define NVS_SSID_KEY ("ssid")
#define NVS_PASSWORD_KEY ("pwd")

static esp_err_t init_netif(void);
static esp_err_t start_wifi(void);
static esp_err_t save_ssid(nvs_handle_t nvs_handle,
                           const char* data,
                           uint8_t len);
static esp_err_t save_password(nvs_handle_t nvs_handle,
                               const char* data,
                               uint8_t len);
static esp_err_t load_ssid(nvs_handle_t nvs_handle, uint8_t* dest, size_t* len);
static esp_err_t load_password(nvs_handle_t nvs_handle,
                               uint8_t* dest,
                               size_t* len);
static inline wifi_auth_mode_t get_auth_mode(uint8_t password_len);
static inline void deauth_all_users(void);

esp_err_t wifi_point_start(wifi_point_t* const self)
{
    ESP_TRY(init_netif());

    self->_netif_handle = esp_netif_create_default_wifi_ap();

    esp_err_t res = start_wifi();
    if (res != ESP_OK)
    {
        esp_netif_destroy(self->_netif_handle);
    }

    return res;
}

void wifi_point_stop(wifi_point_t* const self)
{
    deauth_all_users();
    (void)esp_wifi_stop();
    (void)esp_wifi_deinit();
    esp_netif_destroy(self->_netif_handle);
}

void wifi_point_get_settings(char* const ssid, char* const password)
{
    wifi_config_t config;
    (void)esp_wifi_get_config(ESP_IF_WIFI_AP, &config);

    memcpy((void*)ssid, (void*)config.ap.ssid, config.ap.ssid_len);
    ssid[config.ap.ssid_len] = '\0';
    strlcpy(password, (char*)config.ap.password, MAX_PASSPHRASE_LEN);
}

esp_err_t wifi_point_validate_settings(const uint8_t ssid_len,
                                       const uint8_t password_len)
{
    if (ssid_len < MIN_SSID_LEN || ssid_len > MAX_SSID_LEN)
    {
        return ESP_ERR_WIFI_SSID;
    }

    if (password_len > 0
        && (password_len < MIN_PASSPHRASE_LEN
            || password_len > MIN_PASSPHRASE_LEN))
    {
        return ESP_ERR_WIFI_PASSWORD;
    }

    return ESP_OK;
}

void wifi_point_change_settings(const char* const ssid,
                                const uint8_t ssid_len,
                                const char* const password,
                                const uint8_t password_len)
{
    wifi_config_t config;
    (void)esp_wifi_get_config(ESP_IF_WIFI_AP, &config);

    memcpy((void*)config.ap.ssid, (const void*)ssid, ssid_len);
    config.ap.ssid_len = ssid_len;

    memcpy((void*)config.ap.password, (const void*)password, password_len);
    config.ap.password[password_len] = '\0';
    config.ap.authmode = get_auth_mode(password_len);

    nvs_handle_t nvs_handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle) == ESP_OK)
    {
        esp_err_t res = save_ssid(nvs_handle, ssid, ssid_len);
        if (res == ESP_OK)
        {
            (void)save_password(nvs_handle, password, password_len);
        }

        nvs_close(nvs_handle);
    }

    deauth_all_users();

    (void)esp_wifi_set_config(ESP_IF_WIFI_AP, &config);
}

static esp_err_t init_netif(void)
{
    ESP_TRY(esp_netif_init());
    
    return esp_event_loop_create_default();
}

#define CONFIG_WIFI_DEFAULT_PASSWORD_LEN (sizeof(CONFIG_WIFI_DEFAULT_PASSWORD))
#include "esp_log.h"
static esp_err_t start_wifi(void)
{
    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_TRY(esp_wifi_init(&init_config));
    ESP_TRY(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_TRY(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t config = {
        .ap = {
            .ssid = CONFIG_WIFI_DEFAULT_SSID,
            .ssid_len = sizeof(CONFIG_WIFI_DEFAULT_SSID) - 1,
            .channel = CONFIG_WIFI_CHANNEL,
#ifdef CONFIG_WIFI_HIDDEN
            .ssid_hidden = 1,
#endif //CONFIG_WIFI_HIDDEN
            .password = CONFIG_WIFI_DEFAULT_PASSWORD,
            .max_connection = 1,
            .authmode = get_auth_mode(
                (uint8_t)(sizeof(CONFIG_WIFI_DEFAULT_PASSWORD) - 1)
            ),
            .pmf_cfg = {
                .required = false,
            },
        },
    };

    nvs_handle_t nvs_handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle) == ESP_OK)
    {
        size_t len;
        if (load_ssid(nvs_handle, config.ap.ssid, &len) == ESP_OK)
        {
            config.ap.ssid_len = (uint8_t)len;
        }
        if (load_password(nvs_handle, config.ap.password, &len) == ESP_OK)
        {
            config.ap.password[len] = '\0';
            config.ap.authmode = get_auth_mode((uint8_t)len);
        }

        nvs_close(nvs_handle);
    }

    ESP_LOGW(__FUNCTION__, "Password: %s", config.ap.password);

    ESP_TRY(esp_wifi_set_config(WIFI_IF_AP, &config));

    return esp_wifi_start();
}

static esp_err_t save_ssid(const nvs_handle_t nvs_handle,
                           const char* const data,
                           const uint8_t len)
{
    return nvs_set_blob(nvs_handle, NVS_SSID_KEY, (const void*)data, len);
}

static esp_err_t save_password(const nvs_handle_t nvs_handle,
                               const char* const data,
                               const uint8_t len)
{
    return nvs_set_blob(nvs_handle, NVS_PASSWORD_KEY, (const void*)data, len);
}

static esp_err_t load_ssid(const nvs_handle_t nvs_handle,
                           uint8_t* const dest,
                           size_t* const len)
{
    *len = MAX_SSID_LEN;
    return nvs_get_blob(nvs_handle, NVS_SSID_KEY, (void*)dest, len);
}

static esp_err_t load_password(const nvs_handle_t nvs_handle,
                               uint8_t* const dest,
                               size_t* const len)
{
    *len = MAX_PASSPHRASE_LEN;
    return nvs_get_blob(nvs_handle, NVS_PASSWORD_KEY, (void*)dest, len);
}

static inline wifi_auth_mode_t get_auth_mode(const uint8_t password_len)
{
    return password_len > 0 ? WIFI_AUTH_WPA_WPA2_PSK : WIFI_AUTH_OPEN;
}

static inline void deauth_all_users(void)
{
    (void)esp_wifi_deauth_sta(0);
}
