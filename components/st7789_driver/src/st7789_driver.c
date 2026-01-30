#include "st7789_driver.h"

#include <string.h>

#include <driver/gpio.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#include <freertos/atomic.h>

#pragma GCC diagnostic pop

#include "utils/bit.h"
#include "utils/delay.h"


#define VCC_PIN_NUM ((gpio_num_t)CONFIG_ST7789_VCC_PIN_NUM)
#define SCK_PIN_NUM ((gpio_num_t)CONFIG_ST7789_SCK_PIN_NUM)
#define SDA_PIN_NUM ((gpio_num_t)CONFIG_ST7789_SDA_PIN_NUM)
#define RES_PIN_NUM ((gpio_num_t)CONFIG_ST7789_RES_PIN_NUM)
#define DC_PIN_NUM  ((gpio_num_t)CONFIG_ST7789_DC_PIN_NUM)
#define BLK_PIN_NUM ((gpio_num_t)CONFIG_ST7789_BLK_PIN_NUM)
#define CS_PIN_NUM  ((gpio_num_t)CONFIG_ST7789_CS_PIN_NUM)

#ifdef CONFIG_ST7789_SPI2_HOST
#   define SPI_HOST (SPI2_HOST)
#elif defined(CONFIG_ST7789_SPI3_HOST)
#   define SPI_HOST (SPI3_HOST)
#endif //CONFIG_ST7789_SPI2_HOST

#ifdef CONFIG_ST7789_SPI_FREQ_20_MHZ
#   define SPI_FREQ (SPI_MASTER_FREQ_20M)
#elif defined(CONFIG_ST7789_SPI_FREQ_40_MHZ)
#   define SPI_FREQ (SPI_MASTER_FREQ_40M)
#elif defined(CONFIG_ST7789_SPI_FREQ_80_MHZ)
#   define SPI_FREQ (SPI_MASTER_FREQ_80M)
#endif //CONFIG_ST7789_SPI_FREQ_20_MHZ

#define NOT_USED_PIN (-1)

#define DC_COMMAND_LEVEl (0)
#define DC_DATA_LEVEl    (1)

#define RESET_DELAY_MS            ((uint16_t)100)
#define SOFTWARE_RESET_DELAY_MS   ((uint16_t)150)
#define SLEEP_OUT_DELAY_MS        ((uint16_t)150)
#define SET_PIXEL_FORMAT_DELAY_MS ((uint16_t)10)
#define DISPLAY_ON_DELAY_MS       ((uint16_t)255)

#define PIXEL_FORMAT_16_BIT      (0x55)
#define MEM_DATA_ACCESS_CTRL_RGB (0x00)

#define COLORS_IN_TX_BLOCK (sizeof(uint32_t) / sizeof(uint16_t))
#define BITS_IN_COLOR (sizeof(uint16_t) * BITS_IN_BYTE)

typedef enum lcd_command
{
    SOFTWARE_RESET           = 0x01,
    SLEEP_OUT                = 0x11,
    NORMAL_DISPLAY_MODE_ON   = 0x13,
    DISPLAY_INVERSION_ON     = 0x21,
    DISPLAY_ON               = 0x29,
    SET_COLUMN_ADDR          = 0x2A,
    SET_ROW_ADDR             = 0x2B,
    WRITE_MEMORY             = 0x2C,
    SET_MEM_DATA_ACCESS_CTRL = 0x36,
    SET_PIXEL_FORMAT         = 0x3A,
} lcd_command_t;

static void gpio_init(void);
static void spi_init(spi_device_handle_t* spi_handle);
static void tx_queue_init(st7789_control_t* self);
static void lcd_send_command(st7789_control_t* self,
                             lcd_command_t command);
static void lcd_send_command_sync(st7789_control_t* self,
                                  uint8_t byte);
static void lcd_send_coords(st7789_control_t* self,
                            uint16_t x_begin,
                            uint16_t x_end,
                            uint16_t y_begin,
                            uint16_t y_end);
static void lcd_send_colors(st7789_control_t* self,
                            const uint16_t* colors,
                            uint16_t len);
static void lcd_send_color_line(st7789_control_t* self,
                                uint16_t color);
static void lcd_send_addr(st7789_control_t* self,
                          uint16_t begin,
                          uint16_t end);
