//
// Created by Robert Manzke on 05.09.20.
//

#include "Stimulator.hpp"
#include <cmath>

Stimulator::Stimulator() {
    src.SetSampleRate(fs);
    src.SetFrequency(1.f);
    mode = ModeType::MANUAL;
    value = 1.f;
}

Stimulator::~Stimulator() {

}

void Stimulator::SetMode(const int m) {
    mode = (ModeType)m;
}

void Stimulator::SetValue(const int v) {
    value = (float) v / 4095.f;
    f = value * 1000.f;
    src.SetFrequency(f);
}

float Stimulator::Process() {
    switch(mode){
        case ModeType::MANUAL:
            src.Process();
            return value;
        case ModeType::BISINE:
            return src.Process();
        case ModeType::USINE:
            return (src.Process() + 1.f) / 2.f;
        case ModeType::PULSE:
            return (src.Process() >= 0.f ? 1.f : 0.f);
        case ModeType::BISTEPS:
            return 0.f; // not implemented
        default:
            return 0.f;
    }
}
