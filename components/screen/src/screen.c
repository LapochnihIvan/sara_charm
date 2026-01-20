#include "screen.h"

#include <stdbool.h>

#include "driver/gpio.h"

#include "st7789.h"

#include "gif.h"

#include "sdkconfig.h"


#define LCD_VCC_GPIO_NUM (GPIO_NUM_1)
#define LCD_DC_GPIO_NUM (GPIO_NUM_3)
#define LCD_SCK_GPIO_NUM (GPIO_NUM_4)
#define LCD_SDA_GPIO_NUM (GPIO_NUM_6)
#define LCD_RES_GPIO_NUM (GPIO_NUM_10)
#define LCD_BLK_GPIO_NUM (GPIO_NUM_5)

#define LCD_WIDTH (240)
#define LCD_HEIGHT (240)

#define GET_EMBED_FILE(name, format) asm("_binary_" #name "_" #format "_start");

static void screen_task_impl(void*);

BaseType_t start_screen_task(TaskHandle_t* const task_handle)
{
    return xTaskCreate(screen_task_impl,
                "screen_task",
                configMINIMAL_STACK_SIZE,
                NULL,
                tskIDLE_PRIORITY,
                task_handle);
}

void screen_task_impl(void*)
{
    gpio_set_direction(LCD_VCC_GPIO_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(LCD_VCC_GPIO_NUM, 1);

    TFT_t dev;

	spi_master_init(&dev, LCD_SDA_GPIO_NUM, LCD_SCK_GPIO_NUM, -1, LCD_DC_GPIO_NUM, LCD_RES_GPIO_NUM, -1);
	lcdInit(&dev, LCD_WIDTH, LCD_HEIGHT, 0, 0);

    lcdFillScreen(&dev, BLACK);

    while (true) {
        //Drawing images
    }
}

void init_lcd(void)
{
    
}
