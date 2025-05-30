import inspect
from pathlib import Path
from typing import Callable, Any

import jinja2 as ji

from .api import Api
from .enpoints import Endpoint


def get_env(srcs_path: Path) -> ji.Environment:
    template_path = Path(__file__).parent / srcs_path
    env = ji.Environment(loader=ji.FileSystemLoader(template_path), autoescape=ji.select_autoescape())
    return env


def jilter(f: Callable[[Any], Any]) -> Callable[[Any], Any]:
    """ Mark method as jinja filter. """
    f._is_jilter = True
    return f


class GeneratorBase:
    def __init__(self, api: Api, srcs_path: Path):
        self._api = api
        self._env = get_env(srcs_path)
        self._env.filters |= self._filters()

    def render(self, template_name: str) -> str:
        template = self._env.get_template(template_name)
        return template.render(api=self._api)

    @jilter
    def endpoint_id(self, endpoint: Endpoint) -> int:
        return self._api.get_endpoint_id(endpoint.name)

    @jilter
    def request_id(self, endpoint: Endpoint) -> int:
        return -1

    @jilter
    def request_type(self, endpoint: Endpoint) -> str:
        return self._api.get_request(endpoint).name

    @jilter
    def response_id(self, endpoint: Endpoint) -> int:
        return -1

    @jilter
    def response_type(self, endpoint: Endpoint) -> str:
        return self._api.get_response(endpoint).name

    def _filters(self):
        filters = {}
        for name, _ in inspect.getmembers(self, predicate=inspect.isroutine):
            if name.startswith('_'):
                continue
            method = self.__getattribute__(name)
            if not hasattr(method, '_is_jilter'):
                continue

            sig = inspect.signature(method)
            if len(sig.parameters) != 1:
                raise RuntimeError(f'filter {name} must have exactly 1 parameter')
            filters[name] = method
        return filters


__all__ = ['jilter', 'GeneratorBase']