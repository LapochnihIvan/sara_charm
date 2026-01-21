#include "screen.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "driver/gpio.h"

#include "portmacro.h"
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

#define LCD_WIDTH ((uint16_t)CONFIG_WIDTH)
#define LCD_HEIGHT ((uint16_t)CONFIG_HEIGHT)

static void screen_task_impl(void* _args);
static void init_lcd(TFT_t* const lcd);
static void gif_draw_callback(GIFDRAW* img_line);

BaseType_t start_screen_task(TaskHandle_t* const task_handle)
{
    return xTaskCreate(
        screen_task_impl,
        "screen_task",
        configMINIMAL_STACK_SIZE * 20,
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

    GIFIMAGE sara_gif_parser;
    GIF_begin(&sara_gif_parser, GIF_PALETTE_RGB565_LE);
    GIF_openRAM(
        &sara_gif_parser,
        (uint8_t*)sara_gif.begin,
        (int)sara_gif.len,
        gif_draw_callback
    );

    lcdFillScreen(&lcd, BLACK);

    while (true) {
        GIF_playFrame(&sara_gif_parser, NULL, (void*)&lcd);
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

static void gif_draw_callback(GIFDRAW* const img_line)
{
    const uint16_t line_width = MIN((uint16_t)img_line->iWidth, LCD_WIDTH);
    uint16_t decode_line[LCD_WIDTH];

    uint16_t* decode_color = decode_line;
    const uint8_t* last_img_pixel = img_line->pPixels + line_width;
    for (const uint8_t* pixel = img_line->pPixels;
         pixel != last_img_pixel;
         ++pixel)
    {
        *decode_color = img_line->pPalette[*pixel];
        ++decode_color;
    }

    const uint16_t x_offset = (uint16_t)img_line->iX;
    const uint16_t y = (uint16_t)img_line->y;
    lcdDrawMultiPixels(
        (TFT_t*)img_line->pUser,
        x_offset, y,
        line_width,
        decode_line
    );
}
