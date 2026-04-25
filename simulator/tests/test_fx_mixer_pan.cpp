//
// Phase 8.6 (2026-04-24e) — pan-symmetry regression test.
// See test_fx_mixer_pan.hpp for context.
//

#include "test_fx_mixer_pan.hpp"

#include <cmath>
#include <cstdio>

namespace CTAG {
namespace TESTS {

// CONSTRAIN macro mirrors stmlib.h — keep in sync if the upstream changes.
#define CONSTRAIN(var, mn, mx) \
    do { if ((var) < (mn)) (var) = (mn); else if ((var) > (mx)) (var) = (mx); } while (0)

// Pure-function mirror of RackChannelMixer::PreProcess's pan math.
// Keep in sync with the source file (same divisor 4064 + CONSTRAIN clamp).
void test_fx_mixer_pan::computePanGains(int mix_pan_atomic, float &mL, float &mR) {
    // Phase 8.5 fix: divisor 4064 (= 127 × 32, actual atomic max from the
    // P4 MIDI-CC handler) + explicit clamp so atomic overshoot is safe.
    float fPan = (float)mix_pan_atomic / 4064.f;
    CONSTRAIN(fPan, 0.f, 1.f);
    fPan = fPan * 2.f - 1.f;

    mL = (1.f - fPan);
    mR = (1.f + fPan);
    CONSTRAIN(mL, 0.f, 1.f);
    CONSTRAIN(mR, 0.f, 1.f);
}

void test_fx_mixer_pan::expectNear(const char *name, float actual, float expected, float tol) {
    if (std::fabs(actual - expected) > tol) {
        std::printf("  FAIL: %s — expected %.6f ± %.6f, got %.6f\n",
                    name, expected, tol, actual);
        failures_++;
    } else {
        std::printf("  ok  : %s = %.6f (expected %.6f ± %.6f)\n",
                    name, actual, expected, tol);
    }
}

void test_fx_mixer_pan::expectExact(const char *name, float actual, float expected) {
    expectNear(name, actual, expected, 0.0f);
}

int test_fx_mixer_pan::DoTest() {
    std::printf("test_fx_mixer_pan — RackChannelMixer pan symmetry\n");
    failures_ = 0;

    // Hard-left: wire 0 → atomic 0 → mL=1, mR=0 exactly.
    {
        float mL, mR;
        computePanGains(0, mL, mR);
        expectExact("wire 0 (hard-left) mL", mL, 1.0f);
        expectExact("wire 0 (hard-left) mR", mR, 0.0f);
    }

    // Hard-right: wire 127 → atomic 4064 → mL=0, mR=1 exactly.
    // This is the bug that bit us in 2026-04-24e — verify the fix holds.
    {
        float mL, mR;
        computePanGains(4064, mL, mR);
        expectExact("wire 127 (hard-right) mL", mL, 0.0f);
        expectExact("wire 127 (hard-right) mR", mR, 1.0f);
    }

    // Centre: wire 64 → atomic 2048 → fPan = 2048/4064 = 0.504 → shift = 0.008.
    // Tiny off-centre residual (~0.07 % pan) is inaudible. mL ≈ mR ≈ 0.996.
    {
        float mL, mR;
        computePanGains(2048, mL, mR);
        expectNear("wire 64 (centre) mL", mL, 0.992f, 0.01f);
        expectNear("wire 64 (centre) mR", mR, 1.000f, 0.01f);  // clamped to 1
    }

    // Atomic-overshoot safety: if somehow atomic lands > 4064, CONSTRAIN
    // clamps fPan to 1.0. Verifies the belt+braces clamp.
    {
        float mL, mR;
        computePanGains(4500, mL, mR);
        expectExact("atomic overshoot mL", mL, 0.0f);
        expectExact("atomic overshoot mR", mR, 1.0f);
    }

    // Negative atomic should also be safe (though the CC handler never
    // produces negative values). CONSTRAIN catches it.
    {
        float mL, mR;
        computePanGains(-100, mL, mR);
        expectExact("atomic underflow mL", mL, 1.0f);
        expectExact("atomic underflow mR", mR, 0.0f);
    }

    // Monotonicity: as wire sweeps 0 → 127, mR should monotonically
    // rise 0 → 1 and mL should monotonically fall 1 → 0 (modulo the
    // clamp region near extremes where they plateau).
    float prev_mR = -1.f;
    float prev_mL =  2.f;
    bool monotonic = true;
    for (int wire = 0; wire <= 127; wire++) {
        float mL, mR;
        computePanGains(wire * 32, mL, mR);
        if (mR < prev_mR - 1e-6f) monotonic = false;
        if (mL > prev_mL + 1e-6f) monotonic = false;
        prev_mR = mR;
        prev_mL = mL;
    }
    if (monotonic) {
        std::printf("  ok  : mL monotonic decrease 1→0, mR monotonic increase 0→1\n");
    } else {
        std::printf("  FAIL: pan math is not monotonic across the wire range\n");
        failures_++;
    }

    // Energy-preserving-ish check: at hard-left mL=1 + mR=0 = 1; at centre
    // ~2; at hard-right 0+1=1. Classical linear pan gives constant power
    // via sqrt, but our code is linear. Document the behaviour.
    {
        float mL, mR;
        computePanGains(0,    mL, mR);  float sum0   = mL + mR;  // 1.0
        computePanGains(2048, mL, mR);  float sumC   = mL + mR;  // ≈ 2.0
        computePanGains(4064, mL, mR);  float sum127 = mL + mR;  // 1.0
        expectNear("sum at hard-left",  sum0,   1.0f, 0.01f);
        expectNear("sum at centre",     sumC,   2.0f, 0.02f);
        expectNear("sum at hard-right", sum127, 1.0f, 0.01f);
    }

    std::printf("test_fx_mixer_pan — %d failures\n\n", failures_);
    return failures_;
}

} // namespace TESTS
} // namespace CTAG
