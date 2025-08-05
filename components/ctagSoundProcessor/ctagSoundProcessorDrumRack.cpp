#include "ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// TODOs: fx return before compressor, stereo panning with delay -> when panned right, levels are lower, metallic sound of reverb.

const float maxFXSendLevelDly {4.f};
const float maxFXSendLevelRev {2.f};
const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void ctagSoundProcessorDrumRack::mixRenderOutputMono(float *source, float level, float pan, float fx1, float fx2) {
    float mL = (1.0 - pan) * level;
    float mR = pan * level;
    float sL1 = mL * fx1;
    float sR1 = mR * fx1;
    float sL2 = mL * fx2;
    float sR2 = mR * fx2;

    for (int i = 0; i < 32; i++){
        combined_out[i*2+0] += source[i] * mL;
        combined_out[i*2+0] += source[i] * mR;
        send1_out[i*2+0] += source[i] * sL1;
        send1_out[i*2+0] += source[i] * sR1;
        send2_out[i*2+0] += source[i] * sL2;
        send2_out[i*2+1] += source[i] * sR2;
    }
}

void ctagSoundProcessorDrumRack::mixRenderOutputStereo(float *source, float level, float pan, float fx1, float fx2) {
    float mL = (1.0 - pan) * level;
    float mR = pan * level;
    float sL1 = mL * fx1;
    float sR1 = mR * fx1;
    float sL2 = mL * fx2;
    float sR = mR * fx2;

    for (int i = 0; i < 32; i++){
        combined_out[i*2+0] += source[i*2+0] * mL;
        combined_out[i*2+1] += source[i*2+1] * mR;
        send1_out[i*2+0] += source[i*2+0] * sL1;
        send1_out[i*2+1] += source[i*2+1] * sR1;
        send2_out[i*2+0] += source[i*2+0] * sL2;
        send2_out[i*2+1] += source[i*2+1] * sR;
    }
}

void ctagSoundProcessorDrumRack::renderABD(const ProcessData& data) {
    float abd_out[32];

    // Analog Bass Drum
    MK_BOOL_PAR(bABMute, ab_mute)
    MK_BOOL_PAR(bABTrig, ab_trigger)
    if (bABTrig != abd_trig_prev){
        abd_trig_prev = bABTrig;
    }
    else{
        bABTrig = false;
    }

    MK_FLT_PAR_ABS_PAN(fABPan, ab_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fABLev, ab_lev, 4095.f, 2.f); fABLev *= fABLev;
    MK_FLT_PAR_ABS(fABFX1Send, ab_fx1, 4095.f, maxFXSendLevelDly); fABFX1Send *= fABFX1Send;
    MK_FLT_PAR_ABS(fABFX2Send, ab_fx2, 4095.f, maxFXSendLevelRev); fABFX2Send *= fABFX2Send;

    if (bABMute || fABLev < minVolume) {
        memcpy(abd_out, silence, 32 * 2 * sizeof(float));
        return;
    }

    MK_FLT_PAR_ABS(fABAccent, ab_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fABF0, ab_f0, 4095.f, 0.0001f, 0.01f)
    MK_FLT_PAR_ABS(fABTone, ab_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fABDecay, ab_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fABAfm, ab_a_fm, 4095.f, 0.f, 100.f)
    MK_FLT_PAR_ABS_MIN_MAX(fABSfm, ab_s_fm, 4095.f, 0.f, 100.f)
    abd.Render(
        false,
        bABTrig,
        fABAccent,
        fABF0,
        fABTone,
        fABDecay,
        fABAfm,
        fABSfm,
        abd_out,
        32);

    mixRenderOutputMono(abd_out, fABLev, fABPan, fABFX1Send, fABFX2Send);
}

void ctagSoundProcessorDrumRack::renderASD(const ProcessData& data) {
    float asd_out[32];

    MK_BOOL_PAR(bASMute, as_mute)
    MK_BOOL_PAR(bASTrig, as_trigger)
    if (bASTrig != asd_trig_prev){
        asd_trig_prev = bASTrig;
    }
    else{
        bASTrig = false;
    }

    MK_FLT_PAR_ABS_PAN(fASPan, as_pan, 4095.f, 1.f);
    MK_FLT_PAR_ABS(fASLev, as_lev, 4095.f, 2.f); fASLev *= fASLev;
    MK_FLT_PAR_ABS(fASFX1Send, as_fx1, 4095.f, maxFXSendLevelDly); fASFX1Send *= fASFX1Send;
    MK_FLT_PAR_ABS(fASFX2Send, as_fx2, 4095.f, maxFXSendLevelRev); fASFX2Send *= fASFX2Send;

    if (bASMute || fASLev < minVolume) {
        return;
    }

    MK_FLT_PAR_ABS(fASAccent, as_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fASF0, as_f0, 4095.f, 0.001f, 0.01f)
    MK_FLT_PAR_ABS(fASTone, as_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fASDecay, as_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fASAspy, as_a_spy, 4095.f, 1.f)
    asd.Render(
        false,
        bASTrig,
        fASAccent,
        fASF0,
        fASTone,
        fASDecay,
        fASAspy,
        asd_out,
        32);

    mixRenderOutputMono(asd_out, fASLev, fASPan, fASFX1Send, fASFX2Send);
}

void ctagSoundProcessorDrumRack::renderDBD(const ProcessData& data) {
    float dbd_out[32];

    MK_BOOL_PAR(bDBMute, db_mute)
    MK_BOOL_PAR(bDBTrig, db_trigger)
    if (bDBTrig != dbd_trig_prev){
        dbd_trig_prev = bDBTrig;
    }
    else{
        bDBTrig = false;
    }

    MK_FLT_PAR_ABS_PAN(fDBPan, db_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDBLev, db_lev, 4095.f, 2.f); fDBLev *= fDBLev;
    MK_FLT_PAR_ABS(fDBFX1Send, db_fx1, 4095.f, maxFXSendLevelDly); fDBFX1Send *= fDBFX1Send;
    MK_FLT_PAR_ABS(fDBFX2Send, db_fx2, 4095.f, maxFXSendLevelRev); fDBFX2Send *= fDBFX2Send;

    if (bDBMute || fDBLev < minVolume) {
        return;
    }

    MK_FLT_PAR_ABS(fDBAccent, db_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fDBF0, db_f0, 4095.f, 0.0005f, 0.01f)
    MK_FLT_PAR_ABS(fDBTone, db_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDBDecay, db_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDBDirty, db_dirty, 4095.f, 5.f)
    MK_FLT_PAR_ABS(fDBFmEnv, db_fm_env, 4095.f, 5.f)
    MK_FLT_PAR_ABS(fDBFmDcy, db_fm_dcy, 4095.f, 4.f)
    dbd.Render(
        false,
        bDBTrig,
        fDBAccent,
        fDBF0,
        fDBTone,
        fDBDecay,
        fDBDirty,
        fDBFmEnv,
        fDBFmDcy,
        dbd_out,
        32);

    mixRenderOutputMono(dbd_out, fDBLev, fDBPan, fDBFX1Send, fDBFX2Send);
}

void ctagSoundProcessorDrumRack::renderDSD(const ProcessData& data) {
    float dsd_out[32];

    MK_BOOL_PAR(bDSMute, ds_mute)
    MK_BOOL_PAR(bDSTrig, ds_trigger)
    if (bDSTrig != dsd_trig_prev){
        dsd_trig_prev = bDSTrig;
    }
    else{
        bDSTrig = false;
    }

    MK_FLT_PAR_ABS_PAN(fDSPan, ds_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDSLev, ds_lev, 4095.f, 2.f); fDSLev *= fDSLev;
    MK_FLT_PAR_ABS(fDSFX1Send, ds_fx1, 4095.f, maxFXSendLevelDly); fDSFX1Send *= fDSFX1Send;
    MK_FLT_PAR_ABS(fDSFX2Send, ds_fx2, 4095.f, maxFXSendLevelRev); fDSFX2Send *= fDSFX2Send;

    if (bDSMute || fDSLev < minVolume) {
        return;
    }

    MK_FLT_PAR_ABS(fDSAccent, ds_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fDSF0, ds_f0, 4095.f, 0.0008f, 0.01f)
    MK_FLT_PAR_ABS(fDSFmAmt, ds_fm_amt, 4095.f, 1.5f)
    MK_FLT_PAR_ABS(fDSDecay, ds_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDSSpy, ds_spy, 4095.f, 1.f)
    dsd.Render(
        false,
        bDSTrig,
        fDSAccent,
        fDSF0,
        fDSFmAmt,
        fDSDecay,
        fDSSpy,
        dsd_out,
        32);

    mixRenderOutputMono(dsd_out, fDSLev, fDSPan, fDSFX1Send, fDSFX2Send);
}

void ctagSoundProcessorDrumRack::renderFMB(const ProcessData& data) {
    float fmb_out[32];

    MK_BOOL_PAR(bFMBMute, fmb_mute)
    MK_BOOL_PAR(bFMBTrig, fmb_trigger)
    if (bFMBTrig != fmb_trig_prev && bFMBTrig){
	    fmb_trig_prev = true;
	    fmb.Trigger();
    }
    else if (!bFMBTrig){
	    fmb_trig_prev = false;
    }

    MK_FLT_PAR_ABS_PAN(fFMBPan, fmb_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fFMBLev, fmb_lev, 4095.f, 2.f); fFMBLev *= fFMBLev;
    MK_FLT_PAR_ABS(fFMBFX1Send, fmb_fx1, 4095.f, maxFXSendLevelDly); fFMBFX1Send *= fFMBFX1Send;
    MK_FLT_PAR_ABS(fFMBFX2Send, fmb_fx2, 4095.f, maxFXSendLevelRev); fFMBFX2Send *= fFMBFX2Send;    MK_FLT_PAR_ABS_PAN(fHH1Pan, hh1_pan, 4095.f, 1.f)

    if (bFMBMute || fFMBLev < minVolume) {
        return;
    }

    MK_BOOL_PAR(bFMBUseRatioMode, fmb_use_ratio_mode)
    MK_BOOL_PAR(bFMBModEnvSync, fmb_mod_env_sync)
    float fFMBF0 = fmb_f_b/4095.f * (200.f-20.f)+20.f;
    if(cv_fmb_f_b != -1){
        float fMod = data.cv[cv_fmb_f_b] * 5.f;
        fMod = CTAG::SP::HELPERS::fastpow2(fMod);
        fFMBF0 *= fMod;
    }
    MK_FLT_PAR_ABS_MIN_MAX(fFMBDecayBase, fmb_d_b, 4095.f, 0.001f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fFMBFMod, fmb_f_m, 4095.f, 40.f, 2000.f)
    MK_FLT_PAR_ABS(fFMBRatioModIndex, fmb_f_m, 4095.f, 63.f)
    int iModIndex = static_cast<int>(fFMBRatioModIndex);
    CONSTRAIN(iModIndex, 0, 63)
    MK_FLT_PAR_ABS_MIN_MAX(fFMBI, fmb_I, 4095.f, 0.f, 10.f)
    MK_FLT_PAR_ABS_MIN_MAX(fFMBDecayMod, fmb_d_m, 4095.f, 0.001f, .5f)
    MK_INT_PAR(iModFeedback, fmb_b_m, 16.f)
    MK_FLT_PAR_ABS_MIN_MAX(fFMBAmpFreq, fmb_A_f, 4095.f, 0.f, 1000.f)
    MK_FLT_PAR_ABS_MIN_MAX(fFMBDecayFreq, fmb_d_f, 4095.f, 0.001f, .1f)

    fmb.params.use_ratio_mode = bFMBUseRatioMode;
    fmb.params.mod_env_sync = bFMBModEnvSync;
    fmb.params.f_b = fFMBF0;
    fmb.params.d_b = fFMBDecayBase;
    fmb.params.f_m = fFMBFMod;
    fmb.params.mod_ratio_index = iModIndex;
    fmb.params.I = fFMBI;
    fmb.params.d_m = fFMBDecayMod;
    fmb.params.b_m = static_cast<float>(iModFeedback);
    fmb.params.A_f = fFMBAmpFreq;
    fmb.params.d_f = fFMBDecayFreq;

    fmb.Process(fmb_out, 32);
    mixRenderOutputMono(fmb_out, fFMBLev, fFMBPan, fFMBFX1Send, fFMBFX2Send);
}

void ctagSoundProcessorDrumRack::renderHH1(const ProcessData& data) {
    float hh1_out[32];

    MK_BOOL_PAR(bHH1Mute, hh1_mute)
    MK_BOOL_PAR(bHH1Trig, hh1_trigger)
    if (bHH1Trig != hh1_trig_prev){
        hh1_trig_prev = bHH1Trig;
    }
    else{
        bHH1Trig = false;
    }

    MK_FLT_PAR_ABS_PAN(fHH1Pan, hh1_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH1Lev, hh1_lev, 4095.f, 2.f); fHH1Lev *= fHH1Lev;
    MK_FLT_PAR_ABS(fHH1FX1Send, hh1_fx1, 4095.f, maxFXSendLevelDly); fHH1FX1Send *= fHH1FX1Send;
    MK_FLT_PAR_ABS(fHH1FX2Send, hh1_fx2, 4095.f, maxFXSendLevelRev); fHH1FX2Send *= fHH1FX2Send;

    if (bHH1Mute || fHH1Lev < minVolume) {
        return;
    }

    MK_FLT_PAR_ABS(fHH1Accent, hh1_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fHH1F0, hh1_f0, 4095.f, 0.0005f, 0.1f)
    MK_FLT_PAR_ABS(fHH1Tone, hh1_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH1Decay, hh1_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH1Noise, hh1_noise, 4095.f, 1.f)
    hh1.Render(
        false,
        bHH1Trig,
        fHH1Accent,
        fHH1F0,
        fHH1Tone,
        fHH1Decay,
        fHH1Noise,
        temp1_,
        temp2_,
        hh1_out,
        32);

    mixRenderOutputMono(hh1_out, fHH1Lev, fHH1Pan, fHH1FX1Send, fHH1FX2Send);
}

