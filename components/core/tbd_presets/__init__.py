import tbd_core.buildgen as tbd
from esphome.components.tbd_serialization import get_dto_registry
from tbd_core.buildgen import AutoReflection

from esphome.components.tbd_sound_registry import get_plugins
from esphome.components.tbd_api import get_api_registry


AUTO_LOAD = ['tbd_module', 'tbd_serialization']

CONFIG_SCHEMA = {}


async def to_code(config):
    component = tbd.new_tbd_component(__file__, auto_reflect=AutoReflection.ALL)
    tbd.add_generation_job(generate_presets_meta)

    # api_registry = get_api_registry()
    # api_registry.add_source(component.include_dir / 'tbd' / 'presets' / 'presets.hpp')
    # api_registry.add_source(component.source_dir / 'presets.cpp')


@tbd.build_job_with_priority(tbd.GenerationStages.SERIALIZATION)
def generate_presets_meta():
    plugins = get_plugins()

    dto_registry = get_dto_registry()
    for plugin in plugins.plugin_list:
        dto_registry.create_dto_for_class('presets', plugin.name + 'Dto', plugin.cls)

    # message_dir = tbd.get_build_path() / tbd.get_messages_path() / 'presets'
    # message_dir.mkdir(parents=True, exist_ok=True)
    #
    # message_proto = message_dir / 'presets.proto'
    # with open(message_proto, 'w') as f:
    #     gen.write_proto(f)
    # gen.write_cpp_dtos(message_dir, message_proto)
    # gen.write_cpp_code(message_dir, [plugin.ref() for plugin in plugins.plugin_list])

