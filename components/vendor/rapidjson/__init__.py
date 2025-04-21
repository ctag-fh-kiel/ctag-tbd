from esphome.components.tbd_module import TBD_ROOT
import esphome.codegen as cg

AUTO_LOAD = ['tbd_module']

rapidjson_path = TBD_ROOT / 'vendor' / 'rapidjson'

cg.add_build_flag('-DRAPIDJSON_HAS_STDSTRING=1')
cg.add_build_flag('-DRAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=4096')
cg.add_build_flag(f'-I{rapidjson_path}')
