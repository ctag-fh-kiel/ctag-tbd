from dataclasses import dataclass
from enum import unique, StrEnum
from pathlib import Path
from zlib import crc32

from tbd_core.reflection import (
    ParamType,
    Attributes,
    ScopeDescription,
    ReflectableDescription,
    PropertyDescription,
    ScopePath,
)
import voluptuous as vol


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

@unique
class ParamOperations(StrEnum):
    NO_OP  = 'NO_OP'
    ABS_OP = 'ABS_OP'
    ADD_OP = 'ADD_OP'
    SFT_OP = 'SFT_OP'
    PAN_OP = 'PAN_OP'


@dataclass(frozen=True)
class ParamEntry:
    field: PropertyDescription
    path: ScopePath
    plugin_id: int
    type: ParamType
    norm: float | None = None
    scale: float | None = None
    min: float | None = None
    max: float | None = None
    cut_negatives: bool = False
    operation: ParamOperations = ParamOperations.NO_OP

    @property
    def name(self) -> str:
        return self.field.name

    @property
    def full_name(self) -> ScopeDescription:
        return self.field.full_name

    @property
    def snake_name(self):
        return self.path.path.replace('.', '__')

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
    def new_param_entry(
            field: PropertyDescription,
            path: ScopePath,
            plugin_id: int,
            type: ParamType,
            attrs: Attributes | None
    ):
        attrs = {name: value for attr in attrs for name, value in attr.params.items() if attr.name[0] == 'tbd'}

        if not attrs:
            return ParamEntry(field=field, path=path, plugin_id=plugin_id, type=type)

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
            return ParamEntry(field=field, path=path, plugin_id=plugin_id, type=type)

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
            return ParamEntry(field=field, path=path, plugin_id=plugin_id, type=type, operation=operation, **filtered_attrs)

        if attrs.get(ATTR_ABS):
            cut_negatives = False
        elif attrs.get(ATTR_CUT):
            cut_negatives = True
        else:
            cut_negatives = False

        # [ParamType.UINT_PARAM, ParamType.UFLOAT_PARAM]:
        return ParamEntry(name=name, full_name=full_name, path=path, plugin_id=plugin_id, type=type, operation=operation, cut_negatives=cut_negatives,
                          **filtered_attrs)

    def hash(self):
        return crc32(self.name.encode())

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
    cls: ReflectableDescription
    param_offset: int
    params: PluginParams
    header: Path
    is_stereo: bool

    @property
    def name(self) -> str:
        return self.cls.name

    @property
    def full_name(self) -> ScopeDescription:
        return self.cls.full_name

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

    def hash(self):
        return crc32(self.name.encode())

__all__ = ['ParamEntry', 'PluginParams', 'PluginEntry']