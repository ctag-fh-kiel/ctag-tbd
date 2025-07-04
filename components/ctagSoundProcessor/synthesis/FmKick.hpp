#pragma once
#include "DrumModel.hpp"
#include "plaits/dsp/fm/operator.h"

namespace CTAG::SYNTHESIS{
class FmKick : public DrumModel {
public:
    void Init() override;
    void Trigger() override;
    void Process(float* out, uint32_t size) override;

    struct Params {
        bool use_ratio_mode = false; // Use frequency ratios for modulator frequency
        bool mod_env_sync = false; // Sync modulator frequency envelope to carrier frequency
        float f_b = 35.0f; // Base frequency for carrier (Hz) 20...100
        float d_b = 0.5f; // Decay time for carrier amplitude (seconds) (0.001...2.0)
        float f_m = 180.0f; // Modulator frequency (Hz) 40 ... 200
        float I = 0.1f; // Modulation index (modulator amplitude) 0...10
        float d_m = 0.15f; // Decay time for modulator amplitude (seconds) (0.001...2.0)
        float b_m = 0.5f; // Feedback amount for modulator (0-7) 0 ... 16
        float A_f = 60.0f; // Amplitude envelope scaling for carrier frequency (Hz) 0 ... 1000
        float d_f = 0.1f; // Decay time for carrier frequency envelope (seconds) (0.001...2.0)
    };
    Params params;

private:
    // Ratio mode for modulator frequency
    int ratio_index = 0; // Index into ratio array
    static constexpr int num_ratios = 64;
    static constexpr float ratios[num_ratios][2] = {
        // Integer multiples 2:1 to 40:1
        {2.0f, 1.0f}, {3.0f, 1.0f}, {4.0f, 1.0f}, {5.0f, 1.0f}, {6.0f, 1.0f}, {7.0f, 1.0f}, {8.0f, 1.0f}, {9.0f, 1.0f},
        {10.0f, 1.0f}, {11.0f, 1.0f}, {12.0f, 1.0f}, {13.0f, 1.0f}, {14.0f, 1.0f}, {15.0f, 1.0f}, {16.0f, 1.0f}, {17.0f, 1.0f},
        {18.0f, 1.0f}, {19.0f, 1.0f}, {20.0f, 1.0f}, {21.0f, 1.0f}, {22.0f, 1.0f}, {23.0f, 1.0f}, {24.0f, 1.0f}, {25.0f, 1.0f},
        {26.0f, 1.0f}, {27.0f, 1.0f}, {28.0f, 1.0f}, {29.0f, 1.0f}, {30.0f, 1.0f}, {31.0f, 1.0f}, {32.0f, 1.0f}, {33.0f, 1.0f},
        {34.0f, 1.0f}, {35.0f, 1.0f}, {36.0f, 1.0f}, {37.0f, 1.0f}, {38.0f, 1.0f}, {39.0f, 1.0f}, {40.0f, 1.0f},
        // Odd/weird ratios >1 and <=40 (25 more)
        {8.0f, 5.0f}, {11.0f, 3.0f}, {13.0f, 7.0f}, {17.0f, 5.0f}, {19.0f, 4.0f}, {23.0f, 7.0f}, {31.0f, 9.0f}, {37.0f, 8.0f},
        {15.0f, 4.0f}, {21.0f, 5.0f}, {25.0f, 6.0f}, {27.0f, 8.0f}, {35.0f, 9.0f}, {39.0f, 10.0f}, {29.0f, 7.0f}, {33.0f, 8.0f},
        {22.0f, 3.0f}, {34.0f, 5.0f}, {38.0f, 7.0f}, {5.0f, 2.0f}, {7.0f, 3.0f}, {9.0f, 2.0f}, {12.0f, 5.0f}, {14.0f, 3.0f},
        {16.0f, 5.0f}
    };

    // Iterative decay state
    float amp_env = 1.0f, mod_env = 1.0f, freq_env = 1.0f;
    float amp_decay_const = 0.0f, mod_decay_const = 0.0f, freq_decay_const = 0.0f;

    // Plaits FM operator state
    plaits::fm::Operator ops[2]; // [0]=modulator, [1]=carrier
    float fb_state[2] = {0.0f, 0.0f};
};
}
