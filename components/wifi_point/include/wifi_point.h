#ifndef SARA_CHARM_SERVER_WIFI_POINT_H
#define SARA_CHARM_SERVER_WIFI_POINT_H


#include <stdint.h>

#include <esp_wifi.h>
#include <nvs.h>


typedef struct wifi_point
{
    esp_netif_t* _netif_handle;
    nvs_handle_t _nvs_handle;
} wifi_point_t;

esp_err_t wifi_point_start(wifi_point_t* self);
void wifi_point_stop(wifi_point_t* self);
esp_err_t wifi_point_change_settings(wifi_point_t* self,
                                     const char* ssid,
                                     uint8_t ssid_len,
                                     const char* password,
                                     uint8_t password_len);


#endif //!SARA_CHARM_SERVER_WIFI_POINT_H
