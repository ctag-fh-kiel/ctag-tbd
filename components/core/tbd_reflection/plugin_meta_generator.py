from dataclasses import dataclass
from pathlib import Path
import re

import humps
from .reflectables import ScopeType, PropertyDescription, ReflectableDescription, Reflectables, Attributes
from .parameters import ParamType, PARAM_TYPES
import cxxheaderparser.simple as cpplib
import logging
import voluptuous as vol
from enum import Enum, unique


_LOGGER = logging.getLogger(__file__)


@dataclass(frozen=True)
class ScopePathSegment:
    name: str
    type: ScopeType

    def namespace(self) -> str:
        if self.type != ScopeType.NAMESPACE:
            raise ValueError(f'scope segment is not a namespace: {self.type}')
        return self.name
    
    def cls(self) -> str:
        if self.type != ScopeType.CLASS:
            raise ValueError(f'scope segment is not a namespace: {self.type}')
        return self.name


@dataclass(frozen=True)
class ScopePath:
    segments: list[ScopePathSegment]

    @staticmethod
    def root() -> 'ScopePath':
        return ScopePath([])

    def add_namespace(self, namespace: str) -> 'ScopePath':
        if self.segments and self.segments[-1].type != ScopeType.NAMESPACE:
            raise ValueError('namespaces can only be declared in namespaces')
        return ScopePath([*self.segments, ScopePathSegment(namespace, ScopeType.NAMESPACE)])
    
    def add_class(self, cls: str) -> 'ScopePath':
        if self.segments and self.segments[-1].type not in [ScopeType.NAMESPACE, ScopeType.CLASS]:
            raise ValueError('classes can only be declared in namespaces or other classes')
        return ScopePath([*self.segments, ScopePathSegment(cls, ScopeType.CLASS)])
    
    def add_field(self, field: str) -> 'ScopePath':
        if self.segments and self.segments[-1].type not in [ScopeType.CLASS, ScopeType.FIELD]:
            raise ValueError('fields can only be declared in classes and fields')

        return ScopePath([*self.segments, ScopePathSegment(field, ScopeType.FIELD)])
    
    def add_static_field(self, field: str) -> 'ScopePath':
        if self.segments and self.segments[-1].type != ScopeType.CLASS:
            raise ValueError('static fields can only be declared in classes')

        return ScopePath([*self.segments, ScopePathSegment(field, ScopeType.STATIC_FIELD)])

    def namespace(self) -> str:
        return self.segments[-1].namespace()

    def cls(self) -> str:
        return self.segments[-1].cls()

    @property
    def namespaces(self):
        retval, _, _ = self.split()
        return retval
    
    @property
    def classes(self):
        _, retval, _ = self.split()
        return retval
    
    @property
    def fields(self):
        _, _, retval = self.split()
        return retval

    @property
    def path(self) -> str:
        if not self.segments:
            return ''
        
        fst, *tail = self.segments
        retval = fst.name
        for elem in tail:
            if elem.type == ScopeType.FIELD:
                retval += f'.{elem.name}'
            else:
                 retval += f'::{elem.name}'
        return retval
    
    def __str__(self) -> str:
        return self.path
    
    def __repr__(self) -> str:
        return f'ScopePath({self.path})'

    def split(self) -> tuple['ScopePath', 'ScopePath', 'ScopePath']:
        class SplitDone(BaseException):
            pass

        namespaces = []
        classes = []
        fields = []

        try:
            pos, *rest = self.segments
            while pos.type == ScopeType.NAMESPACE:
                namespaces.append(pos)
                if not rest:
                    raise SplitDone
                pos, *rest = rest

            while pos.type == ScopeType.CLASS:
                classes.append(pos)
                if not rest:
                    raise SplitDone
                pos, *rest = rest

            while pos.type == ScopeType.FIELD:
                fields.append(pos)
                if not rest:
                    raise SplitDone
                pos, *rest = rest
        except SplitDone:
            return ScopePath(namespaces), ScopePath(classes), ScopePath(fields)

        raise ValueError('bad segment order in scope')


@unique
class ParamOperations(Enum):
    NO_OP  = 'NO_OP'
    ABS_OP = 'ABS_OP'
    ADD_OP = 'ADD_OP'
    SFT_OP = 'SFT_OP'
    PAN_OP = 'PAN_OP'


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

