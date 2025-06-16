from dataclasses import dataclass

from pathlib import Path
import cxxheaderparser.types as cpptypes
import cxxheaderparser.simple as cpplib


from .attributes import Attributes
from .scopes import ScopeDescription


@dataclass
class FunctionDescription:
    func_name: str
    full_name: ScopeDescription
    header: Path
    raw: cpptypes.Function
    friendly_name: str | None = None
    description: str | None = None
    attrs: Attributes | None = None
    
    @property
    def return_type(self) -> str:
        return self.raw.return_type.typename.format()

    @property
    def name(self):
        return self.friendly_name if self.friendly_name else self.func_name

    @property
    def arguments(self) -> list[cpptypes.Parameter]:
        return self.raw.parameters

    def __str__(self) -> str:
        return self.full_name.path

    def __repr__(self) -> str:
        return f'Function({self.full_name.path})'


@dataclass
class PropertyDescription:
    field_name: str
    full_name: ScopeDescription
    type: str
    raw: cpptypes.Field
    friendly_name: str | None = None
    description: str | None = None
    attrs: Attributes | None = None
    
    @property
    def name(self):
        return self.friendly_name if self.friendly_name else self.field_name

    def __str__(self) -> str:
        return self.full_name.path

    def __repr__(self) -> str:
        return f'Field({self.full_name.path})'


Properties = list[PropertyDescription]

@dataclass
class ReflectableDescription:
    cls_name: str
    full_name: ScopeDescription
    header: Path
    raw: cpplib.ClassScope
    friendly_name: str | None = None
    description: str | None = None
    properties: list[PropertyDescription] | None = list

    @property
    def name(self):
        return self.friendly_name if self.friendly_name else self.cls_name

    @property
    def meta_name(self):
        return f'{self.cls_name}Meta'

    def __str__(self) -> str:
        return self.full_name.path

    def __repr__(self) -> str:
        return f'Plugin({self.full_name.path})'
    

Reflectables = list[ReflectableDescription]

Headers = set[Path]


__all__ = [
    'PropertyDescription', 
    'Properties', 
    'FunctionDescription',
    'ReflectableDescription',
    'Reflectables',
    'Headers',
]
