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
#include "RackABD.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

void RackABD::Init(const PickSeqRackInitData *initdata) {
    abd.Init();

    initdata->rack->registerParamAndCC(initdata, "f0", 8, [&](const int val){ f0 = val;});
    initdata->rack->registerParamAndCC(initdata, "tone", 9, [&](const int val){ tone = val;});
	initdata->rack->registerParamAndCC(initdata, "decay", 10, [&](const int val){ decay = val;});
    initdata->rack->registerParamAndCC(initdata, "a_fm", 11, [&](const int val){ a_fm = val;});
    initdata->rack->registerParamAndCC(initdata, "s_fm", 12, [&](const int val){ s_fm = val;});
    initdata->rack->registerParamAndCC(initdata, "accent", 13, [&](const int val){ accent = val;});
    
    this->enabled = false;
}

void RackABD::trigger() {
    midi_trig = true;
}

void RackABD::Process(const PicoSeqRackProcessData &data) {
    // MK_BOOL_PAR_NOCV(_trig, trigger)
    bool _trig = false;
    if (midi_trig) {
        _trig = true;
        midi_trig = false;
    }
    if (_trig != trig_prev){
        // if (_trig) {
        //     printf("ABD\n");
        // }
        trig_prev = _trig;
    }

    if (!this->enabled) {
        return;
    }

    std::fill_n(out, BUF_SZ, 0.f);

    MK_FLT_PAR_ABS_NOCV(_accent, accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_f0, f0, 4095.f, 0.0001f, 0.01f)
    MK_FLT_PAR_ABS_NOCV(_tone, tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_decay, decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_a_fm, a_fm, 4095.f, 0.f, 100.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_s_fm, s_fm, 4095.f, 0.f, 100.f)
    abd.Render(
        false,
        _trig,
        _accent,
        _f0,
        _tone,
        _decay,
        _a_fm,
        _s_fm,
        out,
        BUF_SZ);

    if (out[0] != out[0]) {
        printf("RackABD: NaN detected!\n");
        abd.Init();
    }
}
