//
// Phase 8.6 — FX1 tempo-divisor math regression test.
// See test_fx1_tempo_divisor.hpp for context.
//

#include "test_fx1_tempo_divisor.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace CTAG {
namespace TESTS {

#define CONSTRAIN(var, mn, mx) \
    do { if ((var) < (mn)) (var) = (mn); else if ((var) > (mx)) (var) = (mx); } while (0)

// Pure-function mirror of preprocessFX1()'s delay-time path. Kept in sync
// with the source manually — any change in the source math must update
// this helper.
int test_fx1_tempo_divisor::computeDelaySamples(int wire_time, bool sync, float msPerBeat) {
    int wire = wire_time;
    CONSTRAIN(wire, 0, 127);

    float dt_ms;
    if (sync) {
        int idx = (wire * 12) / 128;
        CONSTRAIN(idx, 0, 11);
        const float divisor_factor[12] = {
            1.f/8.f,  1.f/6.f,  1.f/4.f,  3.f/8.f,
            1.f/3.f,  1.f/2.f,  3.f/4.f,  2.f/3.f,
            1.f,      3.f/2.f,  2.f,      4.f
        };
        dt_ms = msPerBeat * divisor_factor[idx];
    } else {
        dt_ms = (float)wire / 127.f * 2000.f;
    }
    float dt = dt_ms * 44.1f;
    CONSTRAIN(dt, 4.0f, 88200.f);
    return (int)dt;
}

void test_fx1_tempo_divisor::expectInt(const char *name, int actual, int expected, int tol) {
    if (std::abs(actual - expected) > tol) {
        std::printf("  FAIL: %s — expected %d ± %d, got %d\n", name, expected, tol, actual);
        failures_++;
    } else {
        std::printf("  ok  : %s = %d (expected %d ± %d)\n", name, actual, expected, tol);
    }
}

int test_fx1_tempo_divisor::DoTest() {
    std::printf("test_fx1_tempo_divisor — preprocessFX1 Sync/Free branches\n");
    failures_ = 0;

    // 120 BPM → msPerBeat = 500 ms.
    const float msPerBeat_120 = 60000.f / 120.f;

    // Test each of the 12 divisors at 120 BPM. Use wire values that land
    // dead-centre in each divisor slot: wire = idx × 128/12 + 5 ≈ middle.
    // Expected samples per divisor at 120 BPM:
    struct DivCase { int wire; const char *label; int expected_samples; };
    const DivCase cases_120[] = {
        {  5, "1/32  @ 120", (int)(500.f / 8.f  * 44.1f)},   //  2 756
        { 16, "1/16T @ 120", (int)(500.f / 6.f  * 44.1f)},   //  3 675
        { 27, "1/16  @ 120", (int)(500.f / 4.f  * 44.1f)},   //  5 512
        { 38, "1/16D @ 120", (int)(500.f * 3.f / 8.f * 44.1f)},  //  8 268
        { 48, "1/8T  @ 120", (int)(500.f / 3.f  * 44.1f)},   //  7 350
        { 59, "1/8   @ 120", (int)(500.f / 2.f  * 44.1f)},   // 11 025
        { 69, "1/8D  @ 120", (int)(500.f * 3.f / 4.f * 44.1f)},  // 16 537
        { 80, "1/4T  @ 120", (int)(500.f * 2.f / 3.f * 44.1f)},  // 14 700
        { 91, "1/4   @ 120", (int)(500.f * 44.1f)},          // 22 050
        {101, "1/4D  @ 120", (int)(500.f * 3.f / 2.f * 44.1f)},  // 33 075
        {112, "1/2   @ 120", (int)(500.f * 2.f * 44.1f)},    // 44 100
        {123, "1/1   @ 120", (int)(500.f * 4.f * 44.1f)},    // 88 200 — clamped to 88200
    };
    for (const auto &c : cases_120) {
        int got = computeDelaySamples(c.wire, true, msPerBeat_120);
        // 1/1 at 120 BPM = exactly 88 200 = the clamp boundary.
        int exp = c.expected_samples;
        if (exp > 88200) exp = 88200;
        expectInt(c.label, got, exp, 2);
    }

    // Edge 1 — highest divisor × slowest BPM (32 BPM → msPerBeat 1875).
    // 1/1 = 4 × 1875 = 7500 ms → 7500 × 44.1 = 330 750 samples → clamped to 88 200.
    {
        float msPerBeat_32 = 60000.f / 32.f;
        int got = computeDelaySamples(123, true, msPerBeat_32);
        expectInt("1/1 @ 32 BPM (clamped)", got, 88200, 0);
    }

    // Edge 2 — shortest divisor × fastest BPM (240 BPM → msPerBeat 250).
    // 1/32 = 250/8 = 31.25 ms → 31.25 × 44.1 = 1 378 samples.
    {
        float msPerBeat_240 = 60000.f / 240.f;
        int got = computeDelaySamples(5, true, msPerBeat_240);
        expectInt("1/32 @ 240 BPM", got, 1378, 2);
    }

    // Free mode at 120 BPM (tempo is irrelevant in Free mode).
    // Wire 0 → 0 ms → clamped to 4 samples (minimum).
    // Wire 127 → 2000 ms → 2000 × 44.1 = 88 200 samples (clamp boundary).
    // Wire 64 → 2000 × 64/127 ≈ 1008 ms → 44 452 samples.
    {
        expectInt("Free wire 0 (clamped to 4)",  computeDelaySamples(0,   false, msPerBeat_120), 4, 0);
        expectInt("Free wire 127 (max)",          computeDelaySamples(127, false, msPerBeat_120), 88200, 2);
        int free_64 = (int)(2000.f * 64.f / 127.f * 44.1f);
        expectInt("Free wire 64 (~1008ms)",       computeDelaySamples(64,  false, msPerBeat_120), free_64, 2);
    }

    // Wire clamp — negative / over-127 atomic inputs must be safe.
    {
        expectInt("wire -1 (clamped to 0)",   computeDelaySamples(-1,  true, msPerBeat_120), computeDelaySamples(0,   true, msPerBeat_120), 0);
        expectInt("wire 200 (clamped to 127)",computeDelaySamples(200, true, msPerBeat_120), computeDelaySamples(127, true, msPerBeat_120), 0);
    }

    std::printf("test_fx1_tempo_divisor — %d failures\n\n", failures_);
    return failures_;
}

} // namespace TESTS
} // namespace CTAG
