from dataclasses import dataclass
from enum import IntEnum, Enum, unique
from typing import List, Set, Optional
from pathlib import Path
import cxxheaderparser.simple as cpplib
import cxxheaderparser.types as cpptypes


AttributeArgTypes = int | float | bool | str

@dataclass
class Attribute:
    name_segments: list[str]
    params: dict[str, AttributeArgTypes]

    @property
    def name(self) -> str:
        return '::'.join(self.name_segments)
    
    def __str__(self):
        return self.name
    
    def __repr__(self):
        return f'Attribute({self.name})'

Attributes = list[Attribute]


@unique
class ScopeType(IntEnum):
    NAMESPACE    = 0
    FUNCTION     = 1
    CLASS        = 2
    STATIC_FIELD = 3
    FIELD        = 4


def normalize_typename(typename: cpptypes.PQName) -> tuple[str, bool]:
    typename = typename.segments[-1]
    # handle anonymous data types
    if isinstance(typename, cpptypes.AnonymousName):
        annon_id = typename.id
        return f'__struct_{annon_id}', True
    else:
        return typename.format(), False

@dataclass(frozen=True)
class ScopeDescriptionSegment:
    raw: cpplib.NamespaceScope | cpplib.ClassScope | cpplib.NamespaceScope

    @property
    def name(self) -> str:
        match self.raw:
            case cpplib.NamespaceScope() as namespace:
                return namespace.name
            case cpptypes.Function() as func:
                return func.name.format()
            case cpplib.ClassScope() as cls:
                typename, _ = normalize_typename(cls.class_decl.typename)
                return typename
            case cpptypes.Field() as field:
                return field.name
            case _:
                raise ValueError(f'invalid scope segment type {type(self.raw)}')

    @property
    def type(self) -> ScopeType:
        match self.raw:
            case cpplib.NamespaceScope():
                return ScopeType.NAMESPACE
            case cpptypes.Function():
                return ScopeType.FUNCTION
            case cpplib.ClassScope():
                return ScopeType.CLASS
            case cpptypes.Field() as field:
                if field.static:
                    return ScopeType.STATIC_FIELD
                else:
                    return ScopeType.FIELD
            case _:
                raise ValueError(f'invalid scope segment type {type(self.raw)}')
            
    @property
    def attrs(self) -> Attributes | None:
        match self.raw:
            case cpplib.NamespaceScope() as namespace:
                return namespace.attrs
            case cpptypes.Function() as func:
                return func.attrs
            case cpplib.ClassScope() as cls:
                return cls.class_decl.attrs
            case cpptypes.Field() as field:
                return field.attrs
            case _:
                raise ValueError(f'invalid scope segment type {type(self.raw)}')
            
    @property
    def is_root(self):
        return self.type == ScopeType.NAMESPACE and self.name == ''
    
    def namespace(self) -> cpplib.NamespaceScope:
        if self.type != ScopeType.NAMESPACE:
            raise ValueError(f'scope segment is not a namespace: {self.type}')
        return self.raw
    
    def func(self) -> cpplib.NamespaceScope:
        if self.type != ScopeType.FUNCTION:
            raise ValueError(f'scope segment is not a function: {self.type}')
        return self.raw

    def cls(self) -> cpplib.ClassScope:
        if self.type != ScopeType.CLASS:
            raise ValueError(f'scope segment is not a class: {self.type}')
        return self.raw
    
    def field(self) -> cpptypes.Field:
        if self.type != ScopeType.FIELD:
            raise ValueError(f'scope segment is not a field: {self.type}')
        return self.raw


@dataclass(frozen=True)
class ScopeDescription:
    segments: list[ScopeDescriptionSegment]

    @staticmethod
    def from_root(root_namespace: cpplib.NamespaceScope) -> 'ScopeDescription':
        if root_namespace.name != '':
            raise ValueError('namespace is not root')
        return ScopeDescription([ScopeDescriptionSegment(root_namespace)])

    def add_namespace(self, namespace: cpplib.NamespaceScope) -> 'ScopeDescription':
        if self.segments and self.segments[-1].type != ScopeType.NAMESPACE:
            raise ValueError('namespaces can only be declared in namespaces')
        return ScopeDescription([*self.segments, ScopeDescriptionSegment(namespace)])
    
    def add_function(self, func: cpptypes.Function) -> 'ScopeDescription':
        if self.segments and self.segments[-1].type not in [ScopeType.NAMESPACE]:
            raise ValueError('function can only be declared in namespaces')
        return ScopeDescription([*self.segments, ScopeDescriptionSegment(func)])

    def add_class(self, cls: cpplib.ClassScope) -> 'ScopeDescription':
        if self.segments and self.segments[-1].type not in [ScopeType.NAMESPACE, ScopeType.CLASS]:
            raise ValueError('classes can only be declared in namespaces or other classes')
        return ScopeDescription([*self.segments, ScopeDescriptionSegment(cls)])
    
    def add_field(self, field: cpptypes.Field) -> 'ScopeDescription':
        if self.segments and self.segments[-1].type not in [ScopeType.CLASS, ScopeType.FIELD]:
            raise ValueError('fields can only be declared in classes and fields')
        if field.static:
            raise ValueError('field is static')

        return ScopeDescription([*self.segments, ScopeDescriptionSegment(field)])
    
    def add_static_field(self, field: cpptypes.Field) -> 'ScopeDescription':
        if self.segments and self.segments[-1].type != ScopeType.CLASS:
            raise ValueError('static fields can only be declared in classes')
        if not field.static:
            raise ValueError('field is static') 

        return ScopeDescription([*self.segments, ScopeDescriptionSegment(field)])

    def namespace(self) -> cpplib.NamespaceScope:
        return self.segments[-1].namespace()

    def name(self):
        return self.segments[-1].name
    
    def func(self) -> cpptypes.Function:
        return self.segments[-1].func()

    def cls(self) -> cpplib.ClassScope:
        return self.segments[-1].cls()

    def field(self) -> cpptypes.Field:
        return self.segments[-1].field()

    def attrs(self) -> Attributes | None:
        return self.segments[-1].attrs

    def root(self) -> cpplib.NamespaceScope:
        if len(self.segments) < 1:
            raise ValueError('empty scope description has no root')
        return self.segments[0].namespace()

    @property
    def parent(self) -> 'ScopeDescription':
        if len(self.segments) < 2:
            return None
        return ScopeDescription(self.segments[:-1])
    
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
        fst, *tail = self.segments
        if fst.is_root:
            if len(tail) == 0:
                return ''
            else:
                fst, *tail = tail

        retval = fst.name
        for elem in tail:
            elem_name = elem.name 
            if elem.type == ScopeType.FIELD:
                retval += f'.{elem_name}'
            else:
                 retval += f'::{elem_name}'
        return retval
    
    def __str__(self) -> str:
        return self.path
    
    def __repr__(self) -> str:
        return f'ScopeDescription({self.path})'

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

            if pos.type == ScopeType.FUNCTION:
                classes.append(pos)
                return

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
    'Attribute',
    'Attributes',
    'normalize_typename',
    'ScopeDescription',
    'PropertyDescription', 
    'Properties', 
    'FunctionDescription',
    'ReflectableDescription',
    'Headers',
]
