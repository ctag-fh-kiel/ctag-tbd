import logging
from pathlib import Path

import humps

import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.core import CORE
from esphome.components.tbd_module import new_tbd_component
from esphome.components.tbd_sound_processor.preprocessor import (
    search_for_plugins, 
    write_plugin_factory_header, 
    write_plugin_reflection_info,
    write_meta_classes,
  )


_LOGGER = logging.getLogger(__name__)


CONF_WHITELIST = 'whitelist'
CONF_BLACKLIST = 'blacklist'

SOUND_COLLECTION_SCHEMA = cv.Schema({
    cv.Optional(cv.Exclusive('whitelist', 'blacklist')): cv.ensure_list(cv.string)
})

def add_tbd_sounds(init_file: str, config):
    """ Add a sound plugin module.

        Every class in a sound plugin component folder that inherits from `ctagSoundProcessor` 
        is assumed to be a plugin. And will automatically be detected by parsing all headers
        in the module include directory. 

        By extending the `CONFIG_SCHEMA` of a sound plugin component with 
        `SOUND_COLLECTION_SCHEMA` the `whitelist` and `blacklist` options will be added to
        the component YAML config. If present in the config only whitelisted or non 
        blacklisted plugins will be included in the firmware.

        To create a sound plugin
        
        - Create a header file and place into the `include` directory of a plugin component.
        - Implement a sound plugin by inheriting `ctagSoundProcessor`.
        - Name the source file for the plugin after the plugin class, where case will be
          ingored and snake and kebap cased versions of the class name are also valid.
        - Any common source files to always include in the build, can be placed in the `src`
          subfolder.


        :param str file: the `__file__` special variable of the component's `__init__.py`
        :param config: the validated component config as passed to `to_code`

        :return tbd_module.ComponentInfo: the newly created component
    """

    component = new_tbd_component(init_file)

    # find all plugins in this source tree
    include_dirs = component.get_include_dirs()
    headers = [file 
                for include_path in include_dirs
                for file in include_path.rglob('*.hpp')
              ]
    processor = search_for_plugins(headers, True)

    # filter plugins if inclusion rules present
    whitelist = config.get(CONF_WHITELIST)
    blacklist = config.get(CONF_BLACKLIST)
    processor.prefilter(whitelist=whitelist, blacklist=blacklist)
    selected_plugins = processor.plugins

    processor.preprocess()
    selected_headers = processor.headers

    # generate plugin factory
    gen_source_path = Path(CORE.build_path) / 'src' / 'generated'
    gen_include_path = gen_source_path / 'include'

    cg.add_build_flag(f'-I{gen_include_path}')
    gen_include_path.mkdir(parents=True, exist_ok=True)
    out_file = gen_include_path / 'tbd' / 'sound_registry' / 'sound_processor_factory.hpp'
    write_plugin_factory_header(selected_headers, selected_plugins, out_file)
    write_plugin_reflection_info(processor, gen_source_path)
    write_meta_classes(processor, gen_source_path)

    # add sources for selected plugins
    sources = (component.path / 'plugin_src').rglob('*.cpp')
    plugin_names = [plugin.cls_name for plugin in selected_plugins]
    _LOGGER.info('using plugins:')
    for plugin_name in plugin_names:
        _LOGGER.info(f'>>> {plugin_name}')

    for source_file in sources:
        for plugin_name in plugin_names:
            file_name = source_file.stem
            if plugin_name.lower() in [file_name.lower(), humps.decamelize(file_name), humps.kebabize(file_name)]:
                component.add_source_file(source_file)
                _LOGGER.info(f'adding extra plugin source {source_file}')
    
    return component
    
