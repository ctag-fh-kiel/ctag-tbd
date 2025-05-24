from dataclasses import dataclass
from pathlib import Path
import logging
from collections import OrderedDict


import proto_schema_parser.parser as protop
import proto_schema_parser.ast as proto

import esphome.components.tbd_reflection as tbr

from .dtos import (
    ParamPayload,
    Payload,
    empty_request,
    Request,
    empty_response,
    Response,
    response_for_output,
    AnyMessageDto,
    dto_from_message,
    request_for_single_arg, MessagePayload, request_for_arguments, SingleArgRequest, MultiArgRequest
)
from .enpoints import Endpoint, endpoint_from_function
from .base_endpoints import BaseEndpoints
from .api import Api

_LOGGER = logging.getLogger(__file__)


def guaranteed_payload_types() -> OrderedDict[str, Payload]:
    return OrderedDict((message.name, message)
        for message in ParamPayload.param_messages())


def guaranteed_request_types() -> OrderedDict[str, Request]:
    return OrderedDict({empty_request.name: empty_request})


def guaranteed_response_types() -> OrderedDict[str, Response]:
    return OrderedDict({empty_response.name: empty_response})


class ApiRegistry:
    def __init__(self):
        self._base_endpoints: list[Endpoint | None] = [None] * BaseEndpoints.num_reserved_ids()
        self._endpoints: list[Endpoint] = []
        self._payload_types: OrderedDict[str, Payload] = guaranteed_payload_types()
        self._shared_request_types: OrderedDict[str, Request] = guaranteed_request_types()
        self._endpoint_request_types: OrderedDict[str, MultiArgRequest] = OrderedDict()
        self._response_types: OrderedDict[str, Response] = guaranteed_response_types()

    def get_api(self) -> Api:
        shared_requests = OrderedDict((request.name, request) for request in self._shared_request_types.values())
        endpoint_requests = OrderedDict((request.name, request) for request in self._endpoint_request_types.values())
        responses = OrderedDict((response.name, response) for response in self._response_types.values())

        return Api(
            endpoints=self._endpoints,
            payload_types=self._payload_types,
            request_types= shared_requests | endpoint_requests,
            response_types=responses,
        )

    @property
    def endpoints(self) -> list[Endpoint]:
        return self._endpoints

    @property
    def payload_types(self) -> list[Payload]:
        return [payload for payload in self._payload_types.values()]

    @property
    def payload_names(self) -> list[str]:
        return [message_name for message_name in self._payload_types]

    def add_message_types(self, proto_path: Path | str) -> None:
        """ Find all message types in proto file and add them as payloads to registry.
        
            :note: Message name postfixes `_request` and `_response` are reserved for internal use.
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

    def _find_payload(self, payload_name: str) -> Payload:
        return self._payload_types[payload_name]

    def _add_message(self, dto: AnyMessageDto) -> None:
        match dto:
            case MessagePayload() as payload:
                payload_name = payload.name
                if payload_name in self._payload_types:
                    raise ValueError(f'duplicate message payload type {payload_name}')
                self._payload_types[payload_name] = payload
            case _:
                raise ValueError(f'can not add request class {type(dto)}')

    def _add_endpoint(self, func: tbr.FunctionDescription) -> None:
        """ Add a C++ function annotated as endpoint to endpoints.

            This will analyze the function signature and generate DTOs for inputs and output, if no predefined
            DTOs are found in `.proto` files.
        """

        if not (endpoint := endpoint_from_function(func)):
            return

        endpoint_name = endpoint.name
        args = endpoint.args

        if args:
            request = self._add_endpoint_args(endpoint_name, args)
        else:
            request = empty_request

        output = endpoint.output
        if output:
            response = self._add_response_type(output)
        else:
            response = empty_response

        endpoint.request_type = request.name
        endpoint.response_type = response.name

        # check if the endpoint has guaranteed id
        if endpoint_name in BaseEndpoints:
            endpoint_id = BaseEndpoints[endpoint_name]
            self._base_endpoints[endpoint_id] = endpoint
        else:
            self._endpoints.append(endpoint)

    def _add_endpoint_args(self, endpoint_name: str, args: OrderedDict[str, str]) -> Request:
        if len(args) == 1:
            request = self._add_single_arg_type(next(iter(args.values())))
        else:
            request = self._add_multi_arg_type(endpoint_name, args)
        return request

    def _add_single_arg_type(self, output_name: str) -> Request:
        """ Add reusable request for a specific single argument type.

            Request type is only created if no request type for argument type exists. Unlike multi argument request
            call signatures these request types will be shared across multiple endpoints to avoid redundancy.
        """

        in_message = self._find_payload(output_name)
        request = self._shared_request_types.get(output_name)
        if not request:
            request = request_for_single_arg(in_message)
            self._shared_request_types[output_name] = request
        return request

    def _add_multi_arg_type(self, endpoint_name: str, args: OrderedDict[str, str]) -> Request:
        arg_messages = OrderedDict()
        for arg_name, arg_type_name in args.items():
            in_message = self._find_payload(arg_type_name)
            arg_messages[arg_name] = in_message
        request = request_for_arguments(endpoint_name, arg_messages)
        if endpoint_name in self._shared_request_types:
            raise RuntimeError(f'request type for endpoint {endpoint_name} already exists')
        self._endpoint_request_types[endpoint_name] = request
        return request

    def _add_response_type(self, output_name: str) -> Response:
        """ Add reusable request for a specific response type.

            Response type is only created if no response for payload type exists.
        """
        out_message = self._find_payload(output_name)
        response = self._response_types.get(output_name)
        if not response:
            response = response_for_output(out_message)
            self._response_types[output_name] = response
        return response


__all__ = [
    'ApiRegistry', 
]