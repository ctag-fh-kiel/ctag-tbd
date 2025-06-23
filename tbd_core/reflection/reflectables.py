from dataclasses import dataclass

from pathlib import Path
from zlib import crc32

import cxxheaderparser.types as cpptypes
import cxxheaderparser.simple as cpplib
from rich import scope

from .attributes import Attributes
from .scopes import ScopeDescription


@dataclass
class NamespaceDescription:
    scope: ScopeDescription
    raw: cpplib.NamespaceScope

    @property
    def full_name(self) -> str:
        return self.scope.path

    @property
    def hash(self) -> int:
        return self.scope.hash

    def __str__(self) -> str:
        return self.full_name

    def __repr__(self) -> str:
        return f'Namespace({self.full_name})'


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

    @property
    def hash(self) -> int:
        return self.scope.hash

    def __str__(self) -> str:
        return self.full_name

    def __repr__(self) -> str:
        return f'Function({self.full_name})'


@dataclass
class PropertyDescription:
    scope: ScopeDescription
    type: str
    raw: cpptypes.Field
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

    @property
    def hash(self) -> int:
        return self.scope.hash

    def __str__(self) -> str:
        return self.full_name

    def __repr__(self) -> str:
        return f'Field({self.full_name})'


Properties = list[PropertyDescription]

@dataclass
class ReflectableDescription:
    raw: cpplib.ClassScope
    scope: ScopeDescription
    header: Path
    friendly_name: str | None = None
    description: str | None = None
    properties: list[PropertyDescription] | None = list

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

    @property
    def hash(self) -> int:
        return self.scope.hash

    def __str__(self) -> str:
        return self.full_name

    def __repr__(self) -> str:
        return f'Plugin({self.full_name})'
    

Reflectables = list[ReflectableDescription]

Headers = set[Path]


__all__ = [
    'NamespaceDescription',
    'PropertyDescription', 
    'Properties', 
    'FunctionDescription',
    'ReflectableDescription',
    'Reflectables',
    'Headers',
]
