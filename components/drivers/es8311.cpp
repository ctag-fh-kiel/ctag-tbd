//
// Created by Robert Manzke on 21.12.24.
//

#include "es8311.hpp"
#include "esp_check.h"
#include "driver/i2s_std.h"

#define EXAMPLE_SAMPLE_RATE     (44100)
#define EXAMPLE_MCLK_MULTIPLE   (256) // If not using 24-bit data width, 256 should be enough
#define EXAMPLE_MCLK_FREQ_HZ    (EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE)
#define I2C_NUM I2C_NUM_1 // 0 is used for OLED
#define I2C_SCL_IO      (GPIO_NUM_8)
#define I2C_SDA_IO      (GPIO_NUM_7)
#define GPIO_OUTPUT_PA  (GPIO_NUM_53)

static const char *TAG = "i2s_es8311";

es8311::es8311() {
    gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << GPIO_OUTPUT_PA), // 选择GPIO48
            .mode = GPIO_MODE_OUTPUT,                  // 配置为输出模式
            .pull_up_en = GPIO_PULLUP_DISABLE,         // 禁用上拉
            .pull_down_en = GPIO_PULLDOWN_DISABLE,     // 禁用下拉
            .intr_type = GPIO_INTR_DISABLE             // 禁用中断
    };
    gpio_config(&io_conf);

    // 设置GPIO48为高电平
    gpio_set_level(GPIO_OUTPUT_PA, 1);

    const i2c_config_t es_i2c_cfg = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = I2C_SDA_IO,
            .scl_io_num = I2C_SCL_IO,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master = {
                    .clk_speed = 100000,
            },
            .clk_flags = 0,
    };
    i2c_param_config(I2C_NUM, &es_i2c_cfg);
    i2c_driver_install(I2C_NUM, I2C_MODE_MASTER,  0, 0, 0);

    /* Initialize es8311 codec */
    es_handle = es8311_create(I2C_NUM, ES8311_ADDRRES_0);
}

es8311::~es8311() {
    es8311_delete(es_handle);
    i2c_driver_delete(i2c_port_t(I2C_NUM));
}

void es8311::init() {

    es8311_microphone_config(es_handle, false);
}

bool es8311::identify() {
    const es8311_clock_config_t es_clk = {
            .mclk_inverted = false,
            .sclk_inverted = false,
            .mclk_from_mclk_pin = true,
            .mclk_frequency = EXAMPLE_MCLK_FREQ_HZ,
            .sample_frequency = EXAMPLE_SAMPLE_RATE
    };
    bool res = es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16);
    es8311_sample_frequency_config(es_handle, EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE, EXAMPLE_SAMPLE_RATE);

    return res;
}

bool es8311::setOutputVolume(uint8_t lvol, uint8_t rvol) {
    return es8311_voice_volume_set(es_handle, lvol, NULL);
}
