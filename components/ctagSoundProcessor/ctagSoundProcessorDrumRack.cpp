#include "ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

void ctagSoundProcessorDrumRack::Process(const ProcessData &data) {

    // Analog Bass Drum
    MK_BOOL_PAR(bABTrig, ab_trigger)
    if(bABTrig != abd_trig_prev) {
        abd_trig_prev = bABTrig;
    }else{
        bABTrig = false;
    }

    MK_FLT_PAR_ABS(fABLev, ab_lev, 4095.f, 4.f)
    fABLev *= fABLev;
    MK_FLT_PAR_ABS_PAN(fABPan, ab_pan, 4095.f, 1.f)
    MK_BOOL_PAR(bABSus, ab_sus)
    MK_FLT_PAR_ABS(fABAccent, ab_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fABF0, ab_f0, 4095.f, 0.0001f, 0.01f)
    MK_FLT_PAR_ABS(fABTone, ab_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fABDecay, ab_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fABAfm, ab_a_fm, 4095.f, 0.f, 100.f)
    MK_FLT_PAR_ABS_MIN_MAX(fABSfm, ab_s_fm, 4095.f, 0.f, 100.f)

    abd.Render(
            bABSus,
            bABTrig,
            fABAccent,
            fABF0,
            fABTone,
            fABDecay,
            fABAfm,
            fABSfm,
            abd_out,
            32);

    // Analog Snare Drum
    MK_BOOL_PAR(bASTrig, as_trigger)
    if(bASTrig != asd_trig_prev) {
        asd_trig_prev = bASTrig;
    }else{
        bASTrig = false;
    }

    MK_FLT_PAR_ABS(fASLev, as_lev, 4095.f, 4.f)
    fASLev *= fASLev;
    MK_FLT_PAR_ABS_PAN(fASPan, as_pan, 4095.f, 1.f)
    MK_BOOL_PAR(bASSus, as_sus)
    MK_FLT_PAR_ABS(fASAccent, as_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fASF0, as_f0, 4095.f, 0.001f, 0.01f)
    MK_FLT_PAR_ABS(fASTone, as_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fASDecay, as_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fASAspy, as_a_spy, 4095.f, 1.f)

    asd.Render(
            bASSus,
            bASTrig,
            fASAccent,
            fASF0,
            fASTone,
            fASDecay,
            fASAspy,
            asd_out,
            32);

    // Digital Bass Drum
    MK_BOOL_PAR(bDBTrig, db_trigger)
    if(bDBTrig != dbd_trig_prev) {
        dbd_trig_prev = bDBTrig;
    }else{
        bDBTrig = false;
    }

    MK_FLT_PAR_ABS(fDBLev, db_lev, 4095.f, 4.f)
    fDBLev *= fDBLev;
    MK_FLT_PAR_ABS_PAN(fDBPan, db_pan, 4095.f, 1.f)
    MK_BOOL_PAR(bDBSus, db_sus)
    MK_FLT_PAR_ABS(fDBAccent, db_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fDBF0, db_f0, 4095.f, 0.0005f, 0.01f)
    MK_FLT_PAR_ABS(fDBTone, db_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDBDecay, db_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDBDirty, db_dirty, 4095.f, 5.f)
    MK_FLT_PAR_ABS(fDBFmEnv, db_fm_env, 4095.f, 5.f)
    MK_FLT_PAR_ABS(fDBFmDcy, db_fm_dcy, 4095.f, 4.f)

    dbd.Render(
            bDBSus,
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

    // Digital Snare Drum
    MK_BOOL_PAR(bDSTrig, ds_trigger)
    if(bDSTrig != dsd_trig_prev) {
        dsd_trig_prev = bDSTrig;
    }else{
        bDSTrig = false;
    }

    MK_FLT_PAR_ABS(fDSLev, ds_lev, 4095.f, 4.f)
    fDSLev *= fDSLev;
    MK_FLT_PAR_ABS_PAN(fDSPan, ds_pan, 4095.f, 1.f)
    MK_BOOL_PAR(bDSSus, ds_sus)
    MK_FLT_PAR_ABS(fDSAccent, ds_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fDSF0, ds_f0, 4095.f, 0.0008f, 0.01f)
    MK_FLT_PAR_ABS(fDSFmAmt, ds_fm_amt, 4095.f, 1.5f)
    MK_FLT_PAR_ABS(fDSDecay, ds_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDSSpy, ds_spy, 4095.f, 1.f)

    dsd.Render(
            bDSSus,
            bDSTrig,
            fDSAccent,
            fDSF0,
            fDSFmAmt,
            fDSDecay,
            fDSSpy,
            dsd_out,
            32);

    // HiHat 1
    MK_BOOL_PAR(bHH1Trig, hh1_trigger)
    if(bHH1Trig != hh1_trig_prev) {
        hh1_trig_prev = bHH1Trig;
    }else{
        bHH1Trig = false;
    }

    MK_FLT_PAR_ABS(fHH1Lev, hh1_lev, 4095.f, 4.f)
    fHH1Lev *= fHH1Lev;
    MK_FLT_PAR_ABS_PAN(fHH1Pan, hh1_pan, 4095.f, 1.f)
    MK_BOOL_PAR(bHH1Sus, hh1_sus)
    MK_FLT_PAR_ABS(fHH1Accent, hh1_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fHH1F0, hh1_f0, 4095.f, 0.0005f, 0.1f)
    MK_FLT_PAR_ABS(fHH1Tone, hh1_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH1Decay, hh1_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH1Noise, hh1_noise, 4095.f, 1.f)

    hh1.Render(
            bHH1Sus,
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

    // HiHat 2
    MK_BOOL_PAR(bHH2Trig, hh2_trigger)
    if(bHH2Trig != hh2_trig_prev) {
        hh2_trig_prev = bHH2Trig;
    }else{
        bHH2Trig = false;
    }

    MK_FLT_PAR_ABS(fHH2Lev, hh2_lev, 4095.f, 4.f)
    fHH2Lev *= fHH2Lev;
    MK_FLT_PAR_ABS_PAN(fHH2Pan, hh2_pan, 4095.f, 1.f)
    MK_BOOL_PAR(bHH2Sus, hh2_sus)
    MK_FLT_PAR_ABS(fHH2Accent, hh2_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fHH2F0, hh2_f0, 4095.f, .00001f, .1f)
    MK_FLT_PAR_ABS(fHH2Tone, hh2_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH2Decay, hh2_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH2Noise, hh2_noise, 4095.f, 1.f)

    hh2.Render(
            bHH2Sus,
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

    // romplers
    MK_BOOL_PAR(bGateS1, s1_gate)
    rompler[0].params.gate = bGateS1;
    MK_FLT_PAR_ABS(fS1Lev, s1_lev, 4095.f, 2.f)
    fS1Lev *= fS1Lev;
    MK_FLT_PAR_ABS_PAN(fS1Pan, s1_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS_SFT(fS1Speed, s1_speed, 4095.f, 2.f)
    rompler[0].params.playbackSpeed = fS1Speed;
    MK_INT_PAR_ABS(iS1Bank, s1_bank, 32.f)
    CONSTRAIN(iS1Bank, 0, 31)
    MK_INT_PAR_ABS(iS1Slice, s1_slice, 32.f)
    CONSTRAIN(iS1Slice, 0, 31)
    MK_BOOL_PAR(bS1SkipWt, s1_skpwt)
    iS1Slice = iS1Bank * 32 + iS1Slice;
    if(bS1SkipWt)
        iS1Slice += sampleRom.GetFirstNonWaveTableSlice();
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
    MK_FLT_PAR_ABS(fS1Attack, s1_atk, 4095.f, 1.f)
    rompler[0].params.a = fS1Attack;
    MK_FLT_PAR_ABS(fS1Decay, s1_dcy, 4095.f, 2.f)
    rompler[0].params.d = fS1Decay;
    MK_FLT_PAR_ABS_SFT(fS1EGFM, s1_eg2fm, 4095.f, 12.f)
    rompler[0].params.egFM = fS1EGFM;
    MK_INT_PAR_ABS(iS1Brr, s1_brr, 16)
    CONSTRAIN(iS1Brr, 0, 14)
    rompler[0].params.bitReduction = iS1Brr;
    rompler[0].Process(s1_out, 32);


    MK_BOOL_PAR(bGateS2, s2_gate)
    rompler[1].params.gate = bGateS2;
    MK_FLT_PAR_ABS(fS2Lev, s2_lev, 4095.f, 2.f)
    fS2Lev *= fS2Lev;
    MK_FLT_PAR_ABS_PAN(fS2Pan, s2_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS_SFT(fS2Speed, s2_speed, 4095.f, 2.f)
    rompler[1].params.playbackSpeed = fS2Speed;
    MK_INT_PAR_ABS(iS2Bank, s2_bank, 32.f)
    CONSTRAIN(iS2Bank, 0, 31)
    MK_INT_PAR_ABS(iS2Slice, s2_slice, 32.f)
    CONSTRAIN(iS2Slice, 0, 31)
    MK_BOOL_PAR(bS2SkipWt, s2_skpwt)
    iS2Slice = iS2Bank * 32 + iS2Slice;
    if(bS2SkipWt)
        iS2Slice += sampleRom.GetFirstNonWaveTableSlice();
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
    MK_FLT_PAR_ABS(fS2Attack, s2_atk, 4095.f, 1.f)
    rompler[1].params.a = fS2Attack;
    MK_FLT_PAR_ABS(fS2Decay, s2_dcy, 4095.f, 2.f)
    rompler[1].params.d = fS2Decay;
    MK_FLT_PAR_ABS_SFT(fS2EGFM, s2_eg2fm, 4095.f, 12.f)
    rompler[1].params.egFM = fS2EGFM;
    MK_INT_PAR_ABS(iS2Brr, s2_brr, 16)
    CONSTRAIN(iS2Brr, 0, 14)
    rompler[1].params.bitReduction = iS2Brr;
    rompler[1].Process(s2_out, 32);

    MK_BOOL_PAR(bGateS3, s3_gate)
    rompler[2].params.gate = bGateS3;
    MK_FLT_PAR_ABS(fS3Lev, s3_lev, 4095.f, 2.f)
    fS3Lev *= fS3Lev;
    MK_FLT_PAR_ABS_PAN(fS3Pan, s3_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS_SFT(fS3Speed, s3_speed, 4095.f, 2.f)
    rompler[2].params.playbackSpeed = fS3Speed;
    MK_INT_PAR_ABS(iS3Bank, s3_bank, 32.f)
    CONSTRAIN(iS3Bank, 0, 31)
    MK_INT_PAR_ABS(iS3Slice, s3_slice, 32.f)
    CONSTRAIN(iS3Slice, 0, 31)
    MK_BOOL_PAR(bS3SkipWt, s3_skpwt)
    iS3Slice = iS3Bank * 32 + iS3Slice;
    if(bS3SkipWt)
        iS3Slice += sampleRom.GetFirstNonWaveTableSlice();
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
    MK_FLT_PAR_ABS(fS3Attack, s3_atk, 4095.f, 1.f)
    rompler[2].params.a = fS3Attack;
    MK_FLT_PAR_ABS(fS3Decay, s3_dcy, 4095.f, 2.f)
    rompler[2].params.d = fS3Decay;
    MK_FLT_PAR_ABS_SFT(fS3EGFM, s3_eg2fm, 4095.f, 12.f)
    rompler[2].params.egFM = fS3EGFM;
    MK_INT_PAR_ABS(iS3Brr, s3_brr, 16)
    CONSTRAIN(iS3Brr, 0, 14)
    rompler[2].params.bitReduction = iS3Brr;
    rompler[2].Process(s3_out, 32);

    MK_BOOL_PAR(bGateS4, s4_gate)
    rompler[3].params.gate = bGateS4;
    MK_FLT_PAR_ABS(fS4Lev, s4_lev, 4095.f, 2.f)
    fS4Lev *= fS4Lev;
    MK_FLT_PAR_ABS_PAN(fS4Pan, s4_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS_SFT(fS4Speed, s4_speed, 4095.f, 2.f)
    rompler[3].params.playbackSpeed = fS4Speed;
    MK_INT_PAR_ABS(iS4Bank, s4_bank, 32.f)
    CONSTRAIN(iS4Bank, 0, 31)
    MK_INT_PAR_ABS(iS4Slice, s4_slice, 32.f)
    CONSTRAIN(iS4Slice, 0, 31)
    MK_BOOL_PAR(bS4SkipWt, s4_skpwt)
    iS4Slice = iS4Bank * 32 + iS4Slice;
    if(bS4SkipWt)
        iS4Slice += sampleRom.GetFirstNonWaveTableSlice();
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
    MK_FLT_PAR_ABS(fS4Attack, s4_atk, 4095.f, 1.f)
    rompler[3].params.a = fS4Attack;
    MK_FLT_PAR_ABS(fS4Decay, s4_dcy, 4095.f, 2.f)
    rompler[3].params.d = fS4Decay;
    MK_FLT_PAR_ABS_SFT(fS4EGFM, s4_eg2fm, 4095.f, 12.f)
    rompler[3].params.egFM = fS4EGFM;
    MK_INT_PAR_ABS(iS4Brr, s4_brr, 16)
    CONSTRAIN(iS4Brr, 0, 14)
    rompler[3].params.bitReduction = iS4Brr;
    rompler[3].Process(s4_out, 32);
    
    // render final buffer
    for(int i=0;i<32;i++){
        float fVal_l;
        float fVal_r;
        fVal_l = abd_out[i] * fABLev * (1.f-fABPan);
        fVal_l += asd_out[i] * fASLev * (1.f-fASPan);
        fVal_l += dbd_out[i] * fDBLev * (1.f-fDBPan);
        fVal_l += dsd_out[i] * fDSLev * (1.f-fDSPan);
        fVal_l += hh1_out[i] * fHH1Lev * (1.f-fHH1Pan);
        fVal_l += hh2_out[i] * fHH2Lev * (1.f-fHH2Pan);
        fVal_l += s1_out[i] * fS1Lev * (1.f-fS1Pan);
        fVal_l += s2_out[i] * fS2Lev * (1.f-fS2Pan);
        fVal_l += s3_out[i] * fS3Lev * (1.f-fS3Pan);
        fVal_l += s4_out[i] * fS4Lev * (1.f-fS4Pan);
        fVal_r = abd_out[i] * fABLev * fABPan;
        fVal_r += asd_out[i] * fASLev * fASPan;
        fVal_r += dbd_out[i] * fDBLev * fDBPan;
        fVal_r += dsd_out[i] * fDSLev * fDSPan;
        fVal_r += hh1_out[i] * fHH1Lev * fHH1Pan;
        fVal_r += hh2_out[i] * fHH2Lev * fHH2Pan;
        fVal_r += s1_out[i] * fS1Lev * fS1Pan;
        fVal_r += s2_out[i] * fS2Lev * fS2Pan;
        fVal_r += s3_out[i] * fS3Lev * fS3Pan;
        fVal_r += s4_out[i] * fS4Lev * fS4Pan;
        data.buf[i*2] = fVal_l;
        data.buf[i*2+1] = fVal_r;
    }
}

void ctagSoundProcessorDrumRack::Init(std::size_t blockSize, void *blockPtr) {
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    // preload samples
    sampleRom.BufferInSPIRAM();

    // init models
    abd.Init();
    asd.Init();
    dbd.Init();
    dsd.Init();
    hh1.Init();
    hh2.Init();

    // init romplers
    for(auto &r : rompler){
        r.Init(44100.f);
    }


    // check if blockMem is large enough
    // blockMem is used just like larger blocks of heap memory
    // assert(blockSize >= memLen);
    // if memory larger than blockMem is needed, use heap_caps_malloc() instead with MALLOC_CAPS_SPIRAM
}

// no ctor, use Init() instead, is called from factory after successful creation
// dtor
ctagSoundProcessorDrumRack::~ctagSoundProcessorDrumRack() {
    // no explicit freeing for blockMem needed, done by ctagSPAllocator
    // explicit free is only needed when using heap_caps_malloc() with MALLOC_CAPS_SPIRAM
}

void ctagSoundProcessorDrumRack::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("ab_trigger", [&](const int val){ ab_trigger = val;});
	pMapTrig.emplace("ab_trigger", [&](const int val){ trig_ab_trigger = val;});
	pMapPar.emplace("ab_lev", [&](const int val){ ab_lev = val;});
	pMapCv.emplace("ab_lev", [&](const int val){ cv_ab_lev = val;});
	pMapPar.emplace("ab_pan", [&](const int val){ ab_pan = val;});
	pMapCv.emplace("ab_pan", [&](const int val){ cv_ab_pan = val;});
	pMapPar.emplace("ab_sus", [&](const int val){ ab_sus = val;});
	pMapTrig.emplace("ab_sus", [&](const int val){ trig_ab_sus = val;});
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
	pMapPar.emplace("db_lev", [&](const int val){ db_lev = val;});
	pMapCv.emplace("db_lev", [&](const int val){ cv_db_lev = val;});
	pMapPar.emplace("db_pan", [&](const int val){ db_pan = val;});
	pMapCv.emplace("db_pan", [&](const int val){ cv_db_pan = val;});
	pMapPar.emplace("db_sus", [&](const int val){ db_sus = val;});
	pMapTrig.emplace("db_sus", [&](const int val){ trig_db_sus = val;});
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
	pMapPar.emplace("as_trigger", [&](const int val){ as_trigger = val;});
	pMapTrig.emplace("as_trigger", [&](const int val){ trig_as_trigger = val;});
	pMapPar.emplace("as_lev", [&](const int val){ as_lev = val;});
	pMapCv.emplace("as_lev", [&](const int val){ cv_as_lev = val;});
	pMapPar.emplace("as_pan", [&](const int val){ as_pan = val;});
	pMapCv.emplace("as_pan", [&](const int val){ cv_as_pan = val;});
	pMapPar.emplace("as_sus", [&](const int val){ as_sus = val;});
	pMapTrig.emplace("as_sus", [&](const int val){ trig_as_sus = val;});
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
	pMapPar.emplace("ds_lev", [&](const int val){ ds_lev = val;});
	pMapCv.emplace("ds_lev", [&](const int val){ cv_ds_lev = val;});
	pMapPar.emplace("ds_pan", [&](const int val){ ds_pan = val;});
	pMapCv.emplace("ds_pan", [&](const int val){ cv_ds_pan = val;});
	pMapPar.emplace("ds_sus", [&](const int val){ ds_sus = val;});
	pMapTrig.emplace("ds_sus", [&](const int val){ trig_ds_sus = val;});
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
	pMapPar.emplace("hh1_lev", [&](const int val){ hh1_lev = val;});
	pMapCv.emplace("hh1_lev", [&](const int val){ cv_hh1_lev = val;});
	pMapPar.emplace("hh1_pan", [&](const int val){ hh1_pan = val;});
	pMapCv.emplace("hh1_pan", [&](const int val){ cv_hh1_pan = val;});
	pMapPar.emplace("hh1_sus", [&](const int val){ hh1_sus = val;});
	pMapTrig.emplace("hh1_sus", [&](const int val){ trig_hh1_sus = val;});
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
	pMapPar.emplace("hh2_lev", [&](const int val){ hh2_lev = val;});
	pMapCv.emplace("hh2_lev", [&](const int val){ cv_hh2_lev = val;});
	pMapPar.emplace("hh2_pan", [&](const int val){ hh2_pan = val;});
	pMapCv.emplace("hh2_pan", [&](const int val){ cv_hh2_pan = val;});
	pMapPar.emplace("hh2_sus", [&](const int val){ hh2_sus = val;});
	pMapTrig.emplace("hh2_sus", [&](const int val){ trig_hh2_sus = val;});
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
	pMapPar.emplace("s1_gate", [&](const int val){ s1_gate = val;});
	pMapTrig.emplace("s1_gate", [&](const int val){ trig_s1_gate = val;});
	pMapPar.emplace("s1_lev", [&](const int val){ s1_lev = val;});
	pMapCv.emplace("s1_lev", [&](const int val){ cv_s1_lev = val;});
	pMapPar.emplace("s1_pan", [&](const int val){ s1_pan = val;});
	pMapCv.emplace("s1_pan", [&](const int val){ cv_s1_pan = val;});
	pMapPar.emplace("s1_speed", [&](const int val){ s1_speed = val;});
	pMapCv.emplace("s1_speed", [&](const int val){ cv_s1_speed = val;});
	pMapPar.emplace("s1_bank", [&](const int val){ s1_bank = val;});
	pMapCv.emplace("s1_bank", [&](const int val){ cv_s1_bank = val;});
	pMapPar.emplace("s1_slice", [&](const int val){ s1_slice = val;});
	pMapCv.emplace("s1_slice", [&](const int val){ cv_s1_slice = val;});
	pMapPar.emplace("s1_skpwt", [&](const int val){ s1_skpwt = val;});
	pMapTrig.emplace("s1_skpwt", [&](const int val){ trig_s1_skpwt = val;});
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
	pMapPar.emplace("s2_gate", [&](const int val){ s2_gate = val;});
	pMapTrig.emplace("s2_gate", [&](const int val){ trig_s2_gate = val;});
	pMapPar.emplace("s2_lev", [&](const int val){ s2_lev = val;});
	pMapCv.emplace("s2_lev", [&](const int val){ cv_s2_lev = val;});
	pMapPar.emplace("s2_pan", [&](const int val){ s2_pan = val;});
	pMapCv.emplace("s2_pan", [&](const int val){ cv_s2_pan = val;});
	pMapPar.emplace("s2_speed", [&](const int val){ s2_speed = val;});
	pMapCv.emplace("s2_speed", [&](const int val){ cv_s2_speed = val;});
	pMapPar.emplace("s2_bank", [&](const int val){ s2_bank = val;});
	pMapCv.emplace("s2_bank", [&](const int val){ cv_s2_bank = val;});
	pMapPar.emplace("s2_slice", [&](const int val){ s2_slice = val;});
	pMapCv.emplace("s2_slice", [&](const int val){ cv_s2_slice = val;});
	pMapPar.emplace("s2_skpwt", [&](const int val){ s2_skpwt = val;});
	pMapTrig.emplace("s2_skpwt", [&](const int val){ trig_s2_skpwt = val;});
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
	pMapPar.emplace("s3_gate", [&](const int val){ s3_gate = val;});
	pMapTrig.emplace("s3_gate", [&](const int val){ trig_s3_gate = val;});
	pMapPar.emplace("s3_lev", [&](const int val){ s3_lev = val;});
	pMapCv.emplace("s3_lev", [&](const int val){ cv_s3_lev = val;});
	pMapPar.emplace("s3_pan", [&](const int val){ s3_pan = val;});
	pMapCv.emplace("s3_pan", [&](const int val){ cv_s3_pan = val;});
	pMapPar.emplace("s3_speed", [&](const int val){ s3_speed = val;});
	pMapCv.emplace("s3_speed", [&](const int val){ cv_s3_speed = val;});
	pMapPar.emplace("s3_bank", [&](const int val){ s3_bank = val;});
	pMapCv.emplace("s3_bank", [&](const int val){ cv_s3_bank = val;});
	pMapPar.emplace("s3_slice", [&](const int val){ s3_slice = val;});
	pMapCv.emplace("s3_slice", [&](const int val){ cv_s3_slice = val;});
	pMapPar.emplace("s3_skpwt", [&](const int val){ s3_skpwt = val;});
	pMapTrig.emplace("s3_skpwt", [&](const int val){ trig_s3_skpwt = val;});
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
	pMapPar.emplace("s4_gate", [&](const int val){ s4_gate = val;});
	pMapTrig.emplace("s4_gate", [&](const int val){ trig_s4_gate = val;});
	pMapPar.emplace("s4_lev", [&](const int val){ s4_lev = val;});
	pMapCv.emplace("s4_lev", [&](const int val){ cv_s4_lev = val;});
	pMapPar.emplace("s4_pan", [&](const int val){ s4_pan = val;});
	pMapCv.emplace("s4_pan", [&](const int val){ cv_s4_pan = val;});
	pMapPar.emplace("s4_speed", [&](const int val){ s4_speed = val;});
	pMapCv.emplace("s4_speed", [&](const int val){ cv_s4_speed = val;});
	pMapPar.emplace("s4_bank", [&](const int val){ s4_bank = val;});
	pMapCv.emplace("s4_bank", [&](const int val){ cv_s4_bank = val;});
	pMapPar.emplace("s4_slice", [&](const int val){ s4_slice = val;});
	pMapCv.emplace("s4_slice", [&](const int val){ cv_s4_slice = val;});
	pMapPar.emplace("s4_skpwt", [&](const int val){ s4_skpwt = val;});
	pMapTrig.emplace("s4_skpwt", [&](const int val){ trig_s4_skpwt = val;});
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
	isStereo = true;
	id = "DrumRack";
	// sectionCpp0
}