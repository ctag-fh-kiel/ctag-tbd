from dataclasses import dataclass
from enum import IntEnum, unique
from typing import Self
from abc import ABC, abstractmethod
from zlib import crc32

import cxxheaderparser.types as cpptypes
import cxxheaderparser.simple as cpplib

from .attributes import Attributes


ANONYMOUS_STRUCT_PREFIX = '__struct_'


def normalize_typename(typename: cpptypes.PQName) -> tuple[str, bool]:
    typename = typename.segments[-1]
    # handle anonymous data types
    if isinstance(typename, cpptypes.AnonymousName):
        annon_id = typename.id
        return f'{ANONYMOUS_STRUCT_PREFIX}{annon_id}', True
    else:
        return typename.format(), False


@unique
class ScopeType(IntEnum):
    NAMESPACE = 0
    FUNCTION = 1
    CLASS = 2
    STATIC_FIELD = 3
    FIELD = 4

VALID_CHILD_TYPES = {
    ScopeType.NAMESPACE: [ScopeType.NAMESPACE, ScopeType.FUNCTION, ScopeType.CLASS],
    ScopeType.FUNCTION: [],
    ScopeType.CLASS: [ScopeType.FUNCTION, ScopeType.CLASS, ScopeType.STATIC_FIELD, ScopeType.FIELD],
    ScopeType.STATIC_FIELD: [ScopeType.STATIC_FIELD, ScopeType.FIELD],
    ScopeType.FIELD: [ScopeType.FIELD],
}

VALID_PARENT_TYPES = {
    scope_type: [parent for parent, children in VALID_CHILD_TYPES.items() if scope_type in children]
    for scope_type in ScopeType
}

def is_valid_child(child_type: ScopeType, parent_type: ScopeType) -> bool:
    return child_type in VALID_CHILD_TYPES[parent_type]

def is_valid_parent(parent_type: ScopeType, child_type: ScopeType) -> bool:
    return parent_type in VALID_PARENT_TYPES[child_type]

class ScopeSegmentBase(ABC):
    @property
    @abstractmethod
    def name(self) -> str:
        raise NotImplementedError()

    @property
    @abstractmethod
    def type(self) -> ScopeType:
        raise NotImplementedError()


class ScopeDescriptionSegment(ScopeSegmentBase):
    """ Single element in a c++ scope.

        Scope segments can be any name not associated with a value. These are

        - namespaces
        - functions
        - classes
        - static fields
        - fields
    """

    def __init__(self, raw: cpplib.NamespaceScope | cpplib.ClassScope | cpplib.NamespaceScope | cpplib.Field | cpplib.Function):
        self._raw = raw

    @property
    def raw(self) -> cpplib.NamespaceScope | cpplib.ClassScope | cpplib.NamespaceScope | cpplib.Field | cpplib.Function:
        return self._raw

    @property
    def name(self) -> str:
        match self._raw:
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
                raise ValueError(f'invalid scope segment type {type(self._raw)}')

    @property
    def type(self) -> ScopeType:
        match self._raw:
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
                raise ValueError(f'invalid scope segment type {type(self._raw)}')

    @property
    def attrs(self) -> Attributes | None:
        match self._raw:
            case cpplib.NamespaceScope() as namespace:
                return namespace.attrs
            case cpptypes.Function() as func:
                return func.attrs
            case cpplib.ClassScope() as cls:
                return cls.class_decl.attrs
            case cpptypes.Field() as field:
                return field.attrs
            case _:
                raise ValueError(f'invalid scope segment type {type(self._raw)}')

    @property
    def is_root(self):
        return self.type == ScopeType.NAMESPACE and self.name == ''

    # convert to tail type

    def namespace(self) -> cpplib.NamespaceScope:
        if self.type != ScopeType.NAMESPACE:
            raise ValueError(f'scope segment is not a namespace: {self.type}')
        return self._raw

    def func(self) -> cpplib.Function:
        if self.type != ScopeType.FUNCTION:
            raise ValueError(f'scope segment is not a function: {self.type}')
        return self._raw

    def cls(self) -> cpplib.ClassScope:
        if self.type != ScopeType.CLASS:
            raise ValueError(f'scope segment is not a class: {self.type}')
        return self._raw

    def field(self) -> cpptypes.Field:
        if self.type != ScopeType.FIELD:
            raise ValueError(f'scope segment is not a field: {self.type}')
        return self._raw


