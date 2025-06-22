from dataclasses import dataclass, field
from pathlib import Path
from typing import Final

from tbd_core.reflection import ReflectableDescription

from .plugin_entry import PluginEntry, ParamEntry


@dataclass
class PluginsOptions:
    reflectables: list[ReflectableDescription] = field(default_factory=list)
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


__all__ = ['Plugins', 'PluginsOptions']