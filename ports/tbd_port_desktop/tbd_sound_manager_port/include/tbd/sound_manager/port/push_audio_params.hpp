#pragma once


#include <string>
#include <tbd/logging.hpp>


namespace tbd::audio {

struct PushAudioParams{
    /** @brief default 'live' only sound processing
     *
     */
    PushAudioParams(const std::string& input_file = "", const std::string& output_file = "", int32_t live_device = -1) {
        auto input = input_file.empty() ? "live" : input_file;
        auto output = output_file.empty() ? "live" : output_file;

        if (input != "live" && output != "live") {
            if (live_device != -1) {
                TBD_LOGW(tag, "sound config has two files and live device parameter");
            }
            _input = -1;
        }

        if (live_device < 0 && (input == "live" || output == "live")) {
            _device = 0;
        }

        _input = input;
        _output = output;
    }

    bool use_live_input() const {
        return _input == "live";
    }

    std::string input_file() const {
        if (use_live_input()) {
            TBD_LOGE(tag, "tried to receive audio input file with live input set");
            return {};
        }
        return _input;
    }

    uint32_t device() const {
        return _device;
    }

private:
    std::string _input;
    std::string _output;
    uint32_t _device;
};

}