from pathlib import Path
from esphome.components.tbd_control_inputs import new_tbd_control_input
import esphome.components.tbd_module as tbd
import esphome.config_validation as cv
from esphome.components.esp32 import add_idf_sdkconfig_option
import esphome.codegen as cg

import tbd_core.buildgen as tbb


DEPENDENCIES = ['esp32']
AUTO_LOAD = ['tbd_control_inputs']

NUM_CHANNELS = 4
NUM_TRIGGERS = 2

CONF_CALIBRATION = 'calibration'

CONFIG_SCHEMA = cv.Schema({
    cv.Optional(CONF_CALIBRATION, default=True): bool
})

async def to_code(config):
    control_input = new_tbd_control_input(__file__, NUM_CHANNELS, NUM_TRIGGERS, config)
    if config[CONF_CALIBRATION]:
        cg.add_define('TBD_CALIBRATION', 1)

    cmake_file = control_input.module.path / 'CMakeLists.txt'
    tbb.update_build_file_if_outdated(cmake_file, Path() / 'src' / 'CMakeLists.txt')
    adc_src_file = control_input.module.path / 'ulp' / 'adc.cpp'
    tbb.update_build_file_if_outdated(adc_src_file, Path() / 'src'/ 'adc.cpp')
    ulp_src_file = control_input.module.path / 'ulp' / 'adc.S'
    tbb.update_build_file_if_outdated(ulp_src_file, Path() / 'ulp'/ 'adc.S')
    control_input.module.add_define('TBD_CV_ADC')


    add_idf_sdkconfig_option('CONFIG_SOC_ULP_SUPPORTED', True)
    add_idf_sdkconfig_option('CONFIG_SOC_ULP_FSM_SUPPORTED', True)
    add_idf_sdkconfig_option('CONFIG_SOC_ULP_HAS_ADC', True)
    add_idf_sdkconfig_option('CONFIG_ULP_COPROC_ENABLED', True)
    add_idf_sdkconfig_option('CONFIG_ULP_COPROC_TYPE_FSM', True)
    add_idf_sdkconfig_option('CONFIG_ULP_COPROC_RESERVE_MEM', 2048)