static void lcd_send_data(st7789_control_t* self,
                          const void* data,
                          uint16_t num_bytes);
static void lcd_send(st7789_control_t* self,
                     const void* data,
                     uint16_t num_bytes);
static void lcd_send_byte(st7789_control_t* self,
                          uint8_t data);
static void lcd_send_data_byte_sync(st7789_control_t* self,
                                    uint8_t byte);
static void lcd_send_impl(st7789_control_t* self);
static void lcd_send_byte_sync(st7789_control_t* self,
                               uint8_t byte,
                               uint8_t dc_level);
static void lcd_wait_sending(void);
static void spi_pre_callback(spi_transaction_t* transaction);
static void spi_post_callback(spi_transaction_t* transaction);

void st7789_init(st7789_control_t* const self)
{
    gpio_init();
    spi_init(&self->_spi_handle);
    tx_queue_init(self);

    lcd_send_command_sync(self, SOFTWARE_RESET);
    delay_ms(SOFTWARE_RESET_DELAY_MS);

	lcd_send_command_sync(self, SLEEP_OUT);
    delay_ms(SLEEP_OUT_DELAY_MS);

    lcd_send_command_sync(self, SET_PIXEL_FORMAT);
	lcd_send_data_byte_sync(self, PIXEL_FORMAT_16_BIT);
    delay_ms(SET_PIXEL_FORMAT_DELAY_MS);
	
    lcd_send_command_sync(self, SET_MEM_DATA_ACCESS_CTRL);
	lcd_send_data_byte_sync(self, MEM_DATA_ACCESS_CTRL_RGB);

	lcd_send_command_sync(self, DISPLAY_INVERSION_ON);
	lcd_send_command_sync(self, NORMAL_DISPLAY_MODE_ON);

#if CONFIG_ST7789_BLK_PIN_NUM != NOT_USED_PIN
    gpio_set_level(BLK_PIN_NUM, 1);
#endif //CONFIG_ST7789_BLK_PIN_NUM != -1
}

void st7789_display_on(st7789_control_t* const self)
{
    lcd_send_command_sync(self, DISPLAY_ON);
    delay_ms(DISPLAY_ON_DELAY_MS);
}

void st7789_enable_drawing_notify(st7789_control_t* const self)
{
    self->_notify.notify_task_handle = xTaskGetCurrentTaskHandle();
}

void st7789_disable_drawing_notify(st7789_control_t* const self)
{
    self->_notify.notify_task_handle = NULL;
}

void st7789_wait_drawing(void)
{
    lcd_wait_sending();
}

void st7789_fill_screen(st7789_control_t* const self, const uint16_t color)
{
    for (uint16_t y = 0; y < ST7789_SCREEN_HEIGHT; ++y)
    {
        lcd_send_coords(self, 0, ST7789_SCREEN_WIDTH - 1 , y, y);
        lcd_send_color_line(self, color);
    }
}

void st7789_draw_multicolor_line(st7789_control_t* const self,
                                 const uint16_t x_offset,
                                 const uint16_t y,
                                 const uint16_t width,
                                 const uint16_t* const colors)
{
    lcd_send_coords(self, x_offset, x_offset + width - 1, y, y);
    lcd_send_colors(self, colors, width);
}

