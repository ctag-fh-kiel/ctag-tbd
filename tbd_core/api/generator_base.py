import inspect
from pathlib import Path
from typing import Callable, Any, Final

import jinja2 as ji

from .api import Api
from .idc_interfaces import Endpoint, Event, Responder


def jilter(f: Callable[[Any], Any]) -> Callable[[Any], Any]:
    """ Mark method as jinja filter. """
    f._is_jilter = True
    return f


class GeneratorBase:
    def __init__(self, api: Api, templates_path: Path):
        self._templates: Final[Path] = templates_path
        self._api: Final[Api] = api
        self._env = ji.Environment(loader=ji.FileSystemLoader(templates_path), autoescape=ji.select_autoescape())
        self._env.filters |= self._filters()

    def render(self, template_file: Path | str, **args) -> str:
        template = self._env.get_template(str(template_file))
        return template.render(api=self._api, **args)

    @jilter
    def endpoint_id(self, endpoint: Endpoint) -> int:
        return self._api.get_endpoint_id(endpoint.name)

    @jilter
    def request_id(self, endpoint: Endpoint) -> int:
        return -1

    @jilter
    def request_type(self, endpoint: Endpoint) -> str:
        return endpoint.request_type

    @jilter
    def response_id(self, endpoint: Endpoint) -> int:
        return -1

    @jilter
    def response_type(self, endpoint: Endpoint) -> str:
        return endpoint.response_type

    @jilter
    def event_id(self, event: Event) -> int:
        return self._api.get_event_id(event.name)

    @jilter
    def event_payload(self, event: Event) -> str:
        return event.payload_type

    @jilter
    def responders(self, event: Event) -> list[Responder]:
        responders = self._api.get_responders(event.name)
        return responders if responders else []

    def _filters(self):
        filters = {}
        for name, _ in inspect.getmembers(self, predicate=inspect.isroutine):
            if name.startswith('_'):
                continue
            method = self.__getattribute__(name)
            if not hasattr(method, '_is_jilter'):
                continue

            # sig = inspect.signature(method)
            # if len(sig.parameters) != 1:
            #     raise RuntimeError(f'filter {name} must have exactly 1 parameter')
            filters[name] = method
        return filters


__all__ = ['jilter', 'GeneratorBase']