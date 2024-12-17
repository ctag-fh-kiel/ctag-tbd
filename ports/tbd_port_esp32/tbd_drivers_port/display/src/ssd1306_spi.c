#include <tbd/drivers/port/ssd1306.h>

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#if ! TBD_DISPLAY_SSD1306_SPI
	#error "spi display not available"
#endif

#define TAG "SSD1306"

#if CONFIG_SPI2_HOST
#define HOST_ID SPI2_HOST
#elif CONFIG_SPI3_HOST
#define HOST_ID SPI3_HOST
#else
#define HOST_ID SPI2_HOST // If i2c is selected
#endif

#define SPI_COMMAND_MODE 0
#define SPI_DATA_MODE 1
#define SPI_DEFAULT_FREQUENCY 1000000; // 1MHz

int clock_speed_hz = SPI_DEFAULT_FREQUENCY;

void spi_clock_speed(int speed) {
	ESP_LOGI(TAG, "SPI clock speed=%d MHz", speed/1000000);
	clock_speed_hz = speed;
}

void ssd1306_hal_master_init(SSD1306_t * dev)
{
	esp_err_t ret;

	gpio_set_direction(TBD_SSD1306_SPI_PIN_CS, GPIO_MODE_OUTPUT);
	gpio_set_level(TBD_SSD1306_SPI_PIN_CS, 0);

	gpio_reset_pin(TBD_SSD1306_SPI_PIN_DC);
	gpio_set_direction(TBD_SSD1306_SPI_PIN_DC, GPIO_MODE_OUTPUT);
	gpio_set_level(TBD_SSD1306_SPI_PIN_DC, 0);

	#if TBD_SSD1306_SPI_PIN_RESET >= 0
		gpio_reset_pin( TBD_SSD1306_SPI_PIN_RESET );
		gpio_set_direction( TBD_SSD1306_SPI_PIN_RESET, GPIO_MODE_OUTPUT );
		gpio_set_level( TBD_SSD1306_SPI_PIN_RESET, 0 );
		vTaskDelay( pdMS_TO_TICKS( 100 ) );
		gpio_set_level( TBD_SSD1306_SPI_PIN_RESET, 1 );
	#endif

	spi_bus_config_t spi_bus_config = {
		.mosi_io_num = TBD_SSD1306_SPI_PIN_MOSI,
		.miso_io_num = -1,
		.sclk_io_num = TBD_SSD1306_SPI_PIN_SCLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 0,
		.flags = 0
	};

	ESP_LOGI(TAG, "SPI HOST_ID=%d", HOST_ID);
	ret = spi_bus_initialize( HOST_ID, &spi_bus_config, SPI_DMA_CH_AUTO );
	ESP_LOGI(TAG, "spi_bus_initialize=%d",ret);
	assert(ret==ESP_OK);

	spi_device_interface_config_t devcfg;
	memset( &devcfg, 0, sizeof( spi_device_interface_config_t ) );
	//devcfg.clock_speed_hz = SPI_DEFAULT_FREQUENCY;
	devcfg.clock_speed_hz = clock_speed_hz;
	devcfg.spics_io_num = TBD_SSD1306_SPI_PIN_CS;
	devcfg.queue_size = 1;

	spi_device_handle_t spi_device_handle;
	ret = spi_bus_add_device( HOST_ID, &devcfg, &spi_device_handle);
	ESP_LOGI(TAG, "spi_bus_add_device=%d",ret);
	assert(ret==ESP_OK);

	dev->_dc = TBD_SSD1306_SPI_PIN_DC;
	dev->_address = SPI_ADDRESS;
	dev->_flip = false;
	dev->_spi_device_handle = spi_device_handle;
}

void ssd1306_hal_device_add(SSD1306_t * dev, int16_t cs, int16_t dc, int16_t reset)
{
	ESP_LOGW(TAG, "Will not install spi master driver");
	esp_err_t ret;

	gpio_reset_pin( cs );
	gpio_set_direction( cs, GPIO_MODE_OUTPUT );
	gpio_set_level( cs, 0 );

	gpio_reset_pin( dc );
	gpio_set_direction( dc, GPIO_MODE_OUTPUT );
	gpio_set_level( dc, 0 );

	if ( reset >= 0 ) {
		gpio_reset_pin( reset );
		gpio_set_direction( reset, GPIO_MODE_OUTPUT );
		gpio_set_level( reset, 0 );
		vTaskDelay( pdMS_TO_TICKS( 100 ) );
		gpio_set_level( reset, 1 );
	}

#if 0
	spi_bus_config_t spi_bus_config = {
		.mosi_io_num = mosi,
		.miso_io_num = -1,
		.sclk_io_num = sclk,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 0,
		.flags = 0
	};

	ESP_LOGI(TAG, "SPI HOST_ID=%d", HOST_ID);
	ret = spi_bus_initialize( HOST_ID, &spi_bus_config, SPI_DMA_CH_AUTO );
	ESP_LOGI(TAG, "spi_bus_initialize=%d",ret);
	assert(ret==ESP_OK);
#endif

	spi_device_interface_config_t devcfg;
	memset( &devcfg, 0, sizeof( spi_device_interface_config_t ) );
	//devcfg.clock_speed_hz = SPI_DEFAULT_FREQUENCY;
	devcfg.clock_speed_hz = clock_speed_hz;
	devcfg.spics_io_num = cs;
	devcfg.queue_size = 1;

	spi_device_handle_t spi_device_handle;
	ret = spi_bus_add_device( HOST_ID, &devcfg, &spi_device_handle);
	ESP_LOGI(TAG, "spi_bus_add_device=%d",ret);
	assert(ret==ESP_OK);

	dev->_dc = dc;
	dev->_address = SPI_ADDRESS;
	dev->_flip = false;
	dev->_spi_device_handle = spi_device_handle;
}


