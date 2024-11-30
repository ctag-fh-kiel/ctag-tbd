#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"

#include "ssd1306.h"

#define TAG "SSD1306"

#define I2C_NUM I2C_NUM_0

#define I2C_MASTER_FREQ_HZ 400000 // I2C clock of SSD1306 can run at 400 kHz max.
#define I2C_TICKS_TO_WAIT 100	  // Maximum ticks to wait before issuing a timeout.


void i2c_master_init(SSD1306_t * dev, int16_t sda, int16_t scl, int16_t reset)
{
	ESP_LOGI(TAG, "Legacy i2c driver is used");
	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = sda,
		.scl_io_num = scl,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = I2C_MASTER_FREQ_HZ
	};
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM, &i2c_config));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM, I2C_MODE_MASTER, 0, 0, ESP_INTR_FLAG_SHARED|ESP_INTR_FLAG_LOWMED));

	if (reset >= 0) {
		//gpio_pad_select_gpio(reset);
		gpio_reset_pin(reset);
		gpio_set_direction(reset, GPIO_MODE_OUTPUT);
		gpio_set_level(reset, 0);
		vTaskDelay(50 / portTICK_PERIOD_MS);
		gpio_set_level(reset, 1);
	}

	dev->_address = I2C_ADDRESS;
	dev->_flip = false;
	dev->_i2c_num = I2C_NUM;
}

void i2c_device_add(SSD1306_t * dev, i2c_port_t i2c_num, int16_t reset, uint16_t i2c_address)
{
	ESP_LOGI(TAG, "Legacy i2c driver is used");
	ESP_LOGW(TAG, "Will not install i2c master driver");
#if 0
	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = sda,
		.scl_io_num = scl,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = I2C_MASTER_FREQ_HZ
	};
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM, &i2c_config));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM, I2C_MODE_MASTER, 0, 0, 0));
#endif

	if (reset >= 0) {
		//gpio_pad_select_gpio(reset);
		gpio_reset_pin(reset);
		gpio_set_direction(reset, GPIO_MODE_OUTPUT);
		gpio_set_level(reset, 0);
		vTaskDelay(50 / portTICK_PERIOD_MS);
		gpio_set_level(reset, 1);
	}

	dev->_address = i2c_address;
	dev->_flip = false;
	dev->_i2c_num = i2c_num;
}

void i2c_init(SSD1306_t * dev, int width, int height) {
	dev->_width = width;
	dev->_height = height;
	dev->_pages = 8;
	if (dev->_height == 32) dev->_pages = 4;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev->_address << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_OFF, true);				// AE
	i2c_master_write_byte(cmd, OLED_CMD_SET_MUX_RATIO, true);			// A8
	if (dev->_height == 64) i2c_master_write_byte(cmd, 0x3F, true);
	if (dev->_height == 32) i2c_master_write_byte(cmd, 0x1F, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_OFFSET, true);		// D3
	i2c_master_write_byte(cmd, 0x00, true);
	//i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);	// 40
	i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_START_LINE, true);	// 40
	//i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true);		// A1
	if (dev->_flip) {
		i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP_0, true); // A0
	} else {
		i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP_1, true);	// A1
	}
	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true);		// C8
	i2c_master_write_byte(cmd, OLED_CMD_SET_DISPLAY_CLK_DIV, true);		// D5
	i2c_master_write_byte(cmd, 0x80, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_PIN_MAP, true);			// DA
	if (dev->_height == 64) i2c_master_write_byte(cmd, 0x12, true);
	if (dev->_height == 32) i2c_master_write_byte(cmd, 0x02, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true);			// 81
	i2c_master_write_byte(cmd, 0xFF, true);
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_RAM, true);				// A4
	i2c_master_write_byte(cmd, OLED_CMD_SET_VCOMH_DESELCT, true);		// DB
	i2c_master_write_byte(cmd, 0x40, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_MEMORY_ADDR_MODE, true);	// 20
	//i2c_master_write_byte(cmd, OLED_CMD_SET_HORI_ADDR_MODE, true);	// 00
	i2c_master_write_byte(cmd, OLED_CMD_SET_PAGE_ADDR_MODE, true);		// 02
	// Set Lower Column Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, 0x00, true);
	// Set Higher Column Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, 0x10, true);
	i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);			// 8D
	i2c_master_write_byte(cmd, 0x14, true);
	i2c_master_write_byte(cmd, OLED_CMD_DEACTIVE_SCROLL, true);			// 2E
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_NORMAL, true);			// A6
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);				// AF

	i2c_master_stop(cmd);

	esp_err_t res = i2c_master_cmd_begin(dev->_i2c_num, cmd, I2C_TICKS_TO_WAIT);
	if (res == ESP_OK) {
		ESP_LOGI(TAG, "OLED configured successfully");
	} else {
		ESP_LOGE(TAG, "OLED configuration failed. code: 0x%.2X", res);
	}
	i2c_cmd_link_delete(cmd);
}


void i2c_display_image(SSD1306_t * dev, int page, int seg, uint8_t * images, int width) {
	if (page >= dev->_pages) return;
	if (seg >= dev->_width) return;

	//int _seg = seg + CONFIG_OFFSETX;

	// display has 2 x offset
	int _seg = seg + 2;
	uint8_t columLow = _seg & 0x0F;
	uint8_t columHigh = (_seg >> 4) & 0x0F;

	int _page = page;
	if (dev->_flip) {
		_page = (dev->_pages - page) - 1;
	}

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev->_address << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	// Set Lower Column Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, (0x00 + columLow), true);
	// Set Higher Column Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, (0x10 + columHigh), true);
	// Set Page Start Address for Page Addressing Mode
	i2c_master_write_byte(cmd, 0xB0 | _page, true);

	i2c_master_stop(cmd);
	esp_err_t res = i2c_master_cmd_begin(dev->_i2c_num, cmd, I2C_TICKS_TO_WAIT);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "Image command failed. code: 0x%.2X", res);
	}
	i2c_cmd_link_delete(cmd);

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev->_address << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
	i2c_master_write(cmd, images, width, true);
	i2c_master_stop(cmd);

	res = i2c_master_cmd_begin(dev->_i2c_num, cmd, I2C_TICKS_TO_WAIT);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "Image command failed. code: 0x%.2X", res);
	}
	i2c_cmd_link_delete(cmd);
}

