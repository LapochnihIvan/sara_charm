#ifndef SARA_CHARM_ST7789_DRIVER_ST7789_DRIVER_H
#define SARA_CHARM_ST7789_DRIVER_ST7789_DRIVER_H


#include <driver/spi_master.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


#define ST7789_SCREEN_WIDTH  ((uint16_t)CONFIG_ST7789_SCREEN_WIDTH)
#define ST7789_SCREEN_HEIGHT ((uint16_t)CONFIG_ST7789_SCREEN_HEIGHT)

#define ST7789_BLACK_COLOR ((uint16_t)0x0000)

#define ST7789_TX_QUEUE_SIZE (CONFIG_ST7789_SPI_QUEUE_SIZE + 1)

typedef struct st7789_done_notify
{
    TaskHandle_t notify_task_handle;
    uint8_t num_packets_in_process;
} st7789_done_notify_t;

typedef struct st7789_packet_ctx
{
    uint8_t dc_level;
    st7789_done_notify_t* notify;

} st7789_packet_ctx_t;

typedef struct st7789_packet
{
    spi_transaction_t spi_transaction;
    uint8_t tx_buf[CONFIG_ST7789_SCREEN_WIDTH * sizeof(uint16_t)];
    st7789_packet_ctx_t ctx;
} st7789_packet_t;

typedef struct st7789_control
{
    spi_device_handle_t _spi_handle;
    st7789_packet_t _tx_queue[ST7789_TX_QUEUE_SIZE];
    st7789_packet_t* _cur_packet;
    st7789_done_notify_t _notify;
} st7789_control_t;

void st7789_init(st7789_control_t* self);
void st7789_display_on(st7789_control_t* self);
void st7789_enable_drawing_notify(st7789_control_t* self);
void st7789_disable_drawing_notify(st7789_control_t* self);
void st7789_wait_drawing(void);
void st7789_fill_screen(st7789_control_t* self, uint16_t color);
void st7789_draw_multicolor_line(st7789_control_t* self,
                                 uint16_t x_offset,
                                 uint16_t y,
                                 uint16_t width,
                                 const uint16_t* colors);
uint16_t st7789_color_from_le(uint16_t color);

#endif //!SARA_CHARM_ST7789_DRIVER_ST7789_DRIVER_H
