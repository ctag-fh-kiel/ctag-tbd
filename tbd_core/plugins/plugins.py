import logging
from dataclasses import dataclass, field
from pathlib import Path
from typing import Final, OrderedDict

import humps

from tbd_core.reflection.db import ReflectableDB

from .plugin_entry import PluginEntry, ParamEntry


def find_plugin_source_file(source_dir: Path, plugin: PluginEntry) -> Path:
    plugin_sources = (source_dir).rglob('*.cpp')
    plugin_name = plugin.cls.cls_name
    for source_file in plugin_sources:
        file_name = source_file.stem.lower()
        possible_cpp_file_names = [
            plugin.header.stem.lower(),
            plugin_name.lower(),
            humps.decamelize(plugin_name),
            humps.kebabize(plugin_name),
        ]
        if file_name in possible_cpp_file_names:
            return source_file

    raise RuntimeError(f'no source file found for plugin {plugin_name}')

@dataclass
class PluginsOptions:
    reflectables: ReflectableDB = field(default_factory=OrderedDict)
    headers: set[Path] = field(default_factory=set)
    plugins: list[PluginEntry] = field(default_factory=list)
    num_params: int = 0

class Plugins:
    def __init__(self, options: PluginsOptions):
        self._reflectables: Final = options.reflectables
        self._headers: Final = options.headers
        self._plugin_entries: Final = options.plugins
        self._num_params: Final = options.num_params

    @property
    def headers(self) -> set[Path]:
        return set(plugin.header for plugin in self._plugin_entries)

    @property
    def plugin_list(self) -> list[PluginEntry]:
        return self._plugin_entries

    @property
    def param_list(self) -> list[ParamEntry]:
        return [param for plugin in self._plugin_entries for param in plugin.param_list()]

    @property
    def reflectables(self) -> ReflectableDB:
        return self._reflectables

__all__ = ['find_plugin_source_file', 'Plugins', 'PluginsOptions']