#pragma once

#include <cstdint>
#include <tbd/ram.hpp>

#include <tbd/sound_processor/processing_metrics.hpp>


namespace tbd {

class ControlInputs final{
public:
    ControlInputs() = delete;

    static void SetCVChannelBiPolar(bool const &v0, bool const &v1, bool const &v2, bool const &v3);

    static void init();

    TBD_IRAM static void update(uint8_t **trigs, float **cvs);
    static void update_metrics(const sound_processor::ProcessingMetrics& metrics);
    static void flush();
};

}
