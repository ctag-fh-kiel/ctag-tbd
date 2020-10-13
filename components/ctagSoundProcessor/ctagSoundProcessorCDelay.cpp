/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "ctagSoundProcessorCDelay.hpp"
#include <iostream>
#include "braids/svf.h"

using namespace CTAG::SP;

ctagSoundProcessorCDelay::ctagSoundProcessorCDelay() {
    setIsStereo();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    model->LoadPreset(0);

    delay.SetSampleRate(44100.f);
    delay.Reset();
}

void ctagSoundProcessorCDelay::Process(const ProcessData &data) {
    float dlyTime = dly_time / 1000.f;
    if (cv_dly_time != -1) {
        dlyTime = fabsf(data.cv[cv_dly_time]) * 1000.f;
    }
    delay.params.delayTime = dlyTime;

    float lfoAmount = lfo_amt / 4095.f * 0.5f;
    if (cv_lfo_amt != -1) {
        lfoAmount = fabsf(data.cv[cv_lfo_amt]) * 0.5f;
    }
    delay.params.lfoAmount = lfoAmount;

    float lfoFreq = lfo_frq / 4095.f * 10.f;
    if (cv_lfo_frq != -1) {
        lfoFreq = fabsf(data.cv[cv_lfo_frq]) * 10.f;
    }
    delay.params.lfoFrequency = lfoFreq;

    float lfoDriftAmt = lfo_drift_amt / 4095.f * 0.5f;
    if (cv_lfo_drift_amt != -1) {
        lfoDriftAmt = fabsf(data.cv[cv_lfo_drift_amt]) * 0.5f;
    }
    delay.params.driftAmount = lfoDriftAmt;

    float driftSpeed = lfo_drift_spd / 4095.f * 10.f;
    if (cv_lfo_drift_spd != -1) {
        driftSpeed = fabsf(data.cv[cv_lfo_drift_spd]) * 10.f;
    }
    delay.params.driftSpeed = driftSpeed;

    float feedback = dly_feedback / 4095.f * 1.2f;
    if (cv_dly_feedback != -1) {
        feedback = data.cv[cv_dly_feedback] * 1.2f;
    }
    if (freeze)feedback = 1.f;
    if (trig_freeze != -1) {
        if (data.trig[trig_freeze] == 0 ? 1 : 0) feedback = 1.f;
    }
    delay.params.feedback = feedback;

    float stereoOffset = dly_st_ofs / 4095.f * .5f;
    if (cv_dly_st_ofs != -1) {
        stereoOffset = data.cv[cv_dly_st_ofs] * .5f;
    }
    delay.params.stereoOffset = stereoOffset;

    float pan = dly_panning / 4095.f * M_PI * .5f;
    if (cv_dly_panning != -1) {
        pan = data.cv[cv_dly_panning] * M_PI * .5f;
    }
    delay.params.pan = pan;

    float duckAmount = duck_amt / 4095.f * 10.f;
    if (cv_duck_amt != -1) {
        duckAmount = fabsf(data.cv[cv_duck_amt]) * 10.f;
    }
    delay.params.duckAmount = duckAmount;

    float duckAttackSpeed = duck_atck / 4095.f * 100.f;
    if (cv_duck_atck != -1) {
        duckAttackSpeed = fabsf(data.cv[cv_duck_atck]) * 100.f;
    }
    delay.params.duckAttackSpeed = duckAttackSpeed;

    float duckReleaseSpeed = duck_rls / 4095.f * 100.f;
    if (cv_duck_rls != -1) {
        duckReleaseSpeed = fabsf(data.cv[cv_duck_rls]) * 100.f;
    }
    delay.params.duckReleaseSpeed = duckReleaseSpeed;

    float flt_cut = flt_co / 4095.f;
    if (cv_flt_co != -1) {
        flt_cut = fabsf(data.cv[cv_flt_co]);
    }
    delay.params.svfCutoffFreq = flt_cut;

    float resp = flt_reso / 4095.f;
    if (cv_flt_reso != -1) {
        resp = fabsf(data.cv[cv_flt_reso]) * 0.99f;
    }
    delay.params.svfResonance = resp;

    float dryVolume = dly_dry / 4095.f * 2.f;
    if (cv_dly_dry != -1) {
        dryVolume = fabsf(data.cv[cv_dly_dry]) * 2.f;
    }
    delay.params.dryVolume = dryVolume;

    float wetVolume = dly_wet / 4095.f * 2.f;
    if (cv_dly_wet != -1) {
        wetVolume = fabsf(data.cv[cv_dly_wet]) * 2.f;
    }
    delay.params.wetVolume = wetVolume;

    int tempoSyncTime = dly_sync;
    if (cv_dly_sync != -1) {
        tempoSyncTime = static_cast<int>(fabsf(data.cv[cv_dly_sync]) * 19.f);
    }
    delay.params.tempoSyncTime = (TempoSyncTimes) tempoSyncTime;

    int panMode = dly_pan_mode;
    if (cv_dly_pan_mode != -1) {
        panMode = static_cast<int>(fabsf(data.cv[cv_dly_pan_mode]) * 3.f);
    }
    delay.params.panMode = (PanModes) panMode;

    int filterMode = flt_mode;
    if (cv_flt_mode != -1) {
        filterMode = static_cast<int>(fabsf(data.cv[cv_flt_mode]) * 4.f);
    }
    delay.params.filterMode = (braids::SvfMode) filterMode;

    float bpm = static_cast<float>(dly_time_bpm);
    if (cv_dly_time_bpm != -1) {
        bpm = fabsf(data.cv[cv_dly_time_bpm]) * 220.f + 30.f;
    }
    delay.params.bpm = bpm;

    delay.params.isMono = dly_mono;

    float filtLfoFreq = flt_lfo_freq / 4095.f * 20.f;
    if (cv_flt_lfo_freq != -1) {
        filtLfoFreq = fabsf(data.cv[cv_flt_lfo_freq]) * 20.f;
    }
    delay.params.svfLfoFreq = filtLfoFreq;

    float svfLfoAmt = flt_lfo_amt / 4095.f;
    if (cv_flt_lfo_amt != -1) {
        svfLfoAmt = data.cv[cv_flt_lfo_amt];
    }
    delay.params.svfLfoAmt = svfLfoAmt;

    float svfMix = flt_mix / 4095.f;
    if (cv_flt_mix != -1) {
        svfMix = data.cv[cv_flt_mix];
    }
    delay.params.svfMix = svfMix;

    delay.Process(data.buf, bufSz);
}

