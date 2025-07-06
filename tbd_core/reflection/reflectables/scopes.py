from dataclasses import dataclass
from enum import IntEnum, unique
from typing import Self
from abc import ABC, abstractmethod
from zlib import crc32

import cxxheaderparser.types as cpptypes




@unique
class ScopeType(IntEnum):
    NAMESPACE = 0
    FUNCTION = 1
    ARGUMENT = 2
    CLASS = 3
    STATIC_FIELD = 4
    FIELD = 5

VALID_CHILD_TYPES = {
    ScopeType.NAMESPACE: [ScopeType.NAMESPACE, ScopeType.FUNCTION, ScopeType.CLASS],
    ScopeType.FUNCTION: [ScopeType.ARGUMENT],
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


class ScopePathSegment:
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

    def is_namespace(self) -> bool:
        return self.type is ScopeType.NAMESPACE

    def namespace(self) -> str:
        if self._type != ScopeType.NAMESPACE:
            raise ValueError(f'scope segment is not a namespace: {self._type}')
        return self._name

    def is_function(self) -> bool:
        return self.type is ScopeType.FUNCTION

    def function(self) -> str:
        if self._type != ScopeType.FUNCTION:
            raise ValueError(f'scope segment is not a function: {self._type}')
        return self._name

    def is_argument(self) -> bool:
        return self.type is ScopeType.ARGUMENT

    def argument(self) -> str:
        if self._type != ScopeType.ARGUMENT:
            raise ValueError(f'scope segment is not an argument: {self._type}')
        return self._name

    def is_cls(self) -> bool:
        return self.type is ScopeType.CLASS

    def cls(self) -> str:
        if self._type != ScopeType.CLASS:
            raise ValueError(f'scope segment is not a class: {self._type}')
        return self._name

    def is_static_field(self) -> bool:
        return self.type is ScopeType.STATIC_FIELD

    def static_field(self) -> str:
        if self._type != ScopeType.STATIC_FIELD:
            raise ValueError(f'scope segment is not a static field: {self._type}')
        return self._name

    def is_field(self) -> bool:
        return self.type is ScopeType.FIELD

    def field(self) -> str:
        if self._type != ScopeType.FIELD:
            raise ValueError(f'scope segment is not a static field: {self._type}')
        return self._name


@dataclass(frozen=True)
class ScopePath:
    segments: list[ScopePathSegment]

    @staticmethod
    def root() -> 'ScopePath':
        return ScopePath([])

    def add_namespace(self, namespace: str, _id: int | None = None) -> 'ScopePath':
        if self.segments and not is_valid_parent(self.segments[-1].type, ScopeType.NAMESPACE):
            raise ValueError('namespaces can only be declared in namespaces')
        return ScopePath([*self.segments, ScopePathSegment(namespace, ScopeType.NAMESPACE, _id)])

    def add_function(self, func: str, _id: int | None = None) -> 'ScopePath':
        if self.segments and not is_valid_parent(self.segments[-1].type, ScopeType.FUNCTION):
            raise ValueError('function can only be declared in namespaces')
        return ScopePath([*self.segments, ScopePathSegment(func, ScopeType.FUNCTION, _id)])

    def add_class(self, cls: str, _id: int | None = None) -> 'ScopePath':
        if self.segments and not is_valid_parent(self.segments[-1].type, ScopeType.CLASS):
            raise ValueError('classes can only be declared in namespaces or other classes')
        return ScopePath([*self.segments, ScopePathSegment(cls, ScopeType.CLASS, _id)])

    def add_field(self, field: str, _id: int | None = None) -> 'ScopePath':
        if self.segments and not is_valid_parent(self.segments[-1].type, ScopeType.FIELD):
            raise ValueError('fields can only be declared in classes and fields')

        return ScopePath([*self.segments, ScopePathSegment(field, ScopeType.FIELD, _id)])

    def add_static_field(self, field: str, _id: int | None = None) -> 'ScopePath':
        if self.segments and not is_valid_parent(self.segments[-1].type, ScopeType.STATIC_FIELD):
            raise ValueError('static fields can only be declared in classes')

        return ScopePath([*self.segments, ScopePathSegment(field, ScopeType.STATIC_FIELD, _id)])

    def add_argument(self, arg: str, _id: int | None = None) -> 'ScopePath':
        if self.segments and not is_valid_parent(self.segments[-1].type, ScopeType.ARGUMENT):
            raise ValueError('arguments can only be declared for functions')
        return ScopePath([*self.segments, ScopePathSegment(arg, ScopeType.ARGUMENT, _id)])

    def is_namespace(self) -> bool:
        return self.segments[-1].is_namespace()

    def namespace(self) -> str:
        return self.segments[-1].namespace()

    def is_function(self) -> bool:
        return self.segments[-1].is_function()

    def function(self) -> str:
        return self.segments[-1].function()

    def is_cls(self) -> bool:
        return self.segments[-1].is_cls()

    def cls(self) -> str:
        return self.segments[-1].cls()

    def is_static_field(self) -> bool:
        return self.segments[-1].is_static_field()

    def static_field(self) -> str:
        return self.segments[-1].static_field()

    def is_field(self) -> bool:
        return self.segments[-1].is_field()

    def field(self) -> str:
        return self.segments[-1].field()

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
    def parent(self) -> Self | None:
        if len(self.segments) < 2:
            return None
        return ScopePath(self.segments[:-1])

    @property
    def path(self) -> str:
        if not self.segments:
            return ''

        fst, *tail = self.segments
        retval = fst.name
        for elem in tail:
            if elem.type == ScopeType.FIELD:
                retval += f'.{elem.name}'
            elif elem.type == ScopeType.ARGUMENT:
                retval += f'<-{elem.name}'
            else:
                retval += f'::{elem.name}'
        return retval

    def hash(self) -> int:
        return crc32(self.path.encode())

    def __len__(self) -> int:
        return len(self.segments)

    def __getitem__(self, item):
        return ScopePath(self.segments[item])

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
    'ScopeType',
    'ScopePath',
    'ScopePathSegment',
]