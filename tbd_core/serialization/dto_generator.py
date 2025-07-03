import io
import logging
from pathlib import Path
from typing import OrderedDict, Final

import proto_schema_parser.ast as proto
import proto_schema_parser.generator as protog

from tbd_core.reflection.db import ReflectableDB, ClassPtr
from tbd_core.reflection.reflectables import ALL_PARAM_TYPES, PARAM_TO_PROTO, Param
from tbd_core.generators import generate_protos

from .cpp_generator import CppGenerator


_LOGGER = logging.getLogger(__name__)


class DTOGenerator:
    def __init__(self, reflectable_db: ReflectableDB):
        self._reflectable_db: Final[ReflectableDB] = reflectable_db
        self._serializables: Final[OrderedDict[int, proto.Message]] = OrderedDict()

    def add_serializable(self, cls: ClassPtr):
        elements = []

        for field_number, prop in enumerate(cls.properties):
            prop_type = prop.type
            match prop_type:
                case ClassPtr():
                    self.add_serializable(prop_type)
                    elements.append(proto.Field(
                        name=prop.name,
                        type=prop_type.name,
                        number=field_number + 1,
                    ))
                case Param():
                    elements.append(proto.Field(
                        name=prop.field_name,
                        type=prop_type.param_type,
                        number=field_number + 1,
                    ))
                case str():
                    pass
                case _:
                    raise ValueError(f'invalid type ID {type(prop_type)}, must be class or str')

        message = proto.Message(
            name=cls.name,
            elements=elements,
        )
        self._serializables[cls.ref()] = message

    def write_proto(self, out_stream: io.TextIOBase) -> None:
        messages = protog.Generator().generate(proto.File(
            syntax='proto3',
            file_elements=list(self._serializables.values()),
        ))
        out_stream.writelines(messages)

    @staticmethod
    def write_cpp_dtos(out_dir: Path, proto_file_name: Path) -> None:
        generate_protos(out_dir, out_dir, [proto_file_name.name])

    def write_cpp_code(self, out_dir: Path, serializables: list[int]) -> None:
        gen = CppGenerator(self._reflectable_db)

        serializables = [self._reflectable_db.get_class(type_id) for type_id in serializables]
        source = gen.render('serializables.cpp.j2', serializables=serializables)

        source_file = out_dir / 'serializables.cpp'
        with open(source_file, 'w') as f:
            f.write(source)

        source = gen.render('serialized.hpp.j2', serializables=serializables)

        source_file = out_dir / 'serialized.hpp'
        with open(source_file, 'w') as f:
            f.write(source)

__all__ = ['DTOGenerator']