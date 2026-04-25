//
// Phase 8.6 (2026-04-24e) — Pico OLED formatParamValue golden-output test.
//
// Mirrors the Phase-1..8 additions to formatParamValue() in
// ../../tbd-pico-seq3/lib/sequencerui/screens/commonrender.cpp and
// asserts golden strings for the PT types introduced during the FX/Master
// UX overhaul:
//   PT_DB_THRESHOLD, PT_DB_GAIN, PT_RATIO, PT_SECONDS, PT_MS,
//   PT_TAPE_DIGITAL, PT_FREE_SYNC, PT_TIME_DIVISOR, PT_PERCENT, PT_ONOFF.
//
// The strings are the literal bytes the Pico writes to the OLED — any
// change in the format (unit suffix, decimal digits, column width) will
// flag a regression here before it ships.
//

#pragma once

namespace CTAG {
namespace TESTS {

class test_format_param_value {
  public:
    int DoTest();

  private:
    // Pure-function mirror of formatParamValue's Phase-1..8 switch arms.
    // `ptype_id` matches the numeric IDs in songtypes.hpp (34,35,36,37,38,
    // 39,78,79,81,82, plus existing PT_LEVEL=17).
    void format(char *buf, int value, int ptype_id);

    void expectStr(const char *name, const char *actual, const char *expected);

    int failures_ = 0;
};

} // namespace TESTS
} // namespace CTAG
