from abc import ABC, abstractmethod
from pathlib import Path
from typing import (
    Final,
    TypeVar,
    Generic,
    Union,
    Protocol,
    Optional, Iterator, OrderedDict
)

from tbd_core.reflection.reflectables import *


CppTypePtr = Union['ClassPtr', Param, UnknownType]


class MissingCppEntry(LookupError):
    pass


class ReflectableDB(Protocol):
    def has_component(self, component: str) -> bool: ...
    def get_component(self, component_id: ComponentID) -> str: ...

    def has_file(self, component: str, file: Path) -> bool: ...
    def get_file(self, file_id: FileID) -> 'FilePtr': ...
    def add_file(self, component: str, file: str) -> 'FilePtr': ...

    def has_namespace(self, namespace: ScopePath) -> bool: ...
    def get_namespace(self, namespace_id: NamespaceID) -> 'NamespacePtr': ...
    def add_namespace(self, namespace: ScopePath) -> 'NamespacePtr': ...

    def has_function(self, function: ScopePath) -> bool: ...
    def get_function(self, function_id: FunctionID) -> 'FunctionPtr': ...

    def has_argument(self, argument: ScopePath) -> bool: ...
    def get_argument(self, argument_id: ArgumentID) -> 'ArgumentPtr': ...

    def get_type(self, type_id: CppType) -> CppTypePtr: ...

    def has_class(self, cls: ScopePath) -> bool: ...
    def get_class(self, class_id: ClassID) -> 'ClassPtr': ...
    def add_class(self,
        cls: ScopePath,
        properties: OrderedDict[str, CppType],
        *,
        component: str,
        files: list[str],
        bases: list[ScopePath] | None,
    ) -> 'ClassPtr': ...

    def has_property(self, prop: ScopePath) -> bool: ...
    def get_property(self, property_id: PropertyID) -> 'PropertyPtr': ...

    # iterators

    def namespaces(self) -> Iterator['NamespacePtr']: ...
    def functions(self) -> Iterator['FunctionPtr']: ...
    def arguments(self) -> Iterator['ArgumentPtr']: ...
    def classes(self) -> Iterator['ClassPtr']: ...
    def properties(self) -> Iterator['PropertyPtr']: ...

    # raw getters

    def get_raw_file(self, file_id: FileID) -> Optional[FileEntry]: ...
    def get_raw_namespace(self, namespace_id: NamespaceID) -> Optional['NamespaceEntry']: ...
    def get_raw_function(self, function_id: FunctionID) -> Optional['FunctionEntry']: ...
    def get_raw_argument(self, argument_id: ArgumentID) -> Optional['ArgumentEntry']: ...
    def get_raw_class(self, class_id: ClassID) -> Optional['ClassEntry']: ...
    def get_raw_property(self, property_id: PropertyID) -> Optional['PropertyEntry']: ...


class FilePtr:
    def __init__(self, _id: FileID, _db: ReflectableDB):
        self._id = _id
        self._db = _db

    @property
    def file(self) -> Path:
        return Path(self._obj().file)

    @property
    def component(self) -> str:
        obj = self._obj()
        return self._db.get_component(obj.component)

    def __str__(self) -> str:
        obj = self._obj()
        return str(obj.file)

    def ref(self) -> int:
        return self._id

    def _obj(self) -> FileEntry:
        raw = self._db.get_raw_file(self._id)
        if raw is None:
            raise LookupError(f'no file with ID {self._id}')
        return raw


PtrType = TypeVar("PtrType", bound=EntryBase)
class PtrBase(Generic[PtrType], ABC):
    def __init__(self, _id: int, _db: ReflectableDB):
        self._id: Final[int] = _id
        self._db: Final[ReflectableDB] = _db

    @property
    def header(self) -> FilePtr:
        obj = self._obj()
        return FilePtr(obj.files[0], self._db)

    @property
    def files(self) -> Iterator[FilePtr]:
        obj = self._obj()
        for file_id in obj.files:
            yield FilePtr(file_id, self._db)

    @property
    def component(self) -> str:
        file = next(self.files)
        return file.component

    @property
    @abstractmethod
    def parent(self):
        raise NotImplementedError()

    @property
    def name(self) -> str:
        return self._obj().name

    @property
    def full_name(self) -> str:
        return self.scope.path

    @property
    def friendly_name(self) -> str | None:
        return self._obj().friendly_name

    @property
    def description(self) -> str | None:
        return self._obj().description

    @property
    def attrs(self) -> Attributes | None:
        return self._obj().attrs

    @property
    @abstractmethod
    def scope(self) -> ScopePath:
        raise NotImplementedError()

    def ref(self) -> int:
        return self._id

    def __str__(self) -> str:
        return self.full_name

    @abstractmethod
    def _obj(self) -> PtrType:
        raise NotImplementedError()


class NamespacePtr(PtrBase[NamespaceEntry]):
    @property
    def namespace_name(self) -> str:
        return self._obj().namespace_name

    @property
    def parent(self) -> Union['NamespacePtr', None]:
        if (parent_id := self._obj().parent) > 0:
            return self._db.get_namespace(parent_id)
        return None

    @property
    def scope(self) -> ScopePath:
        parent = self.parent
        parent_scope = parent.scope if parent else ScopePath.root()
        return parent_scope.add_namespace(self.namespace_name, self._id)

    def __repr__(self) -> str:
        return f'Namespace({self.full_name})'

    def _obj(self) -> NamespaceEntry:
        raw = self._db.get_raw_namespace(self._id)
        if raw is None:
            raise LookupError(f'no namespace with ID {self._id}')
        return raw