void i2c_contrast(SSD1306_t * dev, int contrast) {
	int _contrast = contrast;
	if (contrast < 0x0) _contrast = 0;
	if (contrast > 0xFF) _contrast = 0xFF;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (dev->_address << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true); // 00
	i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true); // 81
	i2c_master_write_byte(cmd, _contrast, true);
	i2c_master_stop(cmd);

	esp_err_t res = i2c_master_cmd_begin(dev->_i2c_num, cmd, I2C_TICKS_TO_WAIT);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "Contrast command failed. code: 0x%.2X", res);
	}
	i2c_cmd_link_delete(cmd);
}


void i2c_hardware_scroll(SSD1306_t * dev, ssd1306_scroll_type_t scroll) {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);

	i2c_master_write_byte(cmd, (dev->_address << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true); // 00

	if (scroll == SCROLL_RIGHT) {
		i2c_master_write_byte(cmd, OLED_CMD_HORIZONTAL_RIGHT, true); // 26
		i2c_master_write_byte(cmd, 0x00, true); // Dummy byte
		i2c_master_write_byte(cmd, 0x00, true); // Define start page address
		i2c_master_write_byte(cmd, 0x07, true); // Frame frequency
		i2c_master_write_byte(cmd, 0x07, true); // Define end page address
		i2c_master_write_byte(cmd, 0x00, true); //
		i2c_master_write_byte(cmd, 0xFF, true); //
		i2c_master_write_byte(cmd, OLED_CMD_ACTIVE_SCROLL, true); // 2F
	}

	if (scroll == SCROLL_LEFT) {
		i2c_master_write_byte(cmd, OLED_CMD_HORIZONTAL_LEFT, true); // 27
		i2c_master_write_byte(cmd, 0x00, true); // Dummy byte
		i2c_master_write_byte(cmd, 0x00, true); // Define start page address
		i2c_master_write_byte(cmd, 0x07, true); // Frame frequency
		i2c_master_write_byte(cmd, 0x07, true); // Define end page address
		i2c_master_write_byte(cmd, 0x00, true); //
		i2c_master_write_byte(cmd, 0xFF, true); //
		i2c_master_write_byte(cmd, OLED_CMD_ACTIVE_SCROLL, true); // 2F
	}

	if (scroll == SCROLL_DOWN) {
		i2c_master_write_byte(cmd, OLED_CMD_CONTINUOUS_SCROLL, true); // 29
		i2c_master_write_byte(cmd, 0x00, true); // Dummy byte
		i2c_master_write_byte(cmd, 0x00, true); // Define start page address
		i2c_master_write_byte(cmd, 0x07, true); // Frame frequency
		//i2c_master_write_byte(cmd, 0x01, true); // Define end page address
		i2c_master_write_byte(cmd, 0x00, true); // Define end page address
		i2c_master_write_byte(cmd, 0x3F, true); // Vertical scrolling offset

		i2c_master_write_byte(cmd, OLED_CMD_VERTICAL, true); // A3
		i2c_master_write_byte(cmd, 0x00, true);
		if (dev->_height == 64)
		//i2c_master_write_byte(cmd, 0x7F, true);
		i2c_master_write_byte(cmd, 0x40, true);
		if (dev->_height == 32)
		i2c_master_write_byte(cmd, 0x20, true);
		i2c_master_write_byte(cmd, OLED_CMD_ACTIVE_SCROLL, true); // 2F
	}

	if (scroll == SCROLL_UP) {
		i2c_master_write_byte(cmd, OLED_CMD_CONTINUOUS_SCROLL, true); // 29
		i2c_master_write_byte(cmd, 0x00, true); // Dummy byte
		i2c_master_write_byte(cmd, 0x00, true); // Define start page address
		i2c_master_write_byte(cmd, 0x07, true); // Frame frequency
		//i2c_master_write_byte(cmd, 0x01, true); // Define end page address
		i2c_master_write_byte(cmd, 0x00, true); // Define end page address
		i2c_master_write_byte(cmd, 0x01, true); // Vertical scrolling offset

		i2c_master_write_byte(cmd, OLED_CMD_VERTICAL, true); // A3
		i2c_master_write_byte(cmd, 0x00, true);
		if (dev->_height == 64)
		//i2c_master_write_byte(cmd, 0x7F, true);
		i2c_master_write_byte(cmd, 0x40, true);
		if (dev->_height == 32)
		i2c_master_write_byte(cmd, 0x20, true);
		i2c_master_write_byte(cmd, OLED_CMD_ACTIVE_SCROLL, true); // 2F
	}

	if (scroll == SCROLL_STOP) {
		i2c_master_write_byte(cmd, OLED_CMD_DEACTIVE_SCROLL, true); // 2E
	}

	i2c_master_stop(cmd);

	esp_err_t res = i2c_master_cmd_begin(dev->_i2c_num, cmd, I2C_TICKS_TO_WAIT);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "Scroll command failed. code: 0x%.2X", res);
	}
	i2c_cmd_link_delete(cmd);
}