bool spi_master_write_byte(spi_device_handle_t SPIHandle, const uint8_t* Data, size_t DataLength )
{
	spi_transaction_t SPITransaction;

	if ( DataLength > 0 ) {
		memset( &SPITransaction, 0, sizeof( spi_transaction_t ) );
		SPITransaction.length = DataLength * 8;
		SPITransaction.tx_buffer = Data;
		spi_device_transmit( SPIHandle, &SPITransaction );
	}

	return true;
}

bool spi_master_write_command(SSD1306_t * dev, uint8_t Command )
{
	static uint8_t CommandByte = 0;
	CommandByte = Command;
	gpio_set_level( dev->_dc, SPI_COMMAND_MODE );
	return spi_master_write_byte( dev->_spi_device_handle, &CommandByte, 1 );
}

bool spi_master_write_data(SSD1306_t * dev, const uint8_t* Data, size_t DataLength )
{
	gpio_set_level( dev->_dc, SPI_DATA_MODE );
	return spi_master_write_byte( dev->_spi_device_handle, Data, DataLength );
}


void ssd1306_hal_init(SSD1306_t * dev, int width, int height)
{
	dev->_width = width;
	dev->_height = height;
	dev->_pages = 8;
	if (dev->_height == 32) dev->_pages = 4;

	spi_master_write_command(dev, OLED_CMD_DISPLAY_OFF);			// AE
	spi_master_write_command(dev, OLED_CMD_SET_MUX_RATIO);			// A8
	if (dev->_height == 64) spi_master_write_command(dev, 0x3F);
	if (dev->_height == 32) spi_master_write_command(dev, 0x1F);
	spi_master_write_command(dev, OLED_CMD_SET_DISPLAY_OFFSET);		// D3
	spi_master_write_command(dev, 0x00);
	spi_master_write_command(dev, OLED_CONTROL_BYTE_DATA_STREAM);	// 40
	if (dev->_flip) {
		spi_master_write_command(dev, OLED_CMD_SET_SEGMENT_REMAP_0);	// A0
	} else {
		spi_master_write_command(dev, OLED_CMD_SET_SEGMENT_REMAP_1);	// A1
	}
	//spi_master_write_command(dev, OLED_CMD_SET_SEGMENT_REMAP);		// A1
	spi_master_write_command(dev, OLED_CMD_SET_COM_SCAN_MODE);		// C8
	spi_master_write_command(dev, OLED_CMD_SET_DISPLAY_CLK_DIV);	// D5
	spi_master_write_command(dev, 0x80);
	spi_master_write_command(dev, OLED_CMD_SET_COM_PIN_MAP);		// DA
	if (dev->_height == 64) spi_master_write_command(dev, 0x12);
	if (dev->_height == 32) spi_master_write_command(dev, 0x02);
	spi_master_write_command(dev, OLED_CMD_SET_CONTRAST);			// 81
	spi_master_write_command(dev, 0xFF);
	spi_master_write_command(dev, OLED_CMD_DISPLAY_RAM);			// A4
	spi_master_write_command(dev, OLED_CMD_SET_VCOMH_DESELCT);		// DB
	spi_master_write_command(dev, 0x40);
	spi_master_write_command(dev, OLED_CMD_SET_MEMORY_ADDR_MODE);	// 20
	//spi_master_write_command(dev, OLED_CMD_SET_HORI_ADDR_MODE);	// 00
	spi_master_write_command(dev, OLED_CMD_SET_PAGE_ADDR_MODE);		// 02
	// Set Lower Column Start Address for Page Addressing Mode
	spi_master_write_command(dev, 0x00);
	// Set Higher Column Start Address for Page Addressing Mode
	spi_master_write_command(dev, 0x10);
	spi_master_write_command(dev, OLED_CMD_SET_CHARGE_PUMP);		// 8D
	spi_master_write_command(dev, 0x14);
	spi_master_write_command(dev, OLED_CMD_DEACTIVE_SCROLL);		// 2E
	spi_master_write_command(dev, OLED_CMD_DISPLAY_NORMAL);			// A6
	spi_master_write_command(dev, OLED_CMD_DISPLAY_ON);				// AF
}


