import tbd_core.buildgen as tbb
from tbd_core.buildgen import get_reflectables, GenerationStages, AutoReflection, get_build_path, \
    get_generated_sources_path
from tbd_core.serialization import DTORegistry, Serializables, SerializableGenerator

DTO_REGISTRY_GLOBAL = 'dto_registry'
DTOS_GLOBAL = 'dtos'


@tbb.generated_tbd_global(DTO_REGISTRY_GLOBAL, after_stage=GenerationStages.REFLECTION)
def get_dto_registry() -> DTORegistry:
    return DTORegistry(get_reflectables())


@tbb.generated_tbd_global(DTOS_GLOBAL, after_stage=GenerationStages.FINALIZE)
def get_dtos() -> dict[str, Serializables]:
    return get_dto_registry().get_dtos()


@tbb.build_job_with_priority(tbb.GenerationStages.SERIALIZATION)
def generate_serialization():
    for domain, serializables in get_dtos().items():
        gen = SerializableGenerator(serializables, get_reflectables())

        headers_dir = get_build_path() / get_generated_sources_path() / 'include' / 'tbd' / domain / 'dtos'
        headers_dir.mkdir(parents=True, exist_ok=True)

        srcs_dir = get_build_path() / get_generated_sources_path() / 'serialization' / domain
        srcs_dir.mkdir(parents=True, exist_ok=True)

        gen.write_cpp_code(domain, headers_dir, srcs_dir)
        # gen.write_protos( / 'dtos.proto')


tbb.new_tbd_component(__file__, auto_reflect=AutoReflection.ALL)
tbb.add_define('PB_WITHOUT_64BIT', 1)
tbb.add_generation_job(generate_serialization)
