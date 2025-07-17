from pathlib import Path
import esphome.components.tbd_module as tbd

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_PORT, CONF_PATH, CONF_ID

import esphome.components.tbd_api as tbd_api

from tbd_core.buildgen.build_deps import cmake_dependency
from tbd_core.buildgen import AutoReflection

AUTO_LOAD = ['tbd_api']
DEPENDENCIES = ['network']


tbd_websocket_ns = cg.esphome_ns.namespace("tbd_websocket")
WebsocketServer = tbd_websocket_ns.class_("WebsocketServer", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(WebsocketServer),
    cv.Optional(CONF_PORT, default='7777'): cv.port,
    cv.Optional(CONF_PATH, '/ws'): str,
})

async def to_code(config):
    websocket_path = Path(config[CONF_PATH])
    if not websocket_path.is_absolute:
        websocket_path = Path('/') / websocket_path

    component = tbd.new_tbd_component(__file__, auto_reflect=AutoReflection.ALL)
    component.add_define('TBD_WEBSOCKET_PORT', config[CONF_PORT])
    component.add_define('TBD_WEBSOCKET_PATH', f'"{websocket_path}"')

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if tbd.is_desktop():
        # cmake_dependency(
        #     name='simplewebserver',
        #     url='https://gitlab.com/eidheim/Simple-Web-Server.git',
        #     ref='v3.1.1',
        # )        
        
        component.add_dependency(cmake_dependency(
            name='simplewebsocketserver',
            url='https://gitlab.com/eidheim/Simple-WebSocket-Server.git',
            ref='v2.0.2',
        ))
        # cg.add_build_flag('-lssl')
        component.add_system_library('crypto')
    elif tbd.is_esp32():
        from esphome.components.esp32 import add_idf_sdkconfig_option
        add_idf_sdkconfig_option('CONFIG_HTTPD_WS_SUPPORT', True)