void ssd1306_hal_display_image(SSD1306_t * dev, int page, int seg, uint8_t * images, int width)
{
	if (page >= dev->_pages) return;
	if (seg >= dev->_width) return;

	//int _seg = seg + CONFIG_OFFSETX;
	int _seg = seg + 2;
	uint8_t columLow = _seg & 0x0F;
	uint8_t columHigh = (_seg >> 4) & 0x0F;

	int _page = page;
	if (dev->_flip) {
		_page = (dev->_pages - page) - 1;
	}

	// Set Lower Column Start Address for Page Addressing Mode
	spi_master_write_command(dev, (0x00 + columLow));
	// Set Higher Column Start Address for Page Addressing Mode
	spi_master_write_command(dev, (0x10 + columHigh));
	// Set Page Start Address for Page Addressing Mode
	spi_master_write_command(dev, 0xB0 | _page);

	spi_master_write_data(dev, images, width);

}

void ssd1306_hal_contrast(SSD1306_t * dev, int contrast) {
	int _contrast = contrast;
	if (contrast < 0x0) _contrast = 0;
	if (contrast > 0xFF) _contrast = 0xFF;

	spi_master_write_command(dev, OLED_CMD_SET_CONTRAST);			// 81
	spi_master_write_command(dev, _contrast);
}

void ssd1306_hal_hardware_scroll(SSD1306_t * dev, ssd1306_scroll_type_t scroll)
{

	if (scroll == SCROLL_RIGHT) {
		spi_master_write_command(dev, OLED_CMD_HORIZONTAL_RIGHT);	// 26
		spi_master_write_command(dev, 0x00); // Dummy byte
		spi_master_write_command(dev, 0x00); // Define start page address
		spi_master_write_command(dev, 0x07); // Frame frequency
		spi_master_write_command(dev, 0x07); // Define end page address
		spi_master_write_command(dev, 0x00); //
		spi_master_write_command(dev, 0xFF); //
		spi_master_write_command(dev, OLED_CMD_ACTIVE_SCROLL);		// 2F
	} 

	if (scroll == SCROLL_LEFT) {
		spi_master_write_command(dev, OLED_CMD_HORIZONTAL_LEFT);	// 27
		spi_master_write_command(dev, 0x00); // Dummy byte
		spi_master_write_command(dev, 0x00); // Define start page address
		spi_master_write_command(dev, 0x07); // Frame frequency
		spi_master_write_command(dev, 0x07); // Define end page address
		spi_master_write_command(dev, 0x00); //
		spi_master_write_command(dev, 0xFF); //
		spi_master_write_command(dev, OLED_CMD_ACTIVE_SCROLL);		// 2F
	} 

	if (scroll == SCROLL_DOWN) {
		spi_master_write_command(dev, OLED_CMD_CONTINUOUS_SCROLL);	// 29
		spi_master_write_command(dev, 0x00); // Dummy byte
		spi_master_write_command(dev, 0x00); // Define start page address
		spi_master_write_command(dev, 0x07); // Frame frequency
		//spi_master_write_command(dev, 0x01); // Define end page address
		spi_master_write_command(dev, 0x00); // Define end page address
		spi_master_write_command(dev, 0x3F); // Vertical scrolling offset

		spi_master_write_command(dev, OLED_CMD_VERTICAL);			// A3
		spi_master_write_command(dev, 0x00);
		if (dev->_height == 64)
			spi_master_write_command(dev, 0x40);
		if (dev->_height == 32)
			spi_master_write_command(dev, 0x20);
		spi_master_write_command(dev, OLED_CMD_ACTIVE_SCROLL);		// 2F
	}

	if (scroll == SCROLL_UP) {
		spi_master_write_command(dev, OLED_CMD_CONTINUOUS_SCROLL);	// 29
		spi_master_write_command(dev, 0x00); // Dummy byte
		spi_master_write_command(dev, 0x00); // Define start page address
		spi_master_write_command(dev, 0x07); // Frame frequency
		//spi_master_write_command(dev, 0x01); // Define end page address
		spi_master_write_command(dev, 0x00); // Define end page address
		spi_master_write_command(dev, 0x01); // Vertical scrolling offset

		spi_master_write_command(dev, OLED_CMD_VERTICAL);			// A3
		spi_master_write_command(dev, 0x00);
		if (dev->_height == 64)
			spi_master_write_command(dev, 0x40);
		if (dev->_height == 32)
			spi_master_write_command(dev, 0x20);
		spi_master_write_command(dev, OLED_CMD_ACTIVE_SCROLL);		// 2F
	}

	if (scroll == SCROLL_STOP) {
		spi_master_write_command(dev, OLED_CMD_DEACTIVE_SCROLL);	// 2E
	}
}
