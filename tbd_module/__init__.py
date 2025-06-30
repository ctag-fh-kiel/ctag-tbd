""" Main helper component for all TBD esphome components.

    
"""
import logging
from pathlib import Path

from esphome.core import CORE
from esphome import loader

from .collect_errors import collect_errors
from .esphome_build_generator import EsphomeBuildGenerator


def setup_tbd_build():
    tbd_core_dir = Path(__file__).parent.parent
    loader.install_meta_finder(tbd_core_dir / 'components' / 'api')
    loader.install_meta_finder(tbd_core_dir / 'components' / 'audio')
    loader.install_meta_finder(tbd_core_dir / 'components' / 'builtin_sounds')
    loader.install_meta_finder(tbd_core_dir / 'components' / 'control_inputs')
    loader.install_meta_finder(tbd_core_dir / 'components' / 'core')

setup_tbd_build()
from tbd_core.buildgen import *

set_build_generator(EsphomeBuildGenerator())

AUTO_LOAD = ['tbd_system']
CONFIG_SCHEMA = {}

_LOGGER = logging.getLogger(__name__)

def is_esp32():
    return CORE.is_esp32

def is_desktop():
    return CORE.is_host


@build_job_with_priority(GenerationStages.ERRORS)
def errors_job():
    collect_errors()


@build_job_with_priority(GenerationStages.REFLECTION)
def auto_reflection_job():
    auto_reflect_component()


@build_job_with_priority(GenerationStages.COMPONENTS)
def prepare_build_job():
    prepare_build()


@build_job_with_priority(GenerationStages.FINALIZE)
def finalize_build_job():
    finalize_build()


def to_code(config):
    new_tbd_component(__file__)

    add_generation_job(auto_reflection_job)
    add_generation_job(prepare_build_job)
    add_generation_job(finalize_build_job)
    add_generation_job(errors_job)
