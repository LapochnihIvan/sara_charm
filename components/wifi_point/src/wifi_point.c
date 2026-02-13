#include "wifi_point.h"

#include <string.h>

#include "utils/esp_try.h"


#define CHANNEL_NUM (6)

#define NVS_NAMESPACE ("wifi")

static esp_err_t init_netif(void);
static esp_err_t start_wifi(void);
static inline void deauth_all_users(void);

esp_err_t wifi_point_start(wifi_point_t* const self)
{
    ESP_TRY(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &self->_nvs_handle));

    esp_err_t res = init_netif();
    if (res != ESP_OK)
    {
        nvs_close(self->_nvs_handle);

        return res;
    }

    self->_netif_handle = esp_netif_create_default_wifi_ap();

    res = start_wifi();
    if (res != ESP_OK)
    {
        esp_netif_destroy(self->_netif_handle);
        nvs_close(self->_nvs_handle);
    }

    return res;
}

void wifi_point_stop(wifi_point_t* const self)
{
    (void)esp_wifi_stop();
}

esp_err_t wifi_point_change_settings(wifi_point_t* const self,
                                     const char* const ssid,
                                     const uint8_t ssid_len,
                                     const char* const password,
                                     const uint8_t password_len)
{
    wifi_config_t wifi_config;
    (void)esp_wifi_get_config(ESP_IF_WIFI_AP, &wifi_config);

    memcpy((void*)wifi_config.ap.ssid, (const void*)ssid, ssid_len);
    wifi_config.ap.ssid_len = ssid_len;

    memcpy((void*)wifi_config.ap.password, (const void*)password, password_len);
    wifi_config.ap.password[password_len] = '\0';

    deauth_all_users();

    return esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
}

static esp_err_t init_netif(void)
{
    ESP_TRY(esp_netif_init());
    
    return esp_event_loop_create_default();
}

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
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };
    ESP_TRY(esp_wifi_set_config(WIFI_IF_AP, &config));

    return esp_wifi_start();
}

static inline void deauth_all_users(void)
{
    (void)esp_wifi_deauth_sta(0);
}
