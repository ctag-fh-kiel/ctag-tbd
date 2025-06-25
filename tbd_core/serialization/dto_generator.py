import io
import logging
from typing import OrderedDict, Final

import proto_schema_parser.ast as proto
import proto_schema_parser.generator as protog

from tbd_core.reflection import ClassDescription, Reflectables, ALL_PARAM_TYPES, PARAM_TO_PROTO

_LOGGER = logging.getLogger(__name__)


class DTOGenerator:
    def __init__(self, reflectables: Reflectables):
        self._reflectables: Final = reflectables
        self._serializables: Final[OrderedDict[int, proto.Message]] = OrderedDict()

    def add_serializable(self, cls: ClassDescription):
        elements = []

        for field_number, prop_ref in enumerate(cls.properties):
            prop = self._reflectables.properties[prop_ref]
            prop_type_id = prop.type
            match prop_type_id:
                case int():
                    if prop_type_id < 0:
                        raise ValueError(f'property {prop.full_name} has unknown type')
                    if prop_type_id not in self._reflectables.classes:
                        raise ValueError(f'property {prop.full_name} type {prop_type_id} missing')

                    prop_type = self._reflectables.classes[prop_type_id]
                    self.add_serializable(prop_type)
                    elements.append(proto.Field(
                        name=prop.name,
                        type=prop_type.name,
                        number=field_number + 1,
                    ))
                case str():
                    if prop_type_id not in ALL_PARAM_TYPES:
                        _LOGGER.warning(f'property {prop.full_name} has unserializable type {prop_type_id}')
                        continue
                    param_type = ALL_PARAM_TYPES[prop_type_id]
                    elements.append(proto.Field(
                        name=prop.field_name,
                        type=PARAM_TO_PROTO[param_type],
                        number=field_number + 1,
                    ))

                case _:
                    raise ValueError(f'invalid type ID {type(prop_type_id)}, must be int or str')

        message = proto.Message(
            name=cls.name,
            elements=elements,
        )
        self._serializables[cls.ref()] = message

    def write(self, out_stream: io.TextIOBase) -> None:
        messages = protog.Generator().generate(proto.File(
            syntax='proto3',
            file_elements=list(self._serializables.values()),
        ))
        out_stream.writelines(messages)

__all__ = ['DTOGenerator']