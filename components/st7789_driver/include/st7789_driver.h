#ifndef SARA_CHARM_ST7789_DRIVER_ST7789_DRIVER_H
#define SARA_CHARM_ST7789_DRIVER_ST7789_DRIVER_H


#include <driver/spi_master.h>


void st7789_init(spi_device_handle_t* spi_handle);
void st7789_draw_multicolor_line(spi_device_handle_t spi_handle,
                                 uint16_t x_offset,
                                 uint16_t y,
                                 uint16_t width,
                                 const uint16_t* colors);
uint16_t st7789_color_from_le(uint16_t color);


#endif //!SARA_CHARM_ST7789_DRIVER_ST7789_DRIVER_H
