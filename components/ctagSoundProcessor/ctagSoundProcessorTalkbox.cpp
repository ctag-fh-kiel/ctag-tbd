/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020/2021 by Robert Manzke. All rights reserved.

(c) 2021 for the "Talkbox"-Plugin by Mathias Brüssel
Talkbox is a combination of a simple (yet optionally "polyphonic") synth and a minimalistic vocoder audio-effect plus optionally a filter and phaser on the carrier-signal.

The "Talkbox"-effect is ported from the mdaTalkbox plugin by Paul Kellet (maxim digital audio).
More information on his plugins and the original code can be found here: http://mda.smartelectronix.com
and got adapted from the Soundpipe version of the plugin at: https://github.com/PaulBatchelor/Soundpipe

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "ctagSoundProcessorTalkbox.hpp"

using namespace CTAG::SP;

// --- VULT "Library for TBD" ---
// #include "./vult/vult_formantor.cpp"  // Already defined in ctagSoundProcessorFormantor, so to avoid double defines we omit it here!
// #include "./vult/vultin.cpp"          // Already defined in ctagSoundProcessorFormantor, so to avoid double defines we omit it here!


// --- Trigger/Gate values ---
#define GATE_HIGH_NEW       2
#define GATE_HIGH           1
#define GATE_LOW            0

// --- Additional macros for oscillator and GUI-parameter processing ---
#define MK_TRIG_PAR(outname, inname)            int outname = process_param_trig(data, trig_##inname, inname, e_##inname);

// --- Power of two for GUI selected slider to simulate logarithmic faders ---
#define MK_FLT_PAR_ABS_POW(outname, inname, norm, scale) \
    float outname = inname / norm;\
    if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]); \
    outname = outname * outname * scale;
// --- Additional Macro for automated parameter evaluations ---
#define MK_PITCH_PAR(outname, inname)     float outname = inname; if(cv_##inname != -1) outname += data.cv[cv_##inname]*60.f;

// --- Hysteresis to calm down Accent Bend just a bit ---
#define interpolate_hysteresis(outname, inname, hysteresis_memory) outname = 0.05f*inname + 0.95f*hysteresis_memory; hysteresis_memory = outname;

using namespace CTAG::SP;

// --- Process trigger signals and keep their state internally ---
inline int ctagSoundProcessorTalkbox::process_param_trig(const ProcessData &data, int trig_myparm, int my_parm,
                                                         int enum_trigger_id, int gate_type = 0) {
    int trig_status = 0;

    if (trig_myparm != -1)       // Trigger given via CV/Gate or button?
    {
        trig_status = (data.trig[trig_myparm] == 0); // HIGH is 0, so we negate for boolean logic
        if (gate_type == 1)
            return (trig_status);

        if (trig_status)    // Statuschange from HIGH to LOW or LOW to HIGH? Startup-Status for prev_trig_state is -1, so first change is always new
        {
            if (low_reached[enum_trigger_id])    // We had a trigger low before the new trigger high
            {
                if (prev_trig_state[enum_trigger_id] == GATE_LOW || gate_type == 2)   // Toggle or AD EG Trigger...
                {
                    prev_trig_state[enum_trigger_id] = GATE_HIGH;       // Remember status for next round
                    low_reached[enum_trigger_id] = false;
                    return (GATE_HIGH_NEW);           // New trigger
                } else        // previous status was high!
                {
                    prev_trig_state[enum_trigger_id] = GATE_LOW;       // Remember status for next round
                    low_reached[enum_trigger_id] = false;
                    return (GATE_LOW);           // New trigger
                }
            }
        } else
            low_reached[enum_trigger_id] = true;
    } else                        // We may have a trigger set by activating the button via the GUI
    {
        if (my_parm != prev_trig_state[enum_trigger_id])    // Statuschange from HIGH to LOW or LOW to HIGH?
        {
            prev_trig_state[enum_trigger_id] = my_parm;       // Remember status
            if (my_parm != 0)                   // LOW if 0
                return (GATE_HIGH_NEW);          // New trigger
            else
                return (GATE_LOW);              // Trigger released
        }
    }
    return (prev_trig_state[enum_trigger_id]);            // No change (1 for active, 0 for inactive)
}


