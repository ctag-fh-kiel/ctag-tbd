from esphome.components.tbd_sound_registry import add_tbd_sounds, SOUND_COLLECTION_SCHEMA

AUTO_LOAD = ['tbd_sound_registry']

CONFIG_SCHEMA = SOUND_COLLECTION_SCHEMA

async def to_code(config):
    add_tbd_sounds(__file__, config)

