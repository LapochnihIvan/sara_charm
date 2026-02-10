#include "screen.h"

#include <stdbool.h>

#include <gif.h>

#include "freertos/projdefs.h"
#include "st7789_driver.h"

#include "utils/embed_file.h"

static void screen_task_impl(void* _args);
static void screen_task_delete_callback(int _index, void* lcd_raw);
static void gif_draw_callback(GIFDRAW* img_line);

BaseType_t screen_task_start(screen_task_t* const self)
{
    st7789_init(&self->_lcd);

    const BaseType_t res = xTaskCreate(
        screen_task_impl,
        "screen_task",
        sizeof(GIFIMAGE) + 1200,
        (void*)&self->_lcd,
        tskIDLE_PRIORITY,
        &self->_handle
    );

    if (res == pdPASS)
    {
        vTaskSetThreadLocalStoragePointerAndDelCallback(
            self->_handle,
            0,
            (void*)&self->_lcd,
            screen_task_delete_callback
        );
    }

    return res;
}

void screen_task_stop(screen_task_t* const self)
{
    vTaskSuspend(self->_handle);
    vTaskDelete(self->_handle);
}

static void screen_task_impl(void* lcd_raw)
{
    st7789_control_t* const lcd = (st7789_control_t*)lcd_raw;
    st7789_send_init_commands(lcd);

    st7789_enable_drawing_notify(lcd);
    st7789_fill_screen(lcd, ST7789_BLACK_COLOR);
    st7789_wait_drawing();
    st7789_disable_drawing_notify(lcd);

    st7789_display_on(lcd);

    EXTERN_EMBED_FILE(sara, gif);
    embed_file_t sara_gif = GET_EMBED_FILE(sara, gif);

    GIFIMAGE sara_gif_parser;
    GIF_begin(&sara_gif_parser, GIF_PALETTE_RGB565_LE);
    GIF_openRAM(
        &sara_gif_parser,
        (uint8_t*)sara_gif.data,
        (int)sara_gif.len,
        gif_draw_callback
    );

    while (true) {
        GIF_playFrame(&sara_gif_parser, NULL, (void*)lcd);
    }
}

static void screen_task_delete_callback(int _index, void* const lcd_raw)
{
    st7789_control_t* const lcd = (st7789_control_t*)lcd_raw;
    st7789_deinit(lcd);
}

static void gif_draw_callback(GIFDRAW* const img_line)
{
    const uint16_t line_width = (uint16_t)img_line->iWidth;
    uint16_t decode_line[ST7789_SCREEN_WIDTH];

    uint16_t* decode_color = decode_line;
    const uint8_t* last_img_pixel = img_line->pPixels + line_width;
    for (const uint8_t* pixel = img_line->pPixels;
         pixel != last_img_pixel;
         ++pixel)
    {
        *decode_color++ = img_line->pPalette[*pixel];
    }

    st7789_control_t* const lcd = (st7789_control_t*)img_line->pUser;
    const uint16_t x_offset = (uint16_t)img_line->iX;
    const uint16_t y = (uint16_t)img_line->y;
    st7789_draw_multicolor_line(
        lcd,
        x_offset,
        y,
        line_width,
        decode_line
    );
}
