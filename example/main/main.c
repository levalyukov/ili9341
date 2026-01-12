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