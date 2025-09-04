from esphome.components.tbd_control_inputs import new_tbd_control_input
from esphome.components.esp32 import add_idf_component, add_idf_sdkconfig_option

import esphome.config_validation as cv

import tbd_core.utils as tbu

REQUIRES = ['tbd_module']
AUTO_LOAD = ['tbd_control_inputs']

NUM_CHANNELS = 90
NUM_TRIGGERS = 40


CONFIG_SCHEMA = cv.Schema({
    cv.Required(tbu.CONF_PINS): tbu.SPIPinConfig,
})


def to_code(config):
    component = new_tbd_control_input(__file__, NUM_CHANNELS, NUM_TRIGGERS, config).module

    defines = tbu.get_spi_pinout_defines(f'TBD_SPI_INPUTS', config[tbu.CONF_PINS])
    for key, value in defines:
        component.add_define(key, value)

    component.add_define('USE_ESP_TINYUSB')
    add_idf_component(
        name="esp_tinyusb",
        ref="1.7.6",
    )

    # configure USB MIDI in IDF
    add_idf_sdkconfig_option('CONFIG_TINYUSB_MIDI_COUNT', 1)
    component.add_define('TBD_MIDI_INPUT_USB')

    # FIXME: USB networking should be its own component!
    # configure USB networking in IDF
    add_idf_sdkconfig_option('CONFIG_TINYUSB_NET_MODE_NCM', True)
    add_idf_sdkconfig_option('CONFIG_TINYUSB_NCM_OUT_NTB_BUFFS_COUNT', 3)
    add_idf_sdkconfig_option('CONFIG_TINYUSB_NCM_IN_NTB_BUFFS_COUNT', 3)
    add_idf_sdkconfig_option('CONFIG_TINYUSB_NCM_OUT_NTB_BUFF_MAX_SIZE', 3200)
    add_idf_sdkconfig_option('CONFIG_TINYUSB_NCM_IN_NTB_BUFF_MAX_SIZE', 3200)
