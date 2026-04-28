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
#include "RackHH2.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

void RackHH2::Init(const PickSeqRackInitData *initdata) {
    hh2.Init();

    initdata->rack->registerParamAndCC(initdata, "f0", 8, [&](const int val){ f0 = val;});
    initdata->rack->registerParamAndCC(initdata, "tone", 9, [&](const int val){ tone = val;});
    initdata->rack->registerParamAndCC(initdata, "decay", 10, [&](const int val){ decay = val;});
    initdata->rack->registerParamAndCC(initdata, "noise", 11, [&](const int val){ noise = val;});
    initdata->rack->registerParamAndCC(initdata, "accent", 12, [&](const int val){ accent = val;});
    
    this->enabled = false;
}

void RackHH2::trigger() {
    midi_trig = true;
}

void RackHH2::Process(const PicoSeqRackProcessData &data) {
    if (!this->enabled) {
        return;
    }

    std::fill_n(out, BUF_SZ, 0.f);

    bool _trig = midi_trig;
    if (_trig != trig_prev) {
        trig_prev = _trig;
    }
    midi_trig = false;

    MK_FLT_PAR_ABS_NOCV(_accent, accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_f0, f0, 4095.f, .00001f, .1f)
    MK_FLT_PAR_ABS_NOCV(_tone, tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_decay, decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_noise, noise, 4095.f, 1.f)
    hh2.Render(
        false,
        _trig,
        _accent,
        _f0,
        _tone,
        _decay,
        _noise,
        temp1,
        temp2,
        out,
        BUF_SZ);

    if (out[0] != out[0]) {
        // Audio-thread: no printf — see RackABD.cpp note. Recovery via Init below.
        // printf("RackHH2: NaN detected!\n");
        hh2.Init();
    }
}
