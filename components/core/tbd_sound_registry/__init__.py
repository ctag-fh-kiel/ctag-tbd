import logging

from esphome.components.tbd_api import get_api_registry

from esphome.const import CONF_ID
import esphome.config_validation as cv
import esphome.codegen as cg

import tbd_core.buildgen as tbd
from tbd_core.plugins import (
    PluginRegistry,
    PluginGenerator,
    Plugins, find_plugin_source_file,
)


_LOGGER = logging.getLogger(__name__)

AUTO_LOAD = ['tbd_serialization']
REQUIRES = ['tbd_module']

tbd_sound_registry_ns = cg.esphome_ns.namespace("sound_registry")
SoundRegistry = tbd_sound_registry_ns.class_("SoundRegistryProxy", cg.Component)

CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(): cv.declare_id(SoundRegistry),
    })
    .extend(cv.COMPONENT_SCHEMA)
)

SOUND_REGISTRY_GLOBAL = 'sound_registry'
SOUND_PLUGINS_GLOBAL = 'plugins'

CONF_WHITELIST = 'whitelist'
CONF_BLACKLIST = 'blacklist'


SOUND_COLLECTION_SCHEMA = cv.Schema({
    cv.Optional(cv.Exclusive('whitelist', 'blacklist')): cv.ensure_list(cv.string)
})


@tbd.generated_tbd_global(SOUND_REGISTRY_GLOBAL)
def get_plugin_registry() -> PluginRegistry:
    return PluginRegistry()


@tbd.generated_tbd_global(SOUND_PLUGINS_GLOBAL, after_stage=tbd.GenerationStages.PLUGINS)
def get_plugins() -> Plugins:
    return get_plugin_registry().get_plugins(tbd.get_reflectables())


def new_plugin_registry(init_file: str, config) -> tbd.ComponentInfo:
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

    component = tbd.new_tbd_component(init_file, auto_reflect=tbd.AutoReflection.HEADERS)

    whitelist = config.get(CONF_WHITELIST)
    blacklist = config.get(CONF_BLACKLIST)
    get_plugin_registry().add_plugin_set(component.name, whitelist=whitelist, blacklist=blacklist)
    return component


async def to_code(config):
    component = tbd.new_tbd_component(__file__, auto_reflect=tbd.AutoReflection.ALL)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    tbd.add_generation_job(finalize_plugin_registry_job)


@tbd.build_job_with_priority(tbd.GenerationStages.PLUGINS)
def finalize_plugin_registry_job():
    plugins = get_plugins()

    gen_source_path = tbd.get_build_path() / tbd.get_generated_sources_path()

    gen = PluginGenerator(plugins)
    gen.write_plugin_factory_header(gen_source_path)
    gen.write_plugin_reflection_info(gen_source_path)
    gen.write_meta_classes(gen_source_path)

    plugin_list = plugins.plugin_list

    # plugin add source files to build
    _LOGGER.info('[[ adding plugins ]]')
    for component in tbd.get_tbd_components().values():
        component_name = component.name

        selected_module_plugins = []
        for plugin in plugin_list:
            if plugin.cls.component == component_name:
                selected_module_plugins.append(plugin)
        if selected_module_plugins:
            _LOGGER.info(f'[ adding plugins from module {component_name} ]')
            for plugin in selected_module_plugins:
                source_file = find_plugin_source_file(component.path / 'plugin_src', plugin)
                component.add_source_file(source_file)
                source_file = source_file.relative_to(component.path)
                _LOGGER.info(f'{plugin.name} ({plugin.header}, {source_file})')

