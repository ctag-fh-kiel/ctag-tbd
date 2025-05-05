import esphome.components.tbd_module as tbd

import esphome.codegen as cg
import esphome.config_validation as cv

AUTO_LOAD = ['tbd_module']

CONFIG_SCHEMA = cv.Schema({})

async def to_code(config):
    tbd.new_tbd_component(__file__)

    websocket_server = cg.global_ns.namespace('tbd').namespace('api').namespace('WebsocketServer')
    cg.add(websocket_server.begin())
