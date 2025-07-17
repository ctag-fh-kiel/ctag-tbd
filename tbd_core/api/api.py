from collections import OrderedDict
from typing import Final
from zlib import crc32

from tbd_core.reflection.reflectables import FunctionID
from tbd_core.reflection.db import ClassPtr

from .base_endpoints import BaseEndpoints
from .idc_interfaces import Endpoint, Event, Responder, EventSink


class Api:
    def __init__(self,
        endpoints: list[Endpoint],
        endpoint_requests: OrderedDict[FunctionID, ClassPtr],
        endpoint_responses: OrderedDict[FunctionID, ClassPtr],
        events: list[Event],
        event_payloads: OrderedDict[FunctionID, ClassPtr],
        event_responders: dict[str, list[Responder]],
        event_sinks: list[EventSink],
    ):
        self._endpoints: Final = endpoints

        self._endpoint_requests: Final = endpoint_requests
        self._endpoint_responses: Final = endpoint_responses

        self._events: Final = events
        self._event_payloads: Final = event_payloads
        self._event_responders: Final = event_responders
        self._event_sinks: Final = event_sinks

    @property
    def endpoints(self) -> list[Endpoint]:
        return self._endpoints

    @property
    def endpoint_requests(self) -> list[ClassPtr]:
        return [*self._endpoint_requests.values()]

    @property
    def endpoint_responses(self) -> list[ClassPtr]:
        return [*self._endpoint_responses.values()]

    @property
    def empty_request(self):
        return next(iter(self._endpoint_requests.values()))

    @property
    def payload_names(self):
        return [message_name for message_name in self._event_payloads]

    @property
    def events(self) -> list[Event]:
        return self._events

    @property
    def event_payloads(self) -> list[ClassPtr]:
        return [*self._event_payloads.values()]

    @property
    def event_sinks(self) -> list[EventSink]:
        return self._event_sinks

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

    def get_endpoint_id(self, endpoint: Endpoint) -> int:
        try:
            return next(index for index, _endpoint in enumerate(self._endpoints) if _endpoint.ref() == endpoint.ref())
        except StopIteration:
            raise ValueError(f'unknown endpoint {endpoint}')

    def get_event_id(self, event: Event) -> int:
        try:
            return next(index for index, event in enumerate(self._events) if event.ref() == event.ref())
        except StopIteration:
            raise ValueError(f'unknown event {event}')

    def get_responders(self, event: Event) -> list[Responder] | None:
        return self._event_responders.get(event.event_name)

    def get_request(self, endpoint: Endpoint) -> ClassPtr | None:
        if not endpoint.has_inputs:
            return None
        return self._endpoint_requests[endpoint.ref()]

    def get_request_id(self, endpoint: Endpoint) -> int:
        try:
            return next(index for index, endpoint_id in enumerate(self._endpoint_requests) if endpoint_id == endpoint.ref())
        except StopIteration:
            return -1

    def get_response(self, endpoint: Endpoint) -> ClassPtr | None:
        if not endpoint.has_output:
            return None
        return self._endpoint_responses[endpoint.ref()]

    def get_response_id(self, endpoint: Endpoint) -> int:
        try:
            return next(index for index, endpoint_id in enumerate(self._endpoint_responses) if endpoint_id == endpoint.ref())
        except StopIteration:
            return -1

    def get_payload(self, event: Event) -> ClassPtr | None:
        if not event.has_inputs:
            return None
        return self._event_payloads[event.ref()]

    def get_payload_id(self, event: Event) -> int:
        try:
            return next(index for index, event_id in enumerate(self._event_payloads) if event_id == event.ref())
        except StopIteration:
            return -1




__all__ = ['Api']