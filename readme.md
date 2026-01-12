# ILI9341 Driver

Basic driver for ILI9341 display on ESP-IDF. The display is connected using the SPI interface.

During the development process, I used the technical documentation (provided below), analyzed the code of working libraries ([Adafruit ILI9341](https://github.com/adafruit/Adafruit_ILI9341), [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)) and [reddit.com/r/esp32](https://reddit.com/r/esp32):
- https://wiki.amperka.ru/_media/products:display-raspberry-pi-2n8in-spi:ili9341_datasheet.pdf
- https://www.alse-fr.com/sites/alse-fr.com/IMG/pdf/an_lt24.pdf
- https://docs.mikroe.com/mikrosdk/ref-manual/group__ili9341__commands.html


## Methods
#### Initializing the display
```c
esp_err_t ili9341_begin(gpio_num_t cs, gpio_num_t dc, gpio_num_t reset, 
  gpio_num_t miso, gpio_num_t mosi, gpio_num_t sclk, uint8_t speed_mhz);
```

#### Deinit the display
```c
esp_err_t ili9341_stop(void);
```

### Basic initialization of the display
```c
esp_err_t ili9341_init(void);
```

#### Connecting the display using the SPI interface
```c
esp_err_t _ili9341_spi_connect(uint8_t speed_mhz);
```

#### Sending command on the display
```c
esp_err_t ili9341_write_command(uint16_t cmd);
```

#### Sending data on the display
```c
esp_err_t ili9341_write_data(uint16_t data);
```

#### Hardware reset the display
```c
void ili9341_reset(void);
```

#### Setting the rendering window
```c
esp_err_t ili9341_set_addr(uint16_t x, uint16_t y, uint16_t height, uint16_t width);
```

## Example code

**CMakeLists.txt**
```cmake
FILE(GLOB_RECURSE app *.*)
FILE(GLOB_RECURSE libs ../lib/*.c)
idf_component_register(
  SRCS ${app} ${libs}
  INCLUDE_DIRS "." ".." "../lib/"
)
```

**main.c**
```c
#include <lib/ili9341.h>

#define TFT_CS    GPIO_NUM_4
#define TFT_DC    GPIO_NUM_17
#define TFT_RESET GPIO_NUM_16
#define TFT_MISO  GPIO_NUM_19
#define TFT_MOSI  GPIO_NUM_23
#define TFT_SCLK  GPIO_NUM_18
#define TFT_SPEED 33 /* MHz Speed of the display */

void draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
  if (!display) {
    puts("Display not initialized.");
    return;
  };

  if ((x >= display->width) || (y >= display->height)) return;
  ili9341_set_addr(x,y,1,1);
  /* The display accepts the BGR format. */ 
  ili9341_write_data(((color>>8)|(color<<8))&0xFF);
  ili9341_write_data(((color>>8)|(color<<8))>>8);
  ili9341_write_command(0x2C);
};

void app_main(void) {
  if (ili9341_begin(TFT_CS, TFT_DC, TFT_RESET, 
  TFT_MISO, TFT_MOSI, TFT_SCLK, TFT_SPEED) != ESP_OK) {
    puts("Error while initialized the ili9341.");
  } else printf("ILI9341 is initialized");

  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 32; j++) 
      draw_pixel(i,j,0xf800);
  };
};
```
### Result:
<img src="result.png" height=512>
