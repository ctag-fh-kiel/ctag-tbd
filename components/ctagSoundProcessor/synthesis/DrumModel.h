#pragma once

#include <iostream>

namespace CTAG::SYNTHESIS{
    class DrumModel {
    public:
        virtual ~DrumModel() {}
        virtual void Init() = 0;
        virtual void Trigger() = 0;
        virtual void Process(float* out, uint32_t size) = 0;
        virtual void RenderControls() = 0;
    };
}