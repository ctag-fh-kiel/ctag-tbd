from collections import OrderedDict
from typing import Final
from zlib import crc32

from .base_endpoints import BaseEndpoints
from .idc_interfaces import Endpoint, Event, Responder
from .dtos import Payload, Request, Response


class Api:
    def __init__(self,
        endpoints: list[Endpoint],
        payload_types: OrderedDict[str, Payload],
        request_types: OrderedDict[str, Request],
        response_types: OrderedDict[str, Response],
        events: list[Event],
        event_payloads: OrderedDict[str, Request],
        event_responders: dict[str, list[Responder]],
    ):
        self._endpoints: Final = endpoints
        self._events: Final = events
        self._payload_types: Final = payload_types
        self._request_types: Final = request_types
        self._response_types: Final = response_types
        self._event_payloads: Final = event_payloads
        self._event_responders: Final = event_responders

    @property
    def endpoints(self) -> list[Endpoint]:
        return self._endpoints

    @property
    def payload_types(self) -> list[Payload]:
        return [*self._payload_types.values()]

    @property
    def request_types(self) -> list[Request]:
        return [*self._request_types.values()]

    @property
    def response_types(self) -> list[Response]:
        return [*self._response_types.values()]

    @property
    def empty_request(self):
        return next(iter(self._request_types.values()))

    @property
    def payload_names(self):
        return [message_name for message_name in self._payload_types]

    @property
    def events(self) -> list[Event]:
        return self._events

    @property
    def event_payloads(self) -> list[Request]:
        return [*self._event_payloads.values()]

    @staticmethod
    def get_version():
        return 1

    def calculate_core_hash(self) -> int:
        signatures = []
        for endpoint in self._endpoints[:BaseEndpoints.num_critical_ids()]:
            signatures.append(endpoint.signature())
        data = '\n'.join(signatures)
        return crc32(data.encode())

    def calculate_base_hash(self) -> int:
        signatures = []
        for endpoint in self._endpoints[:BaseEndpoints.num_reserved_ids()]:
            signatures.append(endpoint.signature())
        data = '\n'.join(signatures)
        return crc32(data.encode())

    def calculate_hash(self) -> int:
        signatures = []
        for endpoint in self._endpoints:
            signatures.append(endpoint.signature())
        for event in self._events:
            signatures.append(event.signature())
        data = '\n'.join(signatures)
        return crc32(data.encode())


    def get_endpoint_id(self, endpoint_name: str) -> int:
        try:
            return next(index for index, endpoint in enumerate(self._endpoints) if endpoint.name == endpoint_name)
        except StopIteration:
            raise ValueError(f'unknown endpoint {endpoint_name}')

    def get_event_id(self, event_name: str) -> int:
        try:
            return next(index for index, event in enumerate(self._events) if event.name == event_name)
        except StopIteration:
            raise ValueError(f'unknown event {event_name}')

    def get_responders(self, event_name: str) -> list[Responder] | None:
        return self._event_responders.get(event_name)

    def get_payload(self, payload_name: str) -> Payload:
        return self._payload_types[payload_name]

    def get_payload_id(self, payload_name: str) -> int:
        try:
            return next(index for index, name in enumerate(self._payload_types) if name == payload_name)
        except StopIteration:
            raise ValueError(f'unknown payload type {payload_name}')

    def get_request(self, endpoint: Endpoint) -> Request | None:
        if not endpoint.request_type:
            return None
        return self._request_types[endpoint.request_type]

    def get_request_id(self, endpoint: Endpoint) -> int:
        try:
            return next(index for index, request_name in enumerate(self._request_types.keys()) if request_name == endpoint.request_type)
        except StopIteration:
            raise ValueError(f'no response type {endpoint.request_type} for endpoint {endpoint.name}')

    def get_response(self, endpoint: Endpoint) -> Response | None:
        if not endpoint.response_type:
            return None
        return self._response_types[endpoint.response_type]

    def get_response_id(self, endpoint: Endpoint) -> int:
        try:
            return next(index for index, response_name in enumerate(self._response_types) if response_name == endpoint.response_type)
        except StopIteration:
            raise ValueError(f'no response type {endpoint.response_type} for endpoint {endpoint.name}')


__all__ = ['Api']