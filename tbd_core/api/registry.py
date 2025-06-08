from pathlib import Path
import logging
from collections import OrderedDict


import proto_schema_parser.parser as protop
import proto_schema_parser.ast as proto

import tbd_core.reflection as tbr

from .dtos import (
    ParamPayload,
    Payload,
    Request,
    Response,
    response_for_output,
    AnyMessageDto,
    dto_from_message,
    request_for_single_arg, MessagePayload, request_for_arguments, MultiArgRequest, event_for_single_arg,
    event_for_arguments
)
from .idc_interfaces import Endpoint, idc_from_function, Event, Responder, EventSink
from .base_endpoints import BaseEndpoints
from .api import Api

_LOGGER = logging.getLogger(__file__)


def guaranteed_payload_types() -> OrderedDict[str, Payload]:
    return OrderedDict((message.name, message)
        for message in ParamPayload.param_messages())


class ApiRegistry:
    def __init__(self):
        self._base_endpoints: list[Endpoint | None] = [None] * BaseEndpoints.num_reserved_ids()
        self._endpoints: list[Endpoint] = []
        self._payload_types: OrderedDict[str, Payload] = guaranteed_payload_types()
        self._shared_request_types: OrderedDict[str, Request] = OrderedDict()
        self._endpoint_request_types: OrderedDict[str, MultiArgRequest] = OrderedDict()
        self._response_types: OrderedDict[str, Response] = OrderedDict()

        self._events: list[Event] = []
        self._responders: list[Responder] = []
        self._event_payloads: OrderedDict[str, Request] = OrderedDict()
        self._event_sinks: list[EventSink] = []

    def get_api(self) -> Api:
        shared_requests = OrderedDict((request.name, request) for request in self._shared_request_types.values())
        endpoint_requests = OrderedDict((request.name, request) for request in self._endpoint_request_types.values())
        responses = OrderedDict((response.name, response) for response in self._response_types.values())

        endpoints = [*self._get_guaranteed_endpoints(), *self._get_optional_endpoints()]

        return Api(
            endpoints=endpoints,
            payload_types=self._payload_types,
            request_types= shared_requests | endpoint_requests,
            response_types=responses,
            events=self._events,
            event_payloads=self._event_payloads,
            event_responders=self._get_event_responders(),
            event_sinks=self._event_sinks,
        )

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

    def add_source(self, source: Path | str) -> None:
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
            self._add_idc(func)

    # private

    def _find_payload(self, payload_name: str) -> Payload:
        return self._payload_types[payload_name]

    def _get_guaranteed_endpoints(self) -> list[Endpoint]:
        endpoints: list[Endpoint | None] = [None] * BaseEndpoints.num_reserved_ids()
        endpoint_mapping = BaseEndpoints.id_mapping()

        for endpoint in self._endpoints:
            endpoint_id = endpoint_mapping.get(endpoint.name)
            if endpoint_id is not None:
                endpoints[endpoint_id] = endpoint
        if None in endpoints:
            found_endpoints = set(endpoint.name for endpoint in endpoints)
            required_endpoints = set(BaseEndpoints.names())
            raise RuntimeError(f'missing required endpoints {required_endpoints - found_endpoints}')
        return endpoints

    def _get_optional_endpoints(self) -> list[Endpoint]:
        required_endpoints = set(BaseEndpoints.names())
        return [endpoint for endpoint in self._endpoints if endpoint.name not in required_endpoints]

    def _add_message(self, dto: AnyMessageDto) -> None:
        match dto:
            case MessagePayload() as payload:
                payload_name = payload.name
                if payload_name in self._payload_types:
                    raise ValueError(f'duplicate message payload type {payload_name}')
                self._payload_types[payload_name] = payload
            case _:
                raise ValueError(f'can not add request class {type(dto)}')

    def _add_idc(self, func: tbr.FunctionDescription) -> None:
        if not (idc := idc_from_function(func)):
            return

        match idc:
            case Endpoint():
                self._add_endpoint(idc)
            case Event():
                self._add_event(idc)
            case Responder():
                self._add_responder(idc)
            case EventSink():
                self._add_event_sink(idc)
            case _:
                raise ValueError(f'unknown IDC type {type(idc)}')


    def _add_endpoint(self, endpoint: Endpoint) -> None:
        """ Add a C++ function annotated as `tbd::endpoint` to endpoints.

            This will analyze the function signature and generate DTOs for inputs and output, if no predefined
            DTOs are found in `.proto` files.
        """
        endpoint_name = endpoint.name
        args = endpoint.args
        output = endpoint.output

        endpoint.request_type = self._add_endpoint_args(endpoint_name, args).name if args else None
        endpoint.response_type = self._add_response_type(output).name if output else None

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

    def _add_single_arg_type(self, arg_type: str) -> Request:
        """ Add reusable request for a specific single argument type.

            Request type is only created if no request type for argument type exists. Unlike multi argument request
            call signatures these request types will be shared across multiple endpoints to avoid redundancy.
        """

        request = self._shared_request_types.get(arg_type)
        if not request:
            in_message = self._find_payload(arg_type)
            request = request_for_single_arg(in_message)
            self._shared_request_types[arg_type] = request
        return request

    def _add_multi_arg_type(self, endpoint_name: str, args: OrderedDict[str, str]) -> Request:
        arg_messages = self._find_arg_payloads(args)
        request = request_for_arguments(endpoint_name, arg_messages)
        if endpoint_name in self._shared_request_types:
            raise RuntimeError(f'request type for endpoint {endpoint_name} already exists')
        self._endpoint_request_types[endpoint_name] = request
        return request

    def _add_response_type(self, output_name: str) -> Response:
        """ Add reusable request for a specific response type.

            Response type is only created if no response for payload type exists.
        """
        response = self._response_types.get(output_name)
        if not response:
            out_message = self._find_payload(output_name)
            response = response_for_output(out_message)
            self._response_types[output_name] = response
        return response

    def _add_event(self, event: Event) -> None:
        """ Add a C++ function annotated as `tbd::event` to events.

            This will analyze the function signature and generate DTOs for inputs and output, if no predefined
            DTOs are found in `.proto` files.
        """
        event_name = event.name
        args = event.args

        event.payload_type = self._add_event_payload(event_name, args).name if args else None

        # check if the endpoint has guaranteed id
        self._events.append(event)

    def _add_event_payload(self, event_name: str, args: OrderedDict[str, str]) -> Request:
        if self._event_payloads.get(event_name) is not None:
            raise ValueError(f'event payload for {event_name} already exists')

        if len(args) == 1:
            event_payload = self._add_single_arg_event_payload(event_name, next(iter(args.values())))
        else:
            event_payload = self._add_multi_arg_event_payload(event_name, args)
        return event_payload

    def _add_single_arg_event_payload(self, event_name: str, event_payload_name: str) -> Request:
        in_message = self._find_payload(event_payload_name)
        event_payload = event_for_single_arg(in_message)
        self._event_payloads[event_name] = event_payload
        return event_payload

    def _add_multi_arg_event_payload(self, event_name: str, args: OrderedDict[str, str]) -> Request:
        arg_messages = self._find_arg_payloads(args)
        event_payload = event_for_arguments(event_name, arg_messages)
        self._event_payloads[event_name] = event_payload
        return event_payload

    def _find_arg_payloads(self, args: OrderedDict[str, str]) -> OrderedDict[str, Payload]:
        arg_messages = OrderedDict()
        for arg_name, arg_type_name in args.items():
            in_message = self._find_payload(arg_type_name)
            arg_messages[arg_name] = in_message
        return arg_messages

    def _add_responder(self, responder: Responder) -> None:
        self._responders.append(responder)

    def _get_event_responders(self) -> dict[str, list[Responder]]:
        event_names = set(event.name for event in self._events)
        responder_event_names = set(responder.event_name for responder in self._responders)
        invalid_events = responder_event_names - event_names
        if invalid_events:
            raise ValueError(f'responders for unknown events: {invalid_events}')

        event_responders = {}
        for responder in self._responders:
            event_name = responder.event_name
            responders_for_event = event_responders.get(event_name, [])
            responders_for_event.append(responder)
            event_responders[event_name] = responders_for_event
        return event_responders

    def _add_event_sink(self, sink: EventSink) -> None:
        self._event_sinks.append(sink)

__all__ = [
    'ApiRegistry', 
]