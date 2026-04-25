//
// Phase 8.6 (2026-04-24e) — FX2 pre-delay ring-buffer regression test.
//
// Mirrors the ring-buffer implementation in renderMasterOutput's pre-delay
// stage (ctagSoundProcessorPicoSeqRack.cpp, "Phase 8 — pre-delay stage"
// block). Asserts:
//   - atomic → samples mapping at min / max / bypass boundary
//   - samples >= buf size is clamped to buf size - 1
//   - write/read behaviour produces the expected delayed output
//   - ring wraps correctly across the 8820-sample boundary
//   - bypass path leaves input untouched (< 2 samples)
//

#pragma once

namespace CTAG {
namespace TESTS {

class test_predelay_ringbuf {
  public:
    int DoTest();

  private:
    // Pure-function mirror of the atomic → preDelaySamples math.
    int computePreDelaySamples(int predly_raw_atomic);

    // Run the 32-sample-block ring-buffer process against a local buffer,
    // mutating writeIdx and the buf contents exactly like the DSP does.
    // Returns the output block written back into inout_l/inout_r.
    void processBlock(float *buf, int bufSize, int &writeIdx,
                      int preDelaySamples,
                      float *inout_l, float *inout_r, int blockSize);

    void expectInt(const char *name, int actual, int expected);
    void expectNear(const char *name, float actual, float expected, float tol);

    int failures_ = 0;
};

} // namespace TESTS
} // namespace CTAG
