/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020/2021 by Robert Manzke. All rights reserved.

(c) 2021 for the "APCpp"-Plugin by Mathias Brüssel
APCpp stands for "Atari Punk Console plus plus", i.e. the digital implementation of a popular electronics circuit with enhancements
As with many simple Oscillators, the original hardware design is based on timerchips (two NE555 or one 556) https://de.wikipedia.org/wiki/NE555
To learn more about the original APC circuit please refer to: https://sdiy.info/wiki/Atari_Punk_Console
Enhancements are optional pitch-modulation, pulse-with-modulation, amplitude/ring-modulation, sinuswaves for the oscillators and a volume-envelope.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "ctagSoundProcessorAntique.hpp"

using namespace CTAG::SP;

void ctagSoundProcessorAntique::Process(const ProcessData &data) {
    // dry buffer
    float dry[32];
    // input shaping
    MK_FLT_PAR_ABS(fInputLevel, inplevel, 4095.f, 1.f)
    fInputLevel *= fInputLevel;
    fInputLevel *= 4.f;
    MK_FLT_PAR_ABS(fInputDistortion, inpdist, 4095.f, 1.f)
    fInputDistortion *= fInputDistortion;
    fInputDistortion *= 20.f;
    fInputDistortion += 1.f;
    MK_FLT_PAR(fInputRepitch, inprepitch, 4095.f, 1.f)

    // Hum --> electric noise 50/60Hz
    MK_FLT_PAR(fHumLevel, humlev, 4095.f, 1.f)
    fHumLevel *= fHumLevel; // poor exp approx.
    MK_INT_PAR_ABS(iHumFreq, humf, 4095.f)
    MK_INT_PAR_ABS(iHumShape, humshape, 32767.f)
    MK_INT_PAR_ABS(iHumAgression, humagr, 30)
    iHumAgression++;
    int16_t ibuf[32];
    const uint8_t sync[32] {0};
    humm.set_shape(braids::AnalogOscillatorShape::OSC_SHAPE_TRIANGLE_FOLD);
    humm.set_parameter(iHumShape);
    humm.set_aux_parameter(0);
    humm.set_pitch(1000 + iHumFreq);
    humm.Render(sync, ibuf, nullptr, 32);

    // hiss
    MK_FLT_PAR_ABS_MIN_MAX(fHissFreq, hissf, 4095.f, 20.f, 20000.f)
    MK_FLT_PAR_ABS_MIN_MAX(fHissQ, hissbw, 4095.f, 0.3f, 8.f)
    MK_FLT_PAR_ABS(fHissShape, hissshp, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHissLevel, hisslevel, 4095.f, 1.f)
    fHissLevel *= fHissLevel;
    hissFlt.set_f_q<stmlib::FrequencyApproximation::FREQUENCY_FAST>(fHissFreq/44100.f, fHissQ);

    // wow + flutter + repitch
    fx.set_size(1.f);
    MK_FLT_PAR_ABS(fLfoWowLev, wowl, 4095.f, 0.1f)
    MK_FLT_PAR_ABS_MIN_MAX(fLfoWowFreq, wowf, 4095.f, 0.1f, 1.f)
    lfoWow.SetFrequency(fLfoWowFreq);
    float lfoWowValue = lfoWow.Process();
    fLfoWowLev *= 0.5f - lfoWowValue;
    MK_FLT_PAR_ABS(fLfoFlutterLev, flutl, 4095.f, 0.1f)
    MK_FLT_PAR_ABS_MIN_MAX(fLfoFlutterFreq, flutf, 4095.f, 8.f, 14.f)
    lfoFlutter.SetFrequency(fLfoFlutterFreq);
    fLfoFlutterLev *= 0.5f - lfoFlutter.Process();
    float ratio = 1.f + fLfoWowLev + fLfoFlutterLev + fInputRepitch;
    CONSTRAIN(ratio, -1.f, 2.f)
    fx.set_ratio(ratio);
    clouds::FloatFrame frames[bufSz];

    // clicks --> random glitches, fine dust
    MK_FLT_PAR_ABS(fClickLevel, clickl, 4095.f, 1.f)
    fClickLevel *= fClickLevel; // cheap approx
    MK_FLT_PAR_ABS(fClickDensity, clickd, 4095.f, .01f)
    MK_FLT_PAR_ABS(fClickFrequency, clickf, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fClickFMod, clickfmod, 4095.f, 0.5f)
    fClickFrequency += fClickFMod * (0.5f - rnd.Process()); // vary frequency
    fClickFrequency *= fClickFrequency;
    fClickFrequency *= 15000.f;
    CONSTRAIN(fClickFrequency, 20.f, 15000.f)
    fClickFrequency /= 44100.f;
    MK_FLT_PAR_ABS_MIN_MAX(fClickQ, clickq, 4095.f, 0.3f, 8.f)
    MK_FLT_PAR_ABS(fClickQMod, clickqm, 4095.f, 6.f)
    fClickQ += fClickQMod * (0.5f - rnd.Process());
    CONSTRAIN(fClickQ, 0.3f, 8.f)
    clickFilter.set_f_q<stmlib::FrequencyApproximation::FREQUENCY_FAST>(fClickFrequency, fClickQ);

    // pops --> repeating damages over longer duration
    MK_FLT_PAR_ABS(fPopLevel, popl, 4095.f, 1.f)
    fPopLevel *= fPopLevel;
    MK_FLT_PAR_ABS(fPopDensity1, popd1, 4095.f, 0.001f)
    MK_FLT_PAR_ABS(fPopDensity2, popd2, 4095.f, 0.001f)
    MK_INT_PAR_ABS(iPatternLen, poplen, 5e5f);
    MK_BOOL_PAR(bSyncToWow, poplensy)
    if(bSyncToWow) iPatternLen = static_cast<int>(44100.f / fLfoWowFreq);
    MK_FLT_PAR_ABS_MIN_MAX(fPopBlendLen, popblen, 4095.f, 0.001f, .1f)
    lfoPopBlend.SetFrequency(fPopBlendLen);
    float popBlend = lfoPopBlend.Process() * 2.f; // lfo only delivers +/- 0.5
    popBlend *= popBlend; // cos^2 shape
    if(popBlend > 0.98f) pop1.ReSeed(seed1++); // change pop pattern
    if(popBlend < 0.02f) pop2.ReSeed(seed2++); // change pop pattern
    MK_FLT_PAR_ABS_MIN_MAX(fPopFrequency, popf, 4095.f, 0.5f, 100.f)
    popSrc.SetFrequency(fPopFrequency);
    MK_FLT_PAR_ABS_MIN_MAX(fPopDecay, popdcy, 4095.f, 0.01f, 0.0001f)
    fPopDecay = 1.f - fPopDecay;
    popShaper.SetCoeff(fPopDecay);

    // create buffer
    for (int i = 0; i < bufSz; i++) {
        // pops
        float r1 = pop1.Process();
        float p1 = r1 < fPopDensity1 ? 1.f : 0.f;
        float r2 = pop2.Process();
        float p2 = r2 < fPopDensity2 ? 1.f : 0.f;
        loopCntr++;

        if(loopCntr > iPatternLen){
            loopCntr = 0;
            pop1.ReSeed(seed1);
            pop2.ReSeed(seed2);
        }

        float pop = p1 + (p2 - p1) * popBlend;
        pop = popSrc.Process() * popShaper.Process(pop);

        // clicks
        float cl = click.Process();
        cl = cl < fClickDensity ? 1.f : 0.f;
        cl *= rnd.Process(); // make loudness random
        cl = clickFilter.Process<stmlib::FILTER_MODE_BAND_PASS>(cl);

        // level + distort
        dry[i] = data.buf[i * 2 + processCh];
        data.buf[i * 2 + processCh] *= fInputLevel;
        data.buf[i * 2 + processCh] = stmlib::SoftClip(data.buf[i * 2 + processCh] * fInputDistortion) / fInputDistortion;

        // mix with input
        frames[i].l = frames[i].r = cl * fClickLevel + pop * fPopLevel + stmlib::SoftClip(data.buf[i * 2 + processCh] * fInputLevel);
    }

    // wow / flutter processing
    fx.Process(frames, bufSz);

    // Output Bandpass / mix
    MK_FLT_PAR_ABS_MIN_MAX(fOutFiltCtr, outfltctr, 4095.f, 7.f, 120.f)
    MK_FLT_PAR_ABS_MIN_MAX(fOutFiltBW, outfltbw, 4095.f, 7.f, 120.f)
    MK_FLT_PAR_ABS_MIN_MAX(fOutQ, outfltq, 4095.f, 0.33f, 8.f);
    MK_FLT_PAR_ABS(fOutLevel, outlevel, 4095.f, 1.f)
    MK_BOOL_PAR(bHisHumPost, hishumpre)
    fOutLevel *= fOutLevel;
    fOutLevel *= 10.f;
    fOutFiltCtr *= fOutFiltCtr; // poor but fast approximation sq instead of exp / log
    fOutFiltBW *= fOutFiltBW;
    float fOutCutLp = fOutFiltCtr + fOutFiltBW;
    float fOutCutHp = fOutFiltCtr - fOutFiltBW;
    CONSTRAIN(fOutCutLp, 20.f, 18000.f)
    CONSTRAIN(fOutCutHp, 20.f, 18000.f)
    fOutCutLp /= 44100.f;
    fOutCutHp /= 44100.f;
    lpMaster.set_f_q<stmlib::FrequencyApproximation::FREQUENCY_FAST>(fOutCutLp, fOutQ);
    hpMaster.set_f_q<stmlib::FrequencyApproximation::FREQUENCY_FAST>(fOutCutHp, fOutQ);
    MK_FLT_PAR_ABS(fWetDry, outdw, 4095.f, 1.f)

    // Scrub --> simulates noise from play head mechanical motion
    MK_FLT_PAR_ABS(fScrubLevel, scrublev, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fScrubModLevel, scrubmodlev, 4095.f, 0.5f)
    float fScrubCenter = static_cast<float>(scrubcen);
    if(cv_scrubcen != -1){
        fScrubCenter = 30.f + data.cv[cv_scrubcen] * 12970.f;
    }
    MK_FLT_PAR_ABS_MIN_MAX(fScrubQ, scrubq, 4095.f, 0.3f, 8.f)
    fScrubLevel *= fScrubLevel;
    fScrubLevel *= 0.2f;
    float fScrubFilterFreq = (fScrubCenter + lfoWowValue * 300.f * fScrubModLevel) ;
    CONSTRAIN(fScrubFilterFreq, 20.f, 18000.f)
    scrubFilt.set_f_q<stmlib::FrequencyApproximation::FREQUENCY_FAST>(fScrubFilterFreq / 44100.f, fScrubQ);

    // process frame
    for (int i = 0; i < bufSz; i++) {
        float rndval = rnd.Process();
        float scrub = scrubFilt.Process<stmlib::FILTER_MODE_BAND_PASS>(rndval) * fScrubLevel;
        scrub *= 1.f + fScrubModLevel * lfoWowValue * 2.f; // synced with wow
        float his = hissFlt.Process<stmlib::FILTER_MODE_BAND_PASS>(rndval >= fHissShape ? rndval : 0.f) * fHissLevel; // allow grainyness
        float hum = 0.f;
        if ((i % iHumAgression) == 0) { // decimate -> aggression
            hum = ibuf[i] * 0.000030517578125f * fHumLevel; // convert to float and level
        }
        float tmp;
        if(bHisHumPost){
            tmp = hpMaster.Process<stmlib::FILTER_MODE_HIGH_PASS>(lpMaster.Process<stmlib::FILTER_MODE_LOW_PASS>(frames[i].l + scrub)) + hum + his;
        }else{
            tmp = hpMaster.Process<stmlib::FILTER_MODE_HIGH_PASS>(lpMaster.Process<stmlib::FILTER_MODE_LOW_PASS>(frames[i].l + scrub + hum + his));
        }
        data.buf[i * 2 + processCh] = stmlib::Crossfade(dry[i], stmlib::SoftClip(tmp * fOutLevel), fWetDry);
    }
}

