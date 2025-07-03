import logging
from pathlib import Path
from typing import Iterable

from tbd_core.reflection.parser import AnnotationParser, Parser

from tbd_core.reflection.reflectables import Reflectables, ScopePath, PropertyEntry, ClassEntry, UnknownType
from tbd_core.reflection.db import ReflectableDB, InMemoryReflectableDB

from .contexts import (
    FileContext,
    NamespaceContext,
    FunctionContext,
    ClassContext,
    FieldContext,
    ArgumentContext,
    is_anon_struct_field,
)


_LOGGER = logging.getLogger(__file__)


class ReflectableFinder:
    def __init__(self):
        self._cached_scopes: dict[int, ScopePath] = dict()
        self._reflectables: Reflectables = Reflectables()

    def get_reflectables(self) -> ReflectableDB:
        """ Finalize reflectables and return consistent reflectable database. """

        self._amend_property_types(self._reflectables)
        self._amend_base_types(self._reflectables)
        return InMemoryReflectableDB(self._reflectables)

    def add_from_file(self, component: str, file_name: Path, *, include_base: Path | None = None) -> ReflectableDB:
        added = Reflectables()
        self._add_from_file(component, file_name, added, include_base=include_base)
        return InMemoryReflectableDB(added)

    def add_from_files(self, component: str, file_names: Iterable[Path], *, include_base: Path | None = None) -> ReflectableDB:
        added = Reflectables()
        for file_name in file_names:
            self._add_from_file(component, file_name, added, include_base=include_base)
        return InMemoryReflectableDB(added)

    ## acquisition phase methods ##

    def _add_from_file(self, component: str, file_name: Path, added: Reflectables, *, include_base: Path | None) -> None:
        try:
            maybe_added = Reflectables()
            relative_file_path = file_name.relative_to(include_base) if include_base else file_name
            file_ctx = self._collect_module_and_file(component, relative_file_path, maybe_added)

                # return
            print(f'>>>>>>>>>>>>>>>>>>>> {file_name}')

            lines = self._read_code_from_file(file_name)
            visitor = AnnotationParser(lines)
            code = '\n'.join(lines)
            Parser(str(file_name), code, visitor).parse()
            parsed = visitor.data

            self._find_entities_in_namespace(NamespaceContext.root(file_ctx, parsed.namespace), maybe_added)

            added |= maybe_added
            self._reflectables |= maybe_added
        except Exception as e:
            _LOGGER.error(f"reflection parsing failed for file {file_name}: {e}")

    def _collect_module_and_file(self, component: str, file_name: Path, added: Reflectables) -> FileContext:
        file_ctx = FileContext(component, file_name)
        component_id = file_ctx.component_ref()
        self._reflectables.components[component_id] = component

        file_id = file_ctx.ref()
        if file_id in self._reflectables.files:
            if file_id in self._reflectables.files:
                _LOGGER.warning(f'reflection already parsed file {file_name}')

        added.files[file_id] = file_ctx.entry()
        return file_ctx

    def _find_entities_in_namespace(self, namespace_ctx: NamespaceContext, added: Reflectables) -> None:
        self._collect_namespace(namespace_ctx, added)
        self._collect_functions_in_namespace(namespace_ctx, added)
        self._collect_classes_in_namespace(namespace_ctx, added)

        for sub_namespace in namespace_ctx.sub_namespaces():
            self._find_entities_in_namespace(sub_namespace, added)

    def _collect_namespace(self, namespace_ctx: NamespaceContext, added: Reflectables) -> None:
        entry = namespace_ctx.entry()

        namespace_id = namespace_ctx.ref()
        added.namespaces[namespace_id] = entry

    def _collect_functions_in_namespace(self, namespace_ctx: NamespaceContext, added: Reflectables) -> None:
        for func_ctx in namespace_ctx.functions():
            self._collect_function(func_ctx, added)

    def _collect_function(self, function_ctx: FunctionContext, added: Reflectables) -> None:
        func = function_ctx.entry()

        func_id = function_ctx.ref()
        if func_id in added.functions:
            _LOGGER.error(f'duplicate function {function_ctx.full_name} in file {function_ctx.file}')
        if (func_entry := self._reflectables.arguments.get(func_id)) is not None:
            first_seen_in_file = self._reflectables.files[func_entry.files[0]].file
            _LOGGER.error(f'duplicate function {function_ctx.full_name} in database [{first_seen_in_file} and {function_ctx.file}]')

        added.functions[func_id] = func

        for argument in function_ctx.arguments():
            self._collect_argument(argument, added)

    def _collect_argument(self, argument_ctx: ArgumentContext, added: Reflectables) -> None:
        argument = argument_ctx.entry()

        argument_id = argument_ctx.ref()
        if argument_id in added.arguments:
            _LOGGER.error(f'duplicate argument {argument_ctx.full_name} in file {argument_ctx.file}')
        if (argument_entry := self._reflectables.arguments.get(argument_id)) is not None:
            first_seen_in_file = self._reflectables.files[argument_entry.files[0]].file
            _LOGGER.error(f'duplicate argument {argument_ctx.full_name} in database [{first_seen_in_file} and {argument_ctx.file}]')

        added.arguments[argument_id] = argument

    def _collect_classes_in_namespace(self, namespace_ctx: NamespaceContext, added: Reflectables) -> None:
        for class_ctx in namespace_ctx.classes():
            self._collect_class_and_nested_classes(class_ctx, added)

    def _collect_class_and_nested_classes(self, class_ctx: ClassContext, added: Reflectables) -> None:
        cls = class_ctx.entry()

        cls_id = class_ctx.ref()
        if cls_id in added.classes:
            _LOGGER.error(f'duplicate class {class_ctx.full_name} in file {class_ctx.file}')
        if (class_entry := self._reflectables.classes.get(cls_id)) is not None:
            first_seen_in_file = self._reflectables.files[class_entry.files[0]].file
            _LOGGER.error(f'duplicate class {class_ctx.full_name} in database [{first_seen_in_file} and {class_ctx.file}]')

        self._cached_scopes[cls_id] = class_ctx.scope
        added.classes[cls_id] = cls

        for prop in class_ctx.fields():
            self._collect_property(prop, added)

        for nested_cls in class_ctx.classes():
            self._collect_class_and_nested_classes(nested_cls, added)

    def _collect_property(self, property_ctx: FieldContext, added: Reflectables) -> None:
        prop = property_ctx.entry()

        prop_id = property_ctx.ref()
        if prop_id in added.properties:
            _LOGGER.error(f'duplicate property {property_ctx.full_name} in file {property_ctx.file}')
        if (prop_entry := self._reflectables.properties.get(prop_id)) is not None:
            first_seen_in_file = self._reflectables.files[prop_entry.files[0]].file
            _LOGGER.error(f'duplicate property {property_ctx.full_name} in database [{first_seen_in_file} and {property_ctx.file}]')

        self._cached_scopes[prop_id] = property_ctx.scope
        added.properties[prop_id] = prop

    def _amend_property_types(self, reflectables: Reflectables) -> None:
        for prop_id, prop in reflectables.properties.items():
            if isinstance(prop.type, UnknownType):
                prop.type = self._find_property_type(prop_id, prop.type)

    def _find_property_type(self, prop_id: int, prop_type: UnknownType) -> UnknownType | int:
        field_scope = self._cached_scopes[prop_id]

        if is_anon_struct_field(prop_type):
            field_type_scope = field_scope.parent.add_class(prop_type.type)
            field_type_id = field_type_scope.hash()
            if field_type_id not in self._reflectables.classes:
                _LOGGER.error(f'missing anonymous type {field_type_scope}')
                return prop_type
            return field_type_id
        else:
            prop_cls_id = self._find_class_from_scope(field_scope, prop_type)
            return  prop_cls_id if prop_cls_id is not None else prop_type

    def _amend_base_types(self, reflectables: Reflectables) -> None:
        for cls_id, cls in reflectables.classes.items():
            cls.bases = [self._find_base_type(cls_id, base_id) for base_id in cls.bases]

    def _find_base_type(self, child_id: int, base_id: UnknownType | int) -> str | int:
        if isinstance(base_id, int):
            return base_id
        child_scope = self._cached_scopes[child_id]
        base_cls_id = self._find_class_from_scope(child_scope, base_id)
        return base_cls_id if base_cls_id is not None else base_id

    def _find_class_from_scope(self, scope: ScopePath, _type: UnknownType) -> int | None:
        for cls_id, cls in self._reflectables.classes.items():
            cls_path = self._cached_scopes[cls_id].path
            pos = scope
            while (pos := pos.parent) is not None:
                combined_scope = f'{pos.path}::{_type.type}'
                if combined_scope == cls_path:
                    return cls_id
        return None

    @staticmethod
    def _read_code_from_file(header_path: Path) -> list[str]:
        ignore_starting_with = ['#', 'TBD_NEW_ERR']

        code = []
        with open(header_path) as f:
            backslashed = False
            for line in f:
                stripped_line = line.strip()
                next_backslashed = stripped_line.endswith('\\')
                if backslashed or any(stripped_line.startswith(start) for start in ignore_starting_with):
                    backslashed = next_backslashed
                    code.append('')
                    continue

                code.append(line.rstrip())
        return code


__all__ = ['ReflectableFinder']
