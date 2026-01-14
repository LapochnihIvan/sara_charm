#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "st7789.h"
#include "esp_sleep.h"

#include "sdkconfig.h"


#define POWER_BTN_GPIO_NUM (GPIO_NUM_0)

#define LCD_VCC_GPIO_NUM (GPIO_NUM_1)
#define LCD_DC_GPIO_NUM (GPIO_NUM_3)
#define LCD_SCK_GPIO_NUM (GPIO_NUM_4)
#define LCD_SDA_GPIO_NUM (GPIO_NUM_6)
#define LCD_RES_GPIO_NUM (GPIO_NUM_10)
#define LCD_BLK_GPIO_NUM (GPIO_NUM_5)

#define LCD_WIDTH (240)
#define LCD_HEIGHT (240)

#define GET_EMBED_FILE(name, format) asm("_binary_" #name "_" #format "_start");

void app_main(void)
{
    gpio_set_direction(POWER_BTN_GPIO_NUM, GPIO_MODE_INPUT);
    gpio_set_pull_mode(POWER_BTN_GPIO_NUM, GPIO_PULLDOWN_ONLY);

    esp_deep_sleep_enable_gpio_wakeup(1 << POWER_BTN_GPIO_NUM, ESP_GPIO_WAKEUP_GPIO_HIGH);

    gpio_set_direction(LCD_VCC_GPIO_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(LCD_VCC_GPIO_NUM, 1);

    TFT_t dev;

	spi_master_init(&dev, LCD_SDA_GPIO_NUM, LCD_SCK_GPIO_NUM, -1, LCD_DC_GPIO_NUM, LCD_RES_GPIO_NUM, -1);
	lcdInit(&dev, LCD_WIDTH, LCD_HEIGHT, 0, 0);

    lcdFillScreen(&dev, BLACK);

    extern const uint8_t sara_image[] GET_EMBED_FILE(sara, bin);

    for (int i = 0; i < LCD_HEIGHT; ++i)
    {
        lcdDrawMultiPixels(&dev, 0, i, LCD_WIDTH, (uint16_t*)sara_image + i * LCD_WIDTH);
    }

    while (true) {
        if (gpio_get_level(POWER_BTN_GPIO_NUM) == 1) {
            while (gpio_get_level(POWER_BTN_GPIO_NUM) == 1) {
                vTaskDelay(1);
            }
            ESP_LOGI(__FUNCTION__, "Start deep sleep");
            esp_deep_sleep_start();
        }

        vTaskDelay(1);
    }
}
