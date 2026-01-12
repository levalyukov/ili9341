#include "ili9341.h"

ili9341_t* display = NULL;
esp_err_t ili9341_begin(gpio_num_t cs, gpio_num_t dc, gpio_num_t reset,
  gpio_num_t miso, gpio_num_t mosi, gpio_num_t sclk, uint8_t speed_mhz) {
  if (display) {
    puts("Display has been initialized.");
    return ESP_FAIL;
  };

  display = malloc(sizeof(ili9341_t));
  if (!display) {
    puts("Error to allocate memory for the display structure.");
    return ESP_ERR_NO_MEM;
  };

  display->height = 320;
  display->width = 240;

  display->cs = cs;
  display->miso = miso;
  display->mosi = mosi;
  display->sclk = sclk;
  if (_ili9341_spi_connect(speed_mhz) != ESP_OK) {
    puts("Error while connection spi interface for the display.");
    return ESP_FAIL;
  };

  display->dc = dc;
  display->reset = reset;
  gpio_reset_pin(display->dc);
  gpio_set_direction(display->dc, GPIO_MODE_OUTPUT);
  gpio_reset_pin(display->reset);
  gpio_set_direction(display->reset, GPIO_MODE_OUTPUT);

  ili9341_reset();
  if (ili9341_init() != ESP_OK) {
    free(display);
    puts("Error while initializating the display.");
    return ESP_FAIL;
  }; return ESP_OK;
};

esp_err_t ili9341_stop(void) {
  if (display) {
    if (spi_bus_remove_device(display->spi) != ESP_OK) {
      puts("Error while remove device.");
      return ESP_FAIL;
    };
    
    free(display);
    display = NULL;
    puts("Display deinit.");
    return ESP_OK;
  } else return ESP_FAIL;
};

esp_err_t _ili9341_spi_connect(uint8_t speed_mhz) {
  if (!display) {
    puts("Display not initialized.");
    return ESP_FAIL;
  };

  display->buscfg = (spi_bus_config_t) {
    .mosi_io_num = display->mosi,
    .miso_io_num = display->miso,
    .sclk_io_num = display->sclk,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
  };

  display->devcfg = (spi_device_interface_config_t) {
    .clock_speed_hz = speed_mhz * 1000 * 1000,
    .mode = 0,
    .spics_io_num = display->cs,
    .queue_size = 7,
    .address_bits = 0 /* <-- The display does not use it. */
  };

  if (spi_bus_initialize(SPI2_HOST, 
  &display->buscfg, SPI_DMA_CH_AUTO) != ESP_OK) {
    puts("Error while initializing SPI bus.");
    return ESP_FAIL;
  };

  if (spi_bus_add_device(SPI2_HOST, 
  &display->devcfg, &display->spi) != ESP_OK) {
    puts("Error while adding device to SPI bus.");
    return ESP_FAIL;
  }; return ESP_OK;
};

esp_err_t ili9341_write_command(uint16_t cmd) {
  if (!display) {
    puts("Display not initialized.");
    return ESP_FAIL;
  };

  gpio_set_level(display->dc, 0);
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length = 8;
  t.tx_buffer = &cmd;
  if (spi_device_polling_transmit(
  display->spi, &t) != ESP_OK) {
    puts("Error while sending the command.");
    return ESP_FAIL;
  }; return ESP_OK;
};

esp_err_t ili9341_write_data(uint16_t data) {
  if (!display) {
    puts("Display not initialized.");
    return ESP_FAIL;
  };

  gpio_set_level(display->dc, 1);
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length = 8;
  t.tx_buffer = &data;
  if (spi_device_polling_transmit(
  display->spi, &t) != ESP_OK) {
    puts("Error while sending the data.");
    return ESP_FAIL;
  }; return ESP_OK;
};

esp_err_t ili9341_init(void) { 
  /* Software reset */
  if (ili9341_write_command(0x01) != ESP_OK) {
    puts("Error while software reset of the display.");
    return ESP_FAIL;
  };

  /* Exit Sleep, need 800 ms delay */
  if (ili9341_write_command(0x11) != ESP_OK) {
    puts("Error while exit the display out of sleep.");
    return ESP_FAIL;
  }; vTaskDelay(pdMS_TO_TICKS(800));

  /* Set COLMOD with parameter "16 bits per pixel, one data per pixel" */
  if (ili9341_write_command(0x3A) != ESP_OK 
  || ili9341_write_data(0x55) != ESP_OK) {
    puts("Error while enabled colmod of the display.");
    return ESP_FAIL;
  };

  /* MADCTL */
  if (ili9341_write_command(0x36) != ESP_OK 
  || ili9341_write_data(0x48) != ESP_OK) {
    puts("Error while config MADCTL of the display.");
    return ESP_FAIL;
  };

  /* Set Gamma */
  if (ili9341_write_command(0xF2) != ESP_OK 
  || ili9341_write_data(0x0F) != ESP_OK) {
    puts("Error while set gamme of the display.");
    return ESP_FAIL;
  };

  /* ili9341 ON */
  if (ili9341_write_command(0x29) != ESP_OK) {
    puts("Error while enabled the display.");
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

esp_err_t ili9341_set_addr(
  uint16_t x, uint16_t y, uint16_t h, uint16_t w) {
  if ((x+w >= display->width) || (y+h >= display->height)) return ESP_ERR_INVALID_ARG;

  if (ili9341_write_command(0x2A) != ESP_OK
  || ili9341_write_data(x>>8) != ESP_OK
  || ili9341_write_data(x&0xFF) != ESP_OK
  || ili9341_write_data((x+w-1)>>8) != ESP_OK 
  || ili9341_write_data((x+w-1)&0xFF) != ESP_OK) {
    puts("Error setting a row column in the frame memory.");
    return ESP_FAIL;
  };

  if (ili9341_write_command(0x2B) != ESP_OK
  || ili9341_write_data(y>>8) != ESP_OK
  || ili9341_write_data(y&0xFF) != ESP_OK
  || ili9341_write_data((y+h-1)>>8) != ESP_OK
  || ili9341_write_data((y+h-1)&0xFF) != ESP_OK) {
    puts("Error setting the page row in the frame memory.");
    return ESP_FAIL;
  }; 

  if (ili9341_write_command(0x2C) != ESP_OK) {
    puts("Error in transferring data from the microcontroller to the frame memory.");
    return ESP_FAIL;
  }; return ESP_OK;
};