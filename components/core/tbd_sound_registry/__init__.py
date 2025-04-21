import logging
from pathlib import Path

import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.core import CORE
from esphome.components.tbd_module import new_tbd_component
from esphome.components.tbd_sound_registry.preprocessor import search_for_plugins, write_plugin_factory_header


_LOGGER = logging.getLogger(__name__)


CONF_WHITELIST = 'whitelist'
CONF_BLACKLIST = 'blacklist'

SOUND_COLLECTION_SCHEMA = cv.Schema({
    cv.Optional(cv.Exclusive('whitelist', 'blacklist')): cv.ensure_list(cv.string)
})

def add_tbd_sounds(file: str, config: cv.Schema):
    component = new_tbd_component(file)
    headers = []
    include_dirs = component.get_include_dirs()

    headers = [file 
                for include_path in include_dirs
                for file in include_path.glob('**/*.hpp')
              ]

    _, plugins = search_for_plugins(headers, False)

    for header in headers:
        print(header)

    if (whitelist := config.get(CONF_WHITELIST)) is not None:
        whitelist = set(plugin.lower() for plugin in whitelist)
        plugins = [plugin for plugin in plugins if plugin.name.lower() in whitelist]
    if (blacklist := config.get(CONF_BLACKLIST)) is not None:
        blacklist = set(plugin.lower() for plugin in whitelist)
        plugins = [plugin for plugin in plugins if plugin.name.lower() not in blacklist]
    else:
        pass

    headers = set(plugin.header for plugin in plugins)


    out_path = Path(CORE.build_path) / 'src' / 'tbd' / 'sound_registry'
    out_path.mkdir(parents=True, exist_ok=True)

    out_file = out_path / 'sound_processor_factory.hpp'
    write_plugin_factory_header(headers, plugins, out_file)

