#pragma once

#include <cstdint>
#include "Stimulator.hpp"
#include "SimDataModel.hpp"
#include <mutex>

class SimStimulus {
public:
    SimStimulus();
    ~SimStimulus();
    void Process(float *cvpot, uint8_t *trig);
    void UpdateStimulus(const int *mode, const int *value);
private:
    std::mutex modeMutex;
    Stimulator s[6];
};

