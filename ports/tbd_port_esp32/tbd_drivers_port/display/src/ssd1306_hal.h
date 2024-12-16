#pragma once

#include <tbd/drivers/port/ssd1306.h>


TBD_C_BEGIN

void ssd1306_hal_master_init(SSD1306_t * dev);
void ssd1306_hal_device_add(SSD1306_t * dev, i2c_port_t i2c_num, int16_t reset, uint16_t i2c_address);
void ssd1306_hal_init(SSD1306_t * dev, int width, int height);
void ssd1306_hal_display_image(SSD1306_t * dev, int page, int seg, uint8_t * images, int width);
void ssd1306_hal_contrast(SSD1306_t * dev, int contrast);
void ssd1306_hal_hardware_scroll(SSD1306_t * dev, ssd1306_scroll_type_t scroll);

TBD_C_END
