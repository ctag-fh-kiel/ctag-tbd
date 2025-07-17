import esphome.config_validation as cv

from tbd_core.buildgen import build_job_with_priority, GenerationStages, prepare_tests, add_generation_job, \
    new_tbd_component

CONFIG_SCHEMA = cv.Schema({})


@build_job_with_priority(GenerationStages.TESTS)
def prepare_tests_job():
    prepare_tests()


async def to_code(config):
    component = new_tbd_component(__file__)
    add_generation_job(prepare_tests_job)
