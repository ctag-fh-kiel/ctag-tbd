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
#include "RackDBD.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

void RackDBD::Init(const PickSeqRackInitData *initdata) {
    dbd.Init();

    initdata->rack->registerParamAndCC(initdata, "f0", 8, [&](const int val){ f0 = val; });
    initdata->rack->registerParamAndCC(initdata, "tone", 9, [&](const int val){ tone = val;});
    initdata->rack->registerParamAndCC(initdata, "decay", 10, [&](const int val){ decay = val;});
    initdata->rack->registerParamAndCC(initdata, "dirty", 11, [&](const int val){ dirty = val;});
    initdata->rack->registerParamAndCC(initdata, "fm_env", 12, [&](const int val){ fm_env = val;});
    initdata->rack->registerParamAndCC(initdata, "fm_dcy", 13, [&](const int val){ fm_dcy = val;});
    initdata->rack->registerParamAndCC(initdata, "accent", 14, [&](const int val){ accent = val;});

    this->enabled = false;
}

void RackDBD::trigger() {
    midi_trig = true;
}

void RackDBD::Process(const PicoSeqRackProcessData &data) {
    // MK_BOOL_PAR_NOCV(_trig, trigger)
    bool _trig = false;
    if (midi_trig) {
        _trig = true;
        midi_trig = false;
    }

    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_f0, f0, 4095.f, 0.0005f, 0.01f)

    if (_trig != trig_prev){
        // if (_trig) {
        //     printf("DBD %f hz\n", _f0);
        // }
        trig_prev = _trig;
    }

    if (!this->enabled) {
        return;
    }

    std::fill_n(out, BUF_SZ, 0.f);

    MK_FLT_PAR_ABS_NOCV(_accent, accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_tone, tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_decay, decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_dirty, dirty, 4095.f, 5.f)
    MK_FLT_PAR_ABS_NOCV(_fmEnv, fm_env, 4095.f, 5.f)
    MK_FLT_PAR_ABS_NOCV(_fmDcy, fm_dcy, 4095.f, 4.f)
    dbd.Render(
        false,
        _trig,
        _accent,
        _f0,
        _tone,
        _decay,
        _dirty,
        _fmEnv,
        _fmDcy,
        out,
        BUF_SZ);

    if (out[0] != out[0]) {
        // Audio-thread: no printf — see RackABD.cpp note. Recovery via Init below.
        // printf("RackDBD: NaN detected!\n");
        dbd.Init();
    }
}