void ctagSoundProcessorAntique::Init(std::size_t blockSize, void *blockPtr) {
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    // multi purpose rnd
    rnd.SetBipolar(false);
    rnd.ReSeed(42);

    // his
    hissLevelMod.SetBipolar(false);
    hissFlt.Init();
    hissFlt.Reset();

    // scrub
    scrubFilt.Init();
    scrubFilt.Reset();

    //hum
    humm.Init();

    // flutter, wow
    assert(blockSize >= 4096 * sizeof(float));
    fx_buffer = (float *) blockPtr;
    fx.Init(fx_buffer);
    lfoWow.SetSampleRate(44100.f / 32.f);
    lfoWow.SetFrequency(.55f);
    lfoFlutter.SetSampleRate(44100.f / 32.f);
    lfoFlutter.SetFrequency(10.f);

    // pop
    loopCntr = 0;
    lfoPopBlend.SetSampleRate(44100.f/32.f);
    lfoPopBlend.SetFrequency(0.01f);
    seed1 = rand();
    seed2 = rand();
    pop1.SetBipolar(false);
    pop2.SetBipolar(false);
    pop1.ReSeed(seed1);
    pop2.ReSeed(seed2);
    popSrc.SetSampleRate(44100.f);
    popSrc.SetFrequency(20.f);
    popShaper.SetSampleRate(44100.f);
    popShaper.SetDecay60dB(0.15f);
    popShaper.SetCoeffSmoothing(false);

    // click
    click.SetBipolar(false);
    click.ReSeed(rand());
    clickFilter.Init();

    // master
    lpMaster.Init();
    hpMaster.Init();
}

ctagSoundProcessorAntique::~ctagSoundProcessorAntique() {
}

void ctagSoundProcessorAntique::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("inplevel", [&](const int val){ inplevel = val;});
	pMapCv.emplace("inplevel", [&](const int val){ cv_inplevel = val;});
	pMapPar.emplace("inpdist", [&](const int val){ inpdist = val;});
	pMapCv.emplace("inpdist", [&](const int val){ cv_inpdist = val;});
	pMapPar.emplace("inprepitch", [&](const int val){ inprepitch = val;});
	pMapCv.emplace("inprepitch", [&](const int val){ cv_inprepitch = val;});
	pMapPar.emplace("hisslevel", [&](const int val){ hisslevel = val;});
	pMapCv.emplace("hisslevel", [&](const int val){ cv_hisslevel = val;});
	pMapPar.emplace("hissf", [&](const int val){ hissf = val;});
	pMapCv.emplace("hissf", [&](const int val){ cv_hissf = val;});
	pMapPar.emplace("hissbw", [&](const int val){ hissbw = val;});
	pMapCv.emplace("hissbw", [&](const int val){ cv_hissbw = val;});
	pMapPar.emplace("hissshp", [&](const int val){ hissshp = val;});
	pMapCv.emplace("hissshp", [&](const int val){ cv_hissshp = val;});
	pMapPar.emplace("scrublev", [&](const int val){ scrublev = val;});
	pMapCv.emplace("scrublev", [&](const int val){ cv_scrublev = val;});
	pMapPar.emplace("scrubcen", [&](const int val){ scrubcen = val;});
	pMapCv.emplace("scrubcen", [&](const int val){ cv_scrubcen = val;});
	pMapPar.emplace("scrubq", [&](const int val){ scrubq = val;});
	pMapCv.emplace("scrubq", [&](const int val){ cv_scrubq = val;});
	pMapPar.emplace("scrubmodlev", [&](const int val){ scrubmodlev = val;});
	pMapCv.emplace("scrubmodlev", [&](const int val){ cv_scrubmodlev = val;});
	pMapPar.emplace("humlev", [&](const int val){ humlev = val;});
	pMapCv.emplace("humlev", [&](const int val){ cv_humlev = val;});
	pMapPar.emplace("humf", [&](const int val){ humf = val;});
	pMapCv.emplace("humf", [&](const int val){ cv_humf = val;});
	pMapPar.emplace("humshape", [&](const int val){ humshape = val;});
	pMapCv.emplace("humshape", [&](const int val){ cv_humshape = val;});
	pMapPar.emplace("humagr", [&](const int val){ humagr = val;});
	pMapCv.emplace("humagr", [&](const int val){ cv_humagr = val;});
	pMapPar.emplace("wowl", [&](const int val){ wowl = val;});
	pMapCv.emplace("wowl", [&](const int val){ cv_wowl = val;});
	pMapPar.emplace("wowf", [&](const int val){ wowf = val;});
	pMapCv.emplace("wowf", [&](const int val){ cv_wowf = val;});
	pMapPar.emplace("flutl", [&](const int val){ flutl = val;});
	pMapCv.emplace("flutl", [&](const int val){ cv_flutl = val;});
	pMapPar.emplace("flutf", [&](const int val){ flutf = val;});
	pMapCv.emplace("flutf", [&](const int val){ cv_flutf = val;});
	pMapPar.emplace("clickl", [&](const int val){ clickl = val;});
	pMapCv.emplace("clickl", [&](const int val){ cv_clickl = val;});
	pMapPar.emplace("clickd", [&](const int val){ clickd = val;});
	pMapCv.emplace("clickd", [&](const int val){ cv_clickd = val;});
	pMapPar.emplace("clickf", [&](const int val){ clickf = val;});
	pMapCv.emplace("clickf", [&](const int val){ cv_clickf = val;});
	pMapPar.emplace("clickfmod", [&](const int val){ clickfmod = val;});
	pMapCv.emplace("clickfmod", [&](const int val){ cv_clickfmod = val;});
	pMapPar.emplace("clickq", [&](const int val){ clickq = val;});
	pMapCv.emplace("clickq", [&](const int val){ cv_clickq = val;});
	pMapPar.emplace("clickqm", [&](const int val){ clickqm = val;});
	pMapCv.emplace("clickqm", [&](const int val){ cv_clickqm = val;});
	pMapPar.emplace("popl", [&](const int val){ popl = val;});
	pMapCv.emplace("popl", [&](const int val){ cv_popl = val;});
	pMapPar.emplace("popd1", [&](const int val){ popd1 = val;});
	pMapCv.emplace("popd1", [&](const int val){ cv_popd1 = val;});
	pMapPar.emplace("popd2", [&](const int val){ popd2 = val;});
	pMapCv.emplace("popd2", [&](const int val){ cv_popd2 = val;});
	pMapPar.emplace("poplen", [&](const int val){ poplen = val;});
	pMapCv.emplace("poplen", [&](const int val){ cv_poplen = val;});
	pMapPar.emplace("poplensy", [&](const int val){ poplensy = val;});
	pMapTrig.emplace("poplensy", [&](const int val){ trig_poplensy = val;});
	pMapPar.emplace("popblen", [&](const int val){ popblen = val;});
	pMapCv.emplace("popblen", [&](const int val){ cv_popblen = val;});
	pMapPar.emplace("popf", [&](const int val){ popf = val;});
	pMapCv.emplace("popf", [&](const int val){ cv_popf = val;});
	pMapPar.emplace("popdcy", [&](const int val){ popdcy = val;});
	pMapCv.emplace("popdcy", [&](const int val){ cv_popdcy = val;});
	pMapPar.emplace("outlevel", [&](const int val){ outlevel = val;});
	pMapCv.emplace("outlevel", [&](const int val){ cv_outlevel = val;});
    pMapPar.emplace("outdw", [&](const int val){ outdw = val;});
    pMapCv.emplace("outdw", [&](const int val){ cv_outdw = val;});
	pMapPar.emplace("outfltctr", [&](const int val){ outfltctr = val;});
	pMapCv.emplace("outfltctr", [&](const int val){ cv_outfltctr = val;});
	pMapPar.emplace("outfltbw", [&](const int val){ outfltbw = val;});
	pMapCv.emplace("outfltbw", [&](const int val){ cv_outfltbw = val;});
	pMapPar.emplace("outfltq", [&](const int val){ outfltq = val;});
	pMapCv.emplace("outfltq", [&](const int val){ cv_outfltq = val;});
	pMapPar.emplace("hishumpre", [&](const int val){ hishumpre = val;});
	pMapTrig.emplace("hishumpre", [&](const int val){ trig_hishumpre = val;});
	isStereo = false;
	id = "Antique";
	// sectionCpp0
}