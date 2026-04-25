//
// Phase 8.6 (2026-04-24e) — pan-symmetry regression test.
//
// Gates Phase 9. Replicates the `RackChannelMixer::PreProcess` pan math
// (see components/ctagSoundProcessor/rack/RackChannelMixer.cpp) and
// asserts every boundary. The Phase 8.5 pan bug left ~1.6 % L-channel
// bleed at hard-right pan because divisor 4096 was used against an
// atomic max of 4064. This test catches that regression.
//

#pragma once

namespace CTAG {
namespace TESTS {

class test_fx_mixer_pan {
  public:
    // Run all pan-symmetry assertions; returns 0 on success, >0 on failure.
    int DoTest();

  private:
    // Pure-function mirror of the pan math in RackChannelMixer.cpp.
    // Takes the atomic (0..4064 for wire 0..127) and emits (mL, mR).
    void computePanGains(int mix_pan_atomic, float &mL, float &mR);

    // Assertion helpers — print + count failures.
    void expectNear(const char *name, float actual, float expected, float tol);
    void expectExact(const char *name, float actual, float expected);

    int failures_ = 0;
};

} // namespace TESTS
} // namespace CTAG
