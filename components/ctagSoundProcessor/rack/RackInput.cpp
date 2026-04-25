/***************
TBD-16 — Macro/Preset System & PicoSeqRack

(c) 2025-2026 Per-Olov Jernberg (possan). https://possan.codes

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

dadamachines has a commercial license to use this code in the TBD-16 product.
Other commercial use requires a separate license agreement.

Provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "RackSynth.hpp"
#include "RackInput.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"
#include <cmath>

using namespace CTAG::SP;

void RackInput::Init(const PickSeqRackInitData *initdata) {
    this->enabled = false;
    in_gain = 2925;   // unity gain (0 dB at norm ≈ 0.714, val = 2925/4095)
    in_mono = 0;
    in_hp = 0;
    in_drive = 0;
    in_ftype = 0;     // Off by default — explicit filter bypass on a fresh input
    in_fcutoff = 4095; // cutoff fully open (12 kHz) as a neutral starting point
    in_freso = 0;     // no resonance
    in_fenv = 0;      // no envelope modulation
    hp_x[0] = hp_x[1] = 0.f;
    hp_y[0] = hp_y[1] = 0.f;
    svf_ic1[0] = svf_ic1[1] = 0.f;
    svf_ic2[0] = svf_ic2[1] = 0.f;
    env_level = 0.f;

    // Register 8 params on ctrls 8..15. When the macro writes to these via
    // the MacroTranslator fan-out, the callback receives a 0..4095-scaled
    // integer value (see ctagSoundProcessorPicoSeqRack::handleMidiControl*).
    initdata->rack->registerParamAndCC(initdata, "in_gain",     8,  [&](const int val){ in_gain    = val; });
    initdata->rack->registerParamAndCC(initdata, "in_mono",     9,  [&](const int val){ in_mono    = val; });
    initdata->rack->registerParamAndCC(initdata, "in_hp",       10, [&](const int val){ in_hp      = val; });
    initdata->rack->registerParamAndCC(initdata, "in_drive",    11, [&](const int val){ in_drive   = val; });
    initdata->rack->registerParamAndCC(initdata, "in_ftype",    12, [&](const int val){ in_ftype   = val; });
    initdata->rack->registerParamAndCC(initdata, "in_fcutoff",  13, [&](const int val){ in_fcutoff = val; });
    initdata->rack->registerParamAndCC(initdata, "in_freso",    14, [&](const int val){ in_freso   = val; });
    initdata->rack->registerParamAndCC(initdata, "in_fenv",     15, [&](const int val){ in_fenv    = val; });
}

void RackInput::Process(const PicoSeqRackProcessData &data) {
    if (!this->enabled) {
        return;
    }

    // Compute per-block coefficients (param values only change between blocks,
    // not per-sample). All params are 0..4095 normalised.
    // Gain: matches the PT_GAIN_LEVEL display on the Input param page.
    // Formula: db = -60 + norm × 84, giving -60..+24 dB range. Unity (0 dB)
    // sits at norm ≈ 0.714. DSP math is dB-linear so what's displayed is
    // what gets applied. The wider +24 dB ceiling (vs TR.MIX LEVEL's +6)
    // accommodates weak sources like dynamic mics and passive instrument DIs.
    // See commonrender.cpp PT_GAIN_LEVEL case for the display formula.
    const float gain_norm = (float)in_gain / 4095.f;
    const float gain_db = -60.f + gain_norm * 84.f;
    const float gain = powf(10.f, gain_db / 20.f);

    const bool mono = in_mono > 2048;

    // HP cutoff 0..500 Hz, linear in the DSP; macro's curve="log" at the
    // authoring layer handles the perceptual feel. Below ~1 Hz we bypass.
    const float fs = 44100.f;
    const float hp_fc = (float)in_hp / 4095.f * 500.f;
    const bool hp_enabled = hp_fc > 1.0f;
    // One-pole HP filter alpha = RC / (RC + dt) = 1 / (1 + 2π·fc·dt).
    const float hp_alpha = hp_enabled
        ? (1.f / (1.f + 2.f * (float)M_PI * hp_fc / fs))
        : 1.f;

    // Drive maps 0..4095 → 1×..4× tanh drive.
    const float drive = 1.f + (float)in_drive / 4095.f * 3.f;
    const bool drive_enabled = drive > 1.02f;

    // Filter mode: matches PT_FILTER_MODE rendering — 4 modes spread across
    // 0..4095. 0 = Off (bypass), 1 = LP, 2 = BP, 3 = HP. Keeping the
    // thresholds aligned with the 4-mode display so the OLED label always
    // agrees with what the DSP is doing.
    const int fmode = (in_ftype < 1024) ? 0
                    : (in_ftype < 2048) ? 1
                    : (in_ftype < 3072) ? 2
                    : 3;

    // Base SVF cutoff — log sweep 20 Hz .. 12 kHz, same curve as the
    // PT_FILTER_CUTOFF display (20 * 1000^norm → ~20..20kHz, clamped to
    // 12 kHz to stay safely below Nyquist at 44.1 kHz).
    const float cutoff_norm = (float)in_fcutoff / 4095.f;
    float base_fc = 20.f * powf(1000.f, cutoff_norm);
    if (base_fc > 12000.f) base_fc = 12000.f;

    // Envelope-to-cutoff amount. 0..4095 → 0..1 (unidirectional positive).
    // At max amount + max envelope, cutoff opens up to ~4 octaves above
    // the base. Negative modulation intentionally not exposed — keeps the
    // knob UX simple; users that want inverse can ride the base cutoff.
    const float env_amount = (float)in_fenv / 4095.f;

    // Envelope follower coefficients — 5 ms attack, 100 ms release give
    // a musical "auto-wah" feel without sounding gated.
    // coef = 1 - exp(-1 / (time_sec * fs)).
    const float atk_coef = 1.f - expf(-1.f / (0.005f * fs));
    const float rel_coef = 1.f - expf(-1.f / (0.100f * fs));

    // SVF TPT (Cytomic / Andy Simper) coefficients. q range is 0.5..~20
    // so 1/q covers mild to self-oscillating resonance. Clamp so the
    // filter never self-osc (1/q below ~0.1 gets squirrely at fs=44.1k).
    const float reso_norm = (float)in_freso / 4095.f;
    const float q = 0.5f + reso_norm * 19.5f;
    const float k = 1.f / q;  // damping; smaller = more resonance

    const float *in = data.inputbuffer;
    for (int i = 0; i < BUF_SZ; i++) {
        float l = in[i * 2];
        float r = in[i * 2 + 1];

        if (mono) {
            const float m = (l + r) * 0.5f;
            l = m;
            r = m;
        }

        l *= gain;
        r *= gain;

        if (hp_enabled) {
            // y[n] = alpha * (y[n-1] + x[n] - x[n-1])
            const float yl = hp_alpha * (hp_y[0] + l - hp_x[0]);
            const float yr = hp_alpha * (hp_y[1] + r - hp_x[1]);
            hp_x[0] = l; hp_y[0] = yl; l = yl;
            hp_x[1] = r; hp_y[1] = yr; r = yr;
        }

        if (drive_enabled) {
            l = tanhf(l * drive);
            r = tanhf(r * drive);
        }

        if (fmode > 0) {
            // Update envelope follower from the mono-summed post-drive signal.
            // Full-wave rectification (fabs) feeds the one-pole with
            // asymmetric attack/release. env_level tracks a rough signal
            // magnitude in [0..~1+].
            const float src_abs = fabsf((l + r) * 0.5f);
            const float coef = (src_abs > env_level) ? atk_coef : rel_coef;
            env_level += coef * (src_abs - env_level);
            // Clamp to avoid runaway if the user cranks gain — the multiplier
            // below multiplies cutoff by up to 16× at max env + max amount.
            if (env_level > 2.f) env_level = 2.f;

            // Modulate cutoff by up to 4 octaves (factor 16) at max env and
            // max amount. The exponential mapping keeps the sweep musical:
            // mod_fc = base_fc * 2^(amount * env_level * 4).
            float mod_fc = base_fc * powf(2.f, env_amount * env_level * 4.f);
            if (mod_fc > 18000.f) mod_fc = 18000.f;
            if (mod_fc < 20.f)    mod_fc = 20.f;

            // TPT SVF per-sample step. g is prewarped; at high fc the
            // one-sided tan goes unstable, hence the cutoff clamp above.
            const float g = tanf((float)M_PI * mod_fc / fs);
            const float a1 = 1.f / (1.f + g * (g + k));
            const float a2 = g * a1;
            const float a3 = g * a2;

            // Channel 0 (L)
            {
                const float v0 = l;
                const float v3 = v0 - svf_ic2[0];
                const float v1 = a1 * svf_ic1[0] + a2 * v3;
                const float v2 = svf_ic2[0] + a2 * svf_ic1[0] + a3 * v3;
                svf_ic1[0] = 2.f * v1 - svf_ic1[0];
                svf_ic2[0] = 2.f * v2 - svf_ic2[0];
                switch (fmode) {
                    case 1: l = v2; break;                    // LP
                    case 2: l = v1; break;                    // BP
                    case 3: l = v0 - k * v1 - v2; break;      // HP
                }
            }
            // Channel 1 (R)
            {
                const float v0 = r;
                const float v3 = v0 - svf_ic2[1];
                const float v1 = a1 * svf_ic1[1] + a2 * v3;
                const float v2 = svf_ic2[1] + a2 * svf_ic1[1] + a3 * v3;
                svf_ic1[1] = 2.f * v1 - svf_ic1[1];
                svf_ic2[1] = 2.f * v2 - svf_ic2[1];
                switch (fmode) {
                    case 1: r = v2; break;                    // LP
                    case 2: r = v1; break;                    // BP
                    case 3: r = v0 - k * v1 - v2; break;      // HP
                }
            }
        }

        out[i * 2]     = l;
        out[i * 2 + 1] = r;
    }
}
