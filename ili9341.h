#ifndef ILI9341_H
#define ILI9341_H

#include <stdint.h>
#include <string.h>
#include "logger.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_err.h"

#define HEIGHT  240
#define WIDTH   320

#define ILI93141_WHITE  0xFFFF
#define ILI93141_BLACK  0x0000
#define ILI9341_RED     0xF800
#define ILI9341_GREEN   0x07E0
#define ILI9341_BLUE    0x00F1

struct tft {
  gpio_num_t led,cs,dc,reset,miso,mosi,sclk;
  spi_bus_config_t buscfg;
  spi_device_interface_config_t devcfg;
  spi_device_handle_t spi;
};

esp_err_t ili9341_setup(gpio_num_t _led, gpio_num_t _cs, gpio_num_t _dc, 
  gpio_num_t _reset, gpio_num_t _miso, gpio_num_t _mosi, gpio_num_t _sclk);
esp_err_t ili9341_spi_connect(void);
esp_err_t write_command(uint16_t cmd);
esp_err_t write_data(uint16_t data);
esp_err_t ili9341_init(void);

void ili9341_reset(void);
esp_err_t ili9341_set_address_window(uint16_t x, uint16_t y, uint16_t height, uint16_t width);
esp_err_t ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

#endif