import io
import logging
from dataclasses import dataclass, field
from pathlib import Path
from typing import OrderedDict

import humps
import proto_schema_parser.ast as proto

from tbd_core.reflection.db import ReflectableDB, ClassPtr, CppTypePtr
from tbd_core.reflection.reflectables import Param, ParamType, ClassID, ScopePath, PARAM_TO_PROTO
from tbd_core.generators import generate_protos


_LOGGER = logging.getLogger(__name__)


PARAM_MESSAGE_POSTFIX = 'Wrapper'
DTO_POSTFIX = 'Dto'


def is_param_wrapper(cls: ClassPtr) -> bool:
    if cls.attrs is None:
        return False
    return 'tbd::wrapper' in cls.attrs


@dataclass
class Serializables:
    classes: list[ClassID] = field(default_factory=list)
    class_dtos: OrderedDict[ClassID, ClassID] = field(default_factory=OrderedDict)
    params: OrderedDict[ParamType, ClassID] = field(default_factory=OrderedDict)
    messages: OrderedDict[ClassID, proto.Message] = field(default_factory=OrderedDict)


class DTORegistry:
    def __init__(self, reflectables: ReflectableDB):
        self._reflectables: ReflectableDB = reflectables
        self._serializables: dict[str, Serializables] = {}

    @property
    def reflectables(self) -> ReflectableDB:
        return self._reflectables

    def get_dtos(self) -> dict[str, Serializables]:
        return self._serializables

    def make_type_serializable(self, domain: str, _type: CppTypePtr) -> ClassPtr:
        """ Add serialization implementations for class.

            Only public fields get serialized.
        """

        serializables = self._get_domain(domain)

        match _type:
            case ClassPtr():
                serializables.classes.append(_type.ref())
                self._add_class_messages(domain, _type)
                return _type
            case Param():
                wrapper = self._find_param_wrapper(_type)
                serializables.params[_type.param_type] = wrapper.ref()
                self._add_class_messages(domain, wrapper)
                return wrapper
            case _:
                raise ValueError(f'type {_type} can not be made serializable')


    def create_dto_for_class(self, domain: str, name: str, _type: ClassPtr) -> ClassPtr:
        """ Create a DTO for class.

            DTO will be a class containing only public fields of original class.
        """

        serializables = self._get_domain(domain)

        properties = OrderedDict((prop.field_name, prop.type) for prop in _type.all_properties)
        dto = self.create_dto_from_field_list(domain, name, properties)
        serializables.class_dtos[_type.ref()] = dto.ref()
        return dto

    def create_dto_from_field_list(self, domain: str, name: str, types: OrderedDict[str, CppTypePtr]) -> ClassPtr:
        """ Create a DTO from a list of fields.

            This will create a DTO from scratch with no associated class.
        """

        file_path = Path('tbd', 'dtos', domain, 'dtos.hpp')
        namespace_scope = ScopePath.root().add_namespace('tbd').add_namespace(domain).add_namespace('dtos')
        cls_scope = namespace_scope.add_class(name)

        new_properties = OrderedDict()
        for prop_name, prop_type in types.items():
            match prop_type:
                case Param():
                    pass
                case ClassPtr():
                    prop_type = prop_type.ref()
                case _:
                    _LOGGER.warning(f'skipping unknown type {prop_type} for field {prop_name}')
            new_properties[prop_name] = prop_type


        cls = self._reflectables.add_class(
            cls=cls_scope,
            properties=new_properties,
            component=domain,
            files=[str(file_path)],
            bases=None,
        )
        self._reflectables.add_namespace(namespace_scope)
        self._add_class_messages(domain, cls)
        return cls

    def _get_domain(self, domain: str) -> Serializables:
        if domain not in self._serializables:
            self._serializables[domain] = Serializables()
        return self._serializables[domain]

    def _find_param_wrapper(self, param: Param) -> ClassPtr:
        class_name = humps.pascalize(param.cls_name) + PARAM_MESSAGE_POSTFIX
        for cls in self._reflectables.classes():
            if cls.name == class_name:
                return cls
        raise ValueError(f'no DTO wrapper {class_name} for parameter type {param.typename} present')

    def _add_class_messages(self, domain: str, cls: ClassPtr):
        serializables = self._get_domain(domain)
        if cls.ref() in serializables.messages:
            return

        fields = []
        field_number = 1
        for prop in cls.all_properties:
            prop_type = prop.type
            match prop_type:
                case ClassPtr():
                    self._add_class_messages(domain, prop_type)
                    fields.append(proto.Field(name=prop.field_name, number=field_number, type=prop.type.cls_name))
                case Param():
                    fields.append(proto.Field(name=prop.name, number=field_number, type=PARAM_TO_PROTO[prop_type.param_type]))
                case _:
                    raise RuntimeError(f'type {prop_type} is not a valid protobuffer message field type')
            field_number += 1
        serializables.messages[cls.ref()] = proto.Message(name=cls.cls_name, elements=fields)


__all__ = [
    'is_param_wrapper',
    'Serializables',
    'DTORegistry',
]