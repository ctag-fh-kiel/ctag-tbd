from enum import Enum
import json
from pathlib import Path
from typing import Any, Dict, Generator, Tuple

from pydantic import RootModel

from .sound_plugin import SoundPlugin, SoundPluginPatches


class ConfigType(Enum):
    config_file = 'mui-*.jsn'
    preset_file = 'mp-*.jsn'
    all = '*.jsn'


def iter_configs(configs_path: Path, config_type: ConfigType) -> Generator[Tuple[str, Dict[str, Any]], None, None]:
    config_files = configs_path.glob(config_type.value)
    for config_path in config_files:
        config_file = config_path.name
        with open(config_path, 'r', encoding='utf-8') as infile:
            yield config_file, json.load(infile)


def write_pretty_configs(configs_path: Path, out_path: Path):
    """ convert the config files into multiline more readable format """
    
    out_path.mkdir(parents=True, exist_ok=True)
    for config_file, config in iter_configs(configs_path, ConfigType.config_file):
        config = SoundPlugin(**config)
        json = RootModel[SoundPlugin](config).model_dump_json(indent=2, by_alias=True)
        with open(out_path / config_file,'w') as out_file:
            out_file.write(json)

    for preset_file, preset in iter_configs(configs_path, ConfigType.preset_file):
        preset = SoundPluginPatches(**preset)
        json = RootModel[SoundPluginPatches](preset).model_dump_json(indent=2, )
        # # json = TypeAdapter[SoundPluginPatches](SoundPluginPatches).dump_json(preset, indent=2, by_alias=True)
        with open(out_path / preset_file,'w') as out_file:
            out_file.write(json)


def get_plugin_schema():
    """ get the schema of the json representation of plugins """
    
    return json.dumps(RootModel[SoundPlugin].model_json_schema(), indent=2)

def get_preset_schema():
    """ get the schema of the json representation of presets """

    return json.dumps(RootModel[SoundPluginPatches].model_json_schema(), indent=2)


__all__ = ['write_pretty_configs', 'get_plugin_schema', 'get_preset_schema']
