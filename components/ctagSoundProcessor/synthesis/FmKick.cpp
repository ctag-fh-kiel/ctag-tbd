#include "FmKick.hpp"
#include <cmath>

using namespace CTAG::SYNTHESIS;

void FmKick::Init() {
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

void FmKick::Trigger() {
    Init();
    const float safe_db = (params.d_b < 0.001f) ? 0.001f : params.d_b;
    const float safe_dm = (params.d_m < 0.001f) ? 0.001f : params.d_m;
    const float safe_df = (params.d_f < 0.001f) ? 0.001f : params.d_f;
    // Calculate decay constants for iterative envelopes WITHOUT std::expf
    // For small x, exp(-x) ≈ 1 - x
    amp_decay_const   = 1.0f - (dt / safe_db);
    mod_decay_const   = 1.0f - (dt / safe_dm);
    freq_decay_const  = 1.0f - (dt / safe_df);
    // Clamp to [0,1] to avoid negative decay in pathological cases
    if (amp_decay_const  < 0.0f) amp_decay_const  = 0.0f;
    if (mod_decay_const  < 0.0f) mod_decay_const  = 0.0f;
    if (freq_decay_const < 0.0f) freq_decay_const = 0.0f;
}

void FmKick::Process(float* out, uint32_t size) {
    float freq_env_scaled = params.A_f * freq_env;

    // Prepare Plaits FM operator parameters
    float f[2];
    float a[2];
    // Modulator frequency selection
    float mod_freq = params.f_m;
    if (params.use_ratio_mode) {
        ratio_index = params.mod_ratio_index;
        mod_freq = params.f_b * (ratios[ratio_index][0] / ratios[ratio_index][1]);
    }
    // Sync modulator freq envelope to carrier if enabled
    if (params.mod_env_sync) {
        mod_freq += freq_env_scaled;
    }
    f[0] = mod_freq / 44100.f; // modulator frequency (normalized)
    f[1] = (params.f_b + freq_env_scaled) / 44100.f; // carrier frequency (normalized)
    a[0] = params.I * mod_env; // modulator amplitude (mod index)
    a[1] = amp_env;     // carrier amplitude

    // Feedback amount for modulator (0-16)
    int fb_amt = static_cast<int>(params.b_m);
    // Render a single sample using Plaits FM operator (2-op, modulator feeds carrier)
    plaits::fm::RenderOperators<2, 0, false>(
        ops, f, a, fb_state, fb_amt, nullptr, out, size);

    // Iterative decay
    amp_env  *= amp_decay_const;
    mod_env  *= mod_decay_const;
    freq_env *= freq_decay_const;
    // Kill denormals before they accumulate
    if (amp_env  < 1e-6f) amp_env  = 0.0f;
    if (mod_env  < 1e-6f) mod_env  = 0.0f;
    if (freq_env < 1e-6f) freq_env = 0.0f;
}