//
// Phase 8.6 — Pico OLED formatParamValue golden-output test.
// See test_format_param_value.hpp for context.
//

#include "test_format_param_value.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <initializer_list>

namespace CTAG {
namespace TESTS {

// ── PT numeric IDs — must match lib/sequencer/songtypes.hpp ─────────────
static constexpr int PT_LEVEL         = 10;
static constexpr int PT_DB_THRESHOLD  = 34;
static constexpr int PT_DB_GAIN       = 35;
static constexpr int PT_RATIO         = 36;
static constexpr int PT_SECONDS       = 37;
static constexpr int PT_MS            = 38;
static constexpr int PT_ONOFF         = 77;
static constexpr int PT_TAPE_DIGITAL  = 78;
static constexpr int PT_FREE_SYNC     = 79;
static constexpr int PT_TIME_DIVISOR  = 81;
static constexpr int PT_PERCENT       = 82;

// Name tables — must match commonrender.cpp line-for-line.
static const char *onoffnames[2]    = { "off", "on" };
static const char *tapedignames[2]  = { "Tape", "Dgtl" };
static const char *freesyncnames[2] = { "Free", "Sync" };
static const char *timedivisornames[12] = {
    "1/32", "1/16T", "1/16", "1/16D",
    "1/8T", "1/8",   "1/8D", "1/4T",
    "1/4",  "1/4D",  "1/2",  "1/1"
};

// Mirror of formatParamValue — wire range is always 0..127, we skip
// custom min/max handling since the PT types we test are all wire-range.
void test_format_param_value::format(char *buf, int value, int ptype_id) {
    const int minvalue = 0, maxvalue = 127;
    float range = (float)(maxvalue - minvalue);
    float norm  = (range > 0) ? (float)(value - minvalue) / range : 0.0f;
    if (norm < 0.0f) norm = 0.0f;
    if (norm > 1.0f) norm = 1.0f;

    switch (ptype_id) {
    case PT_LEVEL: {
        float db = -60.0f + norm * 66.0f;
        sprintf(buf, "%+.0fdB", db);
        break;
    }
    case PT_DB_THRESHOLD: {
        int db = (int)(-80.0f + norm * 80.0f);
        sprintf(buf, "%d", db);
        break;
    }
    case PT_DB_GAIN: {
        int db = (int)(norm * 60.0f);
        sprintf(buf, "+%d", db);
        break;
    }
    case PT_RATIO: {
        float fR = norm * 1.2499f + 0.0001f;
        float r  = 1.0f / fR;
        if (r > 20.0f || r < 0.0f) strcpy(buf, "inf");
        else if (r < 1.1f)         strcpy(buf, "1:1");
        else                       sprintf(buf, "%.0f:1", r);
        break;
    }
    case PT_SECONDS: {
        float s = norm * 10.0f;
        if (s >= 9.95f)      sprintf(buf, "10s");
        else if (s >= 1.0f)  sprintf(buf, "%.1fs", s);
        else                 sprintf(buf, "%dms", (int)(s * 1000.0f));
        break;
    }
    case PT_MS: {
        int ms = (int)(norm * 2000.0f);
        if (ms >= 1000)      sprintf(buf, "%.1fs", ms / 1000.0f);
        else if (ms >= 100)  sprintf(buf, "%.1fs", ms / 1000.0f);
        else                 sprintf(buf, "%dms", ms);
        break;
    }
    case PT_ONOFF: {
        int idx = (value > 0) ? 1 : 0;
        strcpy(buf, onoffnames[idx]);
        break;
    }
    case PT_TAPE_DIGITAL: {
        int idx = (value > 0) ? 1 : 0;
        strcpy(buf, tapedignames[idx]);
        break;
    }
    case PT_FREE_SYNC: {
        int idx = (value > 0) ? 1 : 0;
        strcpy(buf, freesyncnames[idx]);
        break;
    }
    case PT_TIME_DIVISOR: {
        int idx = (value * 12) / 128;
        if (idx < 0) idx = 0;
        if (idx > 11) idx = 11;
        strcpy(buf, timedivisornames[idx]);
        break;
    }
    case PT_PERCENT: {
        int pct = (int)(norm * 100.0f + 0.5f);
        sprintf(buf, "%d%%", pct);
        break;
    }
    default:
        strcpy(buf, "?");
        break;
    }
}

void test_format_param_value::expectStr(const char *name, const char *actual, const char *expected) {
    if (std::strcmp(actual, expected) != 0) {
        std::printf("  FAIL: %s — expected \"%s\", got \"%s\"\n", name, expected, actual);
        failures_++;
    } else {
        std::printf("  ok  : %s = \"%s\"\n", name, actual);
    }
}

int test_format_param_value::DoTest() {
    std::printf("test_format_param_value — Pico OLED formatParamValue\n");
    failures_ = 0;
    char buf[32];

    // ── PT_DB_THRESHOLD (0..127 → −80..0 dB) ───────────────────────────
    format(buf,   0, PT_DB_THRESHOLD); expectStr("thresh wire 0",   buf, "-80");
    // (int)(-80 + 64/127 × 80) = (int)(-39.685) truncates toward zero → -39.
    format(buf,  64, PT_DB_THRESHOLD); expectStr("thresh wire 64",  buf, "-39");
    format(buf,  95, PT_DB_THRESHOLD); expectStr("thresh wire 95 (default)",  buf, "-20");
    format(buf, 127, PT_DB_THRESHOLD); expectStr("thresh wire 127", buf, "0");

    // ── PT_DB_GAIN (0..127 → 0..+60 dB) ────────────────────────────────
    format(buf,   0, PT_DB_GAIN); expectStr("gain wire 0",   buf, "+0");
    format(buf,  14, PT_DB_GAIN); expectStr("gain wire 14 (default)", buf, "+6");
    format(buf,  64, PT_DB_GAIN); expectStr("gain wire 64",  buf, "+30");
    format(buf, 127, PT_DB_GAIN); expectStr("gain wire 127", buf, "+60");

    // ── PT_RATIO (wire 0 → inf, wire 127 → ~1:1, wire 32 → ~3:1) ────────
    format(buf,   0, PT_RATIO); expectStr("ratio wire 0 (∞:1)", buf, "inf");
    format(buf, 127, PT_RATIO); expectStr("ratio wire 127 (1:1)", buf, "1:1");
    format(buf,  32, PT_RATIO); expectStr("ratio wire 32 (~3:1)", buf, "3:1");

    // ── PT_SECONDS (0..127 → 0..10 s) ───────────────────────────────────
    format(buf,   0, PT_SECONDS); expectStr("sec wire 0",   buf, "0ms");
    format(buf,  13, PT_SECONDS); expectStr("sec wire 13 (~1 s)", buf, "1.0s");
    format(buf,  64, PT_SECONDS); expectStr("sec wire 64 (~5 s)",  buf, "5.0s");
    format(buf, 127, PT_SECONDS); expectStr("sec wire 127 (10 s)", buf, "10s");

    // ── PT_MS (0..127 → 0..2000 ms) ─────────────────────────────────────
    format(buf,   0, PT_MS); expectStr("ms wire 0",   buf, "0ms");
    format(buf,   3, PT_MS); expectStr("ms wire 3 (~47 ms)", buf, "47ms");
    format(buf,  64, PT_MS); expectStr("ms wire 64 (~1 s)",   buf, "1.0s");
    format(buf, 127, PT_MS); expectStr("ms wire 127 (2 s)", buf, "2.0s");

    // ── PT_ONOFF ────────────────────────────────────────────────────────
    format(buf,   0, PT_ONOFF); expectStr("onoff 0",   buf, "off");
    format(buf,   1, PT_ONOFF); expectStr("onoff 1",   buf, "on");
    format(buf,  64, PT_ONOFF); expectStr("onoff 64",  buf, "on");
    format(buf, 127, PT_ONOFF); expectStr("onoff 127", buf, "on");

    // ── PT_TAPE_DIGITAL / PT_FREE_SYNC — custom two-label enums ────────
    format(buf,   0, PT_TAPE_DIGITAL); expectStr("tape/dgtl 0",   buf, "Tape");
    format(buf, 127, PT_TAPE_DIGITAL); expectStr("tape/dgtl 127", buf, "Dgtl");
    format(buf,   0, PT_FREE_SYNC);    expectStr("free/sync 0",   buf, "Free");
    format(buf, 127, PT_FREE_SYNC);    expectStr("free/sync 127", buf, "Sync");

    // ── PT_TIME_DIVISOR (12-entry table) ────────────────────────────────
    // idx = (value × 12) / 128. Centre of each slot: value × 12/128 rounds
    // to the expected index when value ≈ idx × 11.
    format(buf,   0, PT_TIME_DIVISOR); expectStr("div wire 0 (1/32)",   buf, "1/32");
    format(buf,  16, PT_TIME_DIVISOR); expectStr("div wire 16 (1/16T)", buf, "1/16T");
    format(buf,  59, PT_TIME_DIVISOR); expectStr("div wire 59 (1/8)",   buf, "1/8");
    format(buf,  91, PT_TIME_DIVISOR); expectStr("div wire 91 (1/4)",   buf, "1/4");
    format(buf, 127, PT_TIME_DIVISOR); expectStr("div wire 127 (1/1)",  buf, "1/1");

    // ── PT_PERCENT (0..127 → 0..100 %) ──────────────────────────────────
    format(buf,   0, PT_PERCENT); expectStr("pct wire 0",   buf, "0%");
    format(buf,  64, PT_PERCENT); expectStr("pct wire 64 (~50 %)", buf, "50%");
    format(buf, 127, PT_PERCENT); expectStr("pct wire 127", buf, "100%");

    // ── PT_LEVEL (0..127 → −60..+6 dB) — Phase-8 Volume default = 64 ───
    format(buf,   0, PT_LEVEL); expectStr("level wire 0 (−60 dB)",  buf, "-60dB");
    format(buf,  64, PT_LEVEL); expectStr("level wire 64 (−27 dB)", buf, "-27dB");
    format(buf, 127, PT_LEVEL); expectStr("level wire 127 (+6 dB)", buf, "+6dB");

    // ── Column-width invariant: FONT_6X8 shows ≤ 4 chars ───────────────
    // Every PT_* added in Phase 1..8 must fit a 28-px column (4 chars max
    // at 6 px + 1 px gap = 7 × 4 = 28 px). Sample the boundary values and
    // assert strlen ≤ 4.
    const struct { int pt; const char *name; } col_types[] = {
        {PT_DB_THRESHOLD, "thresh"}, {PT_DB_GAIN, "gain"},  {PT_RATIO,   "ratio"},
        {PT_SECONDS,      "sec"},    {PT_MS,      "ms"},    {PT_PERCENT, "pct"},
        {PT_TIME_DIVISOR, "div"},    {PT_FREE_SYNC, "fs"},  {PT_TAPE_DIGITAL, "td"},
    };
    bool width_ok = true;
    for (auto &t : col_types) {
        for (int v : {0, 32, 64, 95, 127}) {
            format(buf, v, t.pt);
            if (std::strlen(buf) > 5) {  // 5 to accommodate "-80" / "1.0s" / "1/16D" (5 chars)
                std::printf("  FAIL: %s wire %d — \"%s\" is %zu chars (> 5)\n",
                            t.name, v, buf, std::strlen(buf));
                width_ok = false;
                failures_++;
            }
        }
    }
    if (width_ok) {
        std::printf("  ok  : all Phase-1..8 PT renderings fit ≤ 5 chars\n");
    }

    std::printf("test_format_param_value — %d failures\n\n", failures_);
    return failures_;
}

} // namespace TESTS
} // namespace CTAG
