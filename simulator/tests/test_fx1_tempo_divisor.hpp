//
// Phase 8.6 — FX1 tempo-divisor math regression test.
//
// Mirrors preprocessFX1()'s Sync-branch math from
// ctagSoundProcessorPicoSeqRack.cpp. Asserts delaySamples for all 12
// musical divisors at 3 representative BPMs, plus the clamp edge cases.
//

#pragma once

namespace CTAG {
namespace TESTS {

class test_fx1_tempo_divisor {
  public:
    int DoTest();

  private:
    int computeDelaySamples(int wire_time, bool sync, float msPerBeat);
    void expectInt(const char *name, int actual, int expected, int tol);
    int failures_ = 0;
};

} // namespace TESTS
} // namespace CTAG
