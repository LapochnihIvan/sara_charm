#include "screen.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "driver/gpio.h"

#include "st7789.h"

#include "gif.h"

#include "sdkconfig.h"

#include "embed_file_util.h"


#define LCD_VCC_PIN_NUM ((int16_t)CONFIG_LCD_VCC_PIN_NUM)
#define LCD_SCK_PIN_NUM ((int16_t)CONFIG_SCLK_GPIO)
#define LCD_SDA_PIN_NUM ((int16_t)CONFIG_MOSI_GPIO)
#define LCD_RES_PIN_NUM ((int16_t)CONFIG_RESET_GPIO)
#define LCD_DC_PIN_NUM  ((int16_t)CONFIG_DC_GPIO)
#define LCD_BLK_PIN_NUM ((int16_t)CONFIG_BL_GPIO)
#define LCD_CS_PIN_NUM  ((int16_t)CONFIG_CS_GPIO)

#define LCD_WIDTH (CONFIG_WIDTH)
#define LCD_HEIGHT (CONFIG_HEIGHT)

static void screen_task_impl(void* _args);
static void init_lcd(TFT_t* const lcd);
static void gif_draw_callback(GIFDRAW* img_line);

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
#if CONFIG_LCD_VCC_PIN_NUM != -1
    gpio_set_direction(LCD_VCC_PIN_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(LCD_VCC_PIN_NUM, 1);
#endif

    TFT_t lcd;
    init_lcd(&lcd);

    EXTERN_EMBED_FILE(sara, gif);
    embed_file_t sara_gif = GET_EMBED_FILE(sara, gif);

    while (true) {
        //Drawing images
    }
}

static void init_lcd(TFT_t* const lcd)
{
    spi_master_init(
        lcd, 
        LCD_SDA_PIN_NUM,
        LCD_SCK_PIN_NUM,
        LCD_CS_PIN_NUM,
        LCD_DC_PIN_NUM,
        LCD_RES_PIN_NUM,
        LCD_BLK_PIN_NUM
    );
	lcdInit(lcd, LCD_WIDTH, LCD_HEIGHT, 0, 0);
}
