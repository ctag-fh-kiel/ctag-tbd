""" Build generation global value store.

    ESPHome full reimports modules at times, thus module global values and lru_cache etc, 
    can not be used to store and register values during compile time without resorting to 
    files.

    This module provides a hack that extends the ESPHome `CORE` object which will not be
    reset by imports, to provide such a value storage facility.

"""

import functools
from pathlib import Path
import logging
from typing import Any
from esphome.core import CORE
from dataclasses import dataclass


_LOGGER = logging.getLogger(__file__)


GLOBAL_DOMAIN = 'globals'


def get_tbd_registry() -> dict[str, dict[str, Any]]:
    TBD_REGISTRY_FIELD_NAME = 'tbd_registry'
    if not hasattr(CORE, TBD_REGISTRY_FIELD_NAME):
        setattr(CORE, TBD_REGISTRY_FIELD_NAME, {GLOBAL_DOMAIN: {}}),
    return getattr(CORE, TBD_REGISTRY_FIELD_NAME)


def set_tbd_global(name: str, value: Any, *, domain: str = GLOBAL_DOMAIN, reset_ok: bool = False) -> None:
    domains = get_tbd_registry()
    if domain not in domains:
        domains[domain] = {}
    
    values = domains[domain]
    if not reset_ok and name in values:
        raise ValueError(f'attempting to reset TBD global {domain}.{name}')
    
    values[name] = value


def has_tbd_global(name: str, *, domain: str = GLOBAL_DOMAIN) -> bool:
    domains = get_tbd_registry()
    return domain in domains and name in domains[domain]


def get_tbd_domain(domain: str = GLOBAL_DOMAIN, missing_ok: bool = False) -> dict[Any]:
    domains = get_tbd_registry()

    if domain not in domains:
        if missing_ok:
            return None
        else:
            raise ValueError(f'no such TBD domain {domain}')
    return domains[domain]


def get_tbd_global(name: str, *, domain: str = GLOBAL_DOMAIN, missing_ok: bool = False) -> Any:
    if (values := get_tbd_domain(domain, missing_ok=missing_ok)) is None:
        return None

    if name not in values:
        if missing_ok:
            return None
        else:
            raise ValueError(f'no TBD global {domain}.{name}')
    
    return values[name]


class TBDRegistry:
    def __init__(self):
        self._registry = get_tbd_registry()

    def __getattribute__(self, name: str):
        if name.startswith('_'):
            raise ValueError('private attribute requested')
        


def generated_tbd_global(domain: str = GLOBAL_DOMAIN):
    def decorator(func):
        @functools.wraps(func)
        def wrapper():
            name = func.__name__
            if not (name.startswith('get_')):
                raise ValueError(f'{name} is not a valid TBD global getter name, needs to start with "get_')
            name = name[4:]
            if not has_tbd_global(name, domain=domain):
                set_tbd_global(name, func(), domain=domain)
            return get_tbd_global(name, domain=domain)
            
        return wrapper  
    return decorator


__all__ = [
    'set_tbd_global', 
    'has_tbd_global', 
    'get_tbd_domain',
    'get_tbd_global', 
    'generated_tbd_global'
]