static void gpio_init(void)
{
#if CONFIG_ST7789_VCC_PIN_NUM != NOT_USED_PIN
    gpio_set_direction(VCC_PIN_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(VCC_PIN_NUM, 1);
#endif //CONFIG_ST7789_VCC_PIN_NUM != NOT_USED_PIN

#if CONFIG_ST7789_CS_PIN_NUM != NOT_USED_PIN
    gpio_set_direction(CS_PIN_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(CS_PIN_NUM, 0);
#endif //CONFIG_ST7789_CS_PIN_NUM != NOT_USED_PIN

    gpio_set_direction(DC_PIN_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(DC_PIN_NUM, 0);

#if CONFIG_ST7789_RES_PIN_NUM != NOT_USED_PIN
    gpio_set_direction(RES_PIN_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(RES_PIN_NUM, 1);
    vTaskDelay(pdMS_TO_TICKS(RESET_DELAY_MS));
    gpio_set_level(RES_PIN_NUM, 0);
    vTaskDelay(pdMS_TO_TICKS(RESET_DELAY_MS));
    gpio_set_level(RES_PIN_NUM, 1);
    vTaskDelay(pdMS_TO_TICKS(RESET_DELAY_MS));
#endif //CONFIG_ST7789_RES_PIN_NUM != NOT_USED_PIN

#if CONFIG_ST7789_BLK_PIN_NUM != NOT_USED_PIN
    gpio_set_direction(BLK_PIN_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(BLK_PIN_NUM, 0);
#endif //CONFIG_ST7789_BLK_PIN_NUM != NOT_USED_PIN
}

static void spi_init(spi_device_handle_t* const spi_handle)
{
    spi_bus_config_t spi_conf = {
		.mosi_io_num = CONFIG_ST7789_SDA_PIN_NUM,
		.miso_io_num = NOT_USED_PIN,
		.sclk_io_num = CONFIG_ST7789_SCK_PIN_NUM,
		.quadwp_io_num = NOT_USED_PIN,
		.quadhd_io_num = NOT_USED_PIN,
		.max_transfer_sz = 0,
		.flags = 0
	};

    spi_bus_initialize(SPI_HOST, &spi_conf, SPI_DMA_CH_AUTO);

	spi_device_interface_config_t devcfg = {0};
	devcfg.mode = 3;
	devcfg.clock_speed_hz = SPI_FREQ;
    devcfg.spics_io_num = CS_PIN_NUM;
    devcfg.flags = SPI_DEVICE_NO_DUMMY | SPI_DEVICE_NO_RETURN_RESULT;
	devcfg.queue_size = CONFIG_ST7789_SPI_QUEUE_SIZE;
    devcfg.pre_cb = spi_pre_callback;
    devcfg.post_cb = spi_post_callback;

	spi_bus_add_device(SPI_HOST, &devcfg, spi_handle);

#ifdef CONFIG_ST7789_ACQUIRE_SPI_BUS
    spi_device_acquire_bus(*spi_handle, portMAX_DELAY);
#endif //CONFIG_ST7789_ACQUIRE_SPI_BUS
}

static void tx_queue_init(st7789_control_t* const self)
{
    self->_notify.notify_task_handle = NULL;
    self->_notify.num_packets_in_process = 0;

    st7789_packet_t* packet = self->_tx_queue;
    const st7789_packet_t* const last_packet = packet + ST7789_TX_QUEUE_SIZE;
    for (; packet != last_packet; ++packet)
    {
        packet->inner = (spi_transaction_t){0};
        packet->tx_buf = heap_caps_aligned_alloc(
            4,
            ST7789_SCREEN_WIDTH * sizeof(uint16_t),
            MALLOC_CAP_DMA
        );
        packet->inner.tx_buffer = packet->tx_buf;
        packet->ctx.notify = &self->_notify;
        packet->inner.user = (void*)&packet->ctx;
    }
    self->_cur_packet = self->_tx_queue;
}

static void lcd_send_command(st7789_control_t* const self,
                             const lcd_command_t command)
{
    self->_cur_packet->ctx.dc_level = DC_COMMAND_LEVEl;
    lcd_send_byte(self, (uint8_t)command);
}

static void lcd_send_command_sync(st7789_control_t* const self,
                                  const uint8_t byte)
{
    lcd_send_byte_sync(self, byte, DC_COMMAND_LEVEl);
}

static void lcd_send_coords(st7789_control_t* const self,
                            const uint16_t x_begin,
                            const uint16_t x_end,
                            const uint16_t y_begin,
                            const uint16_t y_end)
{
    lcd_send_command(self, SET_COLUMN_ADDR);
    lcd_send_addr(self, x_begin, x_end);
    lcd_send_command(self, SET_ROW_ADDR);
    lcd_send_addr(self, y_begin, y_end);
}

static void lcd_send_colors(st7789_control_t* const self,
                            const uint16_t* colors,
                            const uint16_t len)
{
    lcd_send_command(self, WRITE_MEMORY);

    uint32_t* const tx_buf = (uint32_t*)self->_cur_packet->tx_buf;
    const uint32_t* const last_tx_block = tx_buf + (len / COLORS_IN_TX_BLOCK);
    for (uint32_t* tx_block = tx_buf; tx_block != last_tx_block; ++tx_block)
    {
        *tx_block = swap_bytes(*colors) << BITS_IN_COLOR;
        ++colors;
        *tx_block |= swap_bytes(*colors);
        ++colors;
    }
    self->_cur_packet->inner.length = len * BITS_IN_COLOR;

    lcd_send_impl(self);
}

static void lcd_send_color_line(st7789_control_t* const self,
                                const uint16_t color)
{
    lcd_send_command(self, WRITE_MEMORY);

    const uint16_t be_color = swap_bytes(color);
    const uint32_t tx_block_val = (be_color << BITS_IN_COLOR) | be_color;

    uint32_t* const tx_buf = (uint32_t*)self->_cur_packet->tx_buf;
    const uint32_t* const last_tx_block =
        tx_buf + (ST7789_SCREEN_WIDTH / COLORS_IN_TX_BLOCK);
    for (uint32_t* tx_block = tx_buf; tx_block != last_tx_block; ++tx_block)
    {
        *tx_block = tx_block_val;
    }
    self->_cur_packet->inner.length = ST7789_SCREEN_WIDTH * BITS_IN_COLOR;

    lcd_send_impl(self);
}

static void lcd_send_addr(st7789_control_t* const self,
                          uint16_t begin,
                          uint16_t end)
{
    begin = swap_bytes(begin);
    end = swap_bytes(end);
    lcd_send_data(
        self,
        (const void*)(uint16_t[]){begin, end},
        2 * sizeof(uint16_t)
    );
}

static void lcd_send_data(st7789_control_t* const self,
                          const void* const data,
                          const uint16_t num_bytes)
{
    self->_cur_packet->ctx.dc_level = DC_DATA_LEVEl;
    lcd_send(self, data, num_bytes);
}

static void lcd_send_data_byte_sync(st7789_control_t* const self,
                                    const uint8_t byte)
{
    lcd_send_byte_sync(self, byte, DC_DATA_LEVEl);
}

static void lcd_send(st7789_control_t* const self,
                     const void* const data,
                     const uint16_t num_bytes)
{
    st7789_packet_t* const packet = self->_cur_packet;
    memcpy(packet->tx_buf, data, num_bytes);
    packet->inner.length = num_bytes * BITS_IN_BYTE;

    lcd_send_impl(self);
}

static void lcd_send_byte(st7789_control_t* const self,
                          const uint8_t data)
{
    lcd_send(self, (const void*)&data, sizeof(uint8_t));
}

static void lcd_send_impl(st7789_control_t* const self)
{
    ATOMIC_ENTER_CRITICAL();
    ++self->_notify.num_packets_in_process;
    ATOMIC_EXIT_CRITICAL();

    spi_transaction_t* const transaction = &self->_cur_packet->inner;
    transaction->rxlength = 0;
    spi_device_queue_trans(self->_spi_handle, transaction, portMAX_DELAY);

    ++self->_cur_packet;
    if (self->_cur_packet == self->_tx_queue + ST7789_TX_QUEUE_SIZE)
    {
        self->_cur_packet = self->_tx_queue;
    }
}

static void lcd_send_byte_sync(st7789_control_t* const self,
                               const uint8_t byte,
                               const uint8_t dc_level)
{
    spi_transaction_t transaction = {0};
    transaction.flags = SPI_TRANS_USE_TXDATA;
    transaction.length = sizeof(uint8_t) * BITS_IN_BYTE;
    st7789_packet_ctx_t ctx = {
        .dc_level = dc_level,
        .notify = NULL
    };
    transaction.user = (void*)&ctx;
    transaction.tx_data[0] = byte;

    spi_device_polling_transmit(self->_spi_handle, &transaction);
}

static void lcd_wait_sending(void)
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

static void spi_pre_callback(spi_transaction_t* const transaction)
{
    const uint8_t dc_level =
        ((st7789_packet_ctx_t*)transaction->user)->dc_level;

    gpio_set_level(DC_PIN_NUM, dc_level);
}

static void spi_post_callback(spi_transaction_t* const transaction)
{
    const st7789_packet_ctx_t* const ctx =
        (st7789_packet_ctx_t*)transaction->user;

    if (ctx->notify == NULL)
    {
        return;
    }

    --ctx->notify->num_packets_in_process;

    const TaskHandle_t task_handle = ctx->notify->notify_task_handle;
    if (task_handle != NULL && ctx->notify->num_packets_in_process == 0)
    {
        vTaskNotifyGiveFromISR(task_handle, NULL);
    }
}
