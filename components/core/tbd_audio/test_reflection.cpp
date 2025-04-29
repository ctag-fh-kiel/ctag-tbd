#include <tbd/sound_processor/parameters.hpp>

using namespace tbd::audio::parameters;

int get_param() {
    TBD_LOGI("hello", "foobar %s", get_param_info(12)->name);
}