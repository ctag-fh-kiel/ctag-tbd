import dataclasses
from dataclasses import dataclass

from pathlib import Path
from typing import OrderedDict

import cxxheaderparser.types as cpptypes
import cxxheaderparser.simple as cpplib

from .attributes import Attributes
from .scopes import ScopeDescription


@dataclass
class NamespaceDescription:
    scope: ScopeDescription
    raw: cpplib.NamespaceScope

    @property
    def full_name(self) -> str:
        return self.scope.path

    def ref(self) -> int:
        return self.scope.hash

    def __str__(self) -> str:
        return self.full_name

    def __repr__(self) -> str:
        return f'Namespace({self.full_name})'

NamespaceDescriptions = OrderedDict[int, NamespaceDescription]


@dataclass
class FunctionDescription:
    raw: cpptypes.Function
    scope: ScopeDescription
    header: Path
    friendly_name: str | None = None
    description: str | None = None
    attrs: Attributes | None = None
    
    @property
    def return_type(self) -> str:
        return self.raw.return_type.typename.format()

    @property
    def name(self) -> str:
        return self.friendly_name if self.friendly_name else self.func_name

    @property
    def full_name(self) -> str:
        return self.scope.path

    @property
    def func_name(self) -> str:
        return self.scope.name()

    @property
    def arguments(self) -> list[cpptypes.Parameter]:
        return self.raw.parameters

    def ref(self) -> int:
        return self.scope.hash

    def __str__(self) -> str:
        return self.full_name

    def __repr__(self) -> str:
        return f'Function({self.full_name})'

FunctionDescriptions = OrderedDict[int, FunctionDescription]


@dataclass
class PropertyDescription:
    scope: ScopeDescription
    raw: cpptypes.Field
    type: int | str
    cls_id: int
    friendly_name: str | None = None
    description: str | None = None
    attrs: Attributes | None = None
    
    @property
    def name(self) -> str:
        return self.friendly_name if self.friendly_name else self.field_name

    @property
    def full_name(self) -> str:
        return self.scope.path

    @property
    def field_name(self) -> str:
        return self.raw.name.format()

    def ref(self) -> int:
        return self.scope.hash

    def __str__(self) -> str:
        return self.full_name

    def __repr__(self) -> str:
        return f'Field({self.full_name})'

PropertyDescriptions = OrderedDict[int, PropertyDescription]


@dataclass
class ClassDescription:
    raw: cpplib.ClassScope
    scope: ScopeDescription
    header: Path
    friendly_name: str | None = None
    description: str | None = None
    properties: list[int] | None = list

    @property
    def name(self) -> str:
        return self.friendly_name if self.friendly_name else self.cls_name

    @property
    def cls_name(self) -> str:
        return self.scope.name()

    @property
    def full_name(self) -> str:
        return self.scope.path

    @property
    def meta_name(self) -> str:
        return f'{self.cls_name}Meta'

    def ref(self) -> int:
        return self.scope.hash

    def __str__(self) -> str:
        return self.full_name

    def __repr__(self) -> str:
        return f'Class({self.full_name})'
    

ClassDescriptions = OrderedDict[int, ClassDescription]


Headers = set[Path]


@dataclasses.dataclass
class Reflectables:
    namespaces: NamespaceDescriptions= dataclasses.field(default_factory=OrderedDict)
    funcs: FunctionDescriptions = dataclasses.field(default_factory=OrderedDict)
    classes: ClassDescriptions = dataclasses.field(default_factory=OrderedDict)
    properties: PropertyDescriptions = dataclasses.field(default_factory=OrderedDict)

    @property
    def class_list(self) -> list[ClassDescription]:
        return [v for v in self.classes.values()]

    @property
    def property_list(self) -> list[PropertyDescription]:
        return [v for v in self.properties.values()]

    @property
    def func_list(self) -> list[FunctionDescription]:
        return [v for v in self.funcs.values()]


__all__ = [
    'NamespaceDescription',
    'NamespaceDescriptions',
    'PropertyDescription', 
    'PropertyDescriptions',
    'FunctionDescription',
    'FunctionDescriptions',
    'ClassDescription',
    'ClassDescriptions',
    'Headers',
    'Reflectables',
]