@dataclass(frozen=True)
class ParamEntry:
    name: str
    plugin_id: int
    type: ParamType
    norm: float | None = None
    scale: float | None = None
    min: float | None = None
    max: float | None = None
    cut_negatives: bool = False
    operation: ParamOperations = ParamOperations.NO_OP

    @property
    def snake_name(self):
        return self.name.replace('.', '__')

    @property
    def is_int(self):
        return self.type == ParamType.INT_PARAM
    
    @property 
    def is_uint(self):
        return self.type == ParamType.UINT_PARAM
    
    @property 
    def is_trigger(self):
        return self.type == ParamType.TRIGGER_PARAM

    @property 
    def is_float(self):
        return self.type == ParamType.FLOAT_PARAM
    
    @property 
    def is_ufloat(self):
        return self.type == ParamType.UFLOAT_PARAM    
    
    @staticmethod
    def new_param_entry(name: str, plugin_id: int, type: ParamType, attrs: Attributes | None):
        attrs = {name: value for attr in attrs for name, value in attr.params.items() if attr.name[0] == 'tbd'}

        if not attrs:
            return ParamEntry(name=name, plugin_id=plugin_id, type=type)

        # validate attributes
        if type == ParamType.TRIGGER_PARAM:
            schema = ATTR_BASE_SCHEMA
        elif type in [ParamType.INT_PARAM, ParamType.FLOAT_PARAM]:
            schema = ATTR_BASE_SCHEMA | ATTR_NUMBER_SCHEMA | ATTR_OPERATIONS_SCHEMA
        elif type in [ParamType.UINT_PARAM, ParamType.UFLOAT_PARAM]:
            schema = ATTR_BASE_SCHEMA | ATTR_NUMBER_SCHEMA | ATTR_OPERATIONS_SCHEMA | ATTR_NEGATIVES_SCHEMA
        else:
            raise ValueError(f"unsupported parameter type: {type.name}")

        vol.Schema(schema)(attrs)

        if type == ParamType.TRIGGER_PARAM:
            return ParamEntry(name=name, plugin_id=plugin_id, type=type)

        filtered_attrs = {key: float(value) for key, value in attrs.items() if key in ATTR_NUMBER_SCHEMA.keys()}

        if attrs.get(ATTR_SFT):
            operation = ParamOperations.SFT_OP
        elif attrs.get(ATTR_PAN):
            operation = ParamOperations.PAN_OP
        elif attrs.get(ATTR_ADD):
            operation = ParamOperations.ADD_OP
        else:
            operation = ParamOperations.NO_OP

        if type in [ParamType.INT_PARAM, ParamType.FLOAT_PARAM]:
            return ParamEntry(name=name, plugin_id=plugin_id, type=type, operation=operation, **filtered_attrs)

        if attrs.get(ATTR_ABS):
            cut_negatives = False
        elif attrs.get(ATTR_CUT):
            cut_negatives = True
        else:
            cut_negatives = False
        
        # [ParamType.UINT_PARAM, ParamType.UFLOAT_PARAM]:             
        return ParamEntry(name=name, plugin_id=plugin_id, type=type, operation=operation, cut_negatives=cut_negatives, **filtered_attrs)


@dataclass(frozen=True)
class PluginParams:
    int_params: list[ParamEntry]
    uint_params: list[ParamEntry]
    trigger_params: list[ParamEntry]
    float_params: list[ParamEntry]
    ufloat_params: list[ParamEntry]

    @property
    def param_sets(self) -> list[list[ParamEntry]]:
        return [self.int_params, self.uint_params, self.trigger_params, self.float_params, self.ufloat_params]

    @property
    def num_ints(self) -> int:  
        return len(self.int_params)
    
    @property
    def num_uints(self) -> int:
        return len(self.uint_params)
    
    @property
    def num_triggers(self) -> int:
        return len(self.trigger_params)
    
    @property
    def num_floats(self) -> int:
        return len(self.float_params)
    
    @property
    def num_ufloats(self) -> int:
        return len(self.ufloat_params)

    @property
    def num_params(self) -> int:
        return len(self.params)

    @property
    def params(self) -> list[ParamEntry]:
        return [param for param_set in self.param_sets for param in param_set]

    @staticmethod
    def from_unsorted(param_list: list[ParamEntry]) -> 'PluginParams':
        params = {
            ParamType.INT_PARAM: [],
            ParamType.UINT_PARAM: [],
            ParamType.TRIGGER_PARAM: [],
            ParamType.FLOAT_PARAM: [],
            ParamType.UFLOAT_PARAM: [],
        }

        for param in param_list:
            params[param.type].append(param)

        return PluginParams(
            int_params=params[ParamType.INT_PARAM],
            uint_params=params[ParamType.UINT_PARAM],
            trigger_params=params[ParamType.TRIGGER_PARAM],
            float_params=params[ParamType.FLOAT_PARAM],
            ufloat_params=params[ParamType.UFLOAT_PARAM],
        )