class ArgumentPtr(PtrBase[ArgumentEntry], Typed):
    @property
    def arg_name(self) -> str:
        return self._obj().arg_name

    @property
    def category(self) -> ArgumentCategory:
        return self._obj().category

    @property
    def type(self) -> CppTypePtr:
        return self._db.get_type(self._obj().type)

    @property
    def typename(self) -> str:
        return self._db.get_type(self._obj().type).typename

    @property
    def parent(self) -> Optional['FunctionPtr']:
        return self._db.get_function(self._obj().parent)

    @property
    def scope(self) -> ScopePath:
        parent_scope = self.parent.scope
        return parent_scope.add_argument(self.arg_name, self._id)

    def __repr__(self) -> str:
        return f'Argument({self.full_name})'

    def _obj(self) -> ArgumentEntry:
        raw = self._db.get_raw_argument(self._id)
        if raw is None:
            raise LookupError(f'no argument with ID {self._id}')
        return raw


class FunctionPtr(PtrBase[FunctionEntry]):
    @property
    def func_name(self) -> str:
        return self._obj().func_name

    @property
    def parent(self) -> Optional[NamespacePtr]:
        return self._db.get_namespace(self._obj().parent)

    @property
    def scope(self) -> ScopePath:
        parent_scope = self.parent.scope
        return parent_scope.add_function(self.func_name, self._id)

    @property
    def has_arguments(self) -> bool:
        return len(self._obj().arguments) > 0

    @property
    def arguments(self) -> list[ArgumentPtr]:
        return [ArgumentPtr(arg_id, self._db) for arg_id in self._obj().arguments]

    @property
    def argument_typenames(self) -> list[str]:
        return [ArgumentPtr(arg_id, self._db).typename for arg_id in self._obj().arguments]

    @property
    def return_type(self) -> CppTypePtr | None:
        return_type_id = self._obj().return_type
        if return_type_id is not None:
            return self._db.get_type(return_type_id)
        return None

    @property
    def return_typename(self) -> str:
        return_type_id = self._obj().return_type
        if return_type_id is not None:
            return self._db.get_type(return_type_id).typename
        return 'void'

    @property
    def generated(self) -> bool:
        return self._obj().generated

    def __repr__(self) -> str:
        return f'Function({self.full_name})'

    def _obj(self) -> FunctionEntry:
        raw = self._db.get_raw_function(self._id)
        if raw is None:
            raise LookupError(f'no function with ID {self._id}')
        return raw


class PropertyPtr(PtrBase[PropertyEntry], Typed):
    @property
    def field_name(self) -> str:
        return self._obj().field_name

    @property
    def type(self) -> CppTypePtr:
        type_id = self._obj().type
        return self._db.get_type(type_id)

    @property
    def typename(self) -> str:
        return str(self._db.get_type(self._obj().type))

    @property
    def parent(self) -> 'ClassPtr':
        return self._db.get_class(self._obj().parent)

    @property
    def scope(self) -> ScopePath:
        parent_scope = self.parent.scope
        return parent_scope.add_field(self.field_name, self._id)

    def __repr__(self) -> str:
        return f'Property({self.full_name})'

    def _obj(self) -> PropertyEntry:
        raw = self._db.get_raw_property(self._id)
        if raw is None:
            raise LookupError(f'no property with ID {self._id}')
        return raw


class ClassPtr(PtrBase, Typed):
    @property
    def cls_name(self) -> str:
        return self._obj().cls_name

    @property
    def typename(self) -> str:
        return str(self.scope)

    @property
    def is_anonymous(self) -> bool:
        return self.cls_name.startswith(ANONYMOUS_STRUCT_PREFIX)

    @property
    def anonymous_id(self) -> int | None:
        if not self.cls_name.startswith(ANONYMOUS_STRUCT_PREFIX):
            return None
        return int(self.cls_name[len(ANONYMOUS_STRUCT_PREFIX):])

    @property
    def bases(self) -> list[CppTypePtr]:
        return [self._db.get_type(type_id) for type_id in self._obj().bases]

    @property
    def properties(self) -> list[PropertyPtr]:
        obj = self._obj()
        properties = []
        for base in self.bases:
            properties += base.all_properties
        properties += [PropertyPtr(prop_id, self._db) for prop_id in obj.properties]
        return properties

    @property
    def all_properties(self) -> list[PropertyPtr]:
        obj = self._obj()
        return [PropertyPtr(prop_id, self._db) for prop_id in obj.properties]

    @property
    def parent(self) -> Union['ClassPtr', NamespacePtr]:
        parent_id = self._obj().parent
        if self._db.get_raw_class(parent_id):
            return ClassPtr(parent_id, self._db)
        return self._db.get_namespace(parent_id)

    @property
    def scope(self) -> ScopePath:
        parent_scope = self.parent.scope
        return parent_scope.add_class(self.cls_name, self._id)

    @property
    def generated(self) -> bool:
        return self._obj().generated

    def __repr__(self) -> str:
        return f'Class({self.full_name})'

    def _obj(self) -> ClassEntry:
        raw = self._db.get_raw_class(self._id)
        if raw is None:
            raise LookupError(f'no class with ID {self._id}')
        return raw


__all__ = [
    'CppTypePtr',
    'MissingCppEntry',
    'ReflectableDB',
    'FilePtr',
    'NamespacePtr',
    'ArgumentPtr',
    'FunctionPtr',
    'PropertyPtr',
    'ClassPtr',
]
