from dataclasses import dataclass
import re
import logging
from typing import Iterator

import humps

from tbd_core.reflection.reflectables import (
    ParamType,
    ScopePath,
    ALL_PARAM_TYPES, Param,
)
from tbd_core.reflection.db import ClassPtr, PropertyPtr, ReflectableDB, CppTypePtr

from .plugin_entry import PluginEntry, PluginParams, ParamEntry
from .plugins import Plugins, PluginsOptions


_LOGGER = logging.getLogger(__file__)


def is_plugin_class(cls: CppTypePtr) -> bool:
    for base in cls.bases:
        base_name = base.name if isinstance(base, ClassPtr) else base
        if base_name in ['MonoSoundProcessor', 'StereoSoundProcessor']:
            return True
    return False


def is_stereo_plugin(cls: ClassPtr) -> bool:
    for base in cls.bases:
        base_name = base.cls_name
        if base_name == 'MonoSoundProcessor':
            return False
        if base_name == 'StereoSoundProcessor':
            return True

    raise ValueError(f'not a valid plugin type')


def is_in_list(plugin: ClassPtr, plugin_list: set[str]):
    """ Check if plugin is in list of plugins tolerating different word separators.

        Plugin names in `plugin_list` can be class names or friendly names. The comparison
        is case insensitive and also allows class names in the list to occur in

        - all lower case
        - camel case
        - snake case
        - kebap case
    
        Additionally the legacy class name prefix 'ctagSoundProcessor' can be omitted in
        the list.
    """
    if plugin.friendly_name and plugin.friendly_name.lower() in plugin_list:
        return True
    
    def get_name_variants(name: str) -> list[str]:
        return [name.lower(), humps.decamelize(name), humps.kebabize(name).lower()]

    cls_name = plugin.cls_name 
    possible_names = get_name_variants(cls_name)

    # convenience for plugins with obsolete prefix
    # TODO: remove once all plugins with old naming convention have been updated
    PLUGIN_NAME_PREFIX = 'soundprocessor'
    if cls_name.lower().startswith(PLUGIN_NAME_PREFIX):
        possible_names += get_name_variants(plugin.cls_name[len(PLUGIN_NAME_PREFIX):])

    for potential_name in possible_names:
        if potential_name in plugin_list:
            return True
    
    return False


@dataclass
class PluginSet:
    component: str
    whitelist: list[str] | None
    blacklist: list[str] | None


class PluginRegistry:
    def __init__(self):
        self._plugin_sets: list[PluginSet] = []

    def add_plugin_set(self,
        component: str,
        *,
        whitelist: list[str] | None = None,
        blacklist: list[str] | None = None
    ) -> None:
        for plugin_set in self._plugin_sets:
            if plugin_set.component == component:
                raise RuntimeError(f"plugin set for component {component} already exists")
        self._plugin_sets.append(PluginSet(component, whitelist, blacklist))

    def get_plugins(self, reflectables: ReflectableDB) -> Plugins:
        return Plugins(self._get_plugins(reflectables))

    def _get_plugins(self, reflectables: ReflectableDB) -> PluginsOptions:
        plugins = []
        for plugin_set in self._plugin_sets:
            plugins += self._add_filtered_plugins(plugin_set, reflectables.classes())

        acc = PluginsOptions(reflectables=reflectables)

        for plugin_id, plugin in enumerate(plugins):
            field_scope = ScopePath.root()
            unsorted_params = self._flatten_plugin_params(reflectables, acc, plugin_id, plugin, field_scope)
            plugin_params = PluginParams.from_unsorted(unsorted_params)
            is_stereo = is_stereo_plugin(plugin)
            header = next(plugin.files).file

            acc.plugins.append(PluginEntry(
                cls=plugin,
                param_offset=acc.num_params,
                params=plugin_params,
                header=header,
                is_stereo=is_stereo,
            ))

            acc.headers.add(header)
            acc.num_params += plugin_params.num_params

        return acc

    @staticmethod
    def _add_filtered_plugins(plugin_set: PluginSet, classes: Iterator[ClassPtr]) -> list[ClassPtr]:
        """ Find all plugin classes from all known classes with optional whitelisting or blacklisting. """
        whitelist = plugin_set.whitelist
        blacklist = plugin_set.blacklist

        if whitelist and blacklist:
            raise ValueError('provide either blacklist or whitelist for plugin prefiltering')

        if whitelist is not None:
            whitelist = set(plugin_name.lower() for plugin_name in whitelist)
            added_plugins = [cls for cls in classes
                             if cls.component == plugin_set.component
                             and is_plugin_class(cls) and is_in_list(cls, whitelist)]
            return added_plugins

        if blacklist is not None:
            blacklist = set(plugin_name.lower() for plugin_name in blacklist)
            added_plugins = [cls for cls in classes
                             if cls.component == plugin_set.component
                             and is_plugin_class(cls) and not is_in_list(cls, blacklist)]
            return added_plugins

        added_plugins = [cls for cls in classes
                         if cls.component == plugin_set.component
                         and classes if is_plugin_class(cls)]
        return added_plugins

    def _flatten_plugin_params(self,
        reflectables: ReflectableDB,
        acc: PluginsOptions,
        plugin_id: int,
        struct: ClassPtr,
        scope: ScopePath
    ) -> list[ParamEntry]:
        flattened_params = []
        for prop in struct.properties:
            param_scope = scope.add_field(prop.field_name, None)
            field_type = prop.type
            match field_type:
                case ClassPtr():
                    header = next(field_type.files).file
                    acc.headers.add(header)
                    flattened_params += self._flatten_plugin_params(reflectables, acc, plugin_id, field_type, param_scope)
                case Param():
                    flattened_params.append(ParamEntry.new_param_entry(
                        field=prop,
                        path=param_scope,
                        plugin_id=plugin_id,
                        type=prop.type.param_type,
                        attrs=prop.attrs,
                    ))
                case str():
                    pass
                case _:
                    raise ValueError(f'unknown param type {field_type}')
        return flattened_params


__all__ = ['PluginRegistry']
