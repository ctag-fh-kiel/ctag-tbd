import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID

import tbd_core.buildgen as tbd
import tbd_core.utils as tbu


REQUIRES = ['tbd_module']
AUTO_LOAD = ['tbd_api']

tbd_spi_api_ns = cg.esphome_ns.namespace("tbd_spi_api")
SPIServer = tbd_spi_api_ns.class_("SPIServer", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SPIServer),
    cv.Required(tbu.CONF_PINS): tbu.SPIPinConfig,
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    component = tbd.new_tbd_component(__file__, auto_reflect=tbd.AutoReflection.ALL)

    defines = tbu.get_spi_pinout_defines(f'TBD_SPI_API', config[tbu.CONF_PINS])
    for key, value in defines:
        component.add_define(key, value)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