void ctagSoundProcessorDrumRack::renderHH2(const ProcessData& data) {
    float hh2_out[32];
    MK_BOOL_PAR(bHH2Mute, hh2_mute)
    MK_BOOL_PAR(bHH2Trig, hh2_trigger)
    if (bHH2Trig != hh2_trig_prev){
        hh2_trig_prev = bHH2Trig;
    }
    else{
        bHH2Trig = false;
    }

    MK_FLT_PAR_ABS_PAN(fHH2Pan, hh2_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH2Lev, hh2_lev, 4095.f, 2.f); fHH2Lev *= fHH2Lev;
    MK_FLT_PAR_ABS(fHH2FX1Send, hh2_fx1, 4095.f, maxFXSendLevelDly); fHH2FX1Send *= fHH2FX1Send;
    MK_FLT_PAR_ABS(fHH2FX2Send, hh2_fx2, 4095.f, maxFXSendLevelRev); fHH2FX2Send *= fHH2FX2Send;

    if (bHH2Mute || fHH2Lev < minVolume) {
        return;
    }

    MK_FLT_PAR_ABS(fHH2Accent, hh2_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fHH2F0, hh2_f0, 4095.f, .00001f, .1f)
    MK_FLT_PAR_ABS(fHH2Tone, hh2_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH2Decay, hh2_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH2Noise, hh2_noise, 4095.f, 1.f)
    hh2.Render(
        false,
        bHH2Trig,
        fHH2Accent,
        fHH2F0,
        fHH2Tone,
        fHH2Decay,
        fHH2Noise,
        temp1_,
        temp2_,
        hh2_out,
        32);

    mixRenderOutputMono(hh2_out, fHH2Lev, fHH2Pan, fHH2FX1Send, fHH2FX2Send);
}

void ctagSoundProcessorDrumRack::renderCL(const ProcessData& data) {
    float cl_out[32];
    MK_BOOL_PAR(bCLMute, cl_mute)

    MK_FLT_PAR_ABS_PAN(fCLPan, cl_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fCLLev, cl_lev, 4095.f, 2.f); fCLLev *= fCLLev;
    MK_FLT_PAR_ABS(fCLFX1Send, cl_fx1, 4095.f, maxFXSendLevelDly); fCLFX1Send *= fCLFX1Send;
    MK_FLT_PAR_ABS(fCLFX2Send, cl_fx2, 4095.f, maxFXSendLevelRev); fCLFX2Send *= fCLFX2Send;

    if (bCLMute || fCLLev < minVolume) {
        return;
    }

    MK_FLT_PAR_ABS_MIN_MAX(cl_pitch1_, cl_f0, 4095.f, 350.f, 4000.f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_pitch2_, cl_f0, 4095.f, 300.f, 3000.f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_reso1_, cl_tone, 4095.f, 1.f, 2.5f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_reso2_, cl_tone, 4095.f, 0.75f, 6.5f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_decay1_, cl_decay, 4095.f, 0.05f, 0.3f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_decay2_, cl_decay, 4095.f, 0.05f, 2.f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_scale_attack_, cl_scale, 4095.f, 0.f, 0.1f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_scale_trans, cl_scale, 4095.f, 1.f, 3.f)
    MK_INT_PAR_ABS(cl_trans_, cl_transient, 16)

    cl.params.pitch1 = cl_pitch1_ / 44100.f;
    cl.params.pitch2 = cl_pitch2_ / 44100.f;
    cl.params.reso1 = cl_reso1_;
    cl.params.reso2 = cl_reso2_;
    cl.params.decay1 = cl_decay1_;
    cl.params.decay2 = cl_decay2_;
    cl.params.attack = cl_scale_attack_;
    cl.params.scale = cl_scale_trans;
    cl.params.transient = cl_trans_ % 16;

    MK_BOOL_PAR(bCLTrig, cl_trigger)
    if (bCLTrig != cl_trig_prev && bCLTrig){
        cl_trig_prev = true;
        cl.Trigger();
    }
    else if (!bCLTrig){
        cl_trig_prev = false;
    }

    cl.Process(cl_out, 32);
    mixRenderOutputMono(cl_out, fCLLev, fCLPan, fCLFX1Send, fCLFX2Send);
}

