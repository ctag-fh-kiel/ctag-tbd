from pathlib import Path
from typing import Final, Optional, Iterator, OrderedDict

from tbd_core.reflection.reflectables import *
from .pointers import *


class InMemoryReflectableDB(ReflectableDB):
    def __init__(self, data: Reflectables):
        self._data: Final[Reflectables] = data

    # component

    def has_component(self, component: str) -> bool:
        component_id = component_ref(component)
        return component_id in self._data.components

    def get_component(self, component_id: ComponentID) -> str:
        if (component := self._data.components.get(component_id)) is None:
            raise MissingCppEntry(f'no header with ID {component_id}')
        return component

    # file

    def has_file(self, component: str, file: Path) -> bool:
        file_id = file_ref(component, str(file))
        return file_id in self._data.files

    def get_file(self, file_id: FileID) -> FilePtr:
        if file_id not in self._data.files:
            raise MissingCppEntry(f'no file with ID {file_id}')
        return FilePtr(file_id, self)

    def add_file(self, component: str, file: str) -> FilePtr:
        if component not in self._data.components:
            component_id = component_ref(component)
            self._data.components[component_id] = component
        new_file_id = file_ref(component, file)
        self._data.files[new_file_id] = FileEntry(component=component_ref(component), file=file)
        return FilePtr(new_file_id, self)

    # namespace

    def has_namespace(self, namespace: ScopePath) -> bool:
        return namespace.hash() in self._data.namespaces

    def get_namespace(self, namespace_id: NamespaceID) -> NamespacePtr:
        if namespace_id not in self._data.namespaces:
            raise MissingCppEntry(f'no namespace with ID {namespace_id}')
        return NamespacePtr(namespace_id, self)

    def add_namespace(self, namespace: ScopePath) -> NamespacePtr:
        namespace_id = namespace.hash()
        if not self.has_namespace(namespace):
            parent = self.add_namespace(namespace.parent).ref() if namespace.parent else -1
            self._data.namespaces[namespace_id] = NamespaceEntry(
                parent=parent,
                namespace_name=namespace.namespace(),
            )

        return NamespacePtr(namespace_id, self)

    # function

    def has_function(self, function: ScopePath) -> bool:
        return function.hash() in self._data.functions

    def get_function(self, function_id: FunctionID) -> FunctionPtr:
        if function_id not in self._data.functions:
            raise MissingCppEntry(f'no function with ID {function_id}')
        return FunctionPtr(function_id, self)

    # argument

    def has_argument(self, argument: ScopePath) -> bool:
        return argument.hash() in self._data.arguments

    def get_argument(self, argument_id: ArgumentID) -> ArgumentPtr:
        if argument_id not in self._data.arguments:
            raise MissingCppEntry(f'no argument with ID {argument_id}')
        return ArgumentPtr(argument_id, self)

    # types

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

    def has_class(self, cls: ScopePath) -> bool:
        return cls.hash() in self._data.classes

    def get_class(self, class_id: ClassID) -> ClassPtr:
        if class_id not in self._data.classes:
            raise MissingCppEntry(f'no class with ID {class_id}')
        return ClassPtr(class_id, self)

    def add_class(self,
        cls: ScopePath,
        properties: OrderedDict[str, CppType],
        *,
        component: str,
        files: list[str],
        bases: list[ScopePath] | None,
    ) -> ClassPtr:
        if bases is None:
            bases = []

        file_ids = [self.add_file(component=component, file=file).ref() for file in files]

        cls_name = cls.cls()
        cls_id = cls.hash()

        namespace = cls.parent
        if not namespace.is_namespace():
            raise ValueError(f'{namespace} is not a namespace')
        namespace_id = namespace.hash()

        if bases is not None:
            bases = [self.get_type(base.hash()) for base in bases]

        prop_ids = []
        for prop_name, prop_type in properties.items():
            prop_id = cls.add_field(prop_name).hash()
            self._data.properties[prop_id] = PropertyEntry(
                files=file_ids,
                parent=cls_id,
                field_name=prop_name,
                type=prop_type,
            )
            prop_ids.append(prop_id)

        self._data.classes[cls_id] = ClassEntry(
            files=file_ids,
            parent=namespace_id,
            cls_name=cls_name,
            bases=bases,
            properties=prop_ids,
            generated=True,
        )
        return ClassPtr(cls_id, self)

    # property

    def has_property(self, prop: ScopePath) -> bool:
        return prop.hash() in self._data.properties

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