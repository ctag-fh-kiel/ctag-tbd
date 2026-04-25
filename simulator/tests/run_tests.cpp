
#include "test_ctagADSREnv.hpp"
#include "test_fx_mixer_pan.hpp"
#include "test_fx1_tempo_divisor.hpp"
#include "test_predelay_ringbuf.hpp"
#include "test_fast_concave.hpp"
#include "test_format_param_value.hpp"
#include "helpers/ctagFastMath.hpp"
#include <cstdio>

using namespace CTAG::TESTS;

int main(int argc, char** argv){
    int total_failures = 0;

    // Legacy ADSR env test (prints values to stdout, no assertion count).
    test_ctagADSREnv testadsr;
    testadsr.DoTest();

    // Phase 8.6 (2026-04-24e) — FX/Master DSP unit tests. Each test
    // returns failure count; overall runner returns non-zero on any fail.
    test_fx_mixer_pan pan;
    total_failures += pan.DoTest();

    test_fx1_tempo_divisor fx1div;
    total_failures += fx1div.DoTest();

    test_predelay_ringbuf predly;
    total_failures += predly.DoTest();

    test_fast_concave concave;
    total_failures += concave.DoTest();

    test_format_param_value fmtpv;
    total_failures += fmtpv.DoTest();

    if (total_failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test failure(s).\n", total_failures);
    }
    return total_failures;
}
