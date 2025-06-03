import esphome.components.tbd_module as tbd

import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.components.uart as uart
from esphome.const import CONF_ID

AUTO_LOAD = ['tbd_api']
DEPENDENCIES = ["uart"]

tbd_websocket_ns = cg.esphome_ns.namespace("tbd_uart")
WebsocketServer = tbd_websocket_ns.class_("UARTServer", cg.Component)

CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(): cv.declare_id(WebsocketServer),
    })
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    component = tbd.new_tbd_component(__file__)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

