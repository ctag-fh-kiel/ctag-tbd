""" Build generation global value store.

    ESPHome full reimports modules at times, thus module global values and lru_cache etc, 
    can not be used to store and register values during compile time without resorting to 
    files.

    This module provides a hack that extends the ESPHome `CORE` object which will not be
    reset by imports, to provide such a value storage facility.
"""

import functools
import logging
from typing import Any, OrderedDict

from .generation_stages import GenerationStages


_LOGGER = logging.getLogger(__file__)


GLOBAL_DOMAIN = 'globals'


class GlobalRegistry:
    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super(GlobalRegistry, cls).__new__(cls)
            cls._instance.stage = GenerationStages.DEFAULT
            cls._instance.domains = {GLOBAL_DOMAIN: {}}
        return cls._instance


def get_generation_stage() -> GenerationStages:
    return GlobalRegistry().stage


def _set_generation_stage(stage: GenerationStages) -> None:
    if GlobalRegistry().stage > stage:
        _LOGGER.info(f'[[==== entering generation stage {stage.name} ====]]')
        GlobalRegistry().stage = stage


def get_tbd_registry() -> dict[str, dict[str, Any]]:
    """ Get storage location of all TBD globals. """

    # from esphome.core import CORE

    # if not hasattr(CORE, TBD_REGISTRY_FIELD_NAME):
    #     setattr(CORE, TBD_REGISTRY_FIELD_NAME, {GLOBAL_DOMAIN: {}}),
    # return getattr(CORE, TBD_REGISTRY_FIELD_NAME)

    return GlobalRegistry().domains


def set_tbd_global(name: str, value: Any, *, domain: str = GLOBAL_DOMAIN, reset_ok: bool = False) -> None:
    domains = get_tbd_registry()
    if domain not in domains:
        domains[domain] = OrderedDict()
    
    values = domains[domain]
    if not reset_ok and name in values:
        raise ValueError(f'attempting to reset TBD global {domain}.{name}')
    
    values[name] = value


def has_tbd_global(name: str, *, domain: str = GLOBAL_DOMAIN) -> bool:
    if (values := get_tbd_domain(domain, missing_ok=True)) is None:
        return False
    return name in values


def get_tbd_domain(domain: str = GLOBAL_DOMAIN, missing_ok: bool = False) -> OrderedDict[str, Any] | None:
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


def generated_tbd_global(domain: str = GLOBAL_DOMAIN, *, after_stage: GenerationStages | None = None):
    """ Define lazily generated TBD global.

        Annotation for simple getter functions that create a global value to allow for the following semantics:

        - on first call anywhere, global gets created and stored
        - subsequent calls return the global

        This can be used for either simple value caching or singletons.

        :note: Due to the way in which esphome loads modules, the usual cache decorators can not be used these purposes.
    """

    def decorator(func):
        @functools.wraps(func)
        def wrapper():
            name = func.__name__
            if not (name.startswith('get_')):
                raise ValueError(f'{name} is not a valid TBD global getter name, needs to start with "get_')
            name = name[4:]

            if after_stage is not None:
                if get_generation_stage() > after_stage:
                    raise ValueError(f'{name} not available in stage {get_generation_stage().name}, becomes available at stage {after_stage.name}')

            if not has_tbd_global(name, domain=domain):
                set_tbd_global(name, func(), domain=domain)
            return get_tbd_global(name, domain=domain)
            
        return wrapper  
    return decorator


__all__ = [
    'get_generation_stage',
    '_set_generation_stage',
    'set_tbd_global', 
    'has_tbd_global', 
    'get_tbd_domain',
    'get_tbd_global', 
    'generated_tbd_global'
]