ctagSoundProcessorCDelay::~ctagSoundProcessorCDelay() {
}

const char *ctagSoundProcessorCDelay::GetCStrID() const {
    return id.c_str();
}


void ctagSoundProcessorCDelay::setParamValueInternal(const string &id, const string &key, const int val) {
// autogenerated code here
// sectionCpp0
    if (id.compare("dly_time") == 0) {
        if (key.compare("current") == 0) {
            dly_time = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_dly_time = val;
        }
        return;
    }
    if (id.compare("dly_time_bpm") == 0) {
        if (key.compare("current") == 0) {
            dly_time_bpm = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_dly_time_bpm = val;
        }
        return;
    }
    if (id.compare("dly_sync") == 0) {
        if (key.compare("current") == 0) {
            dly_sync = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_dly_sync = val;
        }
        return;
    }
    if (id.compare("dly_feedback") == 0) {
        if (key.compare("current") == 0) {
            dly_feedback = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_dly_feedback = val;
        }
        return;
    }
    if (id.compare("freeze") == 0) {
        if (key.compare("current") == 0) {
            freeze = val;
            return;
        } else if (key.compare("trig") == 0) {
            if (val >= -1 && val <= 1)
                trig_freeze = val;
        }
        return;
    }
    if (id.compare("dly_st_ofs") == 0) {
        if (key.compare("current") == 0) {
            dly_st_ofs = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_dly_st_ofs = val;
        }
        return;
    }
    if (id.compare("dly_pan_mode") == 0) {
        if (key.compare("current") == 0) {
            dly_pan_mode = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_dly_pan_mode = val;
        }
        return;
    }
    if (id.compare("dly_panning") == 0) {
        if (key.compare("current") == 0) {
            dly_panning = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_dly_panning = val;
        }
        return;
    }
    if (id.compare("dly_mono") == 0) {
        if (key.compare("current") == 0) {
            dly_mono = val;
            return;
        } else if (key.compare("trig") == 0) {
            if (val >= -1 && val <= 1)
                trig_dly_mono = val;
        }
        return;
    }
    if (id.compare("dly_wet") == 0) {
        if (key.compare("current") == 0) {
            dly_wet = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_dly_wet = val;
        }
        return;
    }
    if (id.compare("dly_dry") == 0) {
        if (key.compare("current") == 0) {
            dly_dry = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_dly_dry = val;
        }
        return;
    }
    if (id.compare("lfo_amt") == 0) {
        if (key.compare("current") == 0) {
            lfo_amt = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_lfo_amt = val;
        }
        return;
    }
    if (id.compare("lfo_frq") == 0) {
        if (key.compare("current") == 0) {
            lfo_frq = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_lfo_frq = val;
        }
        return;
    }
    if (id.compare("lfo_drift_amt") == 0) {
        if (key.compare("current") == 0) {
            lfo_drift_amt = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_lfo_drift_amt = val;
        }
        return;
    }
    if (id.compare("lfo_drift_spd") == 0) {
        if (key.compare("current") == 0) {
            lfo_drift_spd = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_lfo_drift_spd = val;
        }
        return;
    }
    if (id.compare("duck_amt") == 0) {
        if (key.compare("current") == 0) {
            duck_amt = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_duck_amt = val;
        }
        return;
    }
    if (id.compare("duck_atck") == 0) {
        if (key.compare("current") == 0) {
            duck_atck = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_duck_atck = val;
        }
        return;
    }
    if (id.compare("duck_rls") == 0) {
        if (key.compare("current") == 0) {
            duck_rls = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_duck_rls = val;
        }
        return;
    }
    if (id.compare("flt_mode") == 0) {
        if (key.compare("current") == 0) {
            flt_mode = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_flt_mode = val;
        }
        return;
    }
    if (id.compare("flt_co") == 0) {
        if (key.compare("current") == 0) {
            flt_co = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_flt_co = val;
        }
        return;
    }
    if (id.compare("flt_reso") == 0) {
        if (key.compare("current") == 0) {
            flt_reso = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_flt_reso = val;
        }
        return;
    }
    if (id.compare("flt_lfo_freq") == 0) {
        if (key.compare("current") == 0) {
            flt_lfo_freq = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_flt_lfo_freq = val;
        }
        return;
    }
    if (id.compare("flt_lfo_amt") == 0) {
        if (key.compare("current") == 0) {
            flt_lfo_amt = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_flt_lfo_amt = val;
        }
        return;
    }
    if (id.compare("flt_mix") == 0) {
        if (key.compare("current") == 0) {
            flt_mix = val;
            return;
        } else if (key.compare("cv") == 0) {
            if (val >= -1 && val <= 3)
                cv_flt_mix = val;
        }
        return;
    }
// sectionCpp0










}