void ctagSoundProcessorTalkbox::Process(const ProcessData &data) {
    // === Process all parameters from the GUI ===
    // --- Global / Mixer ---
    MK_PITCH_PAR(f_MasterPitch, MasterPitch)
    MK_FLT_PAR_ABS_SFT(f_MasterTune, MasterTune, 1200.f, 1.f)
    MK_FLT_PAR_ABS_POW(f_CarrierAmnt, CarrierAmnt, 4095.f, 1.f)
    MK_FLT_PAR_ABS(f_CarrierBalance, CarrierBalance, 4095.f, 1.f)
    MK_FLT_PAR_ABS(f_CarrierExternalBalance, CarrierExternalBalance, 4095.f, 1.f)
    MK_FLT_PAR_ABS(f_CarrierLeakage, CarrierLeak, 4095.f, 1.f)
    MK_FLT_PAR_ABS_POW(f_ExtCarrierAmnt, ExtCarrierAmnt, 4095.f, 6.f)
    MK_TRIG_PAR(t_AccentBendIsActive, AccentBendIsActive)
    MK_FLT_PAR_ABS(f_AccentBend, AccentBend, 4095.f, 190.f)
    MK_FLT_PAR_ABS_POW(f_ModulatorAmnt, ModulatorAmnt, 4095.f, 4.f)
    MK_FLT_PAR_ABS_POW(f_ModulatorLeakage, ModulatorLeakage, 4095.f, 2.f)
    MK_FLT_PAR_ABS_POW(f_Volume, Volume, 4095.f, 8.f)  // We use a high value to have headroom, so lower the volume normally

    // --- Carrier ---
    MK_TRIG_PAR(t_VibratoIsActive, VibratoIsActive)
    MK_FLT_PAR_ABS_MIN_MAX(f_VibratoSpeed, VibratoSpeed, 4095.f, 0.05f, 20.f)
    MK_FLT_PAR_ABS(f_VibratoDepth, VibratoDepth, 4095.f, 7.f)
    MK_TRIG_PAR(t_PWMactive, PWMactive)
    MK_FLT_PAR_ABS_MIN_MAX(f_PWMspeed, PWMspeed, 4095.f, 0.05f, 20.f)
    MK_FLT_PAR_ABS(f_PWMdepth, PWMdepth, 4095.f, 1.f)
    MK_FLT_PAR_ABS_SFT(f_SubOscTune_A, SubOscTune_A, 1200.f, 1.f)
    MK_TRIG_PAR(t_SubOscTuningBactive, SubOscTuningBactive)
    MK_FLT_PAR_ABS_SFT(f_SubOscTune_B, SubOscTune_B, 1200.f, 1.f)

    // === Precalculate values for DSP-processing ===
    // --- Tuning of oscillator[s] ---
    float f_current_note = f_MasterPitch + f_MasterTune;

    // --- Add optional accent-bend and vibrato  and apply to notes of synth (please note: f_accentBendVal is stored during main DSP loop!) ---
    if (t_AccentBendIsActive) {
        interpolate_hysteresis(f_AccentBend, f_accentBendVal * f_AccentBend, accentBendHyteresis);
        CONSTRAIN(f_AccentBend, 0.f, 7.f);                    // Max Accent Bend is a perfect fifth (7 semitones)
        f_current_note += f_AccentBend;
    }
    if (t_VibratoIsActive) {
        lfoVibrato.SetFrequency(f_VibratoSpeed);                      // Set LFO for vibrato
        f_current_note +=
                lfoVibrato.Process() * f_VibratoDepth;      // Max VibratoDepth is a perfect fifth (7 semitones)
    }
    float pulse_cv = f_current_note /
                     120.f;  // These VULT-oscillators are scaled from 0.0 to 1.0 for values betwean "MIDI"-Note 0...127
    float saw_cv = t_SubOscTuningBactive ? (f_SubOscTune_B + f_current_note) / 120.f :
                   (f_SubOscTune_A + f_current_note) / 120.f;

    // --- Internal/External Carrier + Synth/White noise carrier balance ---
    float internalCarrierAmnt = (1.f - f_CarrierExternalBalance) * f_CarrierAmnt;
    float externalCarrierAmnt = f_CarrierExternalBalance * f_ExtCarrierAmnt * f_CarrierAmnt;
    float carrierSyntAmnt = (1.f - f_CarrierBalance) * internalCarrierAmnt;
    float carrierWnoiseAmnt = f_CarrierBalance * internalCarrierAmnt;

    // --- PW / PWM ---
    float f_PulseWidth = 0.5f;
    if (t_PWMactive) {
        lfoPWM.SetFrequency(f_PWMspeed);                      // Set LFO for PWM
        f_PulseWidth += lfoPWM.Process() * f_PWMdepth;
        CONSTRAIN(f_PulseWidth, 0.f, 1.f);
    }

    // Ensemble
    MK_BOOL_PAR(bFXEnable, FXIsActive)
    MK_BOOL_PAR(bEnsChorus, FXEnsChor)
    MK_FLT_PAR_ABS(fFXDepth, FXDepth, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fFXAmt, FXAmt, 4095.f, 1.f)

    // === Main DSP loop ===
    for (uint32_t i = 0; i < bufSz; i++) {
        float carrier = (Blit_osc_blit(pulse_data, pulse_cv, f_PulseWidth,0.f) +
                Saw_eptr_process(saw_data, saw_cv)) * carrierSyntAmnt + wNoise.Process() * carrierWnoiseAmnt +
                data.buf[i * 2 + 1] * externalCarrierAmnt;

        float modulator = data.buf[i * 2] * f_ModulatorAmnt;

        data.buf[i*2] = data.buf[i*2 + 1] = (ctag_talkbox.Process(modulator, carrier) +
                                                data.buf[i * 2] * f_ModulatorLeakage +
                                                 carrier * f_CarrierLeakage
                                                 ) * f_Volume;
    }

    // Ensemble effect
    if(bFXEnable){
        if(bEnsChorus){
            chorus.Process(data.buf, bufSz);
            chorus.set_depth(fFXDepth);
            chorus.set_amount(fFXAmt);
        }else{
            ens.set_depth(fFXDepth);
            ens.set_amount(fFXAmt);
            ens.Process(data.buf, bufSz);
        }
    }

    // === Postprocessing ===
    f_accentBendVal = averageAccentBend.dejitter(
            fabsf(data.buf[30]));  // Remember average volume of last value to maybe apply as Accent Bend in next round...
}