@dataclass(frozen=True)
class PluginEntry:
    name: str
    full_name: str
    param_offset: int
    params: PluginParams
    header: Path

    @property
    def num_ints(self) -> int:
        return self.params.num_ints

    @property
    def int_params(self) -> list[ParamEntry]:
        return self.params.int_params

    @property
    def num_uints(self) -> int:
        return self.params.num_uints

    @property
    def uint_params(self) -> list[ParamEntry]:
        return self.params.uint_params

    @property
    def num_triggers(self) -> int:
        return self.params.num_triggers

    @property
    def trigger_params(self) -> list[ParamEntry]:
        return self.params.trigger_params

    @property
    def num_floats(self) -> int:
        return self.params.num_floats
    
    @property
    def float_params(self) -> list[ParamEntry]:
        return self.params.float_params

    @property
    def num_ufloats(self) -> int:
        return self.params.num_ufloats
    
    @property
    def ufloat_params(self) -> list[ParamEntry]:
        return self.params.ufloat_params

    @property
    def num_params(self) -> int:
        return self.params.num_params

    def param_list(self) -> list[ParamEntry]:
        return self.params.params


def is_plugin_class(cls: cpplib.ClassScope) -> bool:
    """ All plugins must be direct decendatns of `audio::SoundProcessor`. """

    for base in cls.class_decl.bases:
        parent_name = base.typename.segments[-1].name
        if parent_name == 'SoundProcessor':
            return True
    return False


def is_in_list(plugin: ReflectableDescription, plugin_list: list[str]):
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

class PluginReflectionGenerator:
    def __init__(self):
        self._reflectables: list[ReflectableDescription] = []
        self._plugins: list[ReflectableDescription] = []
        self._headers: set[Path] | None = None
        self._plugin_entries: list[PluginEntry] | None = None
        self._num_params: int = -1

    def add(self, reflectables: Reflectables, *, whitelist: list[str] | None = None, blacklist: list[str] | None = None) -> Reflectables:
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


    def preprocess(self):
         self._flatten_params()
         return self._plugins

    @property
    def plugins(self) -> Reflectables:
        if not self._plugins:
            raise ValueError('no plugin list present, need to run prefilter')
        return self._plugins
    
    @property
    def headers(self) -> set[Path]:
        if not self._plugins:
            raise ValueError('header list present, need to run prcess')
        return set(plugin.header for plugin in self._plugins)

    @property
    def plugin_list(self) -> list[PluginEntry]:
        if not self._plugin_entries:
            raise ValueError('no plugin list present, need to run process')
        return self._plugin_entries
    
    @property
    def param_list(self) -> list[ParamEntry]:
        if self._num_params < 0:
            raise ValueError('parameter list present, need to run process')
        return [param for plugin in self._plugin_entries for param in plugin.param_list()]

    def _flatten_params(self):
        self._plugin_entries = []
        self._num_params = 0
        self._headers = set()
        for plugin_id, plugin in enumerate(self._plugins):
            field_scope = ScopePath.root()
            unsorted_params = self._flatten_plugin_params(plugin_id, plugin, field_scope)
            plugin_params = PluginParams.from_unsorted(unsorted_params)

            self._plugin_entries.append(PluginEntry(
                name=plugin.cls_name, 
                full_name=plugin.full_name.path,
                param_offset=self._num_params, 
                params=plugin_params,
                header=plugin.header,
            ))

            self._headers.add(plugin.header)
            self._num_params += plugin_params.num_params

    def _flatten_plugin_params(self, plugin_id: int, plugin: ReflectableDescription, scope: ScopePath) -> list[ParamEntry]:
        flattened_params = []
        for field in plugin.properties:
            param_scope = scope.add_field(field.field_name)
            found = self._find_field_type(field.type)
            if found:
                if len(found) > 1:
                    _LOGGER.warning(f'ambigous field type for field {field.full_name}')
                    continue
                field_cls = found[0]
                self._headers.add(field_cls.header)
                flattened_params += self._flatten_plugin_params(plugin_id, field_cls, param_scope)
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
              


__all__ = [
    'ParamEntry',
    'PluginParams',
    'PluginEntry',
    'PluginReflectionGenerator',
]