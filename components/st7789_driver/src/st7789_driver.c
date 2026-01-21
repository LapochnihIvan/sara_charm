#include "st7789_driver.h"

#include <driver/gpio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


#define VCC_PIN_NUM ((gpio_num_t)CONFIG_ST7789_VCC_PIN_NUM)
#define SCK_PIN_NUM ((gpio_num_t)CONFIG_ST7789_SCK_PIN_NUM)
#define SDA_PIN_NUM ((gpio_num_t)CONFIG_ST7789_SDA_PIN_NUM)
#define RES_PIN_NUM ((gpio_num_t)CONFIG_ST7789_RES_PIN_NUM)
#define DC_PIN_NUM  ((gpio_num_t)CONFIG_ST7789_DC_PIN_NUM)
#define BLK_PIN_NUM ((gpio_num_t)CONFIG_ST7789_SCK_PIN_NUM)
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

#define BITS_IN_BYTE (CHAR_BIT)

typedef enum lcd_command {
    SoftwareReset        = 0x01,
    SleepOut             = 0x11,
    NormalDisplayModeOn  = 0x13,
    DisplayInversionOn   = 0x21,
    DisplayOn            = 0x29,
    SetColumnAddr        = 0x2A,
    SetRowAddr           = 0x2B,
    WriteMemory          = 0x2C,
    SetMemDataAccessCtrl = 0x36,
    SetPixelFormat       = 0x3A,
} lcd_command_t;

static void gpio_init(void);
static void spi_init(spi_device_handle_t* spi_handle);
static void lcd_send_command(spi_device_handle_t spi_handle,
                             lcd_command_t command);
static void lcd_send_addr(spi_device_handle_t spi_handle,
                          uint16_t begin,
                          uint16_t end);
static void lcd_send_colors(spi_device_handle_t spi_handle,
                            const uint16_t* colors,
                            const uint16_t len);
static void lcd_send_data(spi_device_handle_t spi_handle,
                          const void* data,
                          uint16_t num_bytes);
static void lcd_send_data_byte(spi_device_handle_t spi_handle,
                               uint8_t data);
static void spi_send(spi_device_handle_t spi_handle,
                     const void* data,
                     uint16_t num_bytes);
static void spi_send_byte(spi_device_handle_t spi_handle,
                          uint8_t data);
static inline void delay_ms(uint16_t num_ms);
static inline uint16_t swap_bytes(uint16_t num);

void st7789_init(spi_device_handle_t* const spi_handle)
{
    gpio_init();
    spi_init(spi_handle);

    lcd_send_command(*spi_handle, SoftwareReset);
	delay_ms(SOFTWARE_RESET_DELAY_MS);

	lcd_send_command(*spi_handle, SleepOut);
	delay_ms(SLEEP_OUT_DELAY_MS);

    lcd_send_command(*spi_handle, SetPixelFormat);
	lcd_send_data_byte(*spi_handle, PIXEL_FORMAT_16_BIT);
	delay_ms(SET_PIXEL_FORMAT_DELAY_MS);
	
    lcd_send_command(*spi_handle, SetMemDataAccessCtrl);
	lcd_send_data_byte(*spi_handle, MEM_DATA_ACCESS_CTRL_RGB);

	lcd_send_command(*spi_handle, DisplayInversionOn);
	lcd_send_command(*spi_handle, NormalDisplayModeOn);
	lcd_send_command(*spi_handle, DisplayOn);
	delay_ms(DISPLAY_ON_DELAY_MS);
}

void st7789_draw_multicolor_line(const spi_device_handle_t spi_handle,
                                 const uint16_t x_offset,
                                 const uint16_t y,
                                 const uint16_t width,
                                 const uint16_t* const colors)
{
    lcd_send_command(spi_handle, SetColumnAddr);
    lcd_send_addr(spi_handle, x_offset, x_offset + width - 1);
    lcd_send_command(spi_handle, SetRowAddr);
    lcd_send_addr(spi_handle, y, y);
    lcd_send_command(spi_handle, WriteMemory);
    lcd_send_colors(spi_handle, colors, width);
}

uint16_t st7789_color_from_le(const uint16_t color)
{
    return swap_bytes(color);
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
	devcfg.clock_speed_hz = SPI_FREQ;
	devcfg.queue_size = CONFIG_ST7789_SPI_QUEUE_SIZE;
	devcfg.mode = 3;
	devcfg.flags = SPI_DEVICE_NO_DUMMY;
    devcfg.spics_io_num = CS_PIN_NUM;

	spi_bus_add_device(SPI2_HOST, &devcfg, spi_handle);
}

static void lcd_send_command(const spi_device_handle_t spi_handle,
                             const lcd_command_t command)
{
    gpio_set_level(DC_PIN_NUM, DC_COMMAND_LEVEl);
    spi_send_byte(spi_handle, (uint8_t)command);
}

static void lcd_send_addr(const spi_device_handle_t spi_handle,
                          uint16_t begin,
                          uint16_t end)
{
    begin = swap_bytes(begin);
    end = swap_bytes(end);
    lcd_send_data(
        spi_handle,
        (const void*)(uint16_t[]){begin, end},
        2 * sizeof(uint16_t)
    );
}

static void lcd_send_colors(const spi_device_handle_t spi_handle,
                            const uint16_t* const colors,
                            const uint16_t len)
{
    lcd_send_data(spi_handle, (const void*)colors, len * sizeof(uint16_t));
}

static void lcd_send_data(const spi_device_handle_t spi_handle,
                          const void* const data,
                          const uint16_t num_bytes)
{
    gpio_set_level(DC_PIN_NUM, DC_DATA_LEVEl);
    spi_send(spi_handle, data, num_bytes);
}

static void lcd_send_data_byte(const spi_device_handle_t spi_handle,
                               const uint8_t data)
{
    lcd_send_data(spi_handle, (const void*)&data, sizeof(uint8_t));
}

static void spi_send(const spi_device_handle_t spi_handle,
                     const void* const data,
                     const uint16_t num_bytes)
{
    spi_transaction_t transaction = {0};
    transaction.tx_buffer = data;
    transaction.length = num_bytes * BITS_IN_BYTE;
    spi_device_transmit(spi_handle, &transaction);
}

static void spi_send_byte(const spi_device_handle_t spi_handle,
                          const uint8_t data)
{
    spi_send(spi_handle, (const void*)&data, sizeof(uint8_t));
}

static inline void delay_ms(const uint16_t num_ms)
{
    vTaskDelay(pdMS_TO_TICKS(num_ms));
}

static inline uint16_t swap_bytes(const uint16_t num)
{
    return ((num & 0xFF) << BITS_IN_BYTE) | (num >> BITS_IN_BYTE);
}