void ctagSoundProcessorTalkbox::Init(std::size_t blockSize, void *blockPtr) {
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    // Talk box
    ctag_talkbox.Init(44100.f, 1.f);

    // Ensemble fx memory alloc
    fx_buffer = (float *) heap_caps_malloc(4096 * sizeof(float),
                                           MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (fx_buffer == NULL) {
        ESP_LOGE("MIChorus", "Could not allocate shared buffer!");
    };

    chorus.Init(fx_buffer);
    ens.Init(fx_buffer);

    // --- Initialize VULT stuff ---
    Blit__ctx_type_1_init(pulse_data);    // Variable rectangle wave
    Saw_eptr__ctx_type_0_init(saw_data);  // Saw suboscillator

    // --- Init LFOs ---
    lfoPWM.SetSampleRate(44100.f /
                         bufSz);   // Please note: because the LFO is applied already outside the DSP-loop we reduce it's frequency in a manner to fit
    lfoPWM.SetFrequency(1.f);

    lfoVibrato.SetSampleRate(44100.f /
                             bufSz);   // Please note: because the LFO is applied already outside the DSP-loop we reduce it's frequency in a manner to fit
    lfoVibrato.SetFrequency(1.f);

}

ctagSoundProcessorTalkbox::~ctagSoundProcessorTalkbox() {
    heap_caps_free(fx_buffer);
}

void ctagSoundProcessorTalkbox::knowYourself() {
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("MasterPitch", [&](const int val){ MasterPitch = val;});
	pMapCv.emplace("MasterPitch", [&](const int val){ cv_MasterPitch = val;});
	pMapPar.emplace("MasterTune", [&](const int val){ MasterTune = val;});
	pMapCv.emplace("MasterTune", [&](const int val){ cv_MasterTune = val;});
	pMapPar.emplace("CarrierAmnt", [&](const int val){ CarrierAmnt = val;});
	pMapCv.emplace("CarrierAmnt", [&](const int val){ cv_CarrierAmnt = val;});
	pMapPar.emplace("CarrierBalance", [&](const int val){ CarrierBalance = val;});
	pMapCv.emplace("CarrierBalance", [&](const int val){ cv_CarrierBalance = val;});
	pMapPar.emplace("CarrierExternalBalance", [&](const int val){ CarrierExternalBalance = val;});
	pMapCv.emplace("CarrierExternalBalance", [&](const int val){ cv_CarrierExternalBalance = val;});
	pMapPar.emplace("CarrierLeak", [&](const int val){ CarrierLeak = val;});
	pMapCv.emplace("CarrierLeak", [&](const int val){ cv_CarrierLeak = val;});
	pMapPar.emplace("ExtCarrierAmnt", [&](const int val){ ExtCarrierAmnt = val;});
	pMapCv.emplace("ExtCarrierAmnt", [&](const int val){ cv_ExtCarrierAmnt = val;});
	pMapPar.emplace("AccentBendIsActive", [&](const int val){ AccentBendIsActive = val;});
	pMapTrig.emplace("AccentBendIsActive", [&](const int val){ trig_AccentBendIsActive = val;});
	pMapPar.emplace("AccentBend", [&](const int val){ AccentBend = val;});
	pMapCv.emplace("AccentBend", [&](const int val){ cv_AccentBend = val;});
	pMapPar.emplace("ModulatorAmnt", [&](const int val){ ModulatorAmnt = val;});
	pMapCv.emplace("ModulatorAmnt", [&](const int val){ cv_ModulatorAmnt = val;});
	pMapPar.emplace("ModulatorLeakage", [&](const int val){ ModulatorLeakage = val;});
	pMapCv.emplace("ModulatorLeakage", [&](const int val){ cv_ModulatorLeakage = val;});
	pMapPar.emplace("Volume", [&](const int val){ Volume = val;});
	pMapCv.emplace("Volume", [&](const int val){ cv_Volume = val;});
	pMapPar.emplace("VibratoIsActive", [&](const int val){ VibratoIsActive = val;});
	pMapTrig.emplace("VibratoIsActive", [&](const int val){ trig_VibratoIsActive = val;});
	pMapPar.emplace("VibratoSpeed", [&](const int val){ VibratoSpeed = val;});
	pMapCv.emplace("VibratoSpeed", [&](const int val){ cv_VibratoSpeed = val;});
	pMapPar.emplace("VibratoDepth", [&](const int val){ VibratoDepth = val;});
	pMapCv.emplace("VibratoDepth", [&](const int val){ cv_VibratoDepth = val;});
	pMapPar.emplace("PWMactive", [&](const int val){ PWMactive = val;});
	pMapTrig.emplace("PWMactive", [&](const int val){ trig_PWMactive = val;});
	pMapPar.emplace("PWMspeed", [&](const int val){ PWMspeed = val;});
	pMapCv.emplace("PWMspeed", [&](const int val){ cv_PWMspeed = val;});
	pMapPar.emplace("PWMdepth", [&](const int val){ PWMdepth = val;});
	pMapCv.emplace("PWMdepth", [&](const int val){ cv_PWMdepth = val;});
	pMapPar.emplace("SubOscTune_A", [&](const int val){ SubOscTune_A = val;});
	pMapCv.emplace("SubOscTune_A", [&](const int val){ cv_SubOscTune_A = val;});
	pMapPar.emplace("SubOscTuningBactive", [&](const int val){ SubOscTuningBactive = val;});
	pMapTrig.emplace("SubOscTuningBactive", [&](const int val){ trig_SubOscTuningBactive = val;});
	pMapPar.emplace("SubOscTune_B", [&](const int val){ SubOscTune_B = val;});
	pMapCv.emplace("SubOscTune_B", [&](const int val){ cv_SubOscTune_B = val;});
	pMapPar.emplace("FXIsActive", [&](const int val){ FXIsActive = val;});
	pMapTrig.emplace("FXIsActive", [&](const int val){ trig_FXIsActive = val;});
	pMapPar.emplace("FXEnsChor", [&](const int val){ FXEnsChor = val;});
	pMapTrig.emplace("FXEnsChor", [&](const int val){ trig_FXEnsChor = val;});
	pMapPar.emplace("FXDepth", [&](const int val){ FXDepth = val;});
	pMapCv.emplace("FXDepth", [&](const int val){ cv_FXDepth = val;});
	pMapPar.emplace("FXAmt", [&](const int val){ FXAmt = val;});
	pMapCv.emplace("FXAmt", [&](const int val){ cv_FXAmt = val;});
	isStereo = true;
	id = "Talkbox";
	// sectionCpp0
}