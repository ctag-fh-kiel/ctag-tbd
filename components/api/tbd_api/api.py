from collections import OrderedDict
from typing import Final

from .enpoints import Endpoint
from .dtos import Payload, Request, Response, MultiArgRequest


class Api:
    def __init__(self,
        endpoints: list[Endpoint],
        payload_types: OrderedDict[str, Payload],
        request_types: OrderedDict[str, Request],
        response_types: OrderedDict[str, Response],
    ):
        self._endpoints: Final = endpoints
        self._payload_types: Final = payload_types
        self._request_types: Final = request_types
        self._response_types: Final = response_types

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

    def get_endpoint_id(self, endpoint_name: str) -> int:
        try:
            return next(index for index, endpoint in enumerate(self._endpoints) if endpoint.name == endpoint_name)
        except StopIteration:
            raise ValueError(f'unknown endpoint {endpoint_name}')

    def get_payload(self, payload_name: str) -> Payload:
        return self._payload_types[payload_name]

    def get_payload_id(self, payload_name: str) -> int:
        try:
            return next(index for index, name in enumerate(self._payload_types) if name == payload_name)
        except StopIteration:
            raise ValueError(f'unknown payload type {payload_name}')

    def get_request(self, endpoint: Endpoint) -> Request:
        return self._request_types[endpoint.request_type]

    def get_request_id(self, endpoint: Endpoint) -> int:
        try:
            return next(index for index, request_name in enumerate(self._request_types.keys()) if request_name == endpoint.request_type)
        except StopIteration:
            raise ValueError(f'no response type {endpoint.request_type} for endpoint {endpoint.name}')

    def get_response(self, endpoint: Endpoint) -> Response:
        return self._response_types[endpoint.response_type]

    def get_response_id(self, endpoint: Endpoint) -> int:
        try:
            return next(index for index, response_name in enumerate(self._response_types) if response_name == endpoint.response_type)
        except StopIteration:
            raise ValueError(f'no response type {endpoint.response_type} for endpoint {endpoint.name}')


__all__ = ['Api']