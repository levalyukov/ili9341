#ifndef ILI9341_DRIVER_H
#define ILI9341_DRIVER_H

#if defined(ESP_PLATFORM)
  #include <stdio.h>
  #include <stdint.h>
  #include <string.h>
  #include "freertos/FreeRTOS.h"
  #include "driver/spi_master.h"
  #include "esp_err.h"
  #include "driver/adc.h"
  #include "driver/gpio.h"
#else 
  #error "Invalid platform. Only ESP-IDF."
#endif

typedef struct {
  uint16_t height,width; 
  gpio_num_t cs,dc,reset,miso,mosi,sclk;
  spi_bus_config_t buscfg;
  spi_device_interface_config_t devcfg;
  spi_device_handle_t spi;
} ili9341_t;

extern ili9341_t* display;

esp_err_t ili9341_begin(gpio_num_t cs, gpio_num_t dc, gpio_num_t reset, 
  gpio_num_t miso, gpio_num_t mosi, gpio_num_t sclk, uint8_t speed_mhz);
esp_err_t ili9341_stop(void);
esp_err_t _ili9341_spi_connect(uint8_t speed_mhz);
esp_err_t ili9341_write_command(uint16_t cmd);
esp_err_t ili9341_write_data(uint16_t data);
esp_err_t ili9341_init(void);
esp_err_t ili9341_set_addr(uint16_t x, uint16_t y, uint16_t h, uint16_t w);
void ili9341_reset(void);

#endif