#include <tbd/parameter_types.hpp>
#include <tbd/errors.hpp>

#include <tbd/presets/dtos/message_encoding.hpp>
#include <tbd/presets/dtos/message_decoding.hpp>

namespace tbd::presets {

[[tbd::endpoint]]
Error get_active_preset(const uint_par& channel, uint_par& preset_id) {
    return TBD_OK;
}

}