void ctagSoundProcessorCDelay::loadPresetInternal() {
// autogenerated code here
// sectionCpp1
    dly_time = model->GetParamValue("dly_time", "current");
    cv_dly_time = model->GetParamValue("dly_time", "cv");
    dly_time_bpm = model->GetParamValue("dly_time_bpm", "current");
    cv_dly_time_bpm = model->GetParamValue("dly_time_bpm", "cv");
    dly_sync = model->GetParamValue("dly_sync", "current");
    cv_dly_sync = model->GetParamValue("dly_sync", "cv");
    dly_feedback = model->GetParamValue("dly_feedback", "current");
    cv_dly_feedback = model->GetParamValue("dly_feedback", "cv");
    freeze = model->GetParamValue("freeze", "current");
    trig_freeze = model->GetParamValue("freeze", "trig");
    dly_st_ofs = model->GetParamValue("dly_st_ofs", "current");
    cv_dly_st_ofs = model->GetParamValue("dly_st_ofs", "cv");
    dly_pan_mode = model->GetParamValue("dly_pan_mode", "current");
    cv_dly_pan_mode = model->GetParamValue("dly_pan_mode", "cv");
    dly_panning = model->GetParamValue("dly_panning", "current");
    cv_dly_panning = model->GetParamValue("dly_panning", "cv");
    dly_mono = model->GetParamValue("dly_mono", "current");
    trig_dly_mono = model->GetParamValue("dly_mono", "trig");
    dly_wet = model->GetParamValue("dly_wet", "current");
    cv_dly_wet = model->GetParamValue("dly_wet", "cv");
    dly_dry = model->GetParamValue("dly_dry", "current");
    cv_dly_dry = model->GetParamValue("dly_dry", "cv");
    lfo_amt = model->GetParamValue("lfo_amt", "current");
    cv_lfo_amt = model->GetParamValue("lfo_amt", "cv");
    lfo_frq = model->GetParamValue("lfo_frq", "current");
    cv_lfo_frq = model->GetParamValue("lfo_frq", "cv");
    lfo_drift_amt = model->GetParamValue("lfo_drift_amt", "current");
    cv_lfo_drift_amt = model->GetParamValue("lfo_drift_amt", "cv");
    lfo_drift_spd = model->GetParamValue("lfo_drift_spd", "current");
    cv_lfo_drift_spd = model->GetParamValue("lfo_drift_spd", "cv");
    duck_amt = model->GetParamValue("duck_amt", "current");
    cv_duck_amt = model->GetParamValue("duck_amt", "cv");
    duck_atck = model->GetParamValue("duck_atck", "current");
    cv_duck_atck = model->GetParamValue("duck_atck", "cv");
    duck_rls = model->GetParamValue("duck_rls", "current");
    cv_duck_rls = model->GetParamValue("duck_rls", "cv");
    flt_mode = model->GetParamValue("flt_mode", "current");
    cv_flt_mode = model->GetParamValue("flt_mode", "cv");
    flt_co = model->GetParamValue("flt_co", "current");
    cv_flt_co = model->GetParamValue("flt_co", "cv");
    flt_reso = model->GetParamValue("flt_reso", "current");
    cv_flt_reso = model->GetParamValue("flt_reso", "cv");
    flt_lfo_freq = model->GetParamValue("flt_lfo_freq", "current");
    cv_flt_lfo_freq = model->GetParamValue("flt_lfo_freq", "cv");
    flt_lfo_amt = model->GetParamValue("flt_lfo_amt", "current");
    cv_flt_lfo_amt = model->GetParamValue("flt_lfo_amt", "cv");
    flt_mix = model->GetParamValue("flt_mix", "current");
    cv_flt_mix = model->GetParamValue("flt_mix", "cv");
// sectionCpp1
}
