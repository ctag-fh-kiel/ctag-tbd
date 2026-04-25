//
// Phase 8.6 — FX2 pre-delay ring-buffer regression test.
// See test_predelay_ringbuf.hpp for context.
//

#include "test_predelay_ringbuf.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

namespace CTAG {
namespace TESTS {

// Mirror the buf size constant from ctagSoundProcessorPicoSeqRack.hpp.
// 8820 samples = 200 ms at 44.1 kHz.
static constexpr int kPreDelayBufSize = 8820;

int test_predelay_ringbuf::computePreDelaySamples(int predly_raw_atomic) {
    int predly_raw = predly_raw_atomic;
    if (predly_raw < 0) predly_raw = 0;
    if (predly_raw > 4095) predly_raw = 4095;
    int preDelaySamples = (predly_raw * 200 * 441) / (4095 * 10);
    return preDelaySamples;
}

// Pure-function mirror of the inner block loop in the pre-delay stage.
// Mono-sums inout_l + inout_r into the ring, reads delayed, writes back
// into both channels. Increments writeIdx and wraps at bufSize.
void test_predelay_ringbuf::processBlock(float *buf, int bufSize, int &writeIdx,
                                         int preDelaySamples,
                                         float *inout_l, float *inout_r,
                                         int blockSize) {
    if (preDelaySamples < 2) {
        // bypass — DSP leaves rev_buf untouched. Do the same here.
        return;
    }
    if (preDelaySamples >= bufSize) preDelaySamples = bufSize - 1;
    for (int i = 0; i < blockSize; i++) {
        float preIn = 0.5f * (inout_l[i] + inout_r[i]);
        buf[writeIdx] = preIn;
        int readIdx = writeIdx - preDelaySamples;
        if (readIdx < 0) readIdx += bufSize;
        float preOut = buf[readIdx];
        inout_l[i] = preOut;
        inout_r[i] = preOut;
        writeIdx++;
        if (writeIdx >= bufSize) writeIdx = 0;
    }
}

void test_predelay_ringbuf::expectInt(const char *name, int actual, int expected) {
    if (actual != expected) {
        std::printf("  FAIL: %s — expected %d, got %d\n", name, expected, actual);
        failures_++;
    } else {
        std::printf("  ok  : %s = %d\n", name, actual);
    }
}

void test_predelay_ringbuf::expectNear(const char *name, float actual, float expected, float tol) {
    if (std::fabs(actual - expected) > tol) {
        std::printf("  FAIL: %s — expected %.6f ± %.6f, got %.6f\n",
                    name, expected, tol, actual);
        failures_++;
    } else {
        std::printf("  ok  : %s = %.6f\n", name, actual);
    }
}

int test_predelay_ringbuf::DoTest() {
    std::printf("test_predelay_ringbuf — FX2 pre-delay ring buffer\n");
    failures_ = 0;

    // ── atomic → samples mapping ───────────────────────────────────────
    // At atomic 0: 0 samples (bypass).
    expectInt("atomic 0 → samples", computePreDelaySamples(0), 0);

    // At atomic 4095 (clamp ceiling): (4095 × 200 × 441) / (4095 × 10) = 8820.
    // DSP then clamps to bufSize - 1 = 8819 inside the else branch.
    expectInt("atomic 4095 → samples (pre-clamp)", computePreDelaySamples(4095), 8820);

    // At wire 127 / atomic 4064 (real-world max from MK_FLT_PAR_ABS_NOCV):
    // (4064 × 200 × 441) / 40950 = 358 444 800 / 40950 = 8753 samples ≈ 198.5 ms.
    expectInt("atomic 4064 (wire 127) → samples", computePreDelaySamples(4064), 8753);

    // At atomic 2048 (~wire 64 centre): ≈ 4 411 samples ≈ 100 ms.
    expectInt("atomic 2048 (wire 64) → samples", computePreDelaySamples(2048), 4411);

    // Negative atomic safely clamps to 0.
    expectInt("atomic -50 (underflow) → samples", computePreDelaySamples(-50), 0);

    // Overshoot past 4095 clamps to 4095.
    expectInt("atomic 5000 (overflow) → samples", computePreDelaySamples(5000), 8820);

    // ── bypass path: < 2 samples leaves inout untouched ────────────────
    {
        std::vector<float> buf(kPreDelayBufSize, 0.f);
        int writeIdx = 0;
        float L[32], R[32];
        for (int i = 0; i < 32; i++) { L[i] = 1.f; R[i] = -1.f; }
        processBlock(buf.data(), kPreDelayBufSize, writeIdx,
                     /*preDelaySamples=*/1, L, R, 32);
        expectNear("bypass preserves L[0]", L[0], 1.f, 0.f);
        expectNear("bypass preserves R[31]", R[31], -1.f, 0.f);
        expectInt("bypass leaves writeIdx at 0", writeIdx, 0);
    }

    // ── smallest active delay (2 samples) ──────────────────────────────
    // Write an impulse at i=0, then after 2 samples it should come back.
    {
        std::vector<float> buf(kPreDelayBufSize, 0.f);
        int writeIdx = 0;
        float L[32], R[32];
        for (int i = 0; i < 32; i++) { L[i] = 0.f; R[i] = 0.f; }
        L[0] = 1.f; R[0] = 1.f;  // impulse at i=0, mono sum = 1
        processBlock(buf.data(), kPreDelayBufSize, writeIdx,
                     /*preDelaySamples=*/2, L, R, 32);
        // After the loop, samples 0..1 are zero (impulse not yet arrived),
        // sample 2 = 1.0 (impulse appears two samples late).
        expectNear("preDly=2 L[0] (not yet)", L[0], 0.f, 1e-6f);
        expectNear("preDly=2 L[1] (not yet)", L[1], 0.f, 1e-6f);
        expectNear("preDly=2 L[2] (impulse)", L[2], 1.f, 1e-6f);
        expectNear("preDly=2 R[2] (impulse)", R[2], 1.f, 1e-6f);
        // writeIdx advanced by 32 blockSize.
        expectInt("preDly=2 writeIdx after 1 block", writeIdx, 32);
    }

    // ── delay of exactly blockSize (32) — impulse appears next block ──
    {
        std::vector<float> buf(kPreDelayBufSize, 0.f);
        int writeIdx = 0;
        float L[32], R[32];
        for (int i = 0; i < 32; i++) { L[i] = 0.f; R[i] = 0.f; }
        L[0] = 1.f; R[0] = 1.f;
        processBlock(buf.data(), kPreDelayBufSize, writeIdx,
                     /*preDelaySamples=*/32, L, R, 32);
        // With delaySamples=32, in-block output is all zero (nothing is 32
        // samples old yet because writeIdx only has 32 writes).
        for (int i = 0; i < 32; i++) {
            if (std::fabs(L[i]) > 1e-6f) {
                std::printf("  FAIL: preDly=32 L[%d] expected 0, got %.6f\n", i, L[i]);
                failures_++;
            }
        }
        // Run a second block of silence — impulse should emerge at i=0 of block 2.
        for (int i = 0; i < 32; i++) { L[i] = 0.f; R[i] = 0.f; }
        processBlock(buf.data(), kPreDelayBufSize, writeIdx,
                     /*preDelaySamples=*/32, L, R, 32);
        expectNear("preDly=32 block-2 L[0] (impulse)", L[0], 1.f, 1e-6f);
        expectInt("preDly=32 writeIdx after 2 blocks", writeIdx, 64);
    }

    // ── ring wrap across the 8820 boundary ─────────────────────────────
    // Seed writeIdx near the end, write an impulse, confirm that reading
    // with preDelaySamples that would push readIdx negative wraps correctly.
    {
        std::vector<float> buf(kPreDelayBufSize, 0.f);
        int writeIdx = kPreDelayBufSize - 16;  // 16 samples from the end
        // Pre-seed a known value at offset that should be picked up after wrap.
        buf[kPreDelayBufSize - 16 - 100] = 0.f;  // readIdx when wrapping back
        // Now after 1 block write (32 samples starting at 8804), writeIdx
        // will wrap to 8804 + 32 - 8820 = 16. With preDelaySamples=1000
        // and first-iteration writeIdx=8804, readIdx = 8804 - 1000 = 7804.
        float L[32], R[32];
        for (int i = 0; i < 32; i++) { L[i] = (float)i; R[i] = (float)i; }
        processBlock(buf.data(), kPreDelayBufSize, writeIdx,
                     /*preDelaySamples=*/1000, L, R, 32);
        expectInt("wrap: writeIdx wraps past end", writeIdx, 16);
    }

    // Explicit wrap correctness: seed a single impulse in the buffer at a
    // known index, set writeIdx so that one block's read pulls from that
    // index via the wrap branch (readIdx < 0 + bufSize).
    {
        std::vector<float> buf(kPreDelayBufSize, 0.f);
        // Seed an impulse at the far end of the buffer.
        buf[kPreDelayBufSize - 10] = 0.5f;
        int writeIdx = 0;  // Fresh write at 0.
        int preDelaySamples = 10;  // So readIdx = 0 - 10 = -10 → 8820 - 10 = 8810.
        float L[32], R[32];
        for (int i = 0; i < 32; i++) { L[i] = 0.f; R[i] = 0.f; }
        processBlock(buf.data(), kPreDelayBufSize, writeIdx,
                     preDelaySamples, L, R, 32);
        // First iteration: readIdx = 0 - 10 + 8820 = 8810. buf[8810] = 0.5.
        expectNear("wrap: L[0] picks up impulse from tail", L[0], 0.5f, 1e-6f);
        expectNear("wrap: R[0] mirror", R[0], 0.5f, 1e-6f);
        // Iteration 1: writeIdx now 1, readIdx = 1-10+8820 = 8811 = 0 (seeded 0).
        expectNear("wrap: L[1] (zero)", L[1], 0.f, 1e-6f);
    }

    // ── clamp: preDelaySamples >= bufSize is clamped to bufSize - 1 ────
    // With preDelaySamples=kPreDelayBufSize, the else-branch clamps to 8819.
    // First iteration: readIdx = 0 - 8819 + 8820 = 1 → buf[1] (zero).
    // Subsequent iterations read buf[2], buf[3]... After the block, writeIdx=32.
    {
        std::vector<float> buf(kPreDelayBufSize, 0.f);
        // Pre-seed specific values at indices 1..32 so we can verify the read path.
        for (int i = 1; i <= 32; i++) buf[i] = 0.25f * (float)i;
        int writeIdx = 0;
        float L[32], R[32];
        // Input is silence so the write doesn't perturb readback.
        for (int i = 0; i < 32; i++) { L[i] = 0.f; R[i] = 0.f; }
        processBlock(buf.data(), kPreDelayBufSize, writeIdx,
                     /*preDelaySamples=*/kPreDelayBufSize, L, R, 32);
        // i=0: readIdx = 0 - 8819 + 8820 = 1, then write 0 at buf[0].
        //   L[0] should be seeded buf[1] = 0.25.
        expectNear("clamp: L[0] = buf[1]", L[0], 0.25f, 1e-6f);
        // i=1: readIdx = 1 - 8819 + 8820 = 2 → 0.5.
        expectNear("clamp: L[1] = buf[2]", L[1], 0.5f, 1e-6f);
        // i=31: readIdx = 31 - 8819 + 8820 = 32 → 8.0.
        expectNear("clamp: L[31] = buf[32]", L[31], 8.0f, 1e-6f);
    }

    std::printf("test_predelay_ringbuf — %d failures\n\n", failures_);
    return failures_;
}

} // namespace TESTS
} // namespace CTAG
