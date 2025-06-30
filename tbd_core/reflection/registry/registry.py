import logging
from pathlib import Path
from typing import Iterable

from tbd_core.reflection.parser import AnnotationParser, Parser

from tbd_core.reflection.reflectables import Reflectables, ScopePath, PropertyEntry, ClassEntry
from tbd_core.reflection.db import ReflectableDB, InMemoryReflectableDB

from .contexts import (
    NamespaceContext,
    FunctionContext,
    ClassContext,
    FieldContext,
    ArgumentContext,
    is_anon_struct_field, ref_for_file, FilesContext,
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

    def add_from_file(self, module: str, file_name: Path, *, include_base: Path | None = None) -> ReflectableDB:
        added = Reflectables()
        files_context = self._get_file_context(module, file_name)
        self._add_from_file(module, file_name, added, include_base=include_base)
        return InMemoryReflectableDB(added)

    def add_from_files(self, module: str, file_names: Iterable[Path], *, include_base: Path | None = None) -> ReflectableDB:
        added = Reflectables()
        for file_name in file_names:
            added = Reflectables()
            files_context = self._get_files_context(module, file_name)
            self._add_from_file(module, file_name, added, include_base=include_base)
        return InMemoryReflectableDB(added)

    ## acquisition phase methods ##

    def _get_files_context(self, module: str, file_name: Path) -> FilesContext:
        pass

    def _add_from_file(self, module: str, file_name: Path, added: Reflectables, *, include_base: Path | None) -> None:
        try:
            relative_file_path = file_name.relative_to(include_base) if include_base else file_name
            file_ref = ref_for_file(relative_file_path)
            if file_ref in self._reflectables.headers:
                _LOGGER.warning(f'reflection already parsed header {file_name}')
                # return
            print(f'>>>>>>>>>>>>>>>>>>>> {file_name}')

            lines = self._read_code_from_file(file_name)
            visitor = AnnotationParser(lines)
            code = '\n'.join(lines)
            Parser(str(file_name), code, visitor).parse()
            parsed = visitor.data

            self._find_entities_in_namespace(NamespaceContext.root(relative_file_path, parsed.namespace), added)
            self._reflectables |= added
            self._reflectables.headers[file_ref] = relative_file_path
        except Exception as e:
            _LOGGER.error(f"reflection parsing failed for file {file_name}: {e}")

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
            first_seen_in_file = self._reflectables.headers[func_entry.header]
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
            first_seen_in_file = self._reflectables.headers[argument_entry.header]
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
            first_seen_in_file = self._reflectables.headers[class_entry.header]
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
            first_seen_in_file = self._reflectables.headers[prop_entry.header]
            _LOGGER.error(f'duplicate property {property_ctx.full_name} in database [{first_seen_in_file} and {property_ctx.file}]')

        self._cached_scopes[prop_id] = property_ctx.scope
        added.properties[prop_id] = prop

    def _amend_property_types(self, reflectables: Reflectables) -> None:
        for prop_id, prop in reflectables.properties.items():
            if isinstance(prop.type, str):
                prop.type = self._find_property_type(prop_id, prop)

    def _find_property_type(self, prop_id, prop: PropertyEntry) -> str | int:
        field_scope = self._cached_scopes[prop_id]
        prop_type = prop.type

        if is_anon_struct_field(prop_type):
            field_type_scope = field_scope.parent.add_class(prop_type)
            field_type_id = field_type_scope.hash()
            if field_type_id not in self._reflectables.classes:
                _LOGGER.error(f'missing anonymous type {field_type_scope}')
                return prop_type
            return field_type_id
        else:
            # print('==============')
            for cls_id, cls in self._reflectables.classes.items():
                cls_path = self._cached_scopes[cls_id].path
                pos = field_scope
                while (pos := pos.parent) is not None:
                    combined_scope = f'{pos.path}::{prop_type}'
                    if combined_scope == cls_path:
                        # print('field found:', field_scope.path, prop_type, cls_path)
                        return cls_id
        # print('field not found:', field_scope.path, prop_type)
        return prop_type

    def _amend_base_types(self, reflectables: Reflectables) -> None:
        for cls_id, cls in reflectables.classes.items():
            cls.bases = [self._find_base_type(cls_id, base_id) for base_id in cls.bases]

    def _find_base_type(self, child_id: int, base_id: str | int) -> str | int:
        if isinstance(base_id, int):
            return base_id

        child_scope = self._cached_scopes[child_id]
        # print('==============')
        for cls_id, cls in self._reflectables.classes.items():
            cls_path = self._cached_scopes[cls_id].path
            pos = child_scope
            while (pos := pos.parent) is not None:
                combined_scope = f'{pos.path}::{base_id}'
                if combined_scope == cls_path:
                    # print('base found:', child_scope.path, base_id, cls_path)
                    return cls_id
        # print('base not found:', child_scope.path, base_id)
        return base_id

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
