from abc import ABCMeta, abstractmethod
from collections import OrderedDict
from typing import Iterator

import proto_schema_parser.ast as proto

from tbd_core.reflection.db import ClassPtr, FilePtr
from tbd_core.reflection.reflectables import ParamType, ClassID


class SerializableBase(metaclass=ABCMeta):
    def __init__(self, cls: ClassPtr, message: proto.Message, message_size: int):
        self._cls = cls
        self._message = message
        self._message_size = message_size

    @property
    def cls(self) -> ClassPtr:
        return self._cls

    @property
    @abstractmethod
    def dto_cls(self) -> ClassPtr: ...

    @property
    def message(self) -> proto.Message:
        return self._message

    @property
    def message_size(self) -> int:
        return self._message_size

    def ref(self) -> ClassID:
        return self._cls.ref()


class SerializableClass(SerializableBase):
    def __init__(self, cls: ClassPtr, message: proto.Message, message_size: int):
        super().__init__(cls, message, message_size)

    @property
    def dto_cls(self) -> ClassPtr:
        return self._cls

    @property
    def header(self) -> FilePtr:
        return self._cls.header


class ClassDto(SerializableBase):
    def __init__(self, cls: ClassPtr, dto_cls: ClassPtr, message: proto.Message, message_size: int):
        super().__init__(cls, message, message_size)
        self._dto: ClassPtr = dto_cls

    @property
    def dto_cls(self) -> ClassPtr:
        return self._dto

    @property
    def header(self) -> FilePtr:
        return self._cls.header


class GeneratedDto(SerializableBase):
    def __init__(self, dto_cls: ClassPtr, message: proto.Message, message_size: int):
        super().__init__(dto_cls, message, message_size)

    @property
    def dto_cls(self) -> ClassPtr:
        return self._cls


class ParamWrapper(SerializableBase):
    def __init__(self, param_type: ParamType, wrapper_cls: ClassPtr, message: proto.Message, message_size: int):
        super().__init__(wrapper_cls, message, message_size)
        self._param_type = param_type

    @property
    def param_type(self) -> ParamType:
        return self._param_type

    @property
    def dto_cls(self) -> ClassPtr:
        return self._cls


class AnonymousClassDto(SerializableBase):
    def __init__(self, anon_cls: ClassPtr, dto_cls: ClassPtr, message: proto.Message, message_size: int):
        super().__init__(anon_cls, message, message_size)
        self._dto: ClassPtr = dto_cls

    @property
    def dto_cls(self) -> ClassPtr:
        return self._dto


Serializable = SerializableClass | ClassDto | GeneratedDto | ParamWrapper | AnonymousClassDto


class Serializables:
    """ Collected serializable types in a domain.

        A domain is typically a module that needs to serialize the types, not the module declaring the type.
    """

    def __init__(self, domain: str) -> None:
        self._domain = domain
        self._items: OrderedDict[ClassID, Serializable] = OrderedDict()

    @property
    def domain(self) -> str:
        return self._domain

    @property
    def messages(self) -> OrderedDict[ClassID, proto.Message]:
        return OrderedDict((serializable.ref(), serializable.message) for serializable in self._items.values()
                           if serializable.message is not None)

    @property
    def serializable_classes(self) -> list[SerializableClass]:
        return [serializable for serializable in self._items.values() if isinstance(serializable, SerializableClass)]

    @property
    def class_dtos(self) -> list[ClassDto]:
        return [serializable for serializable in self._items.values() if isinstance(serializable, ClassDto)]

    @property
    def generated_dtos(self) -> list[GeneratedDto]:
        return [serializable for serializable in self._items.values() if isinstance(serializable, GeneratedDto)]

    @property
    def anonymous_class_dtos(self) -> list[AnonymousClassDto]:
        return [serializable for serializable in self._items.values() if isinstance(serializable, AnonymousClassDto)]

    def __iter__(self) -> Iterator[Serializable]:
        for serializable in self._items.values():
            yield serializable

    def __getitem__(self, cls_id: ClassID) -> Serializable:
        return self._items[cls_id]

    def __setitem__(self, cls_id: ClassID, serializable: Serializable) -> None:
        self._items[cls_id] = serializable

    def __contains__(self, cls_id: ClassID) -> bool:
        return cls_id in self._items


__all__ = [
    'SerializableClass',
    'ClassDto',
    'GeneratedDto',
    'ParamWrapper',
    'AnonymousClassDto',
    'Serializable',
    'Serializables',
]