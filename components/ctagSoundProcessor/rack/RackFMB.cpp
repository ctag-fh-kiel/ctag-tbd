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
#include "RackFMB.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

void RackFMB::Init(const PickSeqRackInitData *initdata) {
    fmb.Init();

    initdata->rack->registerParamAndCC(initdata, "f_b", 8, [&](const int val){ f_b = val;});
    initdata->rack->registerParamAndCC(initdata, "d_b", 9, [&](const int val){ d_b = val;});
    initdata->rack->registerParamAndCC(initdata, "f_m", 10, [&](const int val){ f_m = val;});
    initdata->rack->registerParamAndCC(initdata, "d_m", 11, [&](const int val){ d_m = val;});
    initdata->rack->registerParamAndCC(initdata, "b_m", 12, [&](const int val){ b_m = val;});
    initdata->rack->registerParamAndCC(initdata, "A_f", 13, [&](const int val){ A_f = val;});
    initdata->rack->registerParamAndCC(initdata, "d_f", 14, [&](const int val){ d_f = val;});
    initdata->rack->registerParamAndCC(initdata, "I", 15, [&](const int val){ I = val;});
    initdata->rack->registerParamAndCC(initdata, "use_ratio_mode", 16, [&](const int val){ use_ratio_mode = val;}); // not used
	initdata->rack->registerParamAndCC(initdata, "mod_env_sync", 17, [&](const int val){ mod_env_sync = val;}); // not used

    this->enabled = false;
}

void RackFMB::trigger() {
    midi_trig = true;
}

void RackFMB::Process(const PicoSeqRackProcessData &data) {
    // MK_BOOL_PAR_NOCV(_trig, trigger)
    bool _trig = false;
    if (midi_trig) {
        _trig = true;
        midi_trig = false;
    }

    float _f0 = f_b/4095.f * (200.f-20.f)+20.f;
    // if(cv_f_b != -1){
    //     float fMod = data.cv[cv_f_b] * 5.f;
    //     fMod = CTAG::SP::HELPERS::fastpow2(fMod);
    //     _f0 *= fMod;
    // }
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_d_b, d_b, 4095.f, 0.001f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_f_m, f_m, 4095.f, 40.f, 2000.f)
    MK_FLT_PAR_ABS_NOCV(_modindex, f_m, 4095.f, 63.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_I, I, 4095.f, 0.f, 10.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_d_m, d_m, 4095.f, 0.001f, .5f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_A_f, A_f, 4095.f, 0.f, 1000.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_d_f, d_f, 4095.f, 0.001f, .1f)

    if (_trig != trig_prev) {
        if (_trig) {
            // printf("FMB %1.1f %1.1f %1.1f %1.1f %1.1f %1.1f %1.1f\n",
            //     _f0, _d_b, _f_m, _d_m, _A_f, _d_f, _I);
            fmb.Trigger();
        }
        trig_prev = _trig;
    }

    if (!this->enabled) {
        return;
    }

    std::fill_n(out, BUF_SZ, 0.f);

    MK_BOOL_PAR_NOCV(_use_ratio_mode, use_ratio_mode)
    MK_BOOL_PAR_NOCV(_mod_env_sync, mod_env_sync)
    int iModIndex = static_cast<int>(_modindex);
    CONSTRAIN(iModIndex, 0, 63)
    MK_INT_PAR_NOCV(iModFeedback, b_m, 16.f)

    fmb.params.use_ratio_mode = _use_ratio_mode;
    fmb.params.mod_env_sync = _mod_env_sync;
    fmb.params.f_b = _f0;
    fmb.params.d_b = _d_b;
    fmb.params.f_m = _f_m;
    fmb.params.mod_ratio_index = iModIndex;
    fmb.params.I = _I;
    fmb.params.d_m = _d_m;
    fmb.params.b_m = static_cast<float>(iModFeedback);
    fmb.params.A_f = _A_f;
    fmb.params.d_f = _d_f;

    fmb.Process(out, BUF_SZ);
    if (out[0] != out[0]) {
        printf("DrumRackCL: NaN detected!\n");
        fmb.Init();
    }
}
