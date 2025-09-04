#pragma once

namespace tbd::sound_processor {

struct ProcessingMetrics {
    bool warning = false;
    float input_level = 0.f;
    float output_level = 0.f;
};

}
