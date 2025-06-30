#include <tbd/parameter_types.hpp>
#include <tbd/errors.hpp>


namespace tbd::presets {

[[tbd::endpoint]]
Error get_active_preset(const uint_par& channel, uint_par& preset_id) {
    return TBD_OK;
}

}