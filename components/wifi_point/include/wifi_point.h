#ifndef SARA_CHARM_SERVER_WIFI_POINT_H
#define SARA_CHARM_SERVER_WIFI_POINT_H


#include <stdint.h>

#include <esp_err.h>


void start_wifi_point(void);
void stop_wifi_point(void);
esp_err_t change_wifi_point_settings(const char* ssid,
                                     uint8_t ssid_len,
                                     const char* password,
                                     uint8_t password_len);


#endif //!SARA_CHARM_SERVER_WIFI_POINT_H
