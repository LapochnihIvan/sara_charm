#include "screen.h"

#include <stdbool.h>

#include "driver/gpio.h"

#include "st7789.h"

#include "gif.h"

#include "sdkconfig.h"

#define LCD_VCC_PIN_NUM (CONFIG_LCD_VCC_PIN_NUM)
#define LCD_SCK_PIN_NUM (CONFIG_SCLK_GPIO)
#define LCD_SDA_PIN_NUM (CONFIG_MOSI_GPIO)
#define LCD_RES_PIN_NUM (CONFIG_RESET_GPIO)
#define LCD_DC_PIN_NUM  (CONFIG_DC_GPIO)
#define LCD_BLK_PIN_NUM (CONFIG_BL_GPIO)
#define LCD_CS_PIN_NUM  (CONFIG_CS_GPIO)

#define LCD_WIDTH (CONFIG_WIDTH)
#define LCD_HEIGHT (CONFIG_HEIGHT)

#define GET_EMBED_FILE(name, format) asm("_binary_" #name "_" #format "_start");

static void screen_task_impl(void* _args);

BaseType_t start_screen_task(TaskHandle_t* const task_handle)
{
    return xTaskCreate(
        screen_task_impl,
        "screen_task",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY,
        task_handle
    );
}

static void screen_task_impl(void* _args)
{
#if LCD_VCC_PIN_NUM != -1
    gpio_set_direction(LCD_VCC_PIN_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(LCD_VCC_PIN_NUM, 1);
#endif

    TFT_t dev;

	spi_master_init(
        &dev, 
        LCD_SDA_PIN_NUM,
        LCD_SCK_PIN_NUM,
        LCD_CS_PIN_NUM,
        LCD_DC_PIN_NUM,
        LCD_RES_PIN_NUM,
        LCD_BLK_PIN_NUM
    );
	lcdInit(&dev, LCD_WIDTH, LCD_HEIGHT, 0, 0);

    // lcdFillScreen(&dev, BLACK);

    while (true) {
        //Drawing images
    }
}

void init_lcd(void)
{
    
}
