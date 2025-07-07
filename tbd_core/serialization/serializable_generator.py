from pathlib import Path
from typing import Final, Iterator
from io import TextIOBase

from tbd_core.buildgen import get_tbd_components_root
from tbd_core.generators import GeneratorBase, jilter
from tbd_core.reflection.db import ReflectableDB, ClassPtr, flattened_properties, PropertyPtr

import proto_schema_parser.generator as protog
import proto_schema_parser.ast as proto


from .registry import Serializables


class CppFilters:
    def __init__(self, reflectable_db: ReflectableDB):
        self._reflectable_db: Final[ReflectableDB] = reflectable_db

    @jilter
    def field_name(self, property_id: int) -> str:
        return self._reflectable_db.get_property(property_id).field_name

    @jilter
    def snake_name(self, path: str) -> str:
        return path.replace('.', '_')

    @jilter
    def flattened_properties(self, serializable: ClassPtr) -> Iterator[tuple[str, PropertyPtr]]:
        for path, prop in flattened_properties(serializable):
            yield '.'.join(path), prop


class SerializableGenerator(GeneratorBase[CppFilters]):
    def __init__(self, serializables: Serializables, reflectable_db: ReflectableDB):
        super().__init__(get_tbd_components_root() / 'core' / 'tbd_serialization' / 'src', CppFilters(reflectable_db))
        self._serializables: Serializables = serializables
        self._reflectable_db: ReflectableDB = reflectable_db

    def render(self, template_file: Path | str, **args) -> str:
        return super().render(template_file, reflectables=self._reflectable_db, **args)

    def write_cpp_code(self, headers_dir: Path, srcs_dir: Path) -> None:
        serializables = [self._reflectable_db.get_class(cls_id) for cls_id in self._serializables.classes]
        source = self.render('serializables.cpp.j2', serializables=serializables)

        source_file = srcs_dir / 'serializables.cpp'
        with open(source_file, 'w') as f:
            f.write(source)

        source = self.render('serialized.hpp.j2', serializables=serializables)

        source_file = headers_dir / 'serialized.hpp'
        with open(source_file, 'w') as f:
            f.write(source)

    def write_protos(self, out_file: Path) -> None:
        with out_file.open('w') as f:
            messages = protog.Generator().generate(proto.File(
                syntax='proto3',
                file_elements=list(self._serializables.messages.values()),
            ))
            f.writelines(messages)
