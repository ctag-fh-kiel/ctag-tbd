#include <tbd/sounds/SoundProcessorDrumRack.hpp>


using namespace tbd::sounds;

void SoundProcessorDrumRack::Process(const audio::ProcessData& data){
    // Analog Bass Drum
    MK_BOOL_PAR(bABMute, ab_mute)
    MK_BOOL_PAR(bABTrig, ab_trigger)
    if (bABTrig != abd_trig_prev){
        abd_trig_prev = bABTrig;
    }
    else{
        bABTrig = false;
    }

    MK_FLT_PAR_ABS(fABLev, ab_lev, 4095.f, 2.f)
    fABLev *= fABLev;
    MK_FLT_PAR_ABS_PAN(fABPan, ab_pan, 4095.f, 1.f)
    if (!bABMute){
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
        data_ptrs[0] = abd_out;
    }
    else{
        data_ptrs[0] = silence;
    }

    // Analog Snare Drum
    MK_BOOL_PAR(bASMute, as_mute)
    MK_BOOL_PAR(bASTrig, as_trigger)
    if (bASTrig != asd_trig_prev){
        asd_trig_prev = bASTrig;
    }
    else{
        bASTrig = false;
    }

    MK_FLT_PAR_ABS(fASLev, as_lev, 4095.f, 2.f)
    fASLev *= fASLev;
    MK_FLT_PAR_ABS_PAN(fASPan, as_pan, 4095.f, 1.f)


    if (!bASMute){
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
        data_ptrs[1] = asd_out;
    }
    else{
        data_ptrs[1] = silence;
    }

    // Digital Bass Drum
    MK_BOOL_PAR(bDBMute, db_mute)
    MK_BOOL_PAR(bDBTrig, db_trigger)
    if (bDBTrig != dbd_trig_prev){
        dbd_trig_prev = bDBTrig;
    }
    else{
        bDBTrig = false;
    }

    MK_FLT_PAR_ABS(fDBLev, db_lev, 4095.f, 2.f)
    fDBLev *= fDBLev;
    MK_FLT_PAR_ABS_PAN(fDBPan, db_pan, 4095.f, 1.f)

    if (!bDBMute){
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
        data_ptrs[2] = dbd_out;
    }
    else{
        data_ptrs[2] = silence;
    }

    // Digital Snare Drum
    MK_BOOL_PAR(bDSMute, ds_mute)
    MK_BOOL_PAR(bDSTrig, ds_trigger)
    if (bDSTrig != dsd_trig_prev){
        dsd_trig_prev = bDSTrig;
    }
    else{
        bDSTrig = false;
    }

    MK_FLT_PAR_ABS(fDSLev, ds_lev, 4095.f, 2.f)
    fDSLev *= fDSLev;
    MK_FLT_PAR_ABS_PAN(fDSPan, ds_pan, 4095.f, 1.f)


    if (!bDSMute){
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
        data_ptrs[3] = dsd_out;
    }
    else{
        data_ptrs[3] = silence;
    }

    // HiHat 1
    MK_BOOL_PAR(bHH1Mute, hh1_mute)
    MK_BOOL_PAR(bHH1Trig, hh1_trigger)
    if (bHH1Trig != hh1_trig_prev){
        hh1_trig_prev = bHH1Trig;
    }
    else{
        bHH1Trig = false;
    }

    MK_FLT_PAR_ABS(fHH1Lev, hh1_lev, 4095.f, 2.f)
    fHH1Lev *= fHH1Lev;
    MK_FLT_PAR_ABS_PAN(fHH1Pan, hh1_pan, 4095.f, 1.f)


    if (!bHH1Mute){
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
        data_ptrs[4] = hh1_out;
    }
    else{
        data_ptrs[4] = silence;
    }

    // HiHat 2
    MK_BOOL_PAR(bHH2Mute, hh2_mute)
    MK_BOOL_PAR(bHH2Trig, hh2_trigger)
    if (bHH2Trig != hh2_trig_prev){
        hh2_trig_prev = bHH2Trig;
    }
    else{
        bHH2Trig = false;
    }

    MK_FLT_PAR_ABS(fHH2Lev, hh2_lev, 4095.f, 2.f)
    fHH2Lev *= fHH2Lev;
    MK_FLT_PAR_ABS_PAN(fHH2Pan, hh2_pan, 4095.f, 1.f)


    if (!bHH2Mute){
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
        data_ptrs[5] = hh2_out;
    }
    else{
        data_ptrs[5] = silence;
    }

    // rimshot
    MK_BOOL_PAR(bRSMute, rs_mute)
    MK_FLT_PAR_ABS(fRSLev, rs_lev, 4095.f, 2.f)
    fRSLev *= fRSLev;
    MK_FLT_PAR_ABS_PAN(fRSPan, rs_pan, 4095.f, 1.f)
    if (!bRSMute){
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
        data_ptrs[6] = rs_out;
    }
    else{
        data_ptrs[6] = silence;
    }


    // clap
    MK_BOOL_PAR(bCLMute, cl_mute)
    MK_FLT_PAR_ABS(fCLLev, cl_lev, 4095.f, 2.f)
    fCLLev *= fCLLev;
    MK_FLT_PAR_ABS_PAN(fCLPan, cl_pan, 4095.f, 1.f)
    if (!bCLMute){
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
        data_ptrs[7] = cl_out;
    }
    else{
        data_ptrs[7] = silence;
    }


    // romplers
    uint32_t firstNonWtSlice = sampleRom.GetFirstNonWaveTableSlice();
    float fS1Lev = 0.f, fS1Pan = 0.f;
    MK_BOOL_PAR(bMuteS1, s1_mute)
    if (!bMuteS1){
        MK_BOOL_PAR(bGateS1, s1_gate)
        rompler[0].params.gate = bGateS1;
        fS1Lev = s1_lev / 4095.f * 1.5f;
        if (cv_s1_lev != -1) fS1Lev += fabsf(data.cv[cv_s1_lev]);
        fS1Lev *= fS1Lev;
        fS1Pan = (s1_pan / 4095.f + 1.f) / 2.f * 1.f;
        if (cv_s1_pan != -1) fS1Pan = fabsf(data.cv[cv_s1_pan]) * 1.f;
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
        rompler[0].params.filterType = static_cast<sound_utils::synthesis::RomplerVoiceMinimal::FilterType>(iS1FType);
        rompler[0].Process(s1_out, 32);
        data_ptrs[8] = s1_out;
    }
    else{
        data_ptrs[8] = silence;
    }

    float fS2Lev = 0.f, fS2Pan = 0.f;
    MK_BOOL_PAR(bMuteS2, s2_mute)
    if (!bMuteS2){
        MK_BOOL_PAR(bGateS2, s2_gate)
        rompler[1].params.gate = bGateS2;
        fS2Lev = s2_lev / 4095.f * 1.5f;
        if (cv_s2_lev != -1) fS2Lev += fabsf(data.cv[cv_s2_lev]);
        fS2Lev *= fS2Lev;
        fS2Pan = (s2_pan / 4095.f + 1.f) / 2.f * 1.f;
        if (cv_s2_pan != -1) fS2Pan = fabsf(data.cv[cv_s2_pan]) * 1.f;
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
        rompler[1].params.filterType = static_cast<sound_utils::synthesis::RomplerVoiceMinimal::FilterType>(iS2FType);
        rompler[1].Process(s2_out, 32);
        data_ptrs[9] = s2_out;
    }
    else{
        data_ptrs[9] = silence;
    }

    float fS3Lev = 0.f, fS3Pan = 0.f;;
    MK_BOOL_PAR(bMuteS3, s3_mute)
    if (!bMuteS3){
        MK_BOOL_PAR(bGateS3, s3_gate)
        rompler[2].params.gate = bGateS3;
        fS3Lev = s3_lev / 4095.f * 1.5f;
        if (cv_s3_lev != -1) fS3Lev += fabsf(data.cv[cv_s3_lev]);
        fS3Lev *= fS3Lev;
        fS3Pan = (s3_pan / 4095.f + 1.f) / 2.f * 1.f;
        if (cv_s3_pan != -1) fS3Pan = fabsf(data.cv[cv_s3_pan]) * 1.f;
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
        rompler[2].params.filterType = static_cast<sound_utils::synthesis::RomplerVoiceMinimal::FilterType>(iS3FType);
        rompler[2].Process(s3_out, 32);
        data_ptrs[10] = s3_out;
    }
    else{
        data_ptrs[10] = silence;
    }

    float fS4Lev = 0.f, fS4Pan = 0.f;;
    MK_BOOL_PAR(bMuteS4, s4_mute)
    if (!bMuteS4){
        MK_BOOL_PAR(bGateS4, s4_gate)
        rompler[3].params.gate = bGateS4;
        fS4Lev = s4_lev / 4095.f * 1.5f;
        if (cv_s4_lev != -1) fS4Lev += fabsf(data.cv[cv_s4_lev]);
        fS4Lev *= fS4Lev;
        fS4Pan = (s4_pan / 4095.f + 1.f) / 2.f * 1.f;
        if (cv_s4_pan != -1) fS4Pan = fabsf(data.cv[cv_s4_pan]) * 1.f;
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
        rompler[3].params.filterType = static_cast<sound_utils::synthesis::RomplerVoiceMinimal::FilterType>(iS4FType);
        rompler[3].Process(s4_out, 32);
        data_ptrs[11] = s4_out;
    }
    else{
        data_ptrs[11] = silence;
    }


    // sum compressor
    MK_FLT_PAR_ABS_MIN_MAX(fCompThresdB, c_thres, 4095.f, -80.f, 0.f)
    sumCompressor.setThresh(fCompThresdB);
    MK_FLT_PAR_ABS_MIN_MAX(fCompAtk, c_atk, 4095.f, 0.3f, 30.f)
    sumCompressor.setAttack(fCompAtk);
    MK_FLT_PAR_ABS_MIN_MAX(fCompRel, c_rel, 4095.f, 40.f, 2000.f)
    sumCompressor.setRelease(fCompRel);
    MK_FLT_PAR_ABS_MIN_MAX(fCompRatio, c_ratio, 4095.f, 0.0001f, 1.25f)
    sumCompressor.setRatio(fCompRatio);
    MK_BOOL_PAR(bSideChainLPF, c_lpf)
    MK_FLT_PAR_ABS_PAN(fCompMix, c_mix, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fCompMUPGain, c_gain, 4095.f, 0.f, 60.f) // in dB
    if (fCompMUPGain != fCompMUPGain_pre){
        fCompMUPGain = chunkware_simple::dB2lin(fCompMUPGain);
        fCompMUPGain_pre = fCompMUPGain;
    }

    // overall mix
    MK_BOOL_PAR(bSumMute, sum_mute)
    MK_FLT_PAR_ABS(fMixLevel, sum_lev, 4095.f, 3.f)
    fMixLevel *= fMixLevel;

    if (bSumMute){
        memset(data.buf, 0, 32 * 2 * sizeof(float));
        return;
    }

    // render final buffer
    // calc volumes
    float lev_l[12] = {
        fABLev * (1.f - fABPan),
        fASLev * (1.f - fASPan),
        fDBLev * (1.f - fDBPan),
        fDSLev * (1.f - fDSPan),
        fHH1Lev * (1.f - fHH1Pan),
        fHH2Lev * (1.f - fHH2Pan),
        fRSLev * (1.f - fRSPan),
        fCLLev * (1.f - fCLPan),
        fS1Lev * (1.f - fS1Pan),
        fS2Lev * (1.f - fS2Pan),
        fS3Lev * (1.f - fS3Pan),
        fS4Lev * (1.f - fS4Pan)
    };
    float lev_r[12] = {
        fABLev * fABPan,
        fASLev * fASPan,
        fDBLev * fDBPan,
        fDSLev * fDSPan,
        fHH1Lev * fHH1Pan,
        fHH2Lev * fHH2Pan,
        fRSLev * fRSPan,
        fCLLev * fCLPan,
        fS1Lev * fS1Pan,
        fS2Lev * fS2Pan,
        fS3Lev * fS3Pan,
        fS4Lev * fS4Pan
    };
    for (int i = 0; i < 32; i++){
        float fVal_l = 0.f;
        float fVal_r = 0.f;
        fVal_l += data_ptrs[0][i] * lev_l[0];
        fVal_l += data_ptrs[1][i] * lev_l[1];
        fVal_l += data_ptrs[2][i] * lev_l[2];
        fVal_l += data_ptrs[3][i] * lev_l[3];
        fVal_l += data_ptrs[4][i] * lev_l[4];
        fVal_l += data_ptrs[5][i] * lev_l[5];
        fVal_l += data_ptrs[6][i] * lev_l[6];
        fVal_l += data_ptrs[7][i] * lev_l[7];

        fVal_l += data_ptrs[8][i] * lev_l[8];
        fVal_l += data_ptrs[9][i] * lev_l[9];
        fVal_l += data_ptrs[10][i] * lev_l[10];
        fVal_l += data_ptrs[11][i] * lev_l[11];

        fVal_r += data_ptrs[0][i] * lev_r[0];
        fVal_r += data_ptrs[1][i] * lev_r[1];
        fVal_r += data_ptrs[2][i] * lev_r[2];
        fVal_r += data_ptrs[3][i] * lev_r[3];
        fVal_r += data_ptrs[4][i] * lev_r[4];
        fVal_r += data_ptrs[5][i] * lev_r[5];
        fVal_r += data_ptrs[6][i] * lev_r[6];
        fVal_r += data_ptrs[7][i] * lev_r[7];

        fVal_r += data_ptrs[8][i] * lev_r[8];
        fVal_r += data_ptrs[9][i] * lev_r[9];
        fVal_r += data_ptrs[10][i] * lev_r[10];
        fVal_r += data_ptrs[11][i] * lev_r[11];

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
}

void SoundProcessorDrumRack::Init(std::size_t blockSize, void* blockPtr){
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
SoundProcessorDrumRack::~SoundProcessorDrumRack(){
    // no explicit freeing for blockMem needed, done by ctagSPAllocator
    // explicit free is only needed when using heap_caps_malloc() with MALLOC_CAPS_SPIRAM
}
