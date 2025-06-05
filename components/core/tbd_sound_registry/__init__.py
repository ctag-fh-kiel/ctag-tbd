from esphome.components.tbd_module.python_dependencies import python_dependencies
python_dependencies(('humps', 'pyhumps'))

import logging
import humps

import esphome.config_validation as cv

import tbd_core.buildgen as tbd

from esphome.components.tbd_api import get_api_registry
from tbd_core.reflection import (
    search_for_plugins, 
    write_plugin_factory_header, 
    write_plugin_reflection_info,
    write_meta_classes,
    PluginReflectionGenerator,
)


_LOGGER = logging.getLogger(__name__)

SOUND_REGISTRY_GLOBAL = 'sound_registry'

CONF_WHITELIST = 'whitelist'
CONF_BLACKLIST = 'blacklist'


SOUND_COLLECTION_SCHEMA = cv.Schema({
    cv.Optional(cv.Exclusive('whitelist', 'blacklist')): cv.ensure_list(cv.string)
})


@tbd.generated_tbd_global(SOUND_REGISTRY_GLOBAL)
def get_plugin_registry() -> PluginReflectionGenerator:
    return PluginReflectionGenerator()


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

    component = tbd.new_tbd_component(init_file)

    # find all plugins in this source tree
    include_dirs = component.get_default_include_dirs()
    headers = [file 
                for include_path in include_dirs
                for file in include_path.rglob('*.hpp')
              ]
    reflectables = search_for_plugins(headers, True)
    processor = get_plugin_registry()

    # filter plugins if inclusion rules present
    whitelist = config.get(CONF_WHITELIST)
    blacklist = config.get(CONF_BLACKLIST)
    selected_module_plugins = processor.add(reflectables, whitelist=whitelist, blacklist=blacklist)

    # add sources for selected plugins
    plugin_names = [plugin.cls_name for plugin in selected_module_plugins]
    sources = (component.path / 'plugin_src').rglob('*.cpp')
    for source_file in sources:
        for plugin_name in plugin_names:
            file_name = source_file.stem
            if plugin_name.lower() in [file_name.lower(), humps.decamelize(file_name), humps.kebabize(file_name)]:
                component.add_source_file(source_file)
                _LOGGER.info(f'adding extra plugin source {source_file}')

    return component


@tbd.build_job_with_priority(tbd.GenerationStages.REFLECTION)
def finalize_plugin_registry_job():
    processor = get_plugin_registry()
    selected_plugins = processor.plugins

    processor.preprocess()
    selected_headers = processor.headers

    # generate plugin factory
    gen_source_path = tbd.get_generated_sources_path()
    gen_include_path = tbd.get_generated_include_path()

    out_file = gen_include_path / 'tbd' / 'sound_registry' / 'sound_processor_factory.hpp'
    write_plugin_factory_header(selected_headers, selected_plugins, out_file)
    write_plugin_reflection_info(processor, gen_source_path)
    write_meta_classes(processor, gen_source_path)

    plugin_names = [plugin.cls_name for plugin in selected_plugins]
    _LOGGER.info('using plugins:')
    for plugin_name in plugin_names:
        _LOGGER.info(f'>>> {plugin_name}')


async def to_code(config):
    component = tbd.new_tbd_component(__file__)

    api_registry = get_api_registry()

    api_registry.add_message_types(component.path / 'src' / 'plugins.proto')
    api_registry.add_source(component.path / 'src' / 'plugins_endpoints.cpp')

    tbd.add_generation_job(finalize_plugin_registry_job)