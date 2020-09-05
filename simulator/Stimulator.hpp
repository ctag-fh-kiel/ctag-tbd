#pragma once

#include "helpers/ctagSineSource.hpp"

class Stimulator {
public:
    Stimulator();
    ~Stimulator();
    float Process();
    void SetMode(const int m);
    void SetValue(const int v);
private:
    CTAG::SP::HELPERS::ctagSineSource src;
    enum ModeType {MANUAL = 0, PULSE = 1, USINE = 2, BISINE = 3, BISTEPS = 4};
    ModeType mode;
    float value;
    float f;
    const float fs = 44100.f / 32.f;
};
