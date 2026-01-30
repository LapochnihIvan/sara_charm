#include <stdbool.h>

#include <driver/gpio.h>
#include <esp_sleep.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "screen.h"


#define POWER_BTN_GPIO_NUM (GPIO_NUM_0)

void app_main(void)
{
    gpio_set_direction(POWER_BTN_GPIO_NUM, GPIO_MODE_INPUT);
    gpio_set_pull_mode(POWER_BTN_GPIO_NUM, GPIO_PULLDOWN_ONLY);

    esp_deep_sleep_enable_gpio_wakeup(1 << POWER_BTN_GPIO_NUM, ESP_GPIO_WAKEUP_GPIO_HIGH);

    TaskHandle_t screen_task_handle;
    (void)start_screen_task(&screen_task_handle);

    while (true) {
        if (gpio_get_level(POWER_BTN_GPIO_NUM) == 1) {
            while (gpio_get_level(POWER_BTN_GPIO_NUM) == 1) {
                vTaskDelay(1);
            }

            vTaskSuspend(screen_task_handle);
            vTaskDelete(screen_task_handle);

            esp_deep_sleep_start();
        }

        vTaskDelay(1);
    }
}
