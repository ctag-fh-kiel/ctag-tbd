//
// Phase 8.6 — FastConcaveTransfer regression test.
// See test_fast_concave.hpp for context.
//

#include "test_fast_concave.hpp"

#include "helpers/ctagFastMath.hpp"

#include <cmath>
#include <cstdio>
#include <initializer_list>

namespace CTAG {
namespace TESTS {

void test_fast_concave::expectNear(const char *name, float actual, float expected, float tol) {
    if (std::fabs(actual - expected) > tol) {
        std::printf("  FAIL: %s — expected %.6f ± %.6f, got %.6f\n",
                    name, expected, tol, actual);
        failures_++;
    } else {
        std::printf("  ok  : %s = %.6f (expected %.6f ± %.6f)\n",
                    name, actual, expected, tol);
    }
}

int test_fast_concave::DoTest() {
    using CTAG::SP::HELPERS::FastConcaveTransfer;

    std::printf("test_fast_concave — FastConcaveTransfer endpoints + monotonic\n");
    failures_ = 0;

    // ── endpoints: shape is identity on {0, 1} regardless of k ─────────
    for (float k : {0.03f, 0.06f, 0.10f}) {
        char name[64];
        std::snprintf(name, sizeof(name), "x=0, k=%.2f → 0", k);
        expectNear(name, FastConcaveTransfer(0.f, k), 0.f, 1e-7f);
        std::snprintf(name, sizeof(name), "x=1, k=%.2f → 1", k);
        expectNear(name, FastConcaveTransfer(1.f, k), 1.f, 1e-7f);
    }

    // ── documented intermediate values from the docstring ──────────────
    // k = 0.03  → x=0.1 → y≈0.79
    // k = 0.06  → x=0.1 → y≈0.65  (matches upstream usage)
    // k = 0.10  → x=0.1 → y≈0.53
    // Exact formula: y = x / (x + k(1-x))
    //   k=0.03, x=0.1 → 0.1 / (0.1 + 0.027) = 0.7874...
    //   k=0.06, x=0.1 → 0.1 / (0.1 + 0.054) = 0.6494...
    //   k=0.10, x=0.1 → 0.1 / (0.1 + 0.090) = 0.5263...
    expectNear("k=0.03, x=0.1 (aggressive)", FastConcaveTransfer(0.1f, 0.03f), 0.7874f, 0.001f);
    expectNear("k=0.06, x=0.1 (upstream)",    FastConcaveTransfer(0.1f, 0.06f), 0.6494f, 0.001f);
    expectNear("k=0.10, x=0.1 (moderate)",    FastConcaveTransfer(0.1f, 0.10f), 0.5263f, 0.001f);

    // ── midpoint ───────────────────────────────────────────────────────
    // At x=0.5: y = 0.5 / (0.5 + 0.5k) = 1 / (1 + k).
    expectNear("k=0.06, x=0.5 → 1/(1+k)",  FastConcaveTransfer(0.5f, 0.06f), 1.f/(1.f+0.06f), 1e-5f);

    // ── concavity: y ≥ x for k < 1 on (0,1) ────────────────────────────
    // Width knob depends on early rise above the x=y line.
    bool all_concave = true;
    for (int i = 1; i < 100; i++) {
        float x = (float)i / 100.f;
        float y = FastConcaveTransfer(x, 0.06f);
        if (y < x - 1e-6f) {
            std::printf("  FAIL: concavity violated at x=%.3f (y=%.4f)\n", x, y);
            all_concave = false;
            break;
        }
    }
    if (all_concave) {
        std::printf("  ok  : k=0.06 curve is concave (y ≥ x on (0,1))\n");
    } else {
        failures_++;
    }

    // ── monotonicity: y strictly increases on [0,1] ────────────────────
    for (float k : {0.03f, 0.06f, 0.10f}) {
        float prev = -1.f;
        bool mono = true;
        for (int i = 0; i <= 100; i++) {
            float x = (float)i / 100.f;
            float y = FastConcaveTransfer(x, k);
            if (y < prev - 1e-6f) { mono = false; break; }
            prev = y;
        }
        if (mono) {
            std::printf("  ok  : k=%.2f curve is monotonically non-decreasing\n", k);
        } else {
            std::printf("  FAIL: k=%.2f curve is NOT monotonic\n", k);
            failures_++;
        }
    }

    // ── "aggressive-er k → larger y for same x" ordering ───────────────
    // Smaller k = more aggressive lift. y(0.1, 0.03) > y(0.1, 0.06) > y(0.1, 0.10).
    float y03 = FastConcaveTransfer(0.1f, 0.03f);
    float y06 = FastConcaveTransfer(0.1f, 0.06f);
    float y10 = FastConcaveTransfer(0.1f, 0.10f);
    if (y03 > y06 && y06 > y10) {
        std::printf("  ok  : smaller k → larger lift at x=0.1 (%.4f > %.4f > %.4f)\n", y03, y06, y10);
    } else {
        std::printf("  FAIL: k-ordering broken: y03=%.4f y06=%.4f y10=%.4f\n", y03, y06, y10);
        failures_++;
    }

    std::printf("test_fast_concave — %d failures\n\n", failures_);
    return failures_;
}

} // namespace TESTS
} // namespace CTAG
