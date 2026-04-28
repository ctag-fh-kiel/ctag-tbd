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
#include "RackDSD.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

void RackDSD::Init(const PickSeqRackInitData *initdata) {
    dsd.Init();

	initdata->rack->registerParamAndCC(initdata, "f0", 8, [&](const int val){ f0 = val;});
    initdata->rack->registerParamAndCC(initdata, "decay", 9, [&](const int val){ decay = val;});
    initdata->rack->registerParamAndCC(initdata, "fm_amt", 10, [&](const int val){ fm_amt = val;});
    initdata->rack->registerParamAndCC(initdata, "spy", 11, [&](const int val){ spy = val;});
	initdata->rack->registerParamAndCC(initdata, "accent", 12, [&](const int val){ accent = val;});

    this->enabled = false;
}

void RackDSD::trigger() {
    midi_trig = true;
}

void RackDSD::Process(const PicoSeqRackProcessData &data) {
    // MK_BOOL_PAR_NOCV(_trig, trigger)
    bool _trig = false;
    if (midi_trig) {
        _trig = true;
        midi_trig = false;
    }
    if (_trig != trig_prev){
        // if (_trig) {
        //     printf("DSD\n");
        // }
        trig_prev = _trig;
    }   

    if (!this->enabled) {
        return;
    }

    std::fill_n(out, BUF_SZ, 0.f);

    MK_FLT_PAR_ABS_NOCV(_accent, accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_f0, f0, 4095.f, 0.0008f, 0.01f)
    MK_FLT_PAR_ABS_NOCV(_fmAmt, fm_amt, 4095.f, 1.5f)
    MK_FLT_PAR_ABS_NOCV(_decay, decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_spy, spy, 4095.f, 1.f)
    dsd.Render(
        false,
        _trig,
        _accent,
        _f0,
        _fmAmt,
        _decay,
        _spy,
        out,
        BUF_SZ);

    if (out[0] != out[0]) {
        // Audio-thread: no printf — see RackABD.cpp note. Recovery via Init below.
        // printf("RackDSD: NaN detected!\n");
        dsd.Init();
    }
}
