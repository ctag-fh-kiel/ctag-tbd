from pathlib import Path
import re
import logging
from typing import Final, OrderedDict

import humps
import cxxheaderparser.simple as cpplib

from tbd_core.reflection import (
    ParamType,
    ClassDescription,
    PropertyDescription,
    ScopePath,
    Headers,
    ReflectableFinder,
    ClassDescriptions,
    Reflectables,
)
from .plugin_entry import PluginEntry, PluginParams, ParamEntry

from .plugins import Plugins, PluginsOptions
from ..reflection.parameters import ALL_PARAM_TYPES

_LOGGER = logging.getLogger(__file__)


def is_plugin_class(cls: cpplib.ClassScope) -> bool:
    for base in cls.class_decl.bases:
        parent_name = base.typename.segments[-1].name
        if parent_name in ['MonoSoundProcessor', 'StereoSoundProcessor']:
            return True
    return False


def is_stereo_plugin(cls: cpplib.ClassScope) -> bool:
    for base in cls.class_decl.bases:
        parent_name = base.typename.segments[-1].name
        if parent_name == 'MonoSoundProcessor':
            return False
        if parent_name == 'StereoSoundProcessor':
            return True

    raise ValueError(f'not a valid plugin type')


def is_in_list(plugin: ClassDescription, plugin_list: set[str]):
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

    return ALL_PARAM_TYPES.get(param_type)


class PluginRegistry:
    def __init__(self):
        self._collector: Final = ReflectableFinder()
        self._plugins: list[int] = []

    def search_for_plugins(self,
        headers: Headers, strict: bool, *,
        include_base: Path | None = None,
        whitelist: list[str] | None = None,
        blacklist: list[str] | None = None
    ) -> list[ClassDescription]:
        new_classes = {}

        for header in headers:
            if strict:
                added = self._collector.add_from_file(header, include_base=include_base)
                new_classes |= added.classes
            else:
                try:
                    added = self._collector.add_from_file(header, include_base=include_base)
                    new_classes |= added.classes
                except Exception as e:
                    _LOGGER.error(f'error parsing {header}: {e}')
        new_plugins = PluginRegistry._add_filtered_plugins(new_classes, whitelist=whitelist, blacklist=blacklist)
        self._plugins += list(new_plugins.keys())
        return list(new_plugins.values())

    def get_plugins(self) -> Plugins:
        return Plugins(self._get_plugins())

    def _get_plugins(self) -> PluginsOptions:
        reflectables = self._collector.get_reflectables()
        acc = PluginsOptions(reflectables=reflectables)


        for plugin_id, plugin_ref in enumerate(self._plugins):
            plugin = reflectables.classes[plugin_ref]

            field_scope = ScopePath.root()
            unsorted_params = self._flatten_plugin_params(reflectables, acc, plugin_id, plugin, field_scope)
            plugin_params = PluginParams.from_unsorted(unsorted_params)
            is_stereo = is_stereo_plugin(plugin.raw)

            acc.plugins.append(PluginEntry(
                cls=plugin,
                param_offset=acc.num_params,
                params=plugin_params,
                header=plugin.header,
                is_stereo=is_stereo,
            ))

            acc.headers.add(plugin.header)
            acc.num_params += plugin_params.num_params

        return acc

    @staticmethod
    def _add_filtered_plugins(classes: ClassDescriptions, *,
        whitelist: list[str] | None = None,
        blacklist: list[str] | None = None,
    ) -> ClassDescriptions:
        """ Find all plugin classes from all known classes with optional whitelisting or blacklisting. """
        if whitelist and blacklist:
            raise ValueError('provide either blacklist or whitelist for plugin prefiltering')

        if whitelist is not None:
            whitelist = set(plugin_name.lower() for plugin_name in whitelist)
            added_plugins = OrderedDict((ref, cls) for ref, cls in classes.items()
                                        if is_plugin_class(cls.raw) and is_in_list(cls, whitelist))
            return added_plugins

        if blacklist is not None:
            blacklist = set(plugin_name.lower() for plugin_name in blacklist)
            added_plugins = OrderedDict((ref, cls) for ref, cls in classes.items()
                                        if is_plugin_class(cls.raw) and not is_in_list(cls, blacklist))
            return added_plugins

        added_plugins = OrderedDict((ref, cls) for ref, cls in classes.items()
                                    if is_plugin_class(cls.raw))
        return added_plugins

    def _flatten_plugin_params(self,
        reflectables: Reflectables,
        acc: PluginsOptions,
        plugin_id: int,
        struct: ClassDescription,
        scope: ScopePath
    ) -> list[ParamEntry]:
        flattened_params = []
        for field_ref in struct.properties:
            field = reflectables.properties[field_ref]
            param_scope = scope.add_field(field.field_name, None)
            field_cls = self._find_field_type(field.type, reflectables)
            if field_cls:
                # if len(found) > 1:
                #     _LOGGER.warning(f'ambigous field type for field {field.full_name}')
                #     continue
                acc.headers.add(field_cls.header)
                flattened_params += self._flatten_plugin_params(reflectables, acc, plugin_id, field_cls, param_scope)
            else:
                field_type = get_underlying_param_type(field)
                if (field_type := get_param_type(field_type)) is None:
                    _LOGGER.warning(f'field {field.full_name} type {field.type} not known')
                    continue

                flattened_params.append(ParamEntry.new_param_entry(
                    field=field,
                    path=param_scope,
                    plugin_id=plugin_id,
                    type=field_type,
                    attrs=field.attrs,
                ))
        return flattened_params

    def _find_field_type(self, field_type: str | int, reflectables: Reflectables) -> ClassDescription | None:
        found_types = []
        if isinstance(field_type, int) and field_type in reflectables.classes:
            return reflectables.classes[field_type]
        return None
        # for cls in reflectables.class_list:
        #     if field_type
        #
        #     if field_type == cls.full_name:
        #         found_types.append(cls)
        # return found_types

    # @property
    # def _reflectables(self) -> ClassDescriptions:
    #     return self._collector.reflectables

__all__ = ['PluginRegistry']