void ctagSoundProcessorDrumRack::renderRS(const ProcessData& data) {
    float rs_out[32];
    MK_BOOL_PAR(bRSMute, rs_mute)

    MK_FLT_PAR_ABS_PAN(fRSPan, rs_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fRSLev, rs_lev, 4095.f, 2.f); fRSLev *= fRSLev;
    MK_FLT_PAR_ABS(fRSFX1Send, rs_fx1, 4095.f, maxFXSendLevelDly); fRSFX1Send *= fRSFX1Send;
    MK_FLT_PAR_ABS(fRSFX2Send, rs_fx2, 4095.f, maxFXSendLevelRev); fRSFX2Send *= fRSFX2Send;

    if (bRSMute || fRSLev < minVolume) {
        return;
    }

    MK_FLT_PAR_ABS_MIN_MAX(rs_f0_, rs_f0, 4095.f, 70.f, 350.f)
    MK_FLT_PAR_ABS_MIN_MAX(rs_decay_, rs_decay, 4095.f, .1f, .75f)
    MK_FLT_PAR_ABS_MIN_MAX(rs_noise_, rs_noise, 4095.f, 0.f, .2f)
    MK_FLT_PAR_ABS_MIN_MAX(rs_accent_, rs_accent, 4095.f, 0.1f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(rs_base_, rs_tone, 4095.f, .35f, .65f)
    MK_FLT_PAR_ABS_MIN_MAX(rs_reso_hp_, rs_tone, 4095.f, 5.f, 1.f)

    rs.params.f0 = rs_f0_ / 44100.f;
    rs.params.decay = rs_decay_;
    rs.params.accent = rs_accent_;
    rs.params.reso_hp = rs_reso_hp_;
    rs.params.base = rs_base_;
    rs.params.noise_level = rs_noise_;

    MK_BOOL_PAR(bRSTrig, rs_trigger)
    if (bRSTrig != rs_trig_prev && bRSTrig){
        rs_trig_prev = true;
        rs.Trigger();
    }
    else if (!bRSTrig){
        rs_trig_prev = false;
    }

    rs.Process(rs_out, 32);
    mixRenderOutputMono(rs_out, fRSLev, fRSPan, fRSFX1Send, fRSFX2Send);
}

void ctagSoundProcessorDrumRack::renderS1(const ProcessData& data) {
    float s1_out[32];
    uint32_t firstNonWtSlice = sampleRom.GetFirstNonWaveTableSlice();
    float fS1Lev = 0.f, fS1Pan = 0.f;
    MK_BOOL_PAR(bMuteS1, s1_mute)
    fS1Lev = s1_lev / 4095.f * 1.5f;
    if (cv_s1_lev != -1) fS1Lev += fabsf(data.cv[cv_s1_lev]);
    fS1Lev *= fS1Lev;
    fS1Pan = (s1_pan / 4095.f + 1.f) / 2.f * 1.f;
    if (cv_s1_pan != -1) fS1Pan = fabsf(data.cv[cv_s1_pan]) * 1.f;
    MK_FLT_PAR_ABS(fS1FX1Send, s1_fx1, 4095.f, maxFXSendLevelDly); fS1FX1Send *= fS1FX1Send;
    MK_FLT_PAR_ABS(fS1FX2Send, s1_fx2, 4095.f, maxFXSendLevelRev); fS1FX2Send *= fS1FX2Send;

    if (bMuteS1 || fS1Lev < minVolume) {
        return;
    }

    MK_BOOL_PAR(bGateS1, s1_gate)
    rompler[0].params.gate = bGateS1;
    fS1Lev = s1_lev / 4095.f * 1.5f;
    if (cv_s1_lev != -1) fS1Lev += fabsf(data.cv[cv_s1_lev]);
    fS1Lev *= fS1Lev;
    float fS1Speed = s1_speed / 4095.f * 2.f;
    if (cv_s1_speed != -1) fS1Speed += data.cv[cv_s1_speed] * 2.f;
    CONSTRAIN(fS1Speed, -2.f, 2.f)
    rompler[0].params.playbackSpeed = fS1Speed;
    float fS1Pitch = s1_pitch;
    if (cv_s1_pitch != -1){
        fS1Pitch += data.cv[cv_s1_pitch] * 12.f * 5.f;
    }
    rompler[0].params.pitch = fS1Pitch;
    MK_INT_PAR_ABS(iS1Bank, s1_bank, 32.f)
    CONSTRAIN(iS1Bank, 0, 31)
    MK_INT_PAR_ABS(iS1Slice, s1_slice, 32.f)
    CONSTRAIN(iS1Slice, 0, 31)
    iS1Slice = iS1Bank * 32 + iS1Slice + firstNonWtSlice;
    rompler[0].params.slice = iS1Slice;
    MK_FLT_PAR_ABS(fS1Start, s1_start, 4095.f, 1.f)
    rompler[0].params.startOffsetRelative = fS1Start;
    MK_FLT_PAR_ABS(fS1Length, s1_end, 4095.f, 1.f)
    rompler[0].params.lengthRelative = fS1Length;
    MK_FLT_PAR_ABS(fS1LoopPos, s1_lp_pos, 4095.f, 1.f)
    rompler[0].params.loopMarker = fS1LoopPos;
    MK_BOOL_PAR(bS1Loop, s1_lp)
    rompler[0].params.loop = bS1Loop;
    MK_BOOL_PAR(bS1LoopPipo, s1_lp_pp)
    rompler[0].params.loopPiPo = bS1LoopPipo;
    MK_FLT_PAR_ABS(fS1Attack, s1_atk, 4095.f, 2.f)
    rompler[0].params.a = fS1Attack;
    MK_FLT_PAR_ABS(fS1Decay, s1_dcy, 4095.f, 50.f)
    rompler[0].params.d = fS1Decay;
    MK_FLT_PAR_ABS_SFT(fS1EGFM, s1_eg2fm, 4095.f, 12.f)
    rompler[0].params.egFM = fS1EGFM;
    MK_INT_PAR_ABS(iS1Brr, s1_brr, 16)
    CONSTRAIN(iS1Brr, 0, 14)
    rompler[0].params.bitReduction = iS1Brr;
    // filter params
    MK_FLT_PAR_ABS(fS1Cut, s1_fc, 4095.f, 1.f)
    rompler[0].params.cutoff = fS1Cut;
    MK_FLT_PAR_ABS(fS1Reso, s1_fq, 4095.f, 10.f)
    rompler[0].params.resonance = fS1Reso;
    MK_INT_PAR_ABS(iS1FType, s1_ft, 4.f)
    CONSTRAIN(iS1FType, 0, 3);
    rompler[0].params.filterType = static_cast<CTAG::SYNTHESIS::RomplerVoiceMinimal::FilterType>(iS1FType);
    rompler[0].Process(s1_out, 32);
    mixRenderOutputMono(s1_out, fS1Lev, fS1Pan, fS1FX1Send, fS1FX2Send);
}

void ctagSoundProcessorDrumRack::renderS2(const ProcessData& data) {
    float s2_out[32];
    uint32_t firstNonWtSlice = sampleRom.GetFirstNonWaveTableSlice();
    float fS2Lev = 0.f, fS2Pan = 0.f;
    MK_BOOL_PAR(bMuteS2, s2_mute)
    fS2Lev = s2_lev / 4095.f * 1.5f;
    if (cv_s2_lev != -1) fS2Lev += fabsf(data.cv[cv_s2_lev]);
    fS2Lev *= fS2Lev;
    fS2Pan = (s2_pan / 4095.f + 1.f) / 2.f * 1.f;
    if (cv_s2_pan != -1) fS2Pan = fabsf(data.cv[cv_s2_pan]) * 1.f;
    MK_FLT_PAR_ABS(fS2FX1Send, s2_fx1, 4095.f, maxFXSendLevelDly); fS2FX1Send *= fS2FX1Send;
    MK_FLT_PAR_ABS(fS2FX2Send, s2_fx2, 4095.f, maxFXSendLevelRev); fS2FX2Send *= fS2FX2Send;

    if (bMuteS2 || fS2Lev < minVolume) {
        return;
    }

    MK_BOOL_PAR(bGateS2, s2_gate)
    rompler[1].params.gate = bGateS2;
    fS2Lev = s2_lev / 4095.f * 1.5f;
    if (cv_s2_lev != -1) fS2Lev += fabsf(data.cv[cv_s2_lev]);
    fS2Lev *= fS2Lev;
    float fS2Speed = s2_speed / 4095.f * 2.f;
    if (cv_s2_speed != -1) fS2Speed += data.cv[cv_s2_speed] * 2.f;
    CONSTRAIN(fS2Speed, -2.f, 2.f)
    rompler[1].params.playbackSpeed = fS2Speed;
    float fS2Pitch = s2_pitch;
    if (cv_s2_pitch != -1){
        fS2Pitch += data.cv[cv_s2_pitch] * 12.f * 5.f;
    }
    rompler[1].params.pitch = fS2Pitch;
    MK_INT_PAR_ABS(iS2Bank, s2_bank, 32.f)
    CONSTRAIN(iS2Bank, 0, 31)
    MK_INT_PAR_ABS(iS2Slice, s2_slice, 32.f)
    CONSTRAIN(iS2Slice, 0, 31)
    iS2Slice = iS2Bank * 32 + iS2Slice + firstNonWtSlice;
    rompler[1].params.slice = iS2Slice;
    MK_FLT_PAR_ABS(fS2Start, s2_start, 4095.f, 1.f)
    rompler[1].params.startOffsetRelative = fS2Start;
    MK_FLT_PAR_ABS(fS2Length, s2_end, 4095.f, 1.f)
    rompler[1].params.lengthRelative = fS2Length;
    MK_FLT_PAR_ABS(fS2LoopPos, s2_lp_pos, 4095.f, 1.f)
    rompler[1].params.loopMarker = fS2LoopPos;
    MK_BOOL_PAR(bS2Loop, s2_lp)
    rompler[1].params.loop = bS2Loop;
    MK_BOOL_PAR(bS2LoopPipo, s2_lp_pp)
    rompler[1].params.loopPiPo = bS2LoopPipo;
    MK_FLT_PAR_ABS(fS2Attack, s2_atk, 4095.f, 2.f)
    rompler[1].params.a = fS2Attack;
    MK_FLT_PAR_ABS(fS2Decay, s2_dcy, 4095.f, 50.f)
    rompler[1].params.d = fS2Decay;
    MK_FLT_PAR_ABS_SFT(fS2EGFM, s2_eg2fm, 4095.f, 12.f)
    rompler[1].params.egFM = fS2EGFM;
    MK_INT_PAR_ABS(iS2Brr, s2_brr, 16)
    CONSTRAIN(iS2Brr, 0, 14)
    rompler[1].params.bitReduction = iS2Brr;
    // filter params
    MK_FLT_PAR_ABS(fS2Cut, s2_fc, 4095.f, 1.f)
    rompler[1].params.cutoff = fS2Cut;
    MK_FLT_PAR_ABS(fS2Reso, s2_fq, 4095.f, 10.f)
    rompler[1].params.resonance = fS2Reso;
    MK_INT_PAR_ABS(iS2FType, s2_ft, 4.f)
    CONSTRAIN(iS2FType, 0, 3);
    rompler[1].params.filterType = static_cast<CTAG::SYNTHESIS::RomplerVoiceMinimal::FilterType>(iS2FType);
    rompler[1].Process(s2_out, 32);
    mixRenderOutputMono(s2_out, fS2Lev, fS2Pan, fS2FX1Send, fS2FX2Send);
}

void ctagSoundProcessorDrumRack::renderS3(const ProcessData& data) {
    float s3_out[32];
    uint32_t firstNonWtSlice = sampleRom.GetFirstNonWaveTableSlice();

    MK_BOOL_PAR(bMuteS3, s3_mute)
    float fS3Lev = 0.f, fS3Pan = 0.f;
    fS3Lev = s3_lev / 4095.f * 1.5f;
    if (cv_s3_lev != -1) fS3Lev += fabsf(data.cv[cv_s3_lev]);
    fS3Lev *= fS3Lev;
    fS3Pan = (s3_pan / 4095.f + 1.f) / 2.f * 1.f;
    if (cv_s3_pan != -1) fS3Pan = fabsf(data.cv[cv_s3_pan]) * 1.f;
    MK_FLT_PAR_ABS(fS3FX1Send, s3_fx1, 4095.f, maxFXSendLevelDly); fS3FX1Send *= fS3FX1Send;
    MK_FLT_PAR_ABS(fS3FX2Send, s3_fx2, 4095.f, maxFXSendLevelRev); fS3FX2Send *= fS3FX2Send;

    if (bMuteS3 || fS3Lev < minVolume) {
        return;
    }

    MK_BOOL_PAR(bGateS3, s3_gate)
    rompler[2].params.gate = bGateS3;
    fS3Lev = s3_lev / 4095.f * 1.5f;
    if (cv_s3_lev != -1) fS3Lev += fabsf(data.cv[cv_s3_lev]);
    fS3Lev *= fS3Lev;
    float fS3Speed = s3_speed / 4095.f * 2.f;
    if (cv_s3_speed != -1) fS3Speed += data.cv[cv_s3_speed] * 2.f;
    CONSTRAIN(fS3Speed, -2.f, 2.f)
    rompler[2].params.playbackSpeed = fS3Speed;
    float fS3Pitch = s3_pitch;
    if (cv_s3_pitch != -1){
        fS3Pitch += data.cv[cv_s3_pitch] * 12.f * 5.f;
    }
    rompler[2].params.pitch = fS3Pitch;
    MK_INT_PAR_ABS(iS3Bank, s3_bank, 32.f)
    CONSTRAIN(iS3Bank, 0, 31)
    MK_INT_PAR_ABS(iS3Slice, s3_slice, 32.f)
    CONSTRAIN(iS3Slice, 0, 31)
    iS3Slice = iS3Bank * 32 + iS3Slice + firstNonWtSlice;
    rompler[2].params.slice = iS3Slice;
    MK_FLT_PAR_ABS(fS3Start, s3_start, 4095.f, 1.f)
    rompler[2].params.startOffsetRelative = fS3Start;
    MK_FLT_PAR_ABS(fS3Length, s3_end, 4095.f, 1.f)
    rompler[2].params.lengthRelative = fS3Length;
    MK_FLT_PAR_ABS(fS3LoopPos, s3_lp_pos, 4095.f, 1.f)
    rompler[2].params.loopMarker = fS3LoopPos;
    MK_BOOL_PAR(bS3Loop, s3_lp)
    rompler[2].params.loop = bS3Loop;
    MK_BOOL_PAR(bS3LoopPipo, s3_lp_pp)
    rompler[2].params.loopPiPo = bS3LoopPipo;
    MK_FLT_PAR_ABS(fS3Attack, s3_atk, 4095.f, 2.f)
    rompler[2].params.a = fS3Attack;
    MK_FLT_PAR_ABS(fS3Decay, s3_dcy, 4095.f, 50.f)
    rompler[2].params.d = fS3Decay;
    MK_FLT_PAR_ABS_SFT(fS3EGFM, s3_eg2fm, 4095.f, 12.f)
    rompler[2].params.egFM = fS3EGFM;
    MK_INT_PAR_ABS(iS3Brr, s3_brr, 16)
    CONSTRAIN(iS3Brr, 0, 14)
    rompler[2].params.bitReduction = iS3Brr;
    // filter params
    MK_FLT_PAR_ABS(fS3Cut, s3_fc, 4095.f, 1.f)
    rompler[2].params.cutoff = fS3Cut;
    MK_FLT_PAR_ABS(fS3Reso, s3_fq, 4095.f, 10.f)
    rompler[2].params.resonance = fS3Reso;
    MK_INT_PAR_ABS(iS3FType, s3_ft, 4.f)
    CONSTRAIN(iS3FType, 0, 3);
    rompler[2].params.filterType = static_cast<CTAG::SYNTHESIS::RomplerVoiceMinimal::FilterType>(iS3FType);
    rompler[2].Process(s3_out, 32);
    mixRenderOutputMono(s3_out, fS3Lev, fS3Pan, fS3FX1Send, fS3FX2Send);
}

void ctagSoundProcessorDrumRack::renderS4(const ProcessData& data) {
    float s4_out[32];
    uint32_t firstNonWtSlice = sampleRom.GetFirstNonWaveTableSlice();

    MK_BOOL_PAR(bMuteS4, s4_mute)
    float fS4Lev = 0.f, fS4Pan = 0.f;
    fS4Lev = s4_lev / 4095.f * 1.5f;
    if (cv_s4_lev != -1) fS4Lev += fabsf(data.cv[cv_s4_lev]);
    fS4Lev *= fS4Lev;
    fS4Pan = (s4_pan / 4095.f + 1.f) / 2.f * 1.f;
    if (cv_s4_pan != -1) fS4Pan = fabsf(data.cv[cv_s4_pan]) * 1.f;
    MK_FLT_PAR_ABS(fS4FX1Send, s4_fx1, 4095.f, maxFXSendLevelDly); fS4FX1Send *= fS4FX1Send;
    MK_FLT_PAR_ABS(fS4FX2Send, s4_fx2, 4095.f, maxFXSendLevelRev); fS4FX2Send *= fS4FX2Send;

    if (bMuteS4 || fS4Lev < minVolume) {
        return;
    }

    MK_BOOL_PAR(bGateS4, s4_gate)
    rompler[3].params.gate = bGateS4;
    fS4Lev = s4_lev / 4095.f * 1.5f;
    if (cv_s4_lev != -1) fS4Lev += fabsf(data.cv[cv_s4_lev]);
    fS4Lev *= fS4Lev;
    float fS4Speed = s4_speed / 4095.f * 2.f;
    if (cv_s4_speed != -1) fS4Speed += data.cv[cv_s4_speed] * 2.f;
    CONSTRAIN(fS4Speed, -2.f, 2.f)
    rompler[3].params.playbackSpeed = fS4Speed;
    float fS4Pitch = s4_pitch;
    if (cv_s4_pitch != -1){
        fS4Pitch += data.cv[cv_s4_pitch] * 12.f * 5.f;
    }
    rompler[3].params.pitch = fS4Pitch;
    MK_INT_PAR_ABS(iS4Bank, s4_bank, 32.f)
    CONSTRAIN(iS4Bank, 0, 31)
    MK_INT_PAR_ABS(iS4Slice, s4_slice, 32.f)
    CONSTRAIN(iS4Slice, 0, 31)
    iS4Slice = iS4Bank * 32 + iS4Slice + firstNonWtSlice;
    rompler[3].params.slice = iS4Slice;
    MK_FLT_PAR_ABS(fS4Start, s4_start, 4095.f, 1.f)
    rompler[3].params.startOffsetRelative = fS4Start;
    MK_FLT_PAR_ABS(fS4Length, s4_end, 4095.f, 1.f)
    rompler[3].params.lengthRelative = fS4Length;
    MK_FLT_PAR_ABS(fS4LoopPos, s4_lp_pos, 4095.f, 1.f)
    rompler[3].params.loopMarker = fS4LoopPos;
    MK_BOOL_PAR(bS4Loop, s4_lp)
    rompler[3].params.loop = bS4Loop;
    MK_BOOL_PAR(bS4LoopPipo, s4_lp_pp)
    rompler[3].params.loopPiPo = bS4LoopPipo;
    MK_FLT_PAR_ABS(fS4Attack, s4_atk, 4095.f, 2.f)
    rompler[3].params.a = fS4Attack;
    MK_FLT_PAR_ABS(fS4Decay, s4_dcy, 4095.f, 50.f)
    rompler[3].params.d = fS4Decay;
    MK_FLT_PAR_ABS_SFT(fS4EGFM, s4_eg2fm, 4095.f, 12.f)
    rompler[3].params.egFM = fS4EGFM;
    MK_INT_PAR_ABS(iS4Brr, s4_brr, 16)
    CONSTRAIN(iS4Brr, 0, 14)
    rompler[3].params.bitReduction = iS4Brr;
    // filter params
    MK_FLT_PAR_ABS(fS4Cut, s4_fc, 4095.f, 1.f)
    rompler[3].params.cutoff = fS4Cut;
    MK_FLT_PAR_ABS(fS4Reso, s4_fq, 4095.f, 10.f)
    rompler[3].params.resonance = fS4Reso;
    MK_INT_PAR_ABS(iS4FType, s4_ft, 4.f)
    CONSTRAIN(iS4FType, 0, 3);
    rompler[3].params.filterType = static_cast<CTAG::SYNTHESIS::RomplerVoiceMinimal::FilterType>(iS4FType);
    rompler[3].Process(s4_out, 32);
    mixRenderOutputMono(s4_out, fS4Lev, fS4Pan, fS4FX1Send, fS4FX2Send);
}

void ctagSoundProcessorDrumRack::renderIN(const ProcessData& data) {
    float in_out[32];
    MK_BOOL_PAR(bMuteIN, in_mute)

    float fINLev = 0.f, fINPan = 0.f;
    fINLev = in_lev / 4095.f * 1.5f;
    if (cv_in_lev != -1) fINLev += fabsf(data.cv[cv_in_lev]);
    fINLev *= fINLev;
    fINPan = (in_pan / 4095.f + 1.f) / 2.f * 1.f;
    if (cv_in_pan != -1) fINPan = fabsf(data.cv[cv_in_pan]) * 1.f;
    MK_FLT_PAR_ABS(fINFX1Send, in_fx1, 4095.f, maxFXSendLevelDly); fINFX1Send *= fINFX1Send;
    MK_FLT_PAR_ABS(fINFX2Send, in_fx2, 4095.f, maxFXSendLevelRev); fINFX2Send *= fINFX2Send;

    if (bMuteIN || fINLev < minVolume) {
        return;
    }

    fINLev = in_lev / 4095.f * 1.5f;
    if (cv_in_lev != -1) fINLev += fabsf(data.cv[cv_in_lev]);
    fINLev *= fINLev;
    fINPan = (in_pan / 4095.f + 1.f) / 2.f * 1.f;
    if (cv_in_pan != -1) fINPan = fabsf(data.cv[cv_in_pan]) * 1.f;

    mixRenderOutputStereo(data.buf, fINLev, fINPan, fINFX1Send, fINFX2Send);
}

void ctagSoundProcessorDrumRack::renderTD3(const ProcessData& data) {
    float dvcf, dvca;
    bool trg;
    float td3_out[  32];

    MK_FLT_PAR_ABS_PAN(fTD3Pan, td3_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fTD3Lev, td3_lev, 4095.f, 2.f); fTD3Lev *= fTD3Lev;
    MK_FLT_PAR_ABS(fTD3FX1Send, td3_fx1, 4095.f, maxFXSendLevelDly); fTD3FX1Send *= fTD3FX1Send;
    MK_FLT_PAR_ABS(fTD3FX2Send, td3_fx2, 4095.f, maxFXSendLevelRev); fTD3FX2Send *= fTD3FX2Send;

    if (trig_td3_trigger != -1) {
        trg = data.trig[trig_td3_trigger] == 1 ? 0 : 1; // negative logic
    } else {
        trg = td3_trigger;
    }

    if (trg && !td3_pre_trig) {
        td3_isAccent = td3_accent;
        if (trig_td3_accent != -1) {
            td3_isAccent = data.trig[trig_td3_accent] == 0 ? 1 : 0;
        }
        dvcf = td3_decay_vcf / 4095.f * 5.f;
        if (cv_td3_decay_vcf != -1) {
            dvcf = fabsf(data.cv[cv_td3_decay_vcf]) * 5.f;
        }
        // if accent shorten decay of filter eg
        if (td3_isAccent) {
            dvcf = td3_kAccentDecay;
        }
        td3_adVCF.SetDecay(dvcf);
        dvca = td3_decay_vca / 4095.f * 5.f;
        if (cv_td3_decay_vca != -1) {
            dvca = fabsf(data.cv[cv_td3_decay_vca]) * 5.f;
        }
        td3_adVCA.SetDecay(dvca);
        td3_adVCF.Trigger();
        td3_adVCA.Trigger();
        // sync on trigger
        if (td3_sync_trig) td3_sync[0] = 1;
        td3_osc.Strike();
        td3_pre_trig = true;
    } else if (!trg) {
        td3_pre_trig = false;
    }

    float egvalVCA = td3_adVCA.Process();
    // if accent make slightly louder
    if (td3_isAccent) {
        egvalVCA *= td3_kAccentVCAFactor;
    }
    float egvalVCF = td3_adVCF.Process();

    // shape
    int s = td3_shape;
    if (cv_td3_shape != -1) {
        s = fabsf(data.cv[cv_td3_shape]) * (braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META + 1);
    }
    braids::MacroOscillatorShape ms = static_cast<braids::MacroOscillatorShape>(s);
    if (ms >= braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META)
        ms = braids::MacroOscillatorShape::MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META;
    td3_osc.set_shape(ms);

    // Set timbre and color: CV + internal modulation.
    int16_t parameters[2];
    parameters[0] = td3_param_0;
    parameters[1] = td3_param_1;
    if (cv_td3_param_0 != -1) {
        parameters[0] = static_cast<int16_t>(fabsf(data.cv[cv_td3_param_0] * 32767));
    }
    if (cv_td3_param_1 != -1) {
        parameters[1] = static_cast<int16_t>(fabsf(data.cv[cv_td3_param_1] * 32767));
    }
    int32_t mod_amt[2];
    mod_amt[0] = td3_p0_amt;
    mod_amt[1] = td3_p1_amt;
    int32_t mod[2];
    if (cv_td3_p0_amt != -1) {
        mod[0] = static_cast<int32_t >(data.cv[cv_td3_p0_amt] * 65535.f);
    } else {
        mod[0] = static_cast<int32_t >(egvalVCF * 65535.f);
    }
    if (cv_td3_p1_amt != -1) {
        mod[1] = static_cast<int32_t >(data.cv[cv_td3_p1_amt] * 65535.f);
    } else {
        mod[1] = static_cast<int32_t >(egvalVCF * 65535.f);
    }
    for (int i = 0; i < 2; ++i) {
        int32_t value = parameters[i];
        value += (mod[i] * mod_amt[i]) / 8192;
        CONSTRAIN(value, 0, 32767);
        parameters[i] = value;
    }
    td3_osc.set_parameters(parameters[0], parameters[1]);

    // pitch calculation and quantization
    MK_BOOL_PAR(isSlide, td3_slide)
    MK_FLT_PAR_ABS(fSlideLevel, td3_slide_level, 4095.f, 0.099f)
    fSlideLevel += 0.9f;
    int32_t ipitch = td3_pitch;
    if (cv_td3_pitch != -1) {
        float fPitch = data.cv[cv_td3_pitch] * 12.f * 5.f; // five octaves
        if(isSlide){
            fPitch = fSlideLevel * td3_pre_pitch_val + (1.f - fSlideLevel) * fPitch;
        }
        td3_pre_pitch_val = fPitch;
        ipitch += static_cast<int32_t>(fPitch * 128.f);
    }
    CONSTRAIN(ipitch, 0, 16383);
    td3_osc.set_pitch(ipitch);

    // render audio data
    int16_t buffer[32];
    td3_osc.Render(td3_sync, buffer, bufSz);

    // apply filter and EGs
    int ftype = td3_filter_type;
    if (cv_td3_filter_type != -1) {
        ftype = static_cast<int>(fabsf(data.cv[cv_td3_filter_type]) * 5.f);
    }
    CONSTRAIN(ftype, 0, 4)
    float c = td3_cutoff / 4095.f;
    if (cv_td3_cutoff != -1) {
        c = fabsf(data.cv[cv_td3_cutoff]);
    }
    c *= 27000.f;
    c -= 5000.f;
    float fenv = td3_envelope / 4095.f;
    if (cv_td3_envelope != -1) {
        fenv = fabsf(data.cv[cv_td3_envelope]);
    }
    c += fenv * egvalVCF * 22000.f;
    // if accent add to VCF envelope
    float facclev = td3_accent_level / 4095.f;
    if (cv_td3_accent_level != -1) {
        facclev = fabsf(data.cv[cv_td3_accent_level]);
    }
    if (td3_isAccent) {
        c += facclev * egvalVCF * 22000.f;
    }

    float r = td3_resonance / 4095.f;
    if (cv_td3_resonance != -1) {
        r = fabsf(data.cv[cv_td3_resonance]);
    }

    int32_t signature = td3_saturation;
    if (cv_td3_saturation != -1) {
        signature = static_cast<int32_t>(fabsf(data.cv[cv_td3_saturation]) * 65535.f);
    }
    CONSTRAIN(signature, 0, 65535)

    float dri = td3_drive / 4095.f * 30.f;
    if (cv_td3_drive != -1) {
        dri = fabsf(data.cv[cv_td3_drive]) * 30.f;
    }

    CONSTRAIN(c, 20.f, 22000.f)
    CONSTRAIN(r, 0.f, 1.f)
    CONSTRAIN(dri, 1.f, 30.f)
    ctagFilterBase *filter = &td3_pirkle_zdf_boost;
    switch(ftype){
        case 0:
            filter = &td3_pirkle_zdf_boost;
            break;
        case 1:
            filter = &td3_karlson;
            break;
        case 2:
            filter = &td3_blaukraut;
            break;
        case 3:
            filter = &td3_pirkle_zdf;
            break;
        case 4:
            filter = &td3_zavalishin;
            break;
    }
    filter->SetCutoff(c);
    filter->SetResonance(r);
    filter->SetGain(dri);

    float fgain = td3_gain / 4095.f * 2.f;
    if (cv_td3_gain != -1) {
        fgain = fabsf(data.cv[cv_td3_gain]) * 2.f;
    }

    for (int i = 0; i < bufSz; i++) {
        float eg = td3_pre_eg_val +
                   (egvalVCA - td3_pre_eg_val) / (float) bufSz * i; // linear fade from previous eg value to avoid glitches
        // apply non linearity to filter input
        int16_t warped = td3_ws.Transform(buffer[i]);
        buffer[i] = stmlib::Mix(buffer[i], warped, signature);
        // filter, EG and clip
        const float div = 3.0518509476E-5f;

        float f = fgain * stmlib::SoftClip(eg * filter->Process(buffer[i] * div));
        td3_out[i] = f;
    }
    td3_pre_eg_val = egvalVCA;
    // sync on trigger
    td3_sync[0] = 0;

    mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}

void ctagSoundProcessorDrumRack::preprocessFX1(const ProcessData& data) {
    MK_FLT_PAR_ABS(fBase, fx1_base, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fWidth, fx1_width, 4095.f, 1.f)
    bool bSync = fx1_sync;
    bool bSyncTrig {false};
    if(trig_fx1_sync != -1) bSyncTrig = data.trig[trig_fx1_sync] == 1 ? false : true;
    if(!bSync){
        fDelayTime = fx1_time_ms;
        if(cv_fx1_time_ms != -1) fDelayTime = fabsf(data.cv[cv_fx1_time_ms]) * 2000.f;
    }

    fBase = 20.f * stmlib::SemitonesToRatio(fBase * 120.f);
    fWidth = 20.f * stmlib::SemitonesToRatio(fWidth * 120.f);
    CONSTRAIN(fBase, 20.f, 20000.f)
    CONSTRAIN(fWidth, 50.f, 20000.f)
    float hp_cut = fBase;
    float lp_cut = fBase + fWidth;
    CONSTRAIN(lp_cut, 20.f, 20000.f)
    CONSTRAIN(hp_cut, 20.f, 20000.f)
    lp_l.set_f<stmlib::FREQUENCY_ACCURATE>(lp_cut / 44100.f);
    hp_l.set_f<stmlib::FREQUENCY_ACCURATE>(hp_cut / 44100.f);
    lp_r.copy_f(lp_l);
    hp_r.copy_f(hp_l);

    // sync mechanism
    if(bSyncTrig != pre_sync){
        pre_sync = bSyncTrig;
        if(bSyncTrig && bSync){
            int delta = timer - pre_timer;
            if(std::abs(delta) > 1){
                fDelayTime = static_cast<float>(timer) * 32.f / 44.1f;
            }
            pre_timer = timer;
            timer = 0;
        }
    }
    timer++;
}

void ctagSoundProcessorDrumRack::preprocessFX2(const ProcessData& data) {
    MK_FLT_PAR_ABS(fRevTime, fx2_time, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fReverbLPF, fx2_lp, 4095.f, 1.f)
    reverb.set_time(fRevTime);
    reverb.set_lp(fReverbLPF);
}

void ctagSoundProcessorDrumRack::preprocessMaster(const ProcessData& data) {
    MK_FLT_PAR_ABS_MIN_MAX(fCompThresdB, c_thres, 4095.f, -80.f, 0.f)
    sumCompressor.setThresh(fCompThresdB);
    MK_FLT_PAR_ABS_MIN_MAX(fCompAtk, c_atk, 4095.f, 0.3f, 30.f)
    sumCompressor.setAttack(fCompAtk);
    MK_FLT_PAR_ABS_MIN_MAX(fCompRel, c_rel, 4095.f, 40.f, 2000.f)
    sumCompressor.setRelease(fCompRel);
    MK_FLT_PAR_ABS_MIN_MAX(fCompRatio, c_ratio, 4095.f, 0.0001f, 1.25f)
    sumCompressor.setRatio(fCompRatio);
}
 
void ctagSoundProcessorDrumRack::renderMasterOutput(const ProcessData& data) {
    // delay
    MK_BOOL_PAR(bFreeze, fx1_freeze)
    MK_FLT_PAR_ABS(fDelayStereoWidth, fx1_st_width, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDelayReverbSend, fx1_fx_send, 4095.f, maxFXSendLevelRev)
    fDelayReverbSend *= fDelayReverbSend;
    MK_FLT_PAR_ABS(fFeedback, fx1_feedback, 4095.f, 1.5f)
    MK_FLT_PAR_ABS(fDelayAmount, fx1_amount, 4095.f, 2.f)

    // reverb
    MK_FLT_PAR_ABS(fRevAmount, fx2_amount, 4095.f, 2.f)

    // sum compressor
    float buf_fx1_l[32], buf_fx1_r[32], buf_fx2[32];
    MK_BOOL_PAR(bTapeDigital, fx1_tape_digital)
    MK_BOOL_PAR(bSideChainLPF, c_lpf)
    MK_FLT_PAR_ABS_MIN_MAX(fCompMUPGain, c_gain, 4095.f, 0.f, 60.f) // in dB
    if (fCompMUPGain != fCompMUPGain_pre){
        fCompMUPGain = chunkware_simple::dB2lin(fCompMUPGain);
        fCompMUPGain_pre = fCompMUPGain;
    }
    MK_FLT_PAR_ABS_PAN(fCompMix, c_mix, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fCompDlyLevel, c_dly_level, 4095.f, 2.f)
    fCompDlyLevel *= fCompDlyLevel;
    MK_FLT_PAR_ABS(fCompRevLevel, c_rev_level, 4095.f, 2.f)
    fCompRevLevel *= fCompRevLevel;

    // overall mix
    MK_FLT_PAR_ABS(fMixLevel, sum_lev, 4095.f, 3.f)
    fMixLevel *= fMixLevel;

    // Render final buffer
    for (int i = 0; i < 32; i++){
        float fVal_l = combined_out[i * 2 + 0];
        float fVal_r = combined_out[i * 2 + 1];

        // FX1 models
        buf_fx1_l[i] = send1_out[i * 2 + 0];
        buf_fx1_r[i] = send1_out[i * 2 + 1];

        // FX2 models, reverb is mono in stereo out, but input buffer is stereo
        buf_fx2[i] = send2_out[i * 2 + 0];

        float dry_l = fVal_l;
        float dry_r = fVal_r;
        if (bSideChainLPF){
            ONE_POLE(side_l, fVal_l, 0.0005f);
            ONE_POLE(side_r, fVal_r, 0.0005f);
        }
        else{
            side_l = fVal_l;
            side_r = fVal_r;
        }
        side_l = fabsf(side_l);
        side_r = fabsf(side_r);
        float side = std::max(side_l, side_r);
        sumCompressor.process(fVal_l, fVal_r, side);
        fVal_l = fVal_l * fCompMUPGain * fCompMix + dry_l * (1.f - fCompMix);
        fVal_r = fVal_r * fCompMUPGain * fCompMix + dry_r * (1.f - fCompMix);
        data.buf[i * 2] = fVal_l * fMixLevel;
        data.buf[i * 2 + 1] = fVal_r * fMixLevel;
    }

    // fx buffers
    float dly_buf_l[32], dly_buf_r[32];
    float rev_buf_l[32], rev_buf_r[32];

    // delay
    CONSTRAIN(fDelayTime, 0.0001, 2000.f)
    float ofs = fDelayTime * 44.1f;
    if(fabsf(ofs - delayOffset) < 16) ofs = delayOffset;
    for(int i=0; i<32; i++){
        // Calculate the delay offset in samples
        if(delayOffset != ofs){
            if(bTapeDigital){
                if(ofs != delayOffset){
                    duck = 1.f;
                }
                delayOffset = ofs;
            } else {
                float temp = delayOffset;
                delayOffset = ONE_POLE(temp, ofs, 0.0001f);
            }
            readPos = static_cast<float>(writeIndex) - delayOffset;
            if(readPos < 0.f) readPos += float(delayBufferSizeMax);
            if(readPos >= float(delayBufferSizeMax)) readPos -= float(delayBufferSizeMax);
        }

        float inputSample_l = buf_fx1_l[i];
        float inputSample_r = buf_fx1_r[i];
        float outputSample_l, outputSample_r;

        outputSample_l = HELPERS::InterpolateWaveLinearWrap(delayBuffer_l, readPos, delayBufferSizeMax);
        outputSample_r = HELPERS::InterpolateWaveLinearWrap(delayBuffer_r, readPos, delayBufferSizeMax);
        readPos += 1.f;
        readPos > float(delayBufferSizeMax) ? readPos -= float(delayBufferSizeMax) : readPos;

        float temp = duck;
        duck = ONE_POLE(temp, 0.f, 0.35f)
        outputSample_l = outputSample_l * (1.f - duck);
        outputSample_r = outputSample_r * (1.f - duck);
        // Write the input sample to the delay buffer
        float out_l, out_r;
        if(!bFreeze){
            out_l = inputSample_l + fFeedback * ((1.f - fDelayStereoWidth) * outputSample_l + fDelayStereoWidth * outputSample_r);
            out_l = lp_l.Process<stmlib::FILTER_MODE_LOW_PASS>(out_l);
            out_l = hp_l.Process<stmlib::FILTER_MODE_HIGH_PASS>(out_l);
            out_r = (1.f - fDelayStereoWidth) * inputSample_r + fFeedback * ((1.f - fDelayStereoWidth) * outputSample_r + fDelayStereoWidth * outputSample_l);
            out_r = lp_r.Process<stmlib::FILTER_MODE_LOW_PASS>(out_r);
            out_r = hp_r.Process<stmlib::FILTER_MODE_HIGH_PASS>(out_r);
        }
        else{
            out_l = ((1.f - fDelayStereoWidth) * outputSample_l + fDelayStereoWidth * outputSample_r);
            out_r = ((1.f - fDelayStereoWidth) * outputSample_r + fDelayStereoWidth * outputSample_l);
        }

        delayBuffer_l[writeIndex] = stmlib::SoftLimit(out_l);
        delayBuffer_r[writeIndex] = stmlib::SoftLimit(out_r);
        writeIndex = (writeIndex + 1) % delayBufferSizeMax;

        // Mix the dry (input) and wet (delayed) signal
        dly_buf_l[i] = outputSample_l;
        dly_buf_r[i] = outputSample_r;
        rev_buf_l[i] = buf_fx2[i] + dly_buf_l[i] * fDelayReverbSend;
        rev_buf_r[i] = buf_fx2[i] + dly_buf_r[i] * fDelayReverbSend;
    }

    // reverb
    reverb.Process(rev_buf_l, rev_buf_r, 32);

    // add fx to sum
    fRevAmount *= fRevAmount;
    fDelayAmount *= fDelayAmount;
    for (int i = 0; i < 32; i++) {
        data.buf[i * 2] += rev_buf_l[i] * fRevAmount + dly_buf_l[i] * fDelayAmount;
        data.buf[i * 2 + 1] += rev_buf_r[i] * fRevAmount + dly_buf_r[i] * fDelayAmount;
    }
}

void ctagSoundProcessorDrumRack::Process(const ProcessData& data){
    memset(combined_out, 0, 32 * 2 * sizeof(float));
    memset(send1_out, 0, 32 * 2 * sizeof(float));
    memset(send2_out, 0, 32 * 2 * sizeof(float));
    memset(silence, 0, 32 * 2 * sizeof(float));

    // Render sound generators
    renderABD(data);
    renderASD(data); // analog snare drum
    renderDBD(data); // digital bass drum
    renderDSD(data); // digital snare drum
    renderHH1(data); // hihat 1
    renderHH2(data); // hihat 2
    renderRS(data); // rimshot
    renderCL(data); // clap
    renderFMB(data); // fm bass drum
    renderS1(data);
    renderS2(data);
    renderS3(data);
    renderS4(data);
    renderIN(data); // audio input
    renderTD3(data);

    // Process effects
    preprocessFX1(data); // delay
    preprocessFX2(data); // reverb
    preprocessMaster(data); // sum compressor

    MK_BOOL_PAR(bSumMute, sum_mute)
    if (bSumMute){
        memset(data.buf, 0, 32 * 2 * sizeof(float));
        return;
    }

    renderMasterOutput(data);
}

void ctagSoundProcessorDrumRack::Init(std::size_t blockSize, void* blockPtr){
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    // delay
    delayBuffer_l = static_cast<float*>(heap_caps_malloc(delayBufferSizeMax * sizeof(float), MALLOC_CAP_SPIRAM));
    assert(delayBuffer_l != nullptr);
    std::fill_n(delayBuffer_l, delayBufferSizeMax, 0.f);
    delayBuffer_r = static_cast<float*>(heap_caps_malloc(delayBufferSizeMax * sizeof(float), MALLOC_CAP_SPIRAM));
    assert(delayBuffer_r != nullptr);
    std::fill_n(delayBuffer_r, delayBufferSizeMax, 0.f);

    // reverb
    assert(blockSize >= 32768 * 4);
    reverb.Init((float*)blockPtr); // requires 32768*4 bytes = 128KB
    reverb.Clear();
    blockPtr = static_cast<void*>(static_cast<uint8_t*>(blockPtr) + 32768 * 4);
    blockSize -= 32768 * 4;
    reverb.set_diffusion(0.7f);
    reverb.set_input_gain(.5f); // left and right are summed
    reverb.set_amount(1.f);
    reverb.set_lp(0.5f);
    reverb.set_time(0.4f);

    // preload samples
    sampleRom.BufferInSPIRAM();

    // init compressor
    sumCompressor.setSampleRate(44100.f);
    sumCompressor.initRuntime();

    // init models
    abd.Init();
    asd.Init();
    dbd.Init();
    dsd.Init();
    hh1.Init();
    hh2.Init();
    rs.Init();
    cl.Init();
    fmb.Init();

    // td3
    td3_pirkle_zdf_boost.Init();
    td3_karlson.Init();
    td3_blaukraut.Init();
    td3_pirkle_zdf.Init();
    td3_zavalishin.Init();
    td3_osc.Init();
    td3_osc.set_pitch(100);
    td3_osc.set_shape(braids::MacroOscillatorShape::MACRO_OSC_SHAPE_CSAW);
    td3_adVCA.SetSampleRate(44100.f / bufSz);
    td3_adVCA.SetModeExp();
    td3_adVCA.SetAttack(0.f);
    td3_adVCA.SetDecay(0.5f);
    td3_adVCF.SetSampleRate(44100.f / bufSz);
    td3_adVCF.SetModeExp();
    td3_adVCF.SetAttack(0.f);
    td3_adVCF.SetDecay(0.5f);
    td3_ws.Init(0xcafe);

    std::fill_n(silence, 32, 0.f);

    // init romplers
    for (auto& r : rompler){
        r.Init(44100.f);
    }

    // check if blockMem is large enough
    // blockMem is used just like larger blocks of heap memory
    // assert(blockSize >= memLen);
    // if memory larger than blockMem is needed, use heap_caps_malloc() instead with MALLOC_CAPS_SPIRAM
}

// no ctor, use Init() instead, is called from factory after successful creation
// dtor
ctagSoundProcessorDrumRack::~ctagSoundProcessorDrumRack(){
    // no explicit freeing for blockMem needed, done by ctagSPAllocator
    // explicit free is only needed when using heap_caps_malloc() with MALLOC_CAPS_SPIRAM
}

void ctagSoundProcessorDrumRack::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("ab_trigger", [&](const int val){ ab_trigger = val;});
	pMapTrig.emplace("ab_trigger", [&](const int val){ trig_ab_trigger = val;});
	pMapPar.emplace("ab_mute", [&](const int val){ ab_mute = val;});
	pMapTrig.emplace("ab_mute", [&](const int val){ trig_ab_mute = val;});
	pMapPar.emplace("ab_lev", [&](const int val){ ab_lev = val;});
	pMapCv.emplace("ab_lev", [&](const int val){ cv_ab_lev = val;});
	pMapPar.emplace("ab_pan", [&](const int val){ ab_pan = val;});
	pMapCv.emplace("ab_pan", [&](const int val){ cv_ab_pan = val;});
	pMapPar.emplace("ab_fx1", [&](const int val){ ab_fx1 = val;});
	pMapCv.emplace("ab_fx1", [&](const int val){ cv_ab_fx1 = val;});
	pMapPar.emplace("ab_fx2", [&](const int val){ ab_fx2 = val;});
	pMapCv.emplace("ab_fx2", [&](const int val){ cv_ab_fx2 = val;});
	pMapPar.emplace("ab_accent", [&](const int val){ ab_accent = val;});
	pMapCv.emplace("ab_accent", [&](const int val){ cv_ab_accent = val;});
	pMapPar.emplace("ab_f0", [&](const int val){ ab_f0 = val;});
	pMapCv.emplace("ab_f0", [&](const int val){ cv_ab_f0 = val;});
	pMapPar.emplace("ab_tone", [&](const int val){ ab_tone = val;});
	pMapCv.emplace("ab_tone", [&](const int val){ cv_ab_tone = val;});
	pMapPar.emplace("ab_decay", [&](const int val){ ab_decay = val;});
	pMapCv.emplace("ab_decay", [&](const int val){ cv_ab_decay = val;});
	pMapPar.emplace("ab_a_fm", [&](const int val){ ab_a_fm = val;});
	pMapCv.emplace("ab_a_fm", [&](const int val){ cv_ab_a_fm = val;});
	pMapPar.emplace("ab_s_fm", [&](const int val){ ab_s_fm = val;});
	pMapCv.emplace("ab_s_fm", [&](const int val){ cv_ab_s_fm = val;});
	pMapPar.emplace("db_trigger", [&](const int val){ db_trigger = val;});
	pMapTrig.emplace("db_trigger", [&](const int val){ trig_db_trigger = val;});
	pMapPar.emplace("db_mute", [&](const int val){ db_mute = val;});
	pMapTrig.emplace("db_mute", [&](const int val){ trig_db_mute = val;});
	pMapPar.emplace("db_lev", [&](const int val){ db_lev = val;});
	pMapCv.emplace("db_lev", [&](const int val){ cv_db_lev = val;});
	pMapPar.emplace("db_pan", [&](const int val){ db_pan = val;});
	pMapCv.emplace("db_pan", [&](const int val){ cv_db_pan = val;});
	pMapPar.emplace("db_fx1", [&](const int val){ db_fx1 = val;});
	pMapCv.emplace("db_fx1", [&](const int val){ cv_db_fx1 = val;});
	pMapPar.emplace("db_fx2", [&](const int val){ db_fx2 = val;});
	pMapCv.emplace("db_fx2", [&](const int val){ cv_db_fx2 = val;});
	pMapPar.emplace("db_accent", [&](const int val){ db_accent = val;});
	pMapCv.emplace("db_accent", [&](const int val){ cv_db_accent = val;});
	pMapPar.emplace("db_f0", [&](const int val){ db_f0 = val;});
	pMapCv.emplace("db_f0", [&](const int val){ cv_db_f0 = val;});
	pMapPar.emplace("db_tone", [&](const int val){ db_tone = val;});
	pMapCv.emplace("db_tone", [&](const int val){ cv_db_tone = val;});
	pMapPar.emplace("db_decay", [&](const int val){ db_decay = val;});
	pMapCv.emplace("db_decay", [&](const int val){ cv_db_decay = val;});
	pMapPar.emplace("db_dirty", [&](const int val){ db_dirty = val;});
	pMapCv.emplace("db_dirty", [&](const int val){ cv_db_dirty = val;});
	pMapPar.emplace("db_fm_env", [&](const int val){ db_fm_env = val;});
	pMapCv.emplace("db_fm_env", [&](const int val){ cv_db_fm_env = val;});
	pMapPar.emplace("db_fm_dcy", [&](const int val){ db_fm_dcy = val;});
	pMapCv.emplace("db_fm_dcy", [&](const int val){ cv_db_fm_dcy = val;});
	pMapPar.emplace("fmb_trigger", [&](const int val){ fmb_trigger = val;});
	pMapTrig.emplace("fmb_trigger", [&](const int val){ trig_fmb_trigger = val;});
	pMapPar.emplace("fmb_mute", [&](const int val){ fmb_mute = val;});
	pMapTrig.emplace("fmb_mute", [&](const int val){ trig_fmb_mute = val;});
	pMapPar.emplace("fmb_lev", [&](const int val){ fmb_lev = val;});
	pMapCv.emplace("fmb_lev", [&](const int val){ cv_fmb_lev = val;});
	pMapPar.emplace("fmb_pan", [&](const int val){ fmb_pan = val;});
	pMapCv.emplace("fmb_pan", [&](const int val){ cv_fmb_pan = val;});
	pMapPar.emplace("fmb_fx1", [&](const int val){ fmb_fx1 = val;});
	pMapCv.emplace("fmb_fx1", [&](const int val){ cv_fmb_fx1 = val;});
	pMapPar.emplace("fmb_fx2", [&](const int val){ fmb_fx2 = val;});
	pMapCv.emplace("fmb_fx2", [&](const int val){ cv_fmb_fx2 = val;});
	pMapPar.emplace("fmb_use_ratio_mode", [&](const int val){ fmb_use_ratio_mode = val;});
	pMapTrig.emplace("fmb_use_ratio_mode", [&](const int val){ trig_fmb_use_ratio_mode = val;});
	pMapPar.emplace("fmb_mod_env_sync", [&](const int val){ fmb_mod_env_sync = val;});
	pMapTrig.emplace("fmb_mod_env_sync", [&](const int val){ trig_fmb_mod_env_sync = val;});
	pMapPar.emplace("fmb_f_b", [&](const int val){ fmb_f_b = val;});
	pMapCv.emplace("fmb_f_b", [&](const int val){ cv_fmb_f_b = val;});
	pMapPar.emplace("fmb_d_b", [&](const int val){ fmb_d_b = val;});
	pMapCv.emplace("fmb_d_b", [&](const int val){ cv_fmb_d_b = val;});
	pMapPar.emplace("fmb_f_m", [&](const int val){ fmb_f_m = val;});
	pMapCv.emplace("fmb_f_m", [&](const int val){ cv_fmb_f_m = val;});
	pMapPar.emplace("fmb_I", [&](const int val){ fmb_I = val;});
	pMapCv.emplace("fmb_I", [&](const int val){ cv_fmb_I = val;});
	pMapPar.emplace("fmb_d_m", [&](const int val){ fmb_d_m = val;});
	pMapCv.emplace("fmb_d_m", [&](const int val){ cv_fmb_d_m = val;});
	pMapPar.emplace("fmb_b_m", [&](const int val){ fmb_b_m = val;});
	pMapCv.emplace("fmb_b_m", [&](const int val){ cv_fmb_b_m = val;});
	pMapPar.emplace("fmb_A_f", [&](const int val){ fmb_A_f = val;});
	pMapCv.emplace("fmb_A_f", [&](const int val){ cv_fmb_A_f = val;});
	pMapPar.emplace("fmb_d_f", [&](const int val){ fmb_d_f = val;});
	pMapCv.emplace("fmb_d_f", [&](const int val){ cv_fmb_d_f = val;});
	pMapPar.emplace("as_trigger", [&](const int val){ as_trigger = val;});
	pMapTrig.emplace("as_trigger", [&](const int val){ trig_as_trigger = val;});
	pMapPar.emplace("as_mute", [&](const int val){ as_mute = val;});
	pMapTrig.emplace("as_mute", [&](const int val){ trig_as_mute = val;});
	pMapPar.emplace("as_lev", [&](const int val){ as_lev = val;});
	pMapCv.emplace("as_lev", [&](const int val){ cv_as_lev = val;});
	pMapPar.emplace("as_pan", [&](const int val){ as_pan = val;});
	pMapCv.emplace("as_pan", [&](const int val){ cv_as_pan = val;});
	pMapPar.emplace("as_fx1", [&](const int val){ as_fx1 = val;});
	pMapCv.emplace("as_fx1", [&](const int val){ cv_as_fx1 = val;});
	pMapPar.emplace("as_fx2", [&](const int val){ as_fx2 = val;});
	pMapCv.emplace("as_fx2", [&](const int val){ cv_as_fx2 = val;});
	pMapPar.emplace("as_accent", [&](const int val){ as_accent = val;});
	pMapCv.emplace("as_accent", [&](const int val){ cv_as_accent = val;});
	pMapPar.emplace("as_f0", [&](const int val){ as_f0 = val;});
	pMapCv.emplace("as_f0", [&](const int val){ cv_as_f0 = val;});
	pMapPar.emplace("as_tone", [&](const int val){ as_tone = val;});
	pMapCv.emplace("as_tone", [&](const int val){ cv_as_tone = val;});
	pMapPar.emplace("as_decay", [&](const int val){ as_decay = val;});
	pMapCv.emplace("as_decay", [&](const int val){ cv_as_decay = val;});
	pMapPar.emplace("as_a_spy", [&](const int val){ as_a_spy = val;});
	pMapCv.emplace("as_a_spy", [&](const int val){ cv_as_a_spy = val;});
	pMapPar.emplace("ds_trigger", [&](const int val){ ds_trigger = val;});
	pMapTrig.emplace("ds_trigger", [&](const int val){ trig_ds_trigger = val;});
	pMapPar.emplace("ds_mute", [&](const int val){ ds_mute = val;});
	pMapTrig.emplace("ds_mute", [&](const int val){ trig_ds_mute = val;});
	pMapPar.emplace("ds_lev", [&](const int val){ ds_lev = val;});
	pMapCv.emplace("ds_lev", [&](const int val){ cv_ds_lev = val;});
	pMapPar.emplace("ds_pan", [&](const int val){ ds_pan = val;});
	pMapCv.emplace("ds_pan", [&](const int val){ cv_ds_pan = val;});
	pMapPar.emplace("ds_fx1", [&](const int val){ ds_fx1 = val;});
	pMapCv.emplace("ds_fx1", [&](const int val){ cv_ds_fx1 = val;});
	pMapPar.emplace("ds_fx2", [&](const int val){ ds_fx2 = val;});
	pMapCv.emplace("ds_fx2", [&](const int val){ cv_ds_fx2 = val;});
	pMapPar.emplace("ds_accent", [&](const int val){ ds_accent = val;});
	pMapCv.emplace("ds_accent", [&](const int val){ cv_ds_accent = val;});
	pMapPar.emplace("ds_f0", [&](const int val){ ds_f0 = val;});
	pMapCv.emplace("ds_f0", [&](const int val){ cv_ds_f0 = val;});
	pMapPar.emplace("ds_fm_amt", [&](const int val){ ds_fm_amt = val;});
	pMapCv.emplace("ds_fm_amt", [&](const int val){ cv_ds_fm_amt = val;});
	pMapPar.emplace("ds_decay", [&](const int val){ ds_decay = val;});
	pMapCv.emplace("ds_decay", [&](const int val){ cv_ds_decay = val;});
	pMapPar.emplace("ds_spy", [&](const int val){ ds_spy = val;});
	pMapCv.emplace("ds_spy", [&](const int val){ cv_ds_spy = val;});
	pMapPar.emplace("hh1_trigger", [&](const int val){ hh1_trigger = val;});
	pMapTrig.emplace("hh1_trigger", [&](const int val){ trig_hh1_trigger = val;});
	pMapPar.emplace("hh1_mute", [&](const int val){ hh1_mute = val;});
	pMapTrig.emplace("hh1_mute", [&](const int val){ trig_hh1_mute = val;});
	pMapPar.emplace("hh1_lev", [&](const int val){ hh1_lev = val;});
	pMapCv.emplace("hh1_lev", [&](const int val){ cv_hh1_lev = val;});
	pMapPar.emplace("hh1_pan", [&](const int val){ hh1_pan = val;});
	pMapCv.emplace("hh1_pan", [&](const int val){ cv_hh1_pan = val;});
	pMapPar.emplace("hh1_fx1", [&](const int val){ hh1_fx1 = val;});
	pMapCv.emplace("hh1_fx1", [&](const int val){ cv_hh1_fx1 = val;});
	pMapPar.emplace("hh1_fx2", [&](const int val){ hh1_fx2 = val;});
	pMapCv.emplace("hh1_fx2", [&](const int val){ cv_hh1_fx2 = val;});
	pMapPar.emplace("hh1_accent", [&](const int val){ hh1_accent = val;});
	pMapCv.emplace("hh1_accent", [&](const int val){ cv_hh1_accent = val;});
	pMapPar.emplace("hh1_f0", [&](const int val){ hh1_f0 = val;});
	pMapCv.emplace("hh1_f0", [&](const int val){ cv_hh1_f0 = val;});
	pMapPar.emplace("hh1_tone", [&](const int val){ hh1_tone = val;});
	pMapCv.emplace("hh1_tone", [&](const int val){ cv_hh1_tone = val;});
	pMapPar.emplace("hh1_decay", [&](const int val){ hh1_decay = val;});
	pMapCv.emplace("hh1_decay", [&](const int val){ cv_hh1_decay = val;});
	pMapPar.emplace("hh1_noise", [&](const int val){ hh1_noise = val;});
	pMapCv.emplace("hh1_noise", [&](const int val){ cv_hh1_noise = val;});
	pMapPar.emplace("hh2_trigger", [&](const int val){ hh2_trigger = val;});
	pMapTrig.emplace("hh2_trigger", [&](const int val){ trig_hh2_trigger = val;});
	pMapPar.emplace("hh2_mute", [&](const int val){ hh2_mute = val;});
	pMapTrig.emplace("hh2_mute", [&](const int val){ trig_hh2_mute = val;});
	pMapPar.emplace("hh2_lev", [&](const int val){ hh2_lev = val;});
	pMapCv.emplace("hh2_lev", [&](const int val){ cv_hh2_lev = val;});
	pMapPar.emplace("hh2_pan", [&](const int val){ hh2_pan = val;});
	pMapCv.emplace("hh2_pan", [&](const int val){ cv_hh2_pan = val;});
	pMapPar.emplace("hh2_fx1", [&](const int val){ hh2_fx1 = val;});
	pMapCv.emplace("hh2_fx1", [&](const int val){ cv_hh2_fx1 = val;});
	pMapPar.emplace("hh2_fx2", [&](const int val){ hh2_fx2 = val;});
	pMapCv.emplace("hh2_fx2", [&](const int val){ cv_hh2_fx2 = val;});
	pMapPar.emplace("hh2_accent", [&](const int val){ hh2_accent = val;});
	pMapCv.emplace("hh2_accent", [&](const int val){ cv_hh2_accent = val;});
	pMapPar.emplace("hh2_f0", [&](const int val){ hh2_f0 = val;});
	pMapCv.emplace("hh2_f0", [&](const int val){ cv_hh2_f0 = val;});
	pMapPar.emplace("hh2_tone", [&](const int val){ hh2_tone = val;});
	pMapCv.emplace("hh2_tone", [&](const int val){ cv_hh2_tone = val;});
	pMapPar.emplace("hh2_decay", [&](const int val){ hh2_decay = val;});
	pMapCv.emplace("hh2_decay", [&](const int val){ cv_hh2_decay = val;});
	pMapPar.emplace("hh2_noise", [&](const int val){ hh2_noise = val;});
	pMapCv.emplace("hh2_noise", [&](const int val){ cv_hh2_noise = val;});
	pMapPar.emplace("rs_trigger", [&](const int val){ rs_trigger = val;});
	pMapTrig.emplace("rs_trigger", [&](const int val){ trig_rs_trigger = val;});
	pMapPar.emplace("rs_mute", [&](const int val){ rs_mute = val;});
	pMapTrig.emplace("rs_mute", [&](const int val){ trig_rs_mute = val;});
	pMapPar.emplace("rs_lev", [&](const int val){ rs_lev = val;});
	pMapCv.emplace("rs_lev", [&](const int val){ cv_rs_lev = val;});
	pMapPar.emplace("rs_pan", [&](const int val){ rs_pan = val;});
	pMapCv.emplace("rs_pan", [&](const int val){ cv_rs_pan = val;});
	pMapPar.emplace("rs_fx1", [&](const int val){ rs_fx1 = val;});
	pMapCv.emplace("rs_fx1", [&](const int val){ cv_rs_fx1 = val;});
	pMapPar.emplace("rs_fx2", [&](const int val){ rs_fx2 = val;});
	pMapCv.emplace("rs_fx2", [&](const int val){ cv_rs_fx2 = val;});
	pMapPar.emplace("rs_accent", [&](const int val){ rs_accent = val;});
	pMapCv.emplace("rs_accent", [&](const int val){ cv_rs_accent = val;});
	pMapPar.emplace("rs_f0", [&](const int val){ rs_f0 = val;});
	pMapCv.emplace("rs_f0", [&](const int val){ cv_rs_f0 = val;});
	pMapPar.emplace("rs_tone", [&](const int val){ rs_tone = val;});
	pMapCv.emplace("rs_tone", [&](const int val){ cv_rs_tone = val;});
	pMapPar.emplace("rs_decay", [&](const int val){ rs_decay = val;});
	pMapCv.emplace("rs_decay", [&](const int val){ cv_rs_decay = val;});
	pMapPar.emplace("rs_noise", [&](const int val){ rs_noise = val;});
	pMapCv.emplace("rs_noise", [&](const int val){ cv_rs_noise = val;});
	pMapPar.emplace("cl_trigger", [&](const int val){ cl_trigger = val;});
	pMapTrig.emplace("cl_trigger", [&](const int val){ trig_cl_trigger = val;});
	pMapPar.emplace("cl_mute", [&](const int val){ cl_mute = val;});
	pMapTrig.emplace("cl_mute", [&](const int val){ trig_cl_mute = val;});
	pMapPar.emplace("cl_lev", [&](const int val){ cl_lev = val;});
	pMapCv.emplace("cl_lev", [&](const int val){ cv_cl_lev = val;});
	pMapPar.emplace("cl_pan", [&](const int val){ cl_pan = val;});
	pMapCv.emplace("cl_pan", [&](const int val){ cv_cl_pan = val;});
	pMapPar.emplace("cl_fx1", [&](const int val){ cl_fx1 = val;});
	pMapCv.emplace("cl_fx1", [&](const int val){ cv_cl_fx1 = val;});
	pMapPar.emplace("cl_fx2", [&](const int val){ cl_fx2 = val;});
	pMapCv.emplace("cl_fx2", [&](const int val){ cv_cl_fx2 = val;});
	pMapPar.emplace("cl_f0", [&](const int val){ cl_f0 = val;});
	pMapCv.emplace("cl_f0", [&](const int val){ cv_cl_f0 = val;});
	pMapPar.emplace("cl_tone", [&](const int val){ cl_tone = val;});
	pMapCv.emplace("cl_tone", [&](const int val){ cv_cl_tone = val;});
	pMapPar.emplace("cl_decay", [&](const int val){ cl_decay = val;});
	pMapCv.emplace("cl_decay", [&](const int val){ cv_cl_decay = val;});
	pMapPar.emplace("cl_scale", [&](const int val){ cl_scale = val;});
	pMapCv.emplace("cl_scale", [&](const int val){ cv_cl_scale = val;});
	pMapPar.emplace("cl_transient", [&](const int val){ cl_transient = val;});
	pMapCv.emplace("cl_transient", [&](const int val){ cv_cl_transient = val;});
	pMapPar.emplace("s1_gate", [&](const int val){ s1_gate = val;});
	pMapTrig.emplace("s1_gate", [&](const int val){ trig_s1_gate = val;});
	pMapPar.emplace("s1_mute", [&](const int val){ s1_mute = val;});
	pMapTrig.emplace("s1_mute", [&](const int val){ trig_s1_mute = val;});
	pMapPar.emplace("s1_lev", [&](const int val){ s1_lev = val;});
	pMapCv.emplace("s1_lev", [&](const int val){ cv_s1_lev = val;});
	pMapPar.emplace("s1_pan", [&](const int val){ s1_pan = val;});
	pMapCv.emplace("s1_pan", [&](const int val){ cv_s1_pan = val;});
	pMapPar.emplace("s1_fx1", [&](const int val){ s1_fx1 = val;});
	pMapCv.emplace("s1_fx1", [&](const int val){ cv_s1_fx1 = val;});
	pMapPar.emplace("s1_fx2", [&](const int val){ s1_fx2 = val;});
	pMapCv.emplace("s1_fx2", [&](const int val){ cv_s1_fx2 = val;});
	pMapPar.emplace("s1_speed", [&](const int val){ s1_speed = val;});
	pMapCv.emplace("s1_speed", [&](const int val){ cv_s1_speed = val;});
	pMapPar.emplace("s1_pitch", [&](const int val){ s1_pitch = val;});
	pMapCv.emplace("s1_pitch", [&](const int val){ cv_s1_pitch = val;});
	pMapPar.emplace("s1_bank", [&](const int val){ s1_bank = val;});
	pMapCv.emplace("s1_bank", [&](const int val){ cv_s1_bank = val;});
	pMapPar.emplace("s1_slice", [&](const int val){ s1_slice = val;});
	pMapCv.emplace("s1_slice", [&](const int val){ cv_s1_slice = val;});
	pMapPar.emplace("s1_start", [&](const int val){ s1_start = val;});
	pMapCv.emplace("s1_start", [&](const int val){ cv_s1_start = val;});
	pMapPar.emplace("s1_end", [&](const int val){ s1_end = val;});
	pMapCv.emplace("s1_end", [&](const int val){ cv_s1_end = val;});
	pMapPar.emplace("s1_lp", [&](const int val){ s1_lp = val;});
	pMapTrig.emplace("s1_lp", [&](const int val){ trig_s1_lp = val;});
	pMapPar.emplace("s1_lp_pp", [&](const int val){ s1_lp_pp = val;});
	pMapTrig.emplace("s1_lp_pp", [&](const int val){ trig_s1_lp_pp = val;});
	pMapPar.emplace("s1_lp_pos", [&](const int val){ s1_lp_pos = val;});
	pMapCv.emplace("s1_lp_pos", [&](const int val){ cv_s1_lp_pos = val;});
	pMapPar.emplace("s1_atk", [&](const int val){ s1_atk = val;});
	pMapCv.emplace("s1_atk", [&](const int val){ cv_s1_atk = val;});
	pMapPar.emplace("s1_dcy", [&](const int val){ s1_dcy = val;});
	pMapCv.emplace("s1_dcy", [&](const int val){ cv_s1_dcy = val;});
	pMapPar.emplace("s1_eg2fm", [&](const int val){ s1_eg2fm = val;});
	pMapCv.emplace("s1_eg2fm", [&](const int val){ cv_s1_eg2fm = val;});
	pMapPar.emplace("s1_brr", [&](const int val){ s1_brr = val;});
	pMapCv.emplace("s1_brr", [&](const int val){ cv_s1_brr = val;});
	pMapPar.emplace("s1_ft", [&](const int val){ s1_ft = val;});
	pMapCv.emplace("s1_ft", [&](const int val){ cv_s1_ft = val;});
	pMapPar.emplace("s1_fc", [&](const int val){ s1_fc = val;});
	pMapCv.emplace("s1_fc", [&](const int val){ cv_s1_fc = val;});
	pMapPar.emplace("s1_fq", [&](const int val){ s1_fq = val;});
	pMapCv.emplace("s1_fq", [&](const int val){ cv_s1_fq = val;});
	pMapPar.emplace("s2_gate", [&](const int val){ s2_gate = val;});
	pMapTrig.emplace("s2_gate", [&](const int val){ trig_s2_gate = val;});
	pMapPar.emplace("s2_mute", [&](const int val){ s2_mute = val;});
	pMapTrig.emplace("s2_mute", [&](const int val){ trig_s2_mute = val;});
	pMapPar.emplace("s2_lev", [&](const int val){ s2_lev = val;});
	pMapCv.emplace("s2_lev", [&](const int val){ cv_s2_lev = val;});
	pMapPar.emplace("s2_pan", [&](const int val){ s2_pan = val;});
	pMapCv.emplace("s2_pan", [&](const int val){ cv_s2_pan = val;});
	pMapPar.emplace("s2_fx1", [&](const int val){ s2_fx1 = val;});
	pMapCv.emplace("s2_fx1", [&](const int val){ cv_s2_fx1 = val;});
	pMapPar.emplace("s2_fx2", [&](const int val){ s2_fx2 = val;});
	pMapCv.emplace("s2_fx2", [&](const int val){ cv_s2_fx2 = val;});
	pMapPar.emplace("s2_speed", [&](const int val){ s2_speed = val;});
	pMapCv.emplace("s2_speed", [&](const int val){ cv_s2_speed = val;});
	pMapPar.emplace("s2_pitch", [&](const int val){ s2_pitch = val;});
	pMapCv.emplace("s2_pitch", [&](const int val){ cv_s2_pitch = val;});
	pMapPar.emplace("s2_bank", [&](const int val){ s2_bank = val;});
	pMapCv.emplace("s2_bank", [&](const int val){ cv_s2_bank = val;});
	pMapPar.emplace("s2_slice", [&](const int val){ s2_slice = val;});
	pMapCv.emplace("s2_slice", [&](const int val){ cv_s2_slice = val;});
	pMapPar.emplace("s2_start", [&](const int val){ s2_start = val;});
	pMapCv.emplace("s2_start", [&](const int val){ cv_s2_start = val;});
	pMapPar.emplace("s2_end", [&](const int val){ s2_end = val;});
	pMapCv.emplace("s2_end", [&](const int val){ cv_s2_end = val;});
	pMapPar.emplace("s2_lp", [&](const int val){ s2_lp = val;});
	pMapTrig.emplace("s2_lp", [&](const int val){ trig_s2_lp = val;});
	pMapPar.emplace("s2_lp_pp", [&](const int val){ s2_lp_pp = val;});
	pMapTrig.emplace("s2_lp_pp", [&](const int val){ trig_s2_lp_pp = val;});
	pMapPar.emplace("s2_lp_pos", [&](const int val){ s2_lp_pos = val;});
	pMapCv.emplace("s2_lp_pos", [&](const int val){ cv_s2_lp_pos = val;});
	pMapPar.emplace("s2_atk", [&](const int val){ s2_atk = val;});
	pMapCv.emplace("s2_atk", [&](const int val){ cv_s2_atk = val;});
	pMapPar.emplace("s2_dcy", [&](const int val){ s2_dcy = val;});
	pMapCv.emplace("s2_dcy", [&](const int val){ cv_s2_dcy = val;});
	pMapPar.emplace("s2_eg2fm", [&](const int val){ s2_eg2fm = val;});
	pMapCv.emplace("s2_eg2fm", [&](const int val){ cv_s2_eg2fm = val;});
	pMapPar.emplace("s2_brr", [&](const int val){ s2_brr = val;});
	pMapCv.emplace("s2_brr", [&](const int val){ cv_s2_brr = val;});
	pMapPar.emplace("s2_ft", [&](const int val){ s2_ft = val;});
	pMapCv.emplace("s2_ft", [&](const int val){ cv_s2_ft = val;});
	pMapPar.emplace("s2_fc", [&](const int val){ s2_fc = val;});
	pMapCv.emplace("s2_fc", [&](const int val){ cv_s2_fc = val;});
	pMapPar.emplace("s2_fq", [&](const int val){ s2_fq = val;});
	pMapCv.emplace("s2_fq", [&](const int val){ cv_s2_fq = val;});
	pMapPar.emplace("s3_gate", [&](const int val){ s3_gate = val;});
	pMapTrig.emplace("s3_gate", [&](const int val){ trig_s3_gate = val;});
	pMapPar.emplace("s3_mute", [&](const int val){ s3_mute = val;});
	pMapTrig.emplace("s3_mute", [&](const int val){ trig_s3_mute = val;});
	pMapPar.emplace("s3_lev", [&](const int val){ s3_lev = val;});
	pMapCv.emplace("s3_lev", [&](const int val){ cv_s3_lev = val;});
	pMapPar.emplace("s3_pan", [&](const int val){ s3_pan = val;});
	pMapCv.emplace("s3_pan", [&](const int val){ cv_s3_pan = val;});
	pMapPar.emplace("s3_fx1", [&](const int val){ s3_fx1 = val;});
	pMapCv.emplace("s3_fx1", [&](const int val){ cv_s3_fx1 = val;});
	pMapPar.emplace("s3_fx2", [&](const int val){ s3_fx2 = val;});
	pMapCv.emplace("s3_fx2", [&](const int val){ cv_s3_fx2 = val;});
	pMapPar.emplace("s3_speed", [&](const int val){ s3_speed = val;});
	pMapCv.emplace("s3_speed", [&](const int val){ cv_s3_speed = val;});
	pMapPar.emplace("s3_pitch", [&](const int val){ s3_pitch = val;});
	pMapCv.emplace("s3_pitch", [&](const int val){ cv_s3_pitch = val;});
	pMapPar.emplace("s3_bank", [&](const int val){ s3_bank = val;});
	pMapCv.emplace("s3_bank", [&](const int val){ cv_s3_bank = val;});
	pMapPar.emplace("s3_slice", [&](const int val){ s3_slice = val;});
	pMapCv.emplace("s3_slice", [&](const int val){ cv_s3_slice = val;});
	pMapPar.emplace("s3_start", [&](const int val){ s3_start = val;});
	pMapCv.emplace("s3_start", [&](const int val){ cv_s3_start = val;});
	pMapPar.emplace("s3_end", [&](const int val){ s3_end = val;});
	pMapCv.emplace("s3_end", [&](const int val){ cv_s3_end = val;});
	pMapPar.emplace("s3_lp", [&](const int val){ s3_lp = val;});
	pMapTrig.emplace("s3_lp", [&](const int val){ trig_s3_lp = val;});
	pMapPar.emplace("s3_lp_pp", [&](const int val){ s3_lp_pp = val;});
	pMapTrig.emplace("s3_lp_pp", [&](const int val){ trig_s3_lp_pp = val;});
	pMapPar.emplace("s3_lp_pos", [&](const int val){ s3_lp_pos = val;});
	pMapCv.emplace("s3_lp_pos", [&](const int val){ cv_s3_lp_pos = val;});
	pMapPar.emplace("s3_atk", [&](const int val){ s3_atk = val;});
	pMapCv.emplace("s3_atk", [&](const int val){ cv_s3_atk = val;});
	pMapPar.emplace("s3_dcy", [&](const int val){ s3_dcy = val;});
	pMapCv.emplace("s3_dcy", [&](const int val){ cv_s3_dcy = val;});
	pMapPar.emplace("s3_eg2fm", [&](const int val){ s3_eg2fm = val;});
	pMapCv.emplace("s3_eg2fm", [&](const int val){ cv_s3_eg2fm = val;});
	pMapPar.emplace("s3_brr", [&](const int val){ s3_brr = val;});
	pMapCv.emplace("s3_brr", [&](const int val){ cv_s3_brr = val;});
	pMapPar.emplace("s3_ft", [&](const int val){ s3_ft = val;});
	pMapCv.emplace("s3_ft", [&](const int val){ cv_s3_ft = val;});
	pMapPar.emplace("s3_fc", [&](const int val){ s3_fc = val;});
	pMapCv.emplace("s3_fc", [&](const int val){ cv_s3_fc = val;});
	pMapPar.emplace("s3_fq", [&](const int val){ s3_fq = val;});
	pMapCv.emplace("s3_fq", [&](const int val){ cv_s3_fq = val;});
	pMapPar.emplace("s4_gate", [&](const int val){ s4_gate = val;});
	pMapTrig.emplace("s4_gate", [&](const int val){ trig_s4_gate = val;});
	pMapPar.emplace("s4_mute", [&](const int val){ s4_mute = val;});
	pMapTrig.emplace("s4_mute", [&](const int val){ trig_s4_mute = val;});
	pMapPar.emplace("s4_lev", [&](const int val){ s4_lev = val;});
	pMapCv.emplace("s4_lev", [&](const int val){ cv_s4_lev = val;});
	pMapPar.emplace("s4_pan", [&](const int val){ s4_pan = val;});
	pMapCv.emplace("s4_pan", [&](const int val){ cv_s4_pan = val;});
	pMapPar.emplace("s4_fx1", [&](const int val){ s4_fx1 = val;});
	pMapCv.emplace("s4_fx1", [&](const int val){ cv_s4_fx1 = val;});
	pMapPar.emplace("s4_fx2", [&](const int val){ s4_fx2 = val;});
	pMapCv.emplace("s4_fx2", [&](const int val){ cv_s4_fx2 = val;});
	pMapPar.emplace("s4_speed", [&](const int val){ s4_speed = val;});
	pMapCv.emplace("s4_speed", [&](const int val){ cv_s4_speed = val;});
	pMapPar.emplace("s4_pitch", [&](const int val){ s4_pitch = val;});
	pMapCv.emplace("s4_pitch", [&](const int val){ cv_s4_pitch = val;});
	pMapPar.emplace("s4_bank", [&](const int val){ s4_bank = val;});
	pMapCv.emplace("s4_bank", [&](const int val){ cv_s4_bank = val;});
	pMapPar.emplace("s4_slice", [&](const int val){ s4_slice = val;});
	pMapCv.emplace("s4_slice", [&](const int val){ cv_s4_slice = val;});
	pMapPar.emplace("s4_start", [&](const int val){ s4_start = val;});
	pMapCv.emplace("s4_start", [&](const int val){ cv_s4_start = val;});
	pMapPar.emplace("s4_end", [&](const int val){ s4_end = val;});
	pMapCv.emplace("s4_end", [&](const int val){ cv_s4_end = val;});
	pMapPar.emplace("s4_lp", [&](const int val){ s4_lp = val;});
	pMapTrig.emplace("s4_lp", [&](const int val){ trig_s4_lp = val;});
	pMapPar.emplace("s4_lp_pp", [&](const int val){ s4_lp_pp = val;});
	pMapTrig.emplace("s4_lp_pp", [&](const int val){ trig_s4_lp_pp = val;});
	pMapPar.emplace("s4_lp_pos", [&](const int val){ s4_lp_pos = val;});
	pMapCv.emplace("s4_lp_pos", [&](const int val){ cv_s4_lp_pos = val;});
	pMapPar.emplace("s4_atk", [&](const int val){ s4_atk = val;});
	pMapCv.emplace("s4_atk", [&](const int val){ cv_s4_atk = val;});
	pMapPar.emplace("s4_dcy", [&](const int val){ s4_dcy = val;});
	pMapCv.emplace("s4_dcy", [&](const int val){ cv_s4_dcy = val;});
	pMapPar.emplace("s4_eg2fm", [&](const int val){ s4_eg2fm = val;});
	pMapCv.emplace("s4_eg2fm", [&](const int val){ cv_s4_eg2fm = val;});
	pMapPar.emplace("s4_brr", [&](const int val){ s4_brr = val;});
	pMapCv.emplace("s4_brr", [&](const int val){ cv_s4_brr = val;});
	pMapPar.emplace("s4_ft", [&](const int val){ s4_ft = val;});
	pMapCv.emplace("s4_ft", [&](const int val){ cv_s4_ft = val;});
	pMapPar.emplace("s4_fc", [&](const int val){ s4_fc = val;});
	pMapCv.emplace("s4_fc", [&](const int val){ cv_s4_fc = val;});
	pMapPar.emplace("s4_fq", [&](const int val){ s4_fq = val;});
	pMapCv.emplace("s4_fq", [&](const int val){ cv_s4_fq = val;});
	pMapPar.emplace("in_mute", [&](const int val){ in_mute = val;});
	pMapTrig.emplace("in_mute", [&](const int val){ trig_in_mute = val;});
	pMapPar.emplace("in_lev", [&](const int val){ in_lev = val;});
	pMapCv.emplace("in_lev", [&](const int val){ cv_in_lev = val;});
	pMapPar.emplace("in_pan", [&](const int val){ in_pan = val;});
	pMapCv.emplace("in_pan", [&](const int val){ cv_in_pan = val;});
	pMapPar.emplace("in_fx1", [&](const int val){ in_fx1 = val;});
	pMapCv.emplace("in_fx1", [&](const int val){ cv_in_fx1 = val;});
	pMapPar.emplace("in_fx2", [&](const int val){ in_fx2 = val;});
	pMapCv.emplace("in_fx2", [&](const int val){ cv_in_fx2 = val;});
	pMapPar.emplace("td3_trigger", [&](const int val){ td3_trigger = val;});
	pMapTrig.emplace("td3_trigger", [&](const int val){ trig_td3_trigger = val;});
	pMapPar.emplace("td3_sync_trig", [&](const int val){ td3_sync_trig = val;});
	pMapTrig.emplace("td3_sync_trig", [&](const int val){ trig_td3_sync_trig = val;});
	pMapPar.emplace("td3_pitch", [&](const int val){ td3_pitch = val;});
	pMapCv.emplace("td3_pitch", [&](const int val){ cv_td3_pitch = val;});
	pMapPar.emplace("td3_shape", [&](const int val){ td3_shape = val;});
	pMapCv.emplace("td3_shape", [&](const int val){ cv_td3_shape = val;});
	pMapPar.emplace("td3_param_0", [&](const int val){ td3_param_0 = val;});
	pMapCv.emplace("td3_param_0", [&](const int val){ cv_td3_param_0 = val;});
	pMapPar.emplace("td3_param_1", [&](const int val){ td3_param_1 = val;});
	pMapCv.emplace("td3_param_1", [&](const int val){ cv_td3_param_1 = val;});
	pMapPar.emplace("td3_gain", [&](const int val){ td3_gain = val;});
	pMapCv.emplace("td3_gain", [&](const int val){ cv_td3_gain = val;});
	pMapPar.emplace("td3_filter_type", [&](const int val){ td3_filter_type = val;});
	pMapCv.emplace("td3_filter_type", [&](const int val){ cv_td3_filter_type = val;});
	pMapPar.emplace("td3_cutoff", [&](const int val){ td3_cutoff = val;});
	pMapCv.emplace("td3_cutoff", [&](const int val){ cv_td3_cutoff = val;});
	pMapPar.emplace("td3_resonance", [&](const int val){ td3_resonance = val;});
	pMapCv.emplace("td3_resonance", [&](const int val){ cv_td3_resonance = val;});
	pMapPar.emplace("td3_envelope", [&](const int val){ td3_envelope = val;});
	pMapCv.emplace("td3_envelope", [&](const int val){ cv_td3_envelope = val;});
	pMapPar.emplace("td3_saturation", [&](const int val){ td3_saturation = val;});
	pMapCv.emplace("td3_saturation", [&](const int val){ cv_td3_saturation = val;});
	pMapPar.emplace("td3_drive", [&](const int val){ td3_drive = val;});
	pMapCv.emplace("td3_drive", [&](const int val){ cv_td3_drive = val;});
	pMapPar.emplace("td3_accent", [&](const int val){ td3_accent = val;});
	pMapTrig.emplace("td3_accent", [&](const int val){ trig_td3_accent = val;});
	pMapPar.emplace("td3_accent_level", [&](const int val){ td3_accent_level = val;});
	pMapCv.emplace("td3_accent_level", [&](const int val){ cv_td3_accent_level = val;});
	pMapPar.emplace("td3_slide", [&](const int val){ td3_slide = val;});
	pMapTrig.emplace("td3_slide", [&](const int val){ trig_td3_slide = val;});
	pMapPar.emplace("td3_slide_level", [&](const int val){ td3_slide_level = val;});
	pMapCv.emplace("td3_slide_level", [&](const int val){ cv_td3_slide_level = val;});
	pMapPar.emplace("td3_decay_vca", [&](const int val){ td3_decay_vca = val;});
	pMapCv.emplace("td3_decay_vca", [&](const int val){ cv_td3_decay_vca = val;});
	pMapPar.emplace("td3_decay_vcf", [&](const int val){ td3_decay_vcf = val;});
	pMapCv.emplace("td3_decay_vcf", [&](const int val){ cv_td3_decay_vcf = val;});
	pMapPar.emplace("td3_p0_amt", [&](const int val){ td3_p0_amt = val;});
	pMapCv.emplace("td3_p0_amt", [&](const int val){ cv_td3_p0_amt = val;});
	pMapPar.emplace("td3_p1_amt", [&](const int val){ td3_p1_amt = val;});
	pMapCv.emplace("td3_p1_amt", [&](const int val){ cv_td3_p1_amt = val;});
	pMapPar.emplace("td3_lev", [&](const int val){ td3_lev = val;});
	pMapCv.emplace("td3_lev", [&](const int val){ cv_td3_lev = val;});
	pMapPar.emplace("td3_pan", [&](const int val){ td3_pan = val;});
	pMapCv.emplace("td3_pan", [&](const int val){ cv_td3_pan = val;});
	pMapPar.emplace("td3_fx1", [&](const int val){ td3_fx1 = val;});
	pMapCv.emplace("td3_fx1", [&](const int val){ cv_td3_fx1 = val;});
	pMapPar.emplace("td3_fx2", [&](const int val){ td3_fx2 = val;});
	pMapCv.emplace("td3_fx2", [&](const int val){ cv_td3_fx2 = val;});
	pMapPar.emplace("fx1_time_ms", [&](const int val){ fx1_time_ms = val;});
	pMapCv.emplace("fx1_time_ms", [&](const int val){ cv_fx1_time_ms = val;});
	pMapPar.emplace("fx1_sync", [&](const int val){ fx1_sync = val;});
	pMapTrig.emplace("fx1_sync", [&](const int val){ trig_fx1_sync = val;});
	pMapPar.emplace("fx1_freeze", [&](const int val){ fx1_freeze = val;});
	pMapTrig.emplace("fx1_freeze", [&](const int val){ trig_fx1_freeze = val;});
	pMapPar.emplace("fx1_tape_digital", [&](const int val){ fx1_tape_digital = val;});
	pMapTrig.emplace("fx1_tape_digital", [&](const int val){ trig_fx1_tape_digital = val;});
	pMapPar.emplace("fx1_st_width", [&](const int val){ fx1_st_width = val;});
	pMapCv.emplace("fx1_st_width", [&](const int val){ cv_fx1_st_width = val;});
	pMapPar.emplace("fx1_fx_send", [&](const int val){ fx1_fx_send = val;});
	pMapCv.emplace("fx1_fx_send", [&](const int val){ cv_fx1_fx_send = val;});
	pMapPar.emplace("fx1_feedback", [&](const int val){ fx1_feedback = val;});
	pMapCv.emplace("fx1_feedback", [&](const int val){ cv_fx1_feedback = val;});
	pMapPar.emplace("fx1_base", [&](const int val){ fx1_base = val;});
	pMapCv.emplace("fx1_base", [&](const int val){ cv_fx1_base = val;});
	pMapPar.emplace("fx1_width", [&](const int val){ fx1_width = val;});
	pMapCv.emplace("fx1_width", [&](const int val){ cv_fx1_width = val;});
	pMapPar.emplace("fx2_time", [&](const int val){ fx2_time = val;});
	pMapCv.emplace("fx2_time", [&](const int val){ cv_fx2_time = val;});
	pMapPar.emplace("fx2_lp", [&](const int val){ fx2_lp = val;});
	pMapCv.emplace("fx2_lp", [&](const int val){ cv_fx2_lp = val;});
	pMapPar.emplace("c_thres", [&](const int val){ c_thres = val;});
	pMapCv.emplace("c_thres", [&](const int val){ cv_c_thres = val;});
	pMapPar.emplace("c_ratio", [&](const int val){ c_ratio = val;});
	pMapCv.emplace("c_ratio", [&](const int val){ cv_c_ratio = val;});
	pMapPar.emplace("c_atk", [&](const int val){ c_atk = val;});
	pMapCv.emplace("c_atk", [&](const int val){ cv_c_atk = val;});
	pMapPar.emplace("c_rel", [&](const int val){ c_rel = val;});
	pMapCv.emplace("c_rel", [&](const int val){ cv_c_rel = val;});
	pMapPar.emplace("c_lpf", [&](const int val){ c_lpf = val;});
	pMapTrig.emplace("c_lpf", [&](const int val){ trig_c_lpf = val;});
	pMapPar.emplace("c_gain", [&](const int val){ c_gain = val;});
	pMapCv.emplace("c_gain", [&](const int val){ cv_c_gain = val;});
	pMapPar.emplace("c_mix", [&](const int val){ c_mix = val;});
	pMapCv.emplace("c_mix", [&](const int val){ cv_c_mix = val;});
	pMapPar.emplace("c_dly_level", [&](const int val){ c_dly_level = val;});
	pMapCv.emplace("c_dly_level", [&](const int val){ cv_c_dly_level = val;});
	pMapPar.emplace("c_rev_level", [&](const int val){ c_rev_level = val;});
	pMapCv.emplace("c_rev_level", [&](const int val){ cv_c_rev_level = val;});
	pMapPar.emplace("sum_mute", [&](const int val){ sum_mute = val;});
	pMapTrig.emplace("sum_mute", [&](const int val){ trig_sum_mute = val;});
	pMapPar.emplace("sum_lev", [&](const int val){ sum_lev = val;});
	pMapCv.emplace("sum_lev", [&](const int val){ cv_sum_lev = val;});
	pMapPar.emplace("fx1_amount", [&](const int val){ fx1_amount = val;});
	pMapCv.emplace("fx1_amount", [&](const int val){ cv_fx1_amount = val;});
	pMapPar.emplace("fx2_amount", [&](const int val){ fx2_amount = val;});
	pMapCv.emplace("fx2_amount", [&](const int val){ cv_fx2_amount = val;});
	isStereo = true;
	id = "DrumRack";
	// sectionCpp0
}
