#include "ili9341.h"

struct tft *display;
esp_err_t ili9341_setup(gpio_num_t _led, gpio_num_t _cs, 
  gpio_num_t _dc, gpio_num_t _reset, gpio_num_t _miso, 
  gpio_num_t _mosi, gpio_num_t _sclk) {
  display = (struct tft*)malloc(sizeof(struct tft));
  if (display == NULL) {
    printf("Error to allocate memory for the display structure.\n");
    return ESP_ERR_NO_MEM;
  };

  display->cs = _cs;
  display->miso = _miso;
  display->mosi = _mosi;
  display->sclk = _sclk;
  if (ili9341_spi_connect() != ESP_OK) {
    printf("Error while connection spi interface for the display.\n");
    return ESP_FAIL;
  };

  display->dc = _dc;
  display->reset = _reset;
  gpio_reset_pin(display->dc);
  gpio_set_direction(display->dc, GPIO_MODE_OUTPUT);
  gpio_reset_pin(display->reset);
  gpio_set_direction(display->reset, GPIO_MODE_OUTPUT);

  ili9341_reset();
  if (ili9341_init() != ESP_OK) {
    printf("Error while initializating the display.\n");
    return ESP_FAIL;
  };

  return ESP_OK;
};

esp_err_t ili9341_spi_connect(void) {
  display->buscfg = (spi_bus_config_t) {
    .mosi_io_num = display->mosi,
    .miso_io_num = display->miso,
    .sclk_io_num = display->sclk,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
  };

  display->devcfg = (spi_device_interface_config_t) {
    .clock_speed_hz = 33 * 1000 * 1000,
    .mode = 0,
    .spics_io_num = display->cs,
    .queue_size = 7,
    .address_bits = 0 /* <-- The display does not use it. */
  };

  if (spi_bus_initialize(
  HSPI_HOST, &display->buscfg, SPI_DMA_CH_AUTO) != ESP_OK) 
    printf("Error while initializing SPI bus.\n");

  if (spi_bus_add_device(HSPI_HOST, &display->devcfg, &display->spi) != ESP_OK) {
    printf("Error while adding device to SPI bus.\n");
    return ESP_FAIL;
  };

  return ESP_OK;
};

esp_err_t ili9341_write_command(uint16_t cmd) {
  gpio_set_level(display->dc, 0);
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length = 8;
  t.tx_buffer = &cmd;
  if (spi_device_polling_transmit(display->spi, &t) != ESP_OK) {
    printf("Error while sending the command.\n");
    return ESP_FAIL;
  }; return ESP_OK;
};

esp_err_t ili9341_write_data(uint16_t data) {
  gpio_set_level(display->dc, 1);
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length = 8;
  t.tx_buffer = &data;
  if (spi_device_polling_transmit(display->spi, &t) != ESP_OK) {
    printf("Error while sending the data.\n");
    return ESP_FAIL;
  }; return ESP_OK;
};

esp_err_t ili9341_init(void) {
  /* Software reset */
  if (ili9341_write_data(context,0x01) != ESP_OK) {
    LOG_ERROR("Error while software reset of the display.");
    return ESP_FAIL;
  };

  /* Exit Sleep, need 800 ms delay */
  if (ili9341_write_command(context,0x11) != ESP_OK) {
    LOG_ERROR("Error while exit the display out of sleep.");
    return ESP_FAIL;
  }; vTaskDelay(pdMS_TO_TICKS(800));

  /* Set COLMOD with parameter "16 bits per pixel, one data per pixel" */
  if (ili9341_write_command(context,0x3A) != ESP_OK 
  || ili9341_write_data(context,0x55) != ESP_OK) {
    LOG_ERROR("Error while enabled colmod of the display.");
    return ESP_FAIL;
  };

  /* MADCTL */
  if (ili9341_write_command(context,0x36) != ESP_OK 
  || ili9341_write_data(context,0x48) != ESP_OK) {
    LOG_ERROR("Error while config MADCTL of the display.");
    return ESP_FAIL;
  };

  /* Set Gamma */
  if (ili9341_write_command(context,0xF2) != ESP_OK 
  || ili9341_write_data(context,0x0F) != ESP_OK) {
    LOG_ERROR("Error while set gamme of the display.");
    return ESP_FAIL;
  };

  /* ili9341 ON */
  if (ili9341_write_command(context,0x29) != ESP_OK) {
    LOG_ERROR("Error while enabled the display.");
    return ESP_FAIL;
  }; vTaskDelay(pdMS_TO_TICKS(10));

  return ESP_OK;
};

void ili9341_reset(void) {
  /*
    It follows from the documentation that it is worth setting 
    the timing for a hardware reset to 300 ms:

    https://www.alse-fr.com/sites/alse-fr.com/IMG/pdf/an_lt24.pdf
    - Page №5, Section "Init Sequence"
  */
  gpio_set_level(display->reset, 0);
  vTaskDelay(pdMS_TO_TICKS(50));
  gpio_set_level(display->reset, 1);
  vTaskDelay(pdMS_TO_TICKS(300));
};

esp_err_t ili9341_set_address_window(uint16_t x, uint16_t y, uint16_t h, uint16_t w) {
  if ((x+w >= WIDTH) || (y+h >= HEIGHT)) return ESP_ERR_INVALID_ARG;

  ili9341_write_command(0x2A);
  ili9341_write_data(x>>8);
  ili9341_write_data(x&0xFF);
  ili9341_write_data((x+w-1)>>8); 
  ili9341_write_data((x+w-1)&0xFF);

  ili9341_write_command(0x2B);
  ili9341_write_data(y>>8);
  ili9341_write_data(y&0xFF);
  ili9341_write_data((y+h-1)>>8);
  ili9341_write_data((y+h-1)&0xFF);

  ili9341_write_command(0x2C);

  return ESP_OK;
};

esp_err_t ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
  if ((x >= WIDTH) || (y >= HEIGHT)) return ESP_ERR_INVALID_ARG;
  ili9341_set_address_window(x,y,1,1);
  ili9341_write_data(((color>>8)|(color<<8))&0xFF);
  ili9341_write_data(((color>>8)|(color<<8))>>8);
  ili9341_write_command(0x2C);
  return ESP_OK;
};
