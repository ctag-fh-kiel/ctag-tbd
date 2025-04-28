from dataclasses import dataclass
from enum import IntEnum, Enum, unique
from typing import List, Set, Optional
from pathlib import Path
import cxxheaderparser.types as cpptypes


@unique
class ScopeType(IntEnum):
    NAMESPACE    = 0
    CLASS        = 1
    STATIC_FIELD = 2
    FIELD        = 3


@dataclass
class ScopeSegment:
    name: str
    type: ScopeType
    
@dataclass(frozen=True)
class ScopeDescription:
    segments: list[ScopeSegment]

    @staticmethod
    def root():
        return ScopeDescription([])

    def add_namespace(self, namespace_name: str) -> 'ScopeDescription':
        if self.segments and self.segments[-1].type != ScopeType.NAMESPACE:
            raise ValueError('namespaces can only be declared in namespaces')
        return ScopeDescription([*self.segments, ScopeSegment(namespace_name, ScopeType.NAMESPACE)])
    
    def add_class(self, namespace_name: str) -> 'ScopeDescription':
        if self.segments and self.segments[-1].type not in [ScopeType.NAMESPACE, ScopeType.CLASS]:
            raise ValueError('classes can only be declared in namespaces or other classes')
        return ScopeDescription([*self.segments, ScopeSegment(namespace_name, ScopeType.CLASS)])
    
    def add_field(self, field_name: str) -> 'ScopeDescription':
        if self.segments and self.segments[-1].type not in [ScopeType.CLASS, ScopeType.FIELD]:
            raise ValueError('fields can only be declared in classes and fields')
        return ScopeDescription([*self.segments, ScopeSegment(field_name, ScopeType.FIELD)])
    
    def add_static_field(self, field_name: str) -> 'ScopeDescription':
        if self.segments and self.segments[-1].type != ScopeType.CLASS:
            raise ValueError('static fields can only be declared in classes')
        return ScopeDescription([*self.segments, ScopeSegment(field_name, ScopeType.STATIC_FIELD)])

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

    def split(self) -> tuple['ScopeDescription', 'ScopeDescription', 'ScopeDescription']:
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
            return ScopeDescription(namespaces), ScopeDescription(classes), ScopeDescription(fields)

        raise ValueError('bad segment order in scope')


    @property
    def path(self) -> str:
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
        return f'Scope({self.path})'

@unique
class PropertyType(Enum):
    """ valid C++ types for properties """

    int_type    = 'int32_t'
    uint_type   = 'uint32_t'
    float_type  = 'float'

type_map = {t.value: t.name for t in PropertyType}

def get_property_type(cpp_type: str):
    if cpp_type not in type_map:
        raise ValueError(f'{cpp_type} is not a supported property type')

Attribute = int | float | bool | str
Attributes = dict[str, Attribute]


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


# @dataclass
# class GroupDescription:
#     field_name: str
#     full_name: ScopeDescription
#     raw: cpptypes.Field
#     friendly_name: Optional[str] = None
#     description: Optional[str] = None
#     Properties: int = list


Properties = List[PropertyDescription]

@dataclass
class ReflectableDescription:
    cls_name: str
    full_name: ScopeDescription
    header: Path
    raw: cpptypes.ClassDecl
    friendly_name: Optional[str] = None
    description: Optional[str] = None
    properties: Optional[List[PropertyDescription]] = list

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

Headers = Set[str]


__all__ = [
    'PropertyType', 
    'get_property_type', 
    'PropertyDescription', 
    'Properties', 
    'Attribute',
    'Attributes',
    'ReflectableDescription',
    'Headers',
]
