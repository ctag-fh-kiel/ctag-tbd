from esphome.components.tbd_module import get_tbd_source_root
import esphome.codegen as cg

REQUIRES = ['tbd_module']

rapidjson_path = get_tbd_source_root() / 'vendor' / 'rapidjson'

cg.add_build_flag('-DRAPIDJSON_HAS_STDSTRING=1')
cg.add_build_flag('-DRAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=4096')
cg.add_build_flag(f'-I{rapidjson_path}')
