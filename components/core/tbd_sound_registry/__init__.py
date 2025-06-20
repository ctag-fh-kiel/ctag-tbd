from esphome.components.tbd_module.python_dependencies import python_dependencies
from esphome.const import CONF_ID
import esphome.config_validation as cv
import esphome.codegen as cg

python_dependencies(('humps', 'pyhumps'))

import humps
import logging

import tbd_core.buildgen as tbd

from esphome.components.tbd_api import get_api_registry
from tbd_core.plugins import (
    search_for_plugins, 
    write_plugin_factory_header, 
    write_plugin_reflection_info,
    write_meta_classes,
    PluginRegistry,
)


_LOGGER = logging.getLogger(__name__)

tbd_sound_registry_ns = cg.esphome_ns.namespace("sound_registry")
SoundRegistry = tbd_sound_registry_ns.class_("SoundRegistryProxy", cg.Component)

CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(): cv.declare_id(SoundRegistry),
    })
    .extend(cv.COMPONENT_SCHEMA)
)

SOUND_REGISTRY_GLOBAL = 'sound_registry'

CONF_WHITELIST = 'whitelist'
CONF_BLACKLIST = 'blacklist'


SOUND_COLLECTION_SCHEMA = cv.Schema({
    cv.Optional(cv.Exclusive('whitelist', 'blacklist')): cv.ensure_list(cv.string)
})


@tbd.generated_tbd_global(SOUND_REGISTRY_GLOBAL)
def get_plugin_registry() -> PluginRegistry:
    return PluginRegistry()


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

    reflectables = []
    for include_dir in include_dirs:
        headers = set(file for file in include_dir.rglob('*.hpp'))
        reflectables += search_for_plugins(headers, True, include_base=include_dir)

    registry = get_plugin_registry()

    # filter plugins if inclusion rules present
    whitelist = config.get(CONF_WHITELIST)
    blacklist = config.get(CONF_BLACKLIST)
    selected_module_plugins = registry.add_plugins(reflectables, whitelist=whitelist, blacklist=blacklist)

    # add sources for selected plugins
    sources = (component.path / 'plugin_src').rglob('*.cpp')
    for source_file in sources:
        file_name = source_file.stem.lower()
        for plugin in selected_module_plugins:
            plugin_name = plugin.cls_name
            possible_cpp_file_names = [
                plugin.header.stem.lower(),
                plugin_name.lower(),
                humps.decamelize(plugin_name),
                humps.kebabize(plugin_name),
            ]
            if file_name in possible_cpp_file_names:
                component.add_source_file(source_file)
                _LOGGER.info(f'adding extra plugin source {source_file}')

    return component


@tbd.build_job_with_priority(tbd.GenerationStages.REFLECTION)
def finalize_plugin_registry_job():
    plugins = get_plugin_registry().get_plugins()
    selected_plugins = plugins.plugins
    selected_headers = plugins.headers

    # generate plugin factory
    gen_source_path = tbd.get_generated_sources_path()
    gen_include_path = tbd.get_generated_include_path()

    write_plugin_factory_header(selected_headers, plugins, gen_source_path)
    write_plugin_reflection_info(plugins, gen_source_path)
    write_meta_classes(plugins, gen_source_path)

    plugin_names = [plugin.cls_name for plugin in selected_plugins]
    _LOGGER.info('using plugins:')
    for plugin_name in plugin_names:
        _LOGGER.info(f'>>> {plugin_name}')


async def to_code(config):
    component = tbd.new_tbd_component(__file__)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    api_registry = get_api_registry()
    api_registry.add_message_types(component.path / 'src' / 'sound_registry.proto')
    api_registry.add_source(component.path / 'src' / 'plugins_endpoints.cpp')

    tbd.add_generation_job(finalize_plugin_registry_job)
