#include "wifi_point.h"

#include <string.h>

#include <esp_wifi.h>


#define CHANNEL_NUM (6)

void start_wifi_point(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&init_config);
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);

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

    esp_wifi_set_config(WIFI_IF_AP, &config);
    esp_wifi_start();
}

esp_err_t change_wifi_point_settings(const char* const ssid,
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

    (void)esp_wifi_deauth_sta(0);
    return esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
}

void stop_wifi_point(void)
{
    esp_wifi_stop();
}
