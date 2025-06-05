""" Main helper component for all TBD esphome components.

    
"""
import logging
from esphome.core import CORE

from tbd_core.buildgen import *

from .collect_errors import collect_errors
from .esphome_build_generator import EsphomeBuildGenerator

set_build_generator(EsphomeBuildGenerator())

AUTO_LOAD = ['tbd_system']


_LOGGER = logging.getLogger(__name__)

def is_esp32():
    return CORE.is_esp32

def is_desktop():
    return CORE.is_host


@build_job_with_priority(GenerationStages.ERRORS)
def errors_job():
    collect_errors()


@build_job_with_priority(GenerationStages.COMPONENTS)
def prepare_build_job():
    prepare_build()


def to_code(config):
    new_tbd_component(__file__)

    add_generation_job(prepare_build_job)
    add_generation_job(errors_job)
