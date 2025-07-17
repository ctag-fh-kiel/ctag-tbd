from enum import Enum
import json
from pathlib import Path
from typing import Any, Dict, Generator, Tuple

from pydantic import RootModel

from .legacy_json import PluginJson, PluginPatchesJson


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


def parse_plugin_json(json_path: Path) -> PluginJson:
    with open(json_path, 'r') as f:
        json_data = json.load(f)
        return PluginJson(**json_data)

def write_pretty_configs(configs_path: Path, out_path: Path):
    """ convert the config files into multiline more readable format """
    
    out_path.mkdir(parents=True, exist_ok=True)
    for config_file, config in iter_configs(configs_path, ConfigType.config_file):
        config = PluginJson(**config)
        json = RootModel[PluginJson](config).model_dump_json(indent=2, by_alias=True)
        with open(out_path / config_file,'w') as out_file:
            out_file.write(json)

    for preset_file, preset in iter_configs(configs_path, ConfigType.preset_file):
        preset = PluginPatchesJson(**preset)
        json = RootModel[PluginPatchesJson](preset).model_dump_json(indent=2, )
        with open(out_path / preset_file,'w') as out_file:
            out_file.write(json)


def get_plugin_schema():
    """ get the schema of the json representation of plugins """
    
    return json.dumps(RootModel[PluginJson].model_json_schema(), indent=2)

def get_preset_schema():
    """ get the schema of the json representation of presets """

    return json.dumps(RootModel[PluginPatchesJson].model_json_schema(), indent=2)


__all__ = ['parse_plugin_json', 'write_pretty_configs', 'get_plugin_schema', 'get_preset_schema']
