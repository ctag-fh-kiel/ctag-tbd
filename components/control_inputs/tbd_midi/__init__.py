from esphome.components.tbd_control_inputs import new_tbd_control_input
import esphome.config_validation as cv
from esphome.components.esp32 import add_idf_component, add_idf_sdkconfig_option
import esphome.codegen as cg
from dataclasses import dataclass

AUTO_LOAD = ['tbd_control_inputs']
DEPENDENCIES = ['esp32']

NUM_CHANNELS = 90
NUM_TRIGGERS = 40

CONF_BUFFER_SIZE = 'buffer_size'
CONF_INPUTS = 'inputs'
CONF_INPUT_UART = 'UART'
CONF_INPUT_USB = 'USB'
CONF_INPUT_RP2040 = 'RP2040'

CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_BUFFER_SIZE, default=128): cv.uint32_t,
    cv.Required(CONF_INPUTS): [cv.one_of(
        CONF_INPUT_UART, CONF_INPUT_USB, CONF_INPUT_RP2040
    )]
})

async def to_code(config):
    buffer_size = config[CONF_BUFFER_SIZE]
    # fixme this define could easily clash with other defines in the code
    cg.add_build_flag(f'-DRX_BUF_SIZE={buffer_size}u')

    inputs = config[CONF_INPUTS]

    # evaluation should fail at list validator, just for good measure
    if len(inputs) == 0:
        raise cv.Invalid('MIDI control config requires at least one input')
    
    if CONF_INPUT_UART in inputs:
        # TODO: allow configuration

        cg.add_build_flag('-DTBD_MIDI_INPUT_UART=1')
    
    if CONF_INPUT_USB in inputs:
        # confusingly there are two tinyusb packages by espressif and both are required

        cg.add_define('USE_ESP_TINYUSB')
        add_idf_component(
            name="esp_tinyusb",
            repo="https://github.com/espressif/esp-usb.git",
            ref="f2e6b70aa3a9c439397d5c0f56b7c4d462f62c0c", # no version tags available in repo
            path="device/esp_tinyusb",
            # refresh='1day'
        )

        cg.add_define('USE_TINYUSB')
        add_idf_component(
            name="tinyusb",
            repo="https://github.com/espressif/tinyusb.git",
            ref="v0.18.0.2",
            # refresh='1day'
        )

        # required to enable USB MIDI in IDF
        add_idf_sdkconfig_option('CONFIG_TINYUSB_MIDI_COUNT', 1)
        cg.add_build_flag('-DTBD_MIDI_INPUT_USB=1')
    
    if CONF_INPUT_RP2040 in inputs:
        # TODO: allow pin configuration

        cg.add_build_flag('-DTBD_MIDI_INPUT_RP2040=1')

    new_tbd_control_input(__file__, NUM_CHANNELS, NUM_TRIGGERS, config)