class ScopeDescription:
    """ Description of a C++ scope.

        Scopes describe the nesting of named scopes. See `ScopeDescriptionSegment` for segment types. Note that the
        word 'scope' is used more loosely here than in C++: It can refer both to a scope in the C++ sense of the word
        but also to classes and functions within a scope and paths/nested paths to nested fields. For scopes in the
        traditional sense the string representation of scopes uses `::` to indicate nesting, for paths to fields `.`
        is used.

        Examples:
        ---------

        - namespaces: `ns1::ns2::ns2`
        - classes `ns1::ns2::Cls1`
        - nested classes: `ns1::ns2::Cls1::NestedCls1`
        - fields: `ns1::ns2::Cls1.field1`
        - field in field of type struct: `ns1::ns2::Cls1.field1.nested_field1`
        - field of nested class `ns1::ns2::Cls1::nested_Cls1.field1`
        - free functions: `ns1::ns2::free_function1`
        - static methods: `ns1::ns2::Cls1.method1`
        - static methods of nested classes: `ns1::ns2::Cls1.NestedCls1.nested_method1`

    """
    def __init__(self, segments: list[ScopeDescriptionSegment]):
        self._segments = segments

    @property
    def segments(self) -> list[ScopeDescriptionSegment]:
        return self._segments

    @staticmethod
    def from_root(root_namespace: cpplib.NamespaceScope) -> 'ScopeDescription':
        if root_namespace.name != '':
            raise ValueError('namespace is not root')
        return ScopeDescription([ScopeDescriptionSegment(root_namespace)])

    def add_namespace(self, namespace: cpplib.NamespaceScope) -> 'ScopeDescription':
        if self.segments and not is_valid_parent(self.segments[-1].type, ScopeType.NAMESPACE):
            raise ValueError('namespaces can only be declared in namespaces')
        return ScopeDescription([*self.segments, ScopeDescriptionSegment(namespace)])

    def add_function(self, func: cpptypes.Function) -> 'ScopeDescription':
        if self.segments and not is_valid_parent(self.segments[-1].type, ScopeType.FUNCTION):
            raise ValueError('function can only be declared in namespaces')
        return ScopeDescription([*self.segments, ScopeDescriptionSegment(func)])

    def add_class(self, cls: cpplib.ClassScope) -> 'ScopeDescription':
        if self.segments and not is_valid_parent(self.segments[-1].type, ScopeType.CLASS):
            raise ValueError('classes can only be declared in namespaces or other classes')
        return ScopeDescription([*self.segments, ScopeDescriptionSegment(cls)])

    def add_static_field(self, field: cpptypes.Field) -> 'ScopeDescription':
        if self.segments and self.segments[-1].type != ScopeType.CLASS:
            raise ValueError('static fields can only be declared in classes')
        if not field.static:
            raise ValueError('field is not static')
        return ScopeDescription([*self.segments, ScopeDescriptionSegment(field)])

    def add_field(self, field: cpptypes.Field) -> 'ScopeDescription':
        if self.segments and not is_valid_parent(self.segments[-1].type, ScopeType.FIELD):
            raise ValueError('fields can only be declared in classes and fields')
        if field.static:
            raise ValueError('field is static')
        return ScopeDescription([*self.segments, ScopeDescriptionSegment(field)])

    def __add__(self, other: 'ScopeDescription') -> 'ScopeDescription':
        if len(self.segments) == 0:
            return ScopeDescription([*other.segments])
        if len(other.segments) == 0:
            return ScopeDescription([*self.segments])

        self_type = self.segments[-1].type
        other_type = other.segments[0].type
        if not is_valid_child(other_type, self_type):
            raise ValueError(f'can not merge scopes of type {self_type.name} and {other_type.name}')
        return ScopeDescription([*self.segments, *other.segments])

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
    def parent(self) -> Self | None:
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

    @property
    def hash(self) -> int:
        return crc32(self.path.encode())

    def __str__(self) -> str:
        return self.path

    def __repr__(self) -> str:
        return f'ScopeDescription({self.path})'

    def split(self) -> tuple['ScopeDescription', 'ScopeDescription', 'ScopeDescription']:
        """ Split the scope description into scope segment type groups  in order. """

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

            while pos.type == ScopeType.STATIC_FIELD:
                classes.append(pos)

            while pos.type == ScopeType.FIELD:
                fields.append(pos)
                if not rest:
                    raise SplitDone
                pos, *rest = rest
        except SplitDone:
            return ScopeDescription(namespaces), ScopeDescription(classes), ScopeDescription(fields)

        raise ValueError('bad segment order in scope')


class ScopePathSegment(ScopeSegmentBase):
    def __init__(self, name: str, _type: ScopeType, _id: int | None) -> None:
        self._name: str = name
        self._type: ScopeType = _type
        self._id: int | None = _id

    @property
    def name(self) -> str:
        return self._name

    @property
    def type(self) -> ScopeType:
        return self._type

    def namespace(self) -> str:
        if self._type != ScopeType.NAMESPACE:
            raise ValueError(f'scope segment is not a namespace: {self._type}')
        return self._name

    def cls(self) -> str:
        if self._type != ScopeType.CLASS:
            raise ValueError(f'scope segment is not a namespace: {self._type}')
        return self._name


@dataclass(frozen=True)
class ScopePath:
    segments: list[ScopePathSegment]

    @staticmethod
    def root() -> 'ScopePath':
        return ScopePath([])

    def add_namespace(self, namespace: str, _id: int | None = None) -> 'ScopePath':
        if self.segments and self.segments[-1].type != ScopeType.NAMESPACE:
            raise ValueError('namespaces can only be declared in namespaces')
        return ScopePath([*self.segments, ScopePathSegment(namespace, ScopeType.NAMESPACE, _id)])

    def add_class(self, cls: str, _id: int | None = None) -> 'ScopePath':
        if self.segments and self.segments[-1].type not in [ScopeType.NAMESPACE, ScopeType.CLASS]:
            raise ValueError('classes can only be declared in namespaces or other classes')
        return ScopePath([*self.segments, ScopePathSegment(cls, ScopeType.CLASS, _id)])

    def add_field(self, field: str, _id: int | None = None) -> 'ScopePath':
        if self.segments and self.segments[-1].type not in [ScopeType.CLASS, ScopeType.FIELD]:
            raise ValueError('fields can only be declared in classes and fields')

        return ScopePath([*self.segments, ScopePathSegment(field, ScopeType.FIELD, _id)])

    def add_static_field(self, field: str, _id: int | None = None) -> 'ScopePath':
        if self.segments and self.segments[-1].type != ScopeType.CLASS:
            raise ValueError('static fields can only be declared in classes')

        return ScopePath([*self.segments, ScopePathSegment(field, ScopeType.STATIC_FIELD, _id)])

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


__all__ = [
    'normalize_typename',
    'ScopeType',
    'ScopeDescription',
    'ScopePath',
    'ScopePathSegment',
]