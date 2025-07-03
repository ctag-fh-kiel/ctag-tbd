from pathlib import Path
from typing import Final, Optional, Iterator

from tbd_core.reflection.reflectables import (
    NamespaceEntry,
    FunctionEntry,
    PropertyEntry,
    ClassEntry,
    Reflectables,
    CppType,
    FileEntry,
    Param, PropertyID, ComponentID, FileID, NamespaceID, FunctionID, ArgumentID, ClassID, UnknownType,
)
from .pointers import (
    CppTypePtr,
    MissingCppEntry,
    NamespacePtr,
    ReflectableDB,
    FunctionPtr,
    ClassPtr,
    PropertyPtr,
    ArgumentPtr,
    FilePtr
)


class InMemoryReflectableDB(ReflectableDB):
    def __init__(self, data: Reflectables):
        self._data: Final[Reflectables] = data

    ## accessors ##

    def get_component(self, component_id: ComponentID) -> str:
        if (component := self._data.components.get(component_id)) is None:
            raise MissingCppEntry(f'no header with ID {component_id}')
        return component

    def get_file(self, file_id: FileID) -> FilePtr:
        if file_id not in self._data.files:
            raise MissingCppEntry(f'no file with ID {file_id}')
        return FilePtr(file_id, self)

    def get_namespace(self, namespace_id: NamespaceID) -> NamespacePtr:
        if namespace_id not in self._data.namespaces:
            raise MissingCppEntry(f'no namespace with ID {namespace_id}')
        return NamespacePtr(namespace_id, self)

    def get_function(self, function_id: FunctionID) -> FunctionPtr:
        if function_id not in self._data.functions:
            raise MissingCppEntry(f'no function with ID {function_id}')
        return FunctionPtr(function_id, self)

    def get_argument(self, argument_id: ArgumentID) -> ArgumentPtr:
        if argument_id not in self._data.arguments:
            raise MissingCppEntry(f'no argument with ID {argument_id}')
        return ArgumentPtr(argument_id, self)

    def get_type(self, type_id: CppType) -> CppTypePtr:
        match type_id:
            case Param():
                return type_id
            case UnknownType():
                return type_id
            case ClassID():
                if type_id not in self._data.classes:
                    raise ValueError(f'no such type: {type_id}')
                return self.get_class(type_id)
            case _:
                raise ValueError(f'type ID is neither class ID nor typename, expected str or int')

    def get_class(self, class_id: ClassID) -> ClassPtr:
        if class_id not in self._data.classes:
            raise MissingCppEntry(f'no class with ID {class_id}')
        return ClassPtr(class_id, self)

    def get_property(self, property_id: PropertyID) -> PropertyPtr:
        if property_id not in self._data.properties:
            raise MissingCppEntry(f'no property with ID {property_id}')
        return PropertyPtr(property_id, self)

    ## iterators ##

    def namespaces(self) -> Iterator[NamespacePtr]:
        for namespace_id in self._data.namespaces.keys():
            yield NamespacePtr(namespace_id, self)

    def functions(self) -> Iterator[FunctionPtr]:
        for function_id in self._data.functions.keys():
            yield FunctionPtr(function_id, self)

    def arguments(self) -> Iterator[ArgumentPtr]:
        for argument_id in self._data.arguments.keys():
            yield ArgumentPtr(argument_id, self)

    def classes(self) -> Iterator[ClassPtr]:
        for class_id in self._data.classes.keys():
            yield ClassPtr(class_id, self)

    def properties(self) -> Iterator[PropertyPtr]:
        for property_id in self._data.properties.keys():
            yield PropertyPtr(property_id, self)

    ## raw accessors ##

    def get_raw_file(self, file_id: FileID) -> Optional[FileEntry]:
        return self._data.files.get(file_id)

    def get_raw_namespace(self, namespace_id: NamespaceID) -> Optional['NamespaceEntry']:
        return self._data.namespaces.get(namespace_id)

    def get_raw_argument(self, argument_id: ArgumentID) -> Optional['ArgumentPtr']:
        return self._data.arguments.get(argument_id)

    def get_raw_function(self, function_id: FunctionID) -> Optional['FunctionEntry']:
        return self._data.functions.get(function_id)

    def get_raw_property(self, property_id: PropertyID) -> Optional['PropertyEntry']:
        return self._data.properties.get(property_id)

    def get_raw_class(self, class_id: ClassID) -> Optional['ClassEntry']:
        return self._data.classes.get(class_id)



__all__ = ['InMemoryReflectableDB']