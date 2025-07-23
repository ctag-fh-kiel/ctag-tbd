import esphome.components.tbd_module as tbd

import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.components.spi as spi
from esphome.const import CONF_ID, CONF_SPI_ID

from tbd_core.buildgen import AutoReflection

AUTO_LOAD = ['tbd_api']
DEPENDENCIES = ["spi"]


tbd_spi_api_ns = cg.esphome_ns.namespace("tbd_spi_api")
WebsocketServer = tbd_spi_api_ns.class_("SPIServer", cg.Component)

CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(): cv.declare_id(WebsocketServer),
        cv.GenerateID(CONF_SPI_ID): cv.use_id(spi.SPIComponent)
    })
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    component = tbd.new_tbd_component(__file__, auto_reflect=AutoReflection.ALL)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

