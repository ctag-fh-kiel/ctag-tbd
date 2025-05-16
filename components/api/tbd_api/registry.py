from pathlib import Path
import logging
from collections import OrderedDict


import proto_schema_parser.parser as protop
import proto_schema_parser.ast as proto

import esphome.components.tbd_reflection as tbr

from .dtos import (
    MessagePayload, 
    ParamPayload,
    Payload, 
    empty_request,
    Request, 
    request_for_payload,
    empty_response,
    Response, 
    response_for_payload,
    AnyMessageDto, 
    AnyDto,
    dto_from_message 
)
from .enpoints import Endpoint, endpoint_from_function


_LOGGER = logging.getLogger(__file__)


class ApiRegistry:
    def __init__(self):
        self._endpoints: list[Endpoint] = []
        self._payload_types: OrderedDict[str, Payload] = OrderedDict((message.name, message) for message in ParamPayload.scalar_messages())
        self._request_types: OrderedDict[str, Request] = OrderedDict({empty_request.name: empty_request})
        self._response_types: OrderedDict[str, Response] = OrderedDict({empty_response.name: empty_response})
        self._predefined_wrapper_names: set[str] = set()
        self._required_proto_files: set[Path] = set()

        self._cached_request_types: list[Request] | None = None
        self._cached_response_types: list[Response] | None = None

    @property
    def endpoints(self) -> list[Endpoint]:
        return self._endpoints

    @property
    def payload_types(self) -> list[Payload]:
        return [payload for payload in self._payload_types.values()]

    @property
    def payload_names(self) -> list[str]:
        return [message_name for message_name in self._payload_types]
    
    @property
    def proto_files(self) -> set[Path]:
        return self._required_proto_files

    @property
    def request_types(self) -> list[Request]:
        if not self._cached_request_types:
            required_request_names = set(endpoint.in_message for endpoint in self.endpoints if endpoint.in_message)
            self._cached_request_types = [request for request in self._request_types.values() 
                                          if request == empty_request or request.payload in required_request_names]
        return self._cached_request_types
    
    @property
    def response_types(self) -> list[Response]:
        if not self._cached_response_types:
            required_response_names = set(endpoint.out_message for endpoint in self.endpoints if endpoint.out_message)
            self._cached_response_types = [response for response in self._response_types.values() 
                                           if response == empty_response or response.payload in required_response_names]
        return self._cached_response_types
    
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
    
    def get_request(self, payload_name: str) -> Request:
        return self._request_types[payload_name]

    def get_request_id(self, payload_name: str) -> int:
        try:
            return next(index for index, request in enumerate(self.request_types) if request.payload == payload_name)
        except StopIteration:
            raise ValueError(f'no request type for payload type {payload_name}')

    def get_response(self, payload_name: str) -> Response:
        return self._response_types[payload_name]

    def get_response_id(self, payload_name: str) -> int:
        try:
            return next(index for index, response in enumerate(self.response_types) if response.payload == payload_name)
        except StopIteration:
            raise ValueError(f'no response type for payload type {payload_name}')

    def add_message_types(self, proto_path: Path | str):
        """ Find all message descriptions in proto file and add to registry.
        
            This method will determine whether each message is a payload, request, or response.
        """

        proto_path = Path(proto_path)
        if not proto_path.exists():
            raise ValueError(f'invalid message descriptions file/path {proto_path}')
        
        with open(proto_path, 'r') as f:
            data = f.read()

        messages = [message for message in protop.Parser().parse(data).file_elements if isinstance(message, proto.Message)]
        for message in messages:
            self._add_message(dto_from_message(message, proto_path))

    def add_endpoints(self, source: Path | str) -> None:
        """ Find all endpoints in C++ file. 
        
            File can either be source or header file. 

            :note: No public header for endpoints is required, so they can be kept hidden from the rest of the firmware.
        """
        source = Path(source)
        if not source.exists():
            raise ValueError(f'endpoint source {source} does not exist')
        collector = tbr.ReflectableFinder()
        collector.add_from_file(source)
        for func in collector.funcs:
            self._add_endpoint(func)

    # private

    def _add_message(self, dto: AnyMessageDto) -> None:
        match dto:
            case Request() as request:
                payload_name = request.payload
                print(payload_name)
                if payload_name in self._request_types:
                    raise ValueError(f'duplicate request type {request.name}')
                self._request_types[payload_name] = request
                self._cached_request_types = None
            case Response() as response:
                payload_name = response.payload
                if payload_name in self._response_types:
                    raise ValueError(f'duplicate response type {response.name}')
                self._response_types[payload_name] = response
                self._cached_response_types = None
            case MessagePayload() as payload:
                payload_name = payload.name
                if payload_name in self._payload_types:
                    raise ValueError(f'duplicate message payload type {payload_name}')
                self._payload_types[payload_name] = payload
            case _:
                raise ValueError(f'can not add request class {type(dto)}')

    def _add_endpoint(self, func: tbr.FunctionDescription):
        if not (endpoint := endpoint_from_function(func)):
            return
        
        in_message_name = endpoint.in_message
        if in_message_name:
            in_message = self.get_payload(in_message_name)
            if (request_wrapper := self._request_types.get(in_message_name)):
                self._add_proto_file_if_needed(request_wrapper)
            else:
                request_wrapper = self._request_types[in_message_name] = request_for_payload(in_message)
                self._cached_response_types = None

            self._add_proto_file_if_needed(in_message)

        out_message_name = endpoint.out_message
        if out_message_name:
            out_message = self.get_payload(out_message_name)
            if (response_wrapper := self._response_types.get(out_message_name)): 
                self._add_proto_file_if_needed(response_wrapper)
            else:
                response_wrapper = self._response_types[out_message_name] = response_for_payload(out_message)
                self._cached_response_types = None
            
            self._add_proto_file_if_needed(out_message)

        self.endpoints.append(endpoint)
    
    def _add_proto_file_if_needed(self, dto: AnyDto):
        match dto:
            case AnyMessageDto():
                file_path = dto.file_path
            case _:
                return
        if file_path:
            self._required_proto_files.add(file_path)


__all__ = [
    'ApiRegistry', 
]