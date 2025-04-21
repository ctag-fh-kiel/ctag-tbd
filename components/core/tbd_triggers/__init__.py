from esphome.core import CORE
import esphome.config_validation as cv

from esphome.components.tbd_module import new_tbd_component

AUTO_LOAD = ['tbd_module']

CONFIG_SCHEMA = {}

new_tbd_component(__file__)
