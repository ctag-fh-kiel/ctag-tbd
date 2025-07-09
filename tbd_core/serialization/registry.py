import logging
from pathlib import Path
from typing import OrderedDict

import humps
import proto_schema_parser.ast as proto

from tbd_core.reflection.db import ReflectableDB, ClassPtr, CppTypePtr
from tbd_core.reflection.reflectables import Param, ScopePath, PARAM_TO_PROTO

from .serializables import Serializables, SerializableClass, ClassDto, GeneratedDto, AnonymousClassDto, Serializable, \
    ParamWrapper

_LOGGER = logging.getLogger(__name__)

PARAM_MESSAGE_POSTFIX = 'Wrapper'
DTO_POSTFIX = 'Dto'


def is_param_wrapper(cls: ClassPtr) -> bool:
    if cls.attrs is None:
        return False
    return 'tbd::wrapper' in cls.attrs


class DTORegistry:
    def __init__(self, reflectables: ReflectableDB):
        self._reflectables: ReflectableDB = reflectables
        self._serializables: dict[str, Serializables] = {}

    @property
    def reflectables(self) -> ReflectableDB:
        return self._reflectables

    def get_dtos(self) -> dict[str, Serializables]:
        return self._serializables

    def make_type_serializable(
            self, domain: str,
            _type: CppTypePtr,
            *,
            recursive: bool = True,
            add_message: bool = True,
    ) -> ClassPtr:
        """ Add direct serialization implementations for class.

            Only public fields get serialized. No extra DTO object is created.

            Parameters:

            :param domain: name of domain to contain serialization implementations
            :param _type: the type to be made serializable
            :param recursive: recursively declare all struct/class field types serializable
            :param add_message: make type serializable as message
        """

        serializables = self._get_domain(domain)
        serializable = self._make_type_serializable(serializables, _type,
                                                    recursive=recursive, exists_ok=False, add_message=add_message)
        return serializable.dto_cls

    def create_dto_for_class(
            self,
            domain: str,
            name: str,
            _type: ClassPtr,
            add_message: bool = True,
            recursive: bool = True,
    ) -> ClassPtr:
        """ Create a DTO for class.

            DTO will be a class containing only public fields of original class.

            WARNING: Fields that are themselves classes or structs, will be assumed to be directly serializable.

            Parameters:
            :param domain: name of domain to contain serialization implementations
            :param name: name of DTO class
            :param _type: the type for which to create a DTO
            :param recursive: recursively declare all struct/class field types serializable
            :param add_message: make type serializable as message
        """

        serializables = self._get_domain(domain)

        properties = OrderedDict((prop.field_name, prop.type) for prop in _type.all_properties)
        generated_dto = self._create_dto(serializables, name, properties, add_message=add_message, recursive=recursive)
        class_dto = ClassDto(cls=_type, dto_cls=generated_dto.cls, message=generated_dto.message)
        self._add_serializable(serializables, class_dto)
        return class_dto.dto_cls

    def create_dto_from_field_list(
            self, domain: str,
            name: str,
            types: OrderedDict[str, CppTypePtr],
            add_message: bool = True,
            recursive: bool = True,
    ) -> ClassPtr:
        """ Create a DTO from a list of fields.

            This will create a DTO from scratch with no associated class. All struct field types will be recursively
            receive associated DTOs.
        """

        serializables = self._get_domain(domain)
        generated_dto = self._create_dto(serializables, name, types, add_message=add_message, recursive=recursive)
        self._add_serializable(serializables, generated_dto)
        return generated_dto.dto_cls

    # private

    def _get_domain(self, domain: str) -> Serializables:
        if domain not in self._serializables:
            self._serializables[domain] = Serializables(domain=domain)
        return self._serializables[domain]

    @staticmethod
    def _add_serializable(serialzables: Serializables, serializable: Serializable, *, exists_ok=False) -> None:
        serializable_id = serializable.ref()
        if serializable_id in serialzables:
            existing = serialzables[serializable_id]
            if type(serializable) != type(existing):
                raise RuntimeError(
                    f'serializable for {serializable.cls.full_name} redeclared with different serialization type')
            return
        print(f'added serializable for {serializable.cls.full_name}')
        serialzables[serializable.ref()] = serializable

    def _make_type_serializable(
            self, serializables: Serializables,
            _type: CppTypePtr,
            *,
            add_message: bool,
            recursive: bool,
            exists_ok: bool,
    ) -> SerializableClass | ParamWrapper:

        match _type:
            case ClassPtr():

                if recursive:
                    for prop in _type.properties:
                        prop_type = prop.type
                        if isinstance(prop_type, ClassPtr):
                            self._make_type_serializable(serializables, prop_type,
                                                         add_message=add_message, recursive=True, exists_ok=True)

                if _type.is_anonymous:
                    serializable = self._create_anonymous_serializable(serializables, _type,
                                                                       add_message=add_message)
                else:
                    message = self._create_message(_type) if add_message else None
                    serializable = SerializableClass(cls=_type, message=message)
            case Param():
                # ensure wrapper type for this parameter type is present
                serializable = self._find_param_wrapper(_type)
            case _:
                raise ValueError(f'type {_type} can not be made serializable')

        self._add_serializable(serializables, serializable)
        return serializable

    def _create_dto(
            self,
            serializables: Serializables,
            name: str,
            types: OrderedDict[str, CppTypePtr],
            *,
            add_message: bool,
            recursive: bool,
    ) -> GeneratedDto:

        domain = serializables.domain
        file_path = Path('tbd', domain, 'dtos', 'dtos.hpp')
        namespace_scope = ScopePath.root().add_namespace('tbd').add_namespace(domain).add_namespace('dtos')
        cls_scope = namespace_scope.add_class(name)

        new_properties = OrderedDict()
        for prop_name, prop_type in types.items():
            match prop_type:
                case Param():
                    pass
                case ClassPtr():
                    if recursive:
                        self._make_type_serializable(serializables, prop_type,
                                                     add_message=add_message, recursive=True, exists_ok=True)
                    prop_type = prop_type.ref()
                case _:
                    _LOGGER.warning(f'skipping unknown type {prop_type} for field {prop_name}')
            new_properties[prop_name] = prop_type

        self._reflectables.add_namespace(namespace_scope)
        generated_dto = self._reflectables.add_class(
            cls=cls_scope,
            properties=new_properties,
            component=domain,
            files=[str(file_path)],
            bases=None,
        )
        return GeneratedDto(dto_cls=generated_dto, message=self._create_message(generated_dto))

    def _create_anonymous_serializable(
            self,
            serializables: Serializables,
            _type: ClassPtr,
            *,
            add_message: bool,
    ) -> AnonymousClassDto:

        properties = OrderedDict((prop.field_name, prop.type)
                                 for prop in _type.all_properties)
        dto_name = _type.parent.cls_name + _type.cls_name
        generated_dto = self._create_dto(serializables, dto_name, properties, add_message=add_message, recursive=True)

        anon_cls_dto = AnonymousClassDto(
            anon_cls=_type,
            dto_cls=generated_dto.dto_cls,
            message=self._create_message(generated_dto.dto_cls)
        )
        return anon_cls_dto

    def _find_param_wrapper(self, param: Param) -> ParamWrapper:
        class_name = humps.pascalize(param.cls_name) + PARAM_MESSAGE_POSTFIX
        for cls in self._reflectables.classes():
            if cls.name == class_name:
                return ParamWrapper(param_type=param.param_type, wrapper_cls=cls, message=self._create_message(cls))
        raise ValueError(f'no DTO wrapper {class_name} for parameter type {param.typename} present')

    @staticmethod
    def _create_message(cls: ClassPtr) -> proto.Message:
        fields = []
        field_number = 1
        for prop in cls.all_properties:
            prop_type = prop.type
            match prop_type:
                case ClassPtr():
                    fields.append(proto.Field(name=prop.field_name, number=field_number, type=prop.type.cls_name))
                case Param():
                    fields.append(
                        proto.Field(name=prop.name, number=field_number, type=PARAM_TO_PROTO[prop_type.param_type]))
                case _:
                    raise RuntimeError(f'type {prop_type} is not a valid protobuffer message field type')
            field_number += 1
        return proto.Message(name=cls.cls_name, elements=fields)


__all__ = [
    'is_param_wrapper',
    'DTORegistry',
]
