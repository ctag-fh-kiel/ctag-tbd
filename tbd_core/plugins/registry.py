from dataclasses import dataclass
from pathlib import Path
import re
import logging

import humps
import cxxheaderparser.simple as cpplib
import voluptuous as vol

from tbd_core.reflection import (
    ParamType,
    ReflectableDescription,
    PropertyDescription,
    PARAM_TYPES,
    ScopePath, Reflectables
)
from .plugin_entry import PluginEntry, PluginParams, ParamEntry

from .plugins import Plugins, PluginsOptions

_LOGGER = logging.getLogger(__file__)





ATTR_NAME = 'name'
ATTR_DESCRIPTION = 'description'
ATTR_NORM = 'norm'
ATTR_SCALE = 'scale'
ATTR_MIN = 'min'
ATTR_MAX = 'max'
ATTR_SFT = 'sft'
ATTR_PAN = 'pan'
ATTR_ADD = 'add'
ATTR_ABS = 'abs'
ATTR_CUT = 'cut'

ATTR_ALT_NEGATIVES = 'negatives'
ATTR_ALT_OPS = 'oeprations'

ATTR_BASE_SCHEMA = {
    vol.Optional(ATTR_NAME): str,
    vol.Optional(ATTR_DESCRIPTION): str
}

ATTR_NUMBER_SCHEMA = {
    vol.Optional(ATTR_NORM): vol.Or(int, float),
    vol.Optional(ATTR_SCALE) :vol.Or(int, float),
    vol.Optional(ATTR_MIN): vol.Or(int, float),
    vol.Optional(ATTR_MAX): vol.Or(int, float),
}

ATTR_OPERATIONS_SCHEMA = {
    vol.Exclusive(ATTR_SFT, ATTR_ALT_OPS): True,
    vol.Exclusive(ATTR_PAN, ATTR_ALT_OPS): True,
    vol.Exclusive(ATTR_ADD, ATTR_ALT_OPS): True,
}

ATTR_NEGATIVES_SCHEMA = {
    vol.Exclusive(ATTR_ABS, ATTR_ALT_NEGATIVES): True, 
    vol.Exclusive(ATTR_CUT, ATTR_ALT_NEGATIVES): True,
}


VALID_PARAM_ATTRS = set(['name', 'description', 'norm', 'scale', 'min', 'max', 'cut_negative', 'sft', 'pan', 'add'])




def is_plugin_class(cls: cpplib.ClassScope) -> bool:
    """ All plugins must be direct decendatns of `audio::SoundProcessor`. """

    for base in cls.class_decl.bases:
        parent_name = base.typename.segments[-1].name
        if parent_name in ['MonoSoundProcessor', 'StereoSoundProcessor']:
            return True
    return False


def is_in_list(plugin: ReflectableDescription, plugin_list: set[str]):
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


UNPACK_ATOMIC_EXPR = re.compile(r'(std::)?atomic<\s*(?P<WrappedType>.+)\s*>')

def get_underlying_param_type(field: PropertyDescription) -> str:
    """ Plugin parameter types can be wrapped in `std::atomic` for legacy purposes. """

    field_type = field.type
    if (atomic_match := UNPACK_ATOMIC_EXPR.match(field_type)):
        field_type = atomic_match.group('WrappedType')
    return field_type


def get_param_type(param_type: str) -> ParamType | None:
    """ Get parameter_type ID for typename string. """

    return PARAM_TYPES.get(param_type)


class PluginRegistry:
    def __init__(self):
        self._reflectables: list[ReflectableDescription] = []
        self._plugins: list[ReflectableDescription] = []
        self._headers: set[Path] | None = None
        self._plugin_entries: list[PluginEntry] | None = None
        self._num_params: int = -1

    def add_plugins(self, reflectables: Reflectables, *,
            whitelist: list[str] | None = None,
            blacklist: list[str] | None = None
        ) -> Reflectables:
        """ Find all plugin classes from all known classes with optional whitelisting or blacklisting. """

        self._reflectables += reflectables

        if whitelist and blacklist:
            raise ValueError('provide either blacklist or whitelist for plugin prefiltering')
        
        if whitelist is not None:
            whitelist = set(plugin_name.lower() for plugin_name in whitelist)
            added_plugins = [cls for cls in reflectables if is_plugin_class(cls.raw) and is_in_list(cls, whitelist)]
            self._plugins += added_plugins
            return added_plugins

        if blacklist is not None:
            blacklist = set(plugin_name.lower() for plugin_name in blacklist)
            added_plugins= [cls for cls in reflectables if is_plugin_class(cls.raw) and not is_in_list(cls, blacklist)]
            self._plugins += added_plugins
            return added_plugins

        added_plugins = [cls for cls in reflectables if is_plugin_class(cls.raw)]
        self._plugins += added_plugins
        return added_plugins

    def get_plugins(self) -> Plugins:
        return Plugins(self._get_plugins())

    def _get_plugins(self) -> PluginsOptions:
        acc = PluginsOptions()

        for plugin_id, plugin in enumerate(self._plugins):
            field_scope = ScopePath.root()
            unsorted_params = self._flatten_plugin_params(acc, plugin_id, plugin, field_scope)
            plugin_params = PluginParams.from_unsorted(unsorted_params)

            acc.plugin_entries.append(PluginEntry(
                name=plugin.cls_name, 
                full_name=plugin.full_name.path,
                param_offset=acc.num_params,
                params=plugin_params,
                header=plugin.header,
            ))

            acc.headers.add(plugin.header)
            acc.num_params += plugin_params.num_params

        return acc

    def _flatten_plugin_params(self,
        acc: PluginsOptions,
        plugin_id: int,
        plugin: ReflectableDescription,
        scope: ScopePath
    ) -> list[ParamEntry]:
        flattened_params = []
        for field in plugin.properties:
            param_scope = scope.add_field(field.field_name)
            found = self._find_field_type(field.type)
            if found:
                if len(found) > 1:
                    _LOGGER.warning(f'ambigous field type for field {field.full_name}')
                    continue
                field_cls = found[0]
                acc.headers.add(field_cls.header)
                flattened_params += self._flatten_plugin_params(acc, plugin_id, field_cls, param_scope)
            else:
                field_type = get_underlying_param_type(field)
                if (field_type := get_param_type(field_type)) is None:
                    _LOGGER.warning(f'field {field.full_name} type {field.type} not known')
                    continue
                # new_field = dataclasses.replace(field, full_name=field_scope)
                flattened_params.append(ParamEntry.new_param_entry(
                    name=param_scope.path, 
                    plugin_id=plugin_id, 
                    type=field_type,
                    attrs=field.attrs,
                ))
        return flattened_params

    def _find_field_type(self, field_type: str) -> list[ReflectableDescription]:
        found_types = []
        for cls in self._reflectables:
            if field_type == cls.full_name.path:
                found_types.append(cls)
        return found_types


__all__ = ['PluginRegistry']
