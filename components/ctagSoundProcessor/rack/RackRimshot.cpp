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
#include "RackRimshot.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

void RackRimshot::Init(const PickSeqRackInitData *initdata) {
    rs.Init();

    initdata->rack->registerParamAndCC(initdata, "f0", 8, [&](const int val){ f0 = val;});
    initdata->rack->registerParamAndCC(initdata, "tone", 9, [&](const int val){ tone = val;});
    initdata->rack->registerParamAndCC(initdata, "decay", 10, [&](const int val){ decay = val;});
    initdata->rack->registerParamAndCC(initdata, "noise", 11, [&](const int val){ noise = val;});
    initdata->rack->registerParamAndCC(initdata, "accent", 12, [&](const int val){ accent = val;});

    this->enabled = false;
}

void RackRimshot::trigger() {
    midi_trig = true;
}

void RackRimshot::Process(const PicoSeqRackProcessData &data) {
    if (!this->enabled) {
        return;
    }

    std::fill_n(rs_out, BUF_SZ, 0.f);

    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_f0_, f0, 4095.f, 70.f, 350.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_decay, decay, 4095.f, .1f, .75f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_noise, noise, 4095.f, 0.f, .2f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_accent, accent, 4095.f, 0.1f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_base, tone, 4095.f, .35f, .65f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_reso_hp, tone, 4095.f, 5.f, 1.f)

    rs.params.f0 = _f0_ / 44100.f;
    rs.params.decay = _decay;
    rs.params.accent = _accent;
    rs.params.reso_hp = _reso_hp;
    rs.params.base = _base;
    rs.params.noise_level = _noise;

    // MK_BOOL_PAR_NOCV(_trig, trigger)
    bool _trig = false;
    if (midi_trig) {
        _trig = true;
        midi_trig = false;
    }
    if (_trig != trig_prev) {
        if (_trig) {
            // printf("RS\n");
            rs.Trigger();
        }
        trig_prev = _trig;
    }

    rs.Process(rs_out, BUF_SZ);

    if (rs_out[0] != rs_out[0]) {
        printf("RackRimshot: NaN detected!\n");
        rs.Init();
    }
}
