import logging
from collections import OrderedDict

import humps

from tbd_core.reflection.reflectables import ClassID, FunctionID
from tbd_core.reflection.db import FunctionPtr, ArgumentPtr, ClassPtr
from tbd_core.serialization import DTORegistry

from .idc_interfaces import Endpoint, idc_from_function, Event, Responder, EventSink
from .base_endpoints import BaseEndpoints
from .api import Api


_LOGGER = logging.getLogger(__file__)


API_DOMAIN = 'api'

# def guaranteed_payload_types() -> OrderedDict[str, Payload]:
#     return OrderedDict((message.name, message)
#         for message in ParamPayload.param_messages())


class ApiRegistry:
    def __init__(self, dto_registry: DTORegistry):
        self._dto_registry = dto_registry

        self._base_endpoints: list[Endpoint | None] = [None] * BaseEndpoints.num_reserved_ids()
        self._endpoints: list[Endpoint] = []
        self._request_types: OrderedDict[FunctionID, ClassPtr] = OrderedDict()
        self._response_types: OrderedDict[FunctionID, ClassPtr] = OrderedDict()

        self._events: list[Event] = []
        self._responders: list[Responder] = []
        self._event_payloads: OrderedDict[FunctionID, ClassPtr] = OrderedDict()
        self._event_sinks: list[EventSink] = []

    def get_api(self) -> Api:
        reflectables = self._dto_registry.reflectables
        for func in reflectables.functions():
            self._add_idc(func)

        endpoints = [*self._get_guaranteed_endpoints(), *self._get_optional_endpoints()]

        return Api(
            endpoints=endpoints,
            endpoint_requests=self._request_types,
            endpoint_responses=self._response_types,
            events=self._events,
            event_payloads=self._event_payloads,
            event_responders=self._get_event_responders(),
            event_sinks=self._event_sinks,
        )

    def _get_guaranteed_endpoints(self) -> list[Endpoint]:
        endpoints: list[Endpoint | None] = [None] * BaseEndpoints.num_reserved_ids()
        endpoint_mapping = BaseEndpoints.id_mapping()

        for endpoint in self._base_endpoints:
            endpoint_id = endpoint_mapping.get(endpoint.endpoint_name)
            if endpoint_id is not None:
                endpoints[endpoint_id] = endpoint
        if None in endpoints:
            found_endpoints = set(endpoint.func_name for endpoint in endpoints)
            required_endpoints = set(BaseEndpoints.names())
            raise RuntimeError(f'missing required endpoints {required_endpoints - found_endpoints}')
        return endpoints

    def _get_optional_endpoints(self) -> list[Endpoint]:
        required_endpoints = set(BaseEndpoints.names())
        return [endpoint for endpoint in self._endpoints if endpoint.func_name not in required_endpoints]

    def _add_idc(self, func: FunctionPtr) -> None:
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
        endpoint_name = endpoint.func_name
        if (inputs := endpoint.inputs) is not None:
            self._request_types[endpoint.ref()] = self._add_endpoint_request(endpoint_name, inputs)
        if (output := endpoint.output) is not None:
            self._response_types[endpoint.ref()] = self._add_response(output)

        # check if the endpoint has guaranteed id
        if endpoint_name in BaseEndpoints.names():
            endpoint_id = BaseEndpoints[endpoint_name]
            self._base_endpoints[endpoint_id] = endpoint
        else:
            self._endpoints.append(endpoint)

    def _add_endpoint_request(self, endpoint_name: str, inputs: list[ArgumentPtr]) -> ClassPtr:
        if len(inputs) == 1:
            request = self._add_single_input_request(inputs[0])
        else:
            request = self._add_multi_input_request(endpoint_name, inputs)
        return request

    def _add_single_input_request(self, _input: ArgumentPtr) -> ClassPtr:
        """ Add reusable dto for a specific single argument type.

            Unlike multi argument request call signatures, these request types will be shared across multiple endpoints
            to avoid redundancy.
        """

        return self._dto_registry.make_type_serializable(API_DOMAIN, _input.type)

    def _add_multi_input_request(self, endpoint_name: str, inputs: list[ArgumentPtr]) -> ClassPtr:
        field_list = OrderedDict((_input.arg_name, _input.type) for _input in inputs)
        dto_name = f'{humps.pascalize(endpoint_name)}Args'
        return self._dto_registry.create_dto_from_field_list(API_DOMAIN, dto_name, field_list)

    def _add_response(self, output: ArgumentPtr) -> ClassPtr:
        """ Add reusable dto for a specific output type. """

        return self._dto_registry.make_type_serializable(API_DOMAIN, output.type)

    def _add_event(self, event: Event) -> None:
        """ Add a C++ function annotated as `tbd::event` to events.

            This will analyze the function signature and generate DTOs for inputs and output, if no predefined
            DTOs are found in `.proto` files.
        """

        if (inputs := event.inputs) is not None:
            self._event_payloads[event.ref()] = self._add_event_payload(event.event_name, inputs)

        # check if the endpoint has guaranteed id
        self._events.append(event)

    def _add_event_payload(self, event_name: str, inputs: list[ArgumentPtr]) -> ClassPtr:
        if len(inputs) == 1:
            event_payload = self._add_single_input_payload(inputs[0])
        else:
            event_payload = self._add_multi_input_payload(event_name, inputs)
        return event_payload

    def _add_single_input_payload(self, _input: ArgumentPtr) -> ClassPtr:
        return self._dto_registry.make_type_serializable(API_DOMAIN, _input.type)

    def _add_multi_input_payload(self, event_name: str, inputs: list[ArgumentPtr]) -> ClassPtr:
        field_list = OrderedDict((_input.arg_name, _input.type) for _input in inputs)
        dto_name = f'{humps.pascalize(event_name)}Payload'
        return self._dto_registry.create_dto_from_field_list(API_DOMAIN, dto_name, field_list)

    def _add_responder(self, responder: Responder) -> None:
        self._responders.append(responder)

    def _get_event_responders(self) -> dict[str, list[Responder]]:
        event_names = set(event.event_name for event in self._events)
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