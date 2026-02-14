#include <stdbool.h>

#include <driver/gpio.h>
#include <esp_sleep.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "screen.h"
#include "wifi_point.h"
#include "server.h"
#include "utils/nvs.h"
#include "utils/esp_try.h"


#define POWER_BTN_GPIO_NUM (GPIO_NUM_0)

static esp_err_t start_server_part(wifi_point_t* wifi_point,
                                   server_handle_t* server);
#include "esp_log.h"
void app_main(void)
{
    gpio_set_direction(POWER_BTN_GPIO_NUM, GPIO_MODE_INPUT);
    gpio_set_pull_mode(POWER_BTN_GPIO_NUM, GPIO_PULLDOWN_ONLY);

    esp_deep_sleep_enable_gpio_wakeup(1 << POWER_BTN_GPIO_NUM, ESP_GPIO_WAKEUP_GPIO_HIGH);

    screen_task_t screen_task;
    (void)screen_task_start(&screen_task);

    wifi_point_t wifi_point;
    server_handle_t server = NULL;
    const esp_err_t server_init_res = start_server_part(&wifi_point, &server);

    if (server_init_res != ESP_OK)
    {
        ESP_LOGE("", "%s", esp_err_to_name(server_init_res));
    }

    while (true) {
        if (gpio_get_level(POWER_BTN_GPIO_NUM) == 1) {
            while (gpio_get_level(POWER_BTN_GPIO_NUM) == 1) {
                vTaskDelay(1);
            }

            screen_task_stop(&screen_task);

            if (server_init_res == ESP_OK)
            {
                stop_server(server);
                wifi_point_stop(&wifi_point);
                deinit_nvs();
            }

            esp_deep_sleep_start();
        }

        vTaskDelay(1);
    }
}

static esp_err_t start_server_part(wifi_point_t* const wifi_point,
                                   server_handle_t* const server)
{
    ESP_TRY(init_nvs());

    esp_err_t res = wifi_point_start(wifi_point);
    if (res != ESP_OK)
    {
        deinit_nvs();

        return res;
    }

    res = start_server(server);
    if (res != ESP_OK)
    {
        wifi_point_stop(wifi_point);
        deinit_nvs();
    }

    return res;
}
