from esphome.components.tbd_sound_registry import new_plugin_registry, SOUND_COLLECTION_SCHEMA

AUTO_LOAD = ['tbd_sound_registry']

CONFIG_SCHEMA = SOUND_COLLECTION_SCHEMA

async def to_code(config):
    new_plugin_registry(__file__, config)

