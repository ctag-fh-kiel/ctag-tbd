#include "FmKickModel.h"
#include <cmath>

using namespace CTAG::SYNTHESIS;

void FmKickModel::Init() {
    t = 0.0f;
    // Reset iterative decays
    amp_env = 1.0f;
    mod_env = 1.0f;
    freq_env = 1.0f;
    // Reset Plaits operator state
    ops[0].Reset();
    ops[1].Reset();
    fb_state[0] = 0.0f;
    fb_state[1] = 0.0f;
}

void FmKickModel::Trigger() {
    Init();
    // Calculate decay constants for iterative envelopes WITHOUT std::expf
    float dt = 1.0f / (44100.f * 32); // Assuming 44100Hz sample rate and 32 samples per block
    // For small x, exp(-x) ≈ 1 - x
    amp_decay_const = 1.0f - (dt / d_b);
    mod_decay_const = 1.0f - (dt / d_m);
    freq_decay_const = 1.0f - (dt / d_f);
    // Clamp to [0,1] to avoid negative decay in pathological cases
    if (amp_decay_const < 0.0f) amp_decay_const = 0.0f;
    if (mod_decay_const < 0.0f) mod_decay_const = 0.0f;
    if (freq_decay_const < 0.0f) freq_decay_const = 0.0f;
}

void FmKickModel::Process(float* out, uint32_t size) {
    // Iterative decay
    amp_env *= amp_decay_const;
    mod_env *= mod_decay_const;
    freq_env *= freq_decay_const;
    float freq_env_scaled = A_f * freq_env;

    // Prepare Plaits FM operator parameters
    float f[2];
    float a[2];
    // Modulator frequency selection
    float mod_freq = f_m;
    if (use_ratio_mode) {
        mod_freq = f_b * (ratios[ratio_index][0] / ratios[ratio_index][1]);
    }
    // Sync modulator freq envelope to carrier if enabled
    if (mod_env_sync) {
        mod_freq += freq_env_scaled;
    }
    f[0] = mod_freq / 44100.f; // modulator frequency (normalized)
    f[1] = (f_b + freq_env_scaled) / 44100.f; // carrier frequency (normalized)
    a[0] = I * mod_env; // modulator amplitude (mod index)
    a[1] = amp_env;     // carrier amplitude


    // Feedback amount for modulator (0-7)
    int fb_amt = static_cast<int>(b_m);
    // Render a single sample using Plaits FM operator (2-op, modulator feeds carrier)
    plaits::fm::RenderOperators<2, 0, false>(
        ops, f, a, fb_state, fb_amt, nullptr, out, size);
}