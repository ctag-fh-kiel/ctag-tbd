//
// Phase 8.6 (2026-04-24e) — FastConcaveTransfer regression test.
//
// Links against the real CTAG::SP::HELPERS::FastConcaveTransfer so the
// test breaks if the upstream helper drifts (Phase 2 cherry-picked this
// from upstream DrumRack commit 38f2975e; the Pico's Width knob feel
// depends on its exact shape).
//

#pragma once

namespace CTAG {
namespace TESTS {

class test_fast_concave {
  public:
    int DoTest();

  private:
    void expectNear(const char *name, float actual, float expected, float tol);

    int failures_ = 0;
};

} // namespace TESTS
} // namespace CTAG
