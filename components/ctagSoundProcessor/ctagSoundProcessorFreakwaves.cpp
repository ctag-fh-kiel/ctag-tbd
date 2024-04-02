/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020/2021 by Robert Manzke. All rights reserved.

(c) 2021 for the "Freakwaves"-Plugin by Mathias Brüssel
"Freakwaves" provides a synth based on two wavetable-oscillators with different CV/Gate inputs for their notes. 
In result it can be played duophonically, but there also is a third oscillator that either can be used as a sub oscillator for oscillator two 
or to generate a third line of notes resulting on the other two inputs. 
Depending on the two streams of notes coming in, as well as the sound of the synth as whole the overall sound can change. 
These possible interferences in combination with the wavetable sound sources is why the plugin is called "Freakwaves".

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "ctagSoundProcessorFreakwaves.hpp"
#include "braids/quantizer_scales.h"

using namespace CTAG::SP;

// --- Additional macros for oscillator and GUI-parameter processing ---
#define MK_TRIG_PAR(outname, inname)            int outname = process_param_trig(data, trig_##inname, inname, e_##inname);
#define MK_TRIG_PAR_LOC(outname, inname)        outname = process_param_trig(data, trig_##inname, inname, e_##inname);
#define MK_GATE_PAR(outname, inname)            int outname = process_param_trig(data, trig_##inname, inname, e_##inname, 1);

#define MK_INT_PAR_ABS_LOC(outname, inname, scale) outname = inname;  if(cv_##inname != -1) outname = static_cast<int>(fabsf(data.cv[cv_##inname]) * scale);
#define MK_INT_CONSTRAIN_LOC(outname, inname, factor); MK_INT_PAR_ABS_LOC(outname, inname, (float)(factor+1)); CONSTRAIN(outname, 0, factor);

// --- Additional Macro for automated parameter evaluations ---
#define MK_PITCH_PAR(outname, inname)     float outname = inname; if(cv_##inname != -1) outname += data.cv[cv_##inname]*60.f;
#define MK_INT_CONSTRAIN(outname, inname, factor); MK_INT_PAR_ABS(outname, inname, (float)(factor+1)); CONSTRAIN(outname, 0, factor);
#define MK_INT_CONSTRAIN_LOC(outname, inname, factor); MK_INT_PAR_ABS_LOC(outname, inname, (float)(factor+1)); CONSTRAIN(outname, 0, factor);
#define MK_FLT_PAR_HYSTERESIS(outname, inname, hysteresis_memory, norm, scale) \
        float outname = inname / norm * scale; \
        if(cv_##inname != -1) { outname = 0.05f * fabsf(data.cv[cv_##inname])*scale + 0.95f * hysteresis_memory; \
        hysteresis_memory = outname; }
#define MK_FLT_PAR_ABS_MIN_MAX_LOC(outname, inname, norm, out_min, out_max) \
        outname = inname/norm * (out_max-out_min)+out_min; \
        if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]) * (out_max-out_min)+out_min;
#define MK_FLT_PAR_ABS_LOC(outname, inname, norm, scale) \
        outname = inname / norm * scale;\
        if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]) * scale;

// --- Hysteresis to calm down Accent Bend just a bit ---
#define interpolate_hysteresis(outname, inname, hysteresis_memory) { outname = 0.05f*inname + 0.95f*hysteresis_memory; hysteresis_memory = outname; }
#define interpolate_hysteresis_vari(outname, inname, varival, hysteresis_memory) { outname = (1.0-varival)*inname + varival*hysteresis_memory; hysteresis_memory = outname; }


#define MIN_LFO_SPEED               0.05f
#define MAX_LFO_SPEED               20.f
#define MAX_ADSR_RELEASE            10.f
#define MAX_ADSR_RELEASE_MODIFIED   15.f

// --- Convert "Braids-Logic" ---
#define DSP_FLOAT2INT(in_val)         ((int)(in_val*32767.f))
#define DSP_FLOAT2INT_PITCH(in_val)   ((int)(in_val*16383.f))

using namespace CTAG::SP;

// --- Morph sinewave to square, pseudo triangle, sample and hold and similar ---
float ctagSoundProcessorFreakwaves::morph_sine_wave(float sine_val, int morph_mode, int enum_sine) {
    switch (morph_mode) {
        case 0:                                       // SINE_WAVE
            return (sine_val);                           // leave unchanged!
        case 1:
            return ((sine_val >= 0.f) ? 1.f : -1.f);       // SINE_TO_SQUARE
        case 2:
            return (fabsf(sine_val));                   // SINE_MOD_RIGHT
        case 3:
            return (-fabsf(sine_val));                  // SINE_MOD_LEFT
        case 4:
            return (1.0f - fabsf(sine_val));               // SINE_TO_TRI_MOD_RIGH
        case 5:
            return (-(1.0f - fabsf(sine_val)));            // SINE_TO_TRI_MOD_LEFT
        case 6:
            return (applySnH(sine_val, enum_sine));      // Sample & Hold, values -1.f ... +1.f
        default:                                      // unexpected value
            return (sine_val);                           // leave unchanged!
    }
}

// --- Sample & Hold LFOs ---
float ctagSoundProcessorFreakwaves::applySnH(float sine_lfo_val, int enum_val) {
    if (sine_lfo_val >=
        0.f)   // positive half-wave after a previous negative one will trigger a new random value to be held
    {
        if (previous_sine_val[enum_val] <
            0.f)      // Index relies on the member "enum snh_members", we use arrays for the elements needed instead of dynamically instanciating several objects
            saved_sample[enum_val] = oscSnH[enum_val].Process();  // values -1.f ... +1.f
    }
    previous_sine_val[enum_val] = sine_lfo_val;
    return (saved_sample[enum_val]);
}


// --- Process trigger signals and keep their state internally ---
inline int ctagSoundProcessorFreakwaves::process_param_trig(const ProcessData &data, int trig_myparm, int my_parm,
                                                            int enum_trigger_id, int gate_type = 0) {
    int trig_status = 0;

    if (trig_myparm != -1)       // Trigger given via CV/Gate or button?
    {
        trig_status = (data.trig[trig_myparm] == 0); // HIGH is 0, so we negate for boolean logic
        if (gate_type == 1)    // Use Gate-signal
        {
            if (trig_status) {
                if (low_reached[enum_trigger_id])   // We have a positive gate and last time it was negative)
                {
                    low_reached[enum_trigger_id] = false;
                    return (GATE_HIGH_NEW);           // New trigger
                } else
                    return (GATE_HIGH);
            } else {
                low_reached[enum_trigger_id] = true;
                return (GATE_LOW);
            }
        }
        // --- Non-Gate triggers, if we reach here: toggle-trigger or AD-Trigger ---
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


// --- Transform wavetables from the format as to be found on https://waveeditonline.com to Mutable Instruments format, so that we can use the Plaits WT-Oscillator ---
void ctagSoundProcessorFreakwaves::prepareWavetable(const int16_t **wavetables, int currentBank, bool *isWaveTableGood,
                                                    float **fbuffer, int16_t **ibuffer) {
    // Precalculates wavetable data according to https://www.dafx12.york.ac.uk/papers/dafx12_submission_69.pdf
    // plaits uses integrated wavetable synthesis, i.e. integrated wavetables, order K=1 (one integration), N=1 (linear interpolation)
    // check if sample rom seems to have current bank
    if (!sample_rom.HasSliceGroup(currentBank * 64, currentBank * 64 + 63)) {
        *isWaveTableGood = false;
        return;
    }
    int size = sample_rom.GetSliceGroupSize(currentBank * 64, currentBank * 64 + 63);
    if (size != 256 * 64) {
        *isWaveTableGood = false;
        return;
    }
    int bankOffset = currentBank * 64 * 256;
    int bufferOffset =
            4 * 64; // load sample data into buffer at offset, due to pre-calculation each wave will be 260 words long
    sample_rom.Read(&((*ibuffer)[bufferOffset]), bankOffset, 256 * 64);
    // --- Start conversion of data, 64 wavetables per bank ---
    int c = 0;
    for (int i = 0; i < 64; i++)   // iterate all waves
    {
        int startOffset = bufferOffset + i * 256; // which wave
        // prepare long array, i.e. x = numpy.array(list(wave) * 2 + wave[0] + wave[1] + wave[2] + wave[3])
        float sum4 = (*ibuffer)[startOffset] + (*ibuffer)[startOffset + 1] + (*ibuffer)[startOffset + 2] +
                     (*ibuffer)[startOffset + 3]; // add dc
        for (int j = 0; j < 512; j++)
            (*fbuffer)[j] = (*ibuffer)[startOffset + (j % 256)] + sum4;
        removeMeanOfFloatArray(*fbuffer, 512);      // x -= x.mean()
        scaleFloatArrayToAbsMax(*fbuffer, 512);     // x /= numpy.abs(x).max()
        accumulateFloatArray(*fbuffer, 512);        // x = numpy.cumsum(x)
        removeMeanOfFloatArray(*fbuffer, 512);      // x -= x.mean()
        wavetables[i] = &((*ibuffer)[c]);                // create pointer map
        for (int j = 512 - 256 - 4; j < 512; j++) {
            int16_t v = static_cast<int16_t >(roundf(
                    (*fbuffer)[j] * 4.f * 32768.f / 256.f)); // x = numpy.round(x * (4 * 32768.0 / WAVETABLE_SIZE)
            (*ibuffer)[c++] = v;
        }
    }
    *isWaveTableGood = true;
}

void ctagSoundProcessorFreakwaves::Process(const ProcessData &data) {
    float vol_eg_A = 1.f;
    float vol_eg_B = 1.f;
    float vol_eg_C = 1.f;

    // === Read Control-Data from GUI or TBD controls ===
    // --- Global ---
    MK_GATE_PAR(g_GateA, GateA);
    MK_GATE_PAR(g_GateB, GateB);
    MK_PITCH_PAR(f_MasterPitch, MasterPitch);
    MK_FLT_PAR_ABS_SFT(f_MasterTune, MasterTune, 1200.f, 12.f);
    MK_TRIG_PAR(t_CoutOnRightCh, CoutOnRightCh);
    MK_TRIG_PAR(t_QuantInputA, QuantInputA);MK_INT_CONSTRAIN(iQscale_A, Qscale_A, 47);
    MK_TRIG_PAR(t_QuantInputB, QuantInputB);MK_INT_CONSTRAIN(iQscale_B, Qscale_B, 47);MK_INT_CONSTRAIN(iQscale_C,
                                                                                                       Qscale_C, 47);
    MK_FLT_PAR_ABS_MIN_MAX(f_PortamentoTimeA, PortamentoTimeA, 4095.f, 0.95f, 0.99999f);
    MK_FLT_PAR_ABS_MIN_MAX(f_PortamentoTimeB, PortamentoTimeB, 4095.f, 0.95f, 0.99999f);
    MK_FLT_PAR_ABS_MIN_MAX(f_PortamentoTimeC, PortamentoTimeC, 4095.f, 0.95f, 0.99999f);
    MK_TRIG_PAR(t_Porta_A, PortaA);
    MK_TRIG_PAR(t_Porta_B, PortaB);
    MK_TRIG_PAR(t_Porta_C, PortaC);
    MK_FLT_PAR_ABS(f_Vol_A, Vol_A, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_Vol_B, Vol_B, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_Vol_C, Vol_C, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_Vol_Ext, Vol_Ext, 4095.f, 3.f);
    MK_FLT_PAR_ABS(f_ExternalWet, ExternalWet, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_BalanceAB_C, BalanceAB_C, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_Volume, Volume, 4095.f,
                   4.f);   // We use a high value to have headroom, so lower the volume normally

    // --- Oscillators ---
    MK_INT_CONSTRAIN(i_WaveTblA, WaveTblA, 31);    // Only selectable via GUI because of possibly slow performance!
    MK_FLT_PAR_ABS(f_ScanWavTblA, ScanWavTblA, 4095.f, 1.f);
    MK_PITCH_PAR(f_pitch_A, pitch_A);
    MK_FLT_PAR_ABS_SFT(f_tune_A, tune_A, 1200.f, 12.f);

    MK_INT_CONSTRAIN(i_WaveTblB, WaveTblB, 31);   // Only selectable via GUI because of possibly slow performance!
    MK_FLT_PAR_ABS(f_ScanWavTblB, ScanWavTblB, 4095.f, 1.f);
    MK_PITCH_PAR(f_pitch_B, pitch_B);
    MK_FLT_PAR_ABS_SFT(f_tune_B, tune_B, 1200.f, 12.f);

    MK_INT_CONSTRAIN(i_WaveTblC, WaveTblC, 31);   // Only selectable via GUI because of possibly slow performance!
    MK_FLT_PAR_ABS(f_ScanWavTblC, ScanWavTblC, 4095.f, 1.f);
    MK_FLT_PAR_ABS_SFT(f_relative_tune_C, relative_tune_C, 1200.f, 12.f);

    // --- Envelope ---
    MK_TRIG_PAR(t_EGvolActive_A, EGvolActive_A);
    MK_TRIG_PAR(t_EGvolSlow_A, EGvolSlow_A);
    MK_FLT_PAR_ABS(f_AttackVol_A, AttackVol_A, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_DecayVol_A, DecayVol_A, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_SustainVol_A, SustainVol_A, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_ReleaseVol_A, ReleaseVol_A, 4095.f, MAX_ADSR_RELEASE);

    MK_TRIG_PAR(t_EGvolActive_B, EGvolActive_B);
    MK_TRIG_PAR(t_EGvolSlow_B, EGvolSlow_B);
    MK_FLT_PAR_ABS(f_AttackVol_B, AttackVol_B, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_DecayVol_B, DecayVol_B, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_SustainVol_B, SustainVol_B, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_ReleaseVol_B, ReleaseVol_B, 4095.f, MAX_ADSR_RELEASE);

    MK_TRIG_PAR(t_EGvolActive_C, EGvolActive_C);
    MK_TRIG_PAR(t_EGvolSlow_C, EGvolSlow_C);
    MK_FLT_PAR_ABS(f_AttackVol_C, AttackVol_C, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_DecayVol_C, DecayVol_C, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_SustainVol_C, SustainVol_C, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_ReleaseVol_C, ReleaseVol_C, 4095.f, MAX_ADSR_RELEASE);

    // --- Modifiers ---
    MK_TRIG_PAR(t_GeneratePitchC, GeneratePitchC);
    MK_TRIG_PAR(t_CnotAorB, CnotAorB);
    MK_TRIG_PAR(t_ChooseBforNot, ChooseBforNot);
    MK_TRIG_PAR(t_UseABGate, UseABGate);
    MK_TRIG_PAR(t_GateForCisB, GateForCisB);
    MK_TRIG_PAR(t_ExtModCisOn, ExtModCisOn);
    MK_FLT_PAR_ABS(f_ExtModGain, ExtModGain, 4095.f, 200.f);
    MK_FLT_PAR_ABS_SFT(f_AmountForExternalMod, AmountForExternalMod, 2048.f, 1.f);
    MK_TRIG_PAR(t_ExternalDetectToRelease, ExternalDetectToRelease);
    MK_FLT_PAR_ABS_MIN_MAX(f_LFOspeedC, LFOspeedC, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
    MK_FLT_PAR_ABS(f_LFOamountC, LFOamountC, 4095.f, 1.f);
    MK_TRIG_PAR(t_AccentModCisOn, AccentModCisOn);
    MK_FLT_PAR_ABS(f_DetectAccentLevel, DetectAccentLevel, 4095.f, 200.f);
    MK_TRIG_PAR(t_AccentToScanC, AccentToScanC);
    MK_FLT_PAR_ABS_SFT(f_AmountForAccentMod, AmountForAccentMod, 2048.f, 1.f);
    MK_TRIG_PAR(t_KeytrackModCisOn, KeytrackModCisOn);
    MK_TRIG_PAR(t_KeytrackSlew, KeytrackSlew);
    MK_FLT_PAR_ABS(f_KeytrackingLevel, KeytrackingLevel, 4095.f, 3.f);
    MK_TRIG_PAR(t_KeytrackingToEcho, KeytrackingToEcho);

    // --- Delay ---
    MK_TRIG_PAR(t_DelayEnable, DelayEnable);
    MK_TRIG_PAR(t_AddDelayAfterResonator, AddDelayAfterResonator);
    MK_TRIG_PAR(t_DelayTimeShortened, DelayTimeShortened);
    MK_FLT_PAR_HYSTERESIS(f_DelayTime, DelayTime, m_DelayTime, 4095.f, 1.f);
    MK_FLT_PAR_HYSTERESIS(f_DelayFeedback, DelayFeedback, m_DelayFeedback, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_DelayDryWet, DelayDryWet, 4095.f, 1.f);

    // --- Resonator ---
    MK_TRIG_PAR(t_ResonatorOn, ResonatorOn);
    MK_FLT_PAR_ABS(f_ResonatorDryWet, ResonatorDryWet, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_WaveShaperDryWet, WaveShaperDryWet, 4095.f, 1.f);
    float f_ResonatorPosition = ResonatorPosition / 23.f * 0.9999f; // only in GUI, no CV
    MK_FLT_PAR_ABS_MIN_MAX(f_ResonatorFreq, ResonatorFreq, 4095.f, 10.f, 6000.f);         // Frequency in Hz
    MK_FLT_PAR_ABS(f_ResonatorStructure, ResonatorStructure, 4095.f, 1.f);                          // Values 0-1
    MK_FLT_PAR_ABS(f_ResonatorBrightness, ResonatorBrightness, 4095.f, 1.f);                        // Values 0-1
    MK_FLT_PAR_ABS(f_ResonatorDamping, ResonatorDamping, 4095.f, 1.f);                              // Values 0-1

    // --- LFOs ---
    MK_TRIG_PAR_LOC(t_lfoActive[0], lfoActive_1);MK_INT_CONSTRAIN_LOC(i_lfoDestination[0], lfoDestination_1,
                                                                      17);   // Destinations 0-17 are currently available
    MK_INT_CONSTRAIN_LOC(i_lfoType[0], lfoType_1,
                         6);                 // LFO-wavetype i_lfoType[x] is fixed to 0-6, 6 == S&H and initialized already
    MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[0], lfoSpeed_1, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
    MK_FLT_PAR_ABS_LOC(f_lfoAmnt[0], lfoAmnt_1, 4095.f, 1.f);

    MK_TRIG_PAR_LOC(t_lfoActive[1], lfoActive_2);MK_INT_CONSTRAIN_LOC(i_lfoDestination[1], lfoDestination_2,
                                                                      17);MK_INT_CONSTRAIN_LOC(i_lfoType[1], lfoType_2,
                                                                                               6);
    MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[1], lfoSpeed_2, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
    MK_FLT_PAR_ABS_LOC(f_lfoAmnt[1], lfoAmnt_2, 4095.f, 1.f);

    MK_TRIG_PAR_LOC(t_lfoActive[2], lfoActive_3);MK_INT_CONSTRAIN_LOC(i_lfoDestination[2], lfoDestination_3,
                                                                      17);MK_INT_CONSTRAIN_LOC(i_lfoType[2], lfoType_3,
                                                                                               6);
    MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[2], lfoSpeed_3, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
    MK_FLT_PAR_ABS_LOC(f_lfoAmnt[2], lfoAmnt_3, 4095.f, 1.f);

    MK_TRIG_PAR_LOC(t_lfoActive[3], lfoActive_4);MK_INT_CONSTRAIN_LOC(i_lfoDestination[3], lfoDestination_4,
                                                                      17);MK_INT_CONSTRAIN_LOC(i_lfoType[3], lfoType_4,
                                                                                               6);
    MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[3], lfoSpeed_4, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
    MK_FLT_PAR_ABS_LOC(f_lfoAmnt[3], lfoAmnt_4, 4095.f, 1.f);

    MK_TRIG_PAR_LOC(t_lfoActive[4], lfoActive_5);MK_INT_CONSTRAIN_LOC(i_lfoDestination[4], lfoDestination_5,
                                                                      17);MK_INT_CONSTRAIN_LOC(i_lfoType[4], lfoType_5,
                                                                                               6);
    MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[4], lfoSpeed_5, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
    MK_FLT_PAR_ABS_LOC(f_lfoAmnt[4], lfoAmnt_5, 4095.f, 1.f);

    MK_TRIG_PAR_LOC(t_lfoActive[5], lfoActive_6);MK_INT_CONSTRAIN_LOC(i_lfoDestination[5], lfoDestination_6,
                                                                      17);MK_INT_CONSTRAIN_LOC(i_lfoType[5], lfoType_6,
                                                                                               6);
    MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[5], lfoSpeed_6, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
    MK_FLT_PAR_ABS_LOC(f_lfoAmnt[5], lfoAmnt_6, 4095.f, 1.f);

    MK_TRIG_PAR_LOC(t_lfoActive[6], lfoActive_7);MK_INT_CONSTRAIN_LOC(i_lfoDestination[6], lfoDestination_7,
                                                                      17);MK_INT_CONSTRAIN_LOC(i_lfoType[6], lfoType_7,
                                                                                               6);
    MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[6], lfoSpeed_7, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
    MK_FLT_PAR_ABS_LOC(f_lfoAmnt[6], lfoAmnt_7, 4095.f, 1.f);

    // === Precalculations ===
    // --- Pitch ---
    float f_current_note = f_MasterPitch + f_MasterTune;

    // --- Calculate portamento-effects for OSC A and/or OSC B, if set ---
    if (t_Porta_A) {
        if (g_GateA == GATE_HIGH_NEW) {
            portamentoHyteresisA_ = portaPrevNoteA_;  // We have a new gate, so reset OSC A for portamento-effect
            portaPrevNoteA_ = f_pitch_A;
        }
        interpolate_hysteresis_vari(f_pitch_A, f_pitch_A, f_PortamentoTimeA,
                                    portamentoHyteresisA_);  // should work without the reset obove, too
    }
    if (t_Porta_B) {
        if (g_GateB == GATE_HIGH_NEW) {
            portamentoHyteresisB_ = portaPrevNoteB_;  // We have a new gate, so reset OSC A for portamento-effect, just to be sure
            portaPrevNoteB_ = f_pitch_B;
        }
        interpolate_hysteresis_vari(f_pitch_B, f_pitch_B, f_PortamentoTimeB,
                                    portamentoHyteresisB_);  // should work without the reset obove, too
    }
    // --- External modulation, see Modifier section ---
    if (t_ExtModCisOn) {
        // --- Calculate modulation caused by external input, if any ---
        interpolate_hysteresis(f_AmountForExternalMod, f_externalVal_ * f_AmountForExternalMod * f_ExtModGain,
                               externalHyteresis_);
        if (t_ExternalDetectToRelease) // Increase or decrease release for OSC C according to signal-level of external input, adjusted by additional parameters
        {
            f_ReleaseVol_C += f_AmountForExternalMod * MAX_ADSR_RELEASE_MODIFIED;
            CONSTRAIN(f_ReleaseVol_C, 0.f, MAX_ADSR_RELEASE_MODIFIED);
        } else  // Increase or decrease pitch-modulation for OSC C according to signal-level of external input, adjusted by additional parameters
        {
            f_LFOamountC += f_AmountForExternalMod;
            CONSTRAIN(f_LFOamountC, 0.f, 1.f)
        }
    }
    // --- Calculate assignable LFOs (we also do this as early as possible, because it may change other values we will use as we go on) ---
    bool lfoPitchedA = false;
    bool lfoPitchedB = false;
    for (int i = 0; i < NUM_OF_LFOS_FW; i++) {
        if (t_lfoActive[i])  // Please note: we process the S&H LFOs (IDs 0-3) at first, because they can change speeds of LFOs 1-4 with IDs 4-7 as well!
        {
            lfo[i].SetFrequency(f_lfoSpeed[i]);
            f_lfo_val[i] = morph_sine_wave(lfo[i].Process(), i_lfoType[i], i) *
                           f_lfoAmnt[i];      // (float sine_val, int morph_mode, int enum_sine)
            switch (i_lfoDestination[i]) {
                case 0:     // Tune A
                    f_MasterTune += f_lfo_val[i] * 11.f;   // Random +- one octave
                    f_current_note = f_MasterPitch + f_MasterTune;
                    break;
                case 1:     // Modulate volume of external signal
                    f_Vol_Ext += f_lfo_val[i];
                    CONSTRAIN(f_Vol_Ext, 0.f, 3.f);
                    break;
                case 2:     // Modulate Balance of OSC A+ OSC B+external signal vs. OSC C
                    f_BalanceAB_C += f_lfo_val[i];
                    CONSTRAIN(f_BalanceAB_C, 0.f, 1.f);
                    break;
                case 3:     // Scan Wavetable A
                    f_ScanWavTblA += f_lfo_val[i];
                    CONSTRAIN(f_ScanWavTblA, 0.f, 1.f);
                    break;
                case 4:     // Pitch A
                    f_pitch_A += f_lfo_val[i] * 23.f;   // Modulation or Random +- 2 octaves
                    lfoPitchedA = true;
                    break;
                case 5:     // Scan Wavetable B
                    f_ScanWavTblB += f_lfo_val[i];
                    CONSTRAIN(f_ScanWavTblB, 0.f, 1.f);
                    break;
                case 6:     // Pitch B
                    f_pitch_B += f_lfo_val[i] * 23.f;     // Modulation or Random +- 2 octaves
                    lfoPitchedB = true;
                    break;
                case 7:     // Scan Wavetable C
                    f_ScanWavTblC += f_lfo_val[i];
                    CONSTRAIN(f_ScanWavTblC, 0.f, 1.f);
                    break;
                case 8:     // Pitch C
                    f_pitch_stored_C_ += f_lfo_val[i] * 23.f;
                    break;
                case 9:     // Delay Dry<=>Wet
                    f_DelayDryWet += f_lfo_val[i];
                    CONSTRAIN(f_DelayDryWet, 0.f, 1.f);
                    break;
                case 10:    // Modulate the delay-time
                    f_DelayTime += f_lfo_val[i];
                    CONSTRAIN(f_DelayTime, 0.f, 1.f);
                    break;
                case 11:    // Modulate the Delay feedback
                    f_DelayFeedback += f_lfo_val[i];
                    CONSTRAIN(f_DelayFeedback, 0.f, 1.f);
                    break;
                    break;
                case 12:     // Resonator Dry<=>Wet
                    f_ResonatorDryWet += f_lfo_val[i];
                    CONSTRAIN(f_ResonatorDryWet, 0.f, 1.f);
                    break;
                case 13:    // Modulate the drive-amount for the resonator
                    f_WaveShaperDryWet += f_lfo_val[i];
                    CONSTRAIN(f_WaveShaperDryWet, 0.f, 1.f);
                    break;
                case 14:    // Resonator frequency in Hz
                    f_ResonatorFreq += f_lfo_val[i] * 1000.f;
                    CONSTRAIN(f_ResonatorFreq, 10.f, 6000.f);
                    break;
                case 15:    // Resonator structure
                    f_ResonatorStructure += f_lfo_val[i];
                    CONSTRAIN(f_ResonatorStructure, 0.f, 1.f);
                    break;
                case 16:    // Resonator brightness
                    f_ResonatorBrightness += f_lfo_val[i];
                    CONSTRAIN(f_ResonatorBrightness, 0.f, 1.f);
                    break;
                case 17:    // Resonator Damping
                    f_ResonatorDamping += f_lfo_val[i];
                    CONSTRAIN(f_ResonatorDamping, 0.f, 1.f);
                    break;
            }
        }
    }
    // --- OSC C: Check for pitch and Gate to be possible generated as a result of OSC A and OSC B ---
    if (t_GeneratePitchC) {
        if (t_CnotAorB)    // Logical NOT connective
        {
            if (!(fabsf(f_pitch_A - f_pitch_B) <
                  0.05f))  // Pitch of OSC A and OSC B are NOT identical within a range of less than 5 Cent
            {
                if (t_ChooseBforNot)  // Use pitch of OSC B for output
                    f_pitch_stored_C_ = f_pitch_B;   // Set pitch of OSC C and also remember it internally as "most recent valid pitch"
                else  // Use pitch of OSC A for output
                    f_pitch_stored_C_ = f_pitch_A;   // Set pitch of OSC C and also remember it internally as "most recent valid pitch"
                if (t_UseABGate)   // Only apply gate on OSC C if we encountered a gate for OSC A or OSC B
                {
                    if (!t_GateForCisB && g_GateA)       // We are looking for Gate A and A has a gate
                        g_GateC_ = g_GateA;
                    else if (t_GateForCisB && g_GateB)   // We are looking for Gate A and A has a gate
                        g_GateC_ = g_GateB;
                    else
                        g_GateC_ = GATE_LOW;                 // We found a match in frequencies but no active gate for A or B, so we don't enable C
                } else {
                    if (g_GateC_ == GATE_LOW)
                        g_GateC_ = GATE_HIGH_NEW;
                    else
                        g_GateC_ = GATE_HIGH;                    // We found a match in frequencies and don't care for gates from A or C, so we enable the gate of C
                }
            } else
                g_GateC_ = GATE_LOW;       // No NOT matching frequencies of A and B, so we don't have a gate for C anyways...
        } else    // Logical AND connective
        {
            if (fabsf(f_pitch_A - f_pitch_B) <
                0.05f)  // Pitch of OSC A and OSC B are identical within a range of less than 5 Cent
            {
                f_pitch_stored_C_ = f_pitch_A;   // Set pitch of OSC C and also remember it internally as "most recent valid pitch"
                if (t_UseABGate)   // Only apply gate on OSC C if we encountered a gate for OSC A or OSC B
                {
                    if (!t_GateForCisB && g_GateA)       // We are looking for Gate A and A has a gate
                        g_GateC_ = g_GateA;
                    else if (t_GateForCisB && g_GateB)   // We are looking for Gate A and A has a gate
                        g_GateC_ = g_GateB;
                    else
                        g_GateC_ = GATE_LOW;                 // We found a match in frequencies but no active gate for A or B, so we don't enable C
                } else {
                    if (g_GateC_ == GATE_LOW)
                        g_GateC_ = GATE_HIGH_NEW;
                    else
                        g_GateC_ = GATE_HIGH;                    // We found a match in frequencies and don't care for gates from A or C, so we enable the gate of C
                }
            } else
                g_GateC_ = GATE_LOW;                     // No matching frequencies of A and B, so we don't have a gate for C anyways...
        }
    } else    // No logical connective chosen to generate OSC C, OSC C will be a "suboscillator" of OSC B instead...
    {
        f_pitch_stored_C_ = f_pitch_B;
        g_GateC_ = g_GateB;
    }
    // --- Possible portamento for OSC C ---
    if (t_Porta_C) {
        if (g_GateC_ == GATE_HIGH_NEW) {
            portamentoHyteresisC_ = portaPrevNoteC_;  // We have a new gate, so reset OSC A for  portamento-effect, just to be sure...
            portaPrevNoteC_ = f_pitch_stored_C_;
        }
        interpolate_hysteresis_vari(f_pitch_stored_C_, f_pitch_stored_C_, f_PortamentoTimeC,
                                    portamentoHyteresisC_);  // should work without the reset obove, too
    }
    // --- Calculate LFO of modifier section ---
    lfoPitchC.SetFrequency(
            f_LFOspeedC);  // Please note: Pitch mod will also happen if we have no external signal to increase or decrease it!
    f_pitch_stored_C_ +=
            lfoPitchC.Process() * f_LFOamountC * 7.f;  // We allow pitchmodulation (sine LFO) of max a fifht.

    // --- Keytracking ---
    if (t_KeytrackModCisOn) {
        if (t_KeytrackSlew) // Use "slew limiter" to calm down keytracking, especially useful when changining delay-times
        interpolate_hysteresis(f_KeytrackingLevel, f_KeytrackingLevel, keytrackHyteresis_);
        if (t_KeytrackingToEcho) {
            f_DelayTime += f_KeytrackingLevel * (f_pitch_stored_C_ / 128.f);
            CONSTRAIN(f_DelayTime, 0.f, 1.f);
        } else {
            f_ResonatorStructure += f_KeytrackingLevel * (f_pitch_stored_C_ / 128.f);
            CONSTRAIN(f_ResonatorStructure, 0.f, 1.f);
        }
    }
    // --- Calculate accent, if any ---
    if (t_AccentModCisOn) {
        float f_accent_target = f_DetectAccentLevel * f_AmountForAccentMod;
        interpolate_hysteresis(f_accent_target, f_accentVal_ * f_accent_target, accentHyteresis_);
        if (!t_AccentToScanC)    // Accent bends Pitch of OSC C
        {
            f_accent_target *= 7.f;
            CONSTRAIN(f_accent_target, -7.f,
                      7.f);                    // Max Accent Bend is a perfect fifth (7 semitones)
            f_pitch_stored_C_ += f_accent_target;
        } else                      // Accent "bends" scan of wavetable for OSC C
        {
            f_ScanWavTblC += f_accent_target;
            CONSTRAIN(f_ScanWavTblC, 0.f, 1.f);                     // Max Accent Bend for Wavetable scan of OSC C
        }
    }
    // --- Scale quantize OSC A,B,C now (possible pitch-LFO by variable destinations / OSC C also has possible modifier induced pitch-LFO ---
    if (t_QuantInputA || (!t_QuantInputA &&
                          lfoPitchedA))   // Pitch-quantize input and/or quantize modulation , index 0 does not quantize
    {
        quantiz_A.Configure(
                braids::scales[iQscale_A]); // 48 different scales, Braids uses integers, we have to multiply our floats by 128 below!
        f_pitch_A = quantiz_A.Process((int) (averageA.dejitter(f_pitch_A + f_tune_A) * 128.f),
                                      (int) (f_MasterPitch * 128.f)) /
                    128.f;  // possible negative values are "fixed" in the quantizer
    }
    if (t_QuantInputB ||
        (!t_QuantInputB && lfoPitchedB))  // Pitch-quantize input and/or quantize modulation , index 0 does not quantize
    {
        quantiz_B.Configure(
                braids::scales[iQscale_B]); // 48 different scales, Braids uses integers, we have to multiply our floats by 128 below!
        f_pitch_B = quantiz_B.Process((int) (averageB.dejitter(f_pitch_B + f_tune_B) * 128.f),
                                      (int) (f_MasterPitch * 128.f)) /
                    128.f;  // possible negative values are "fixed" in the quantizer
    }
    // Pitch-quantize OSC C, index 0 does not quantize
    quantiz_C.Configure(
            braids::scales[iQscale_C]); // 48 different scales, Braids uses integers, we have to multiply our floats by 128 below!
    f_pitch_stored_C_ =
            quantiz_C.Process((int) (averageC.dejitter(f_pitch_stored_C_) * 128.f), (int) (f_MasterPitch * 128.f)) /
            128.f;  // possible negative values are "fixed" in the quantizer

    // --- Envelopes ---
    if (t_EGvolActive_A)   // ADSR mode
    {
        if (t_EGvolSlow_A)   // We extend the EG-times (for AD _and_ ADSR) if slow envelope is selected!
        {
            f_AttackVol_A *= 30.f;
            f_DecayVol_A *= 30.f;
            f_ReleaseVol_A *= 30.f;
        }
        env_A.SetAttack(f_AttackVol_A);
        env_A.SetDecay(f_DecayVol_A);
        env_A.SetSustain(f_SustainVol_A);
        env_A.SetRelease(f_ReleaseVol_A);
        env_A.Gate(g_GateA);
        vol_eg_A = env_A.Process();
    }
    if (t_EGvolActive_B)   // ADSR mode
    {
        if (t_EGvolSlow_B)   // We extend the EG-times (for AD _and_ ADSR) if slow envelope is selected!
        {
            f_AttackVol_B *= 30.f;
            f_DecayVol_B *= 30.f;
            f_ReleaseVol_B *= 30.f;
        }
        env_B.SetAttack(f_AttackVol_B);
        env_B.SetDecay(f_DecayVol_B);
        env_B.SetSustain(f_SustainVol_B);
        env_B.SetRelease(f_ReleaseVol_B);
        env_B.Gate(g_GateB);
        vol_eg_B = env_B.Process();
    }
    if (t_EGvolActive_C)   // ADSR mode
    {
        if (t_EGvolSlow_C)   // We extend the EG-times (for AD _and_ ADSR) if slow envelope is selected!
        {
            f_AttackVol_C *= 30.f;
            f_DecayVol_C *= 30.f;
            f_ReleaseVol_C *= 30.f;
        }
        env_C.SetAttack(f_AttackVol_C);
        env_C.SetDecay(f_DecayVol_C);
        env_C.SetSustain(f_SustainVol_C);
        env_C.SetRelease(f_ReleaseVol_C);
        if (t_GeneratePitchC)
            env_C.Gate(g_GateC_);
        else
            env_C.Gate(g_GateB);
        vol_eg_C = env_C.Process();   // Precalculate current Volume EG, it will be added in the "main" DSP-loop below
    }
    // --- Resonator ---
    if (fabs(f_ResonatorPosition - lastResonatorPos_) > 0.05f)  // Is the devation relevant enough to reinit?
    {
        lastResonatorPos_ = f_ResonatorPosition;
        resonator.Init(f_ResonatorPosition, 4.f);
    }
    // --- Wave select A ---
    currentBank_A = WaveTblA;
    if (lastBank_A != currentBank_A) { // this is slow, hence not modulated by CV
        prepareWavetable((const int16_t **) &wavetables_A, currentBank_A, &isWaveTableGood_A, &fbuffer_A, &buffer_A);
        lastBank_A = currentBank_A;
    }
    float totalPitchA = t_QuantInputA ? f_pitch_A : f_pitch_A +
                                                    f_tune_A; // If we quantize the incoming pitch, tuning is already part of the quantisation!
    const float f_freq_A = plaits::NoteToFrequency(60 + totalPitchA + f_current_note) * 0.998f;

    // --- Wave select B ---
    currentBank_B = WaveTblB;
    if (lastBank_B != currentBank_B) { // this is slow, hence not modulated by CV
        prepareWavetable((const int16_t **) &wavetables_B, currentBank_B, &isWaveTableGood_B, &fbuffer_B, &buffer_B);
        lastBank_B = currentBank_B;
    }
    float totalPitchB = t_QuantInputB ? f_pitch_B : f_pitch_B +
                                                    f_tune_B; // If we quantize the incoming pitch, tuning is already part of the quantisation!
    const float f_freq_B = plaits::NoteToFrequency(60 + totalPitchB + f_current_note) * 0.998f;

    // --- Wave select C ---
    currentBank_C = WaveTblC;
    if (lastBank_C != currentBank_C) { // this is slow, hence not modulated by CV
        prepareWavetable((const int16_t **) &wavetables_C, currentBank_C, &isWaveTableGood_C, &fbuffer_C, &buffer_C);
        lastBank_C = currentBank_C;
    } // For OSC C pitch, even if quantized, tuning is additinally possible to allow "beating" frequencies
    const float f_freq_C =
            plaits::NoteToFrequency(60 + f_relative_tune_C + f_pitch_stored_C_ + f_current_note) * 0.998f;

    // --- Balance of Oscillators+external Input / voices ---
    float f_ExternalDry = (1.f - f_ExternalWet) * f_Vol_Ext;
    f_ExternalWet *= f_Vol_Ext;
    f_Vol_A *= 1.f - f_BalanceAB_C;
    f_Vol_B *= 1.f - f_BalanceAB_C;
    f_Vol_Ext *= 1.f - f_BalanceAB_C;
    f_Vol_C *= f_BalanceAB_C;

    // --- Dry/wet of resonator ---
    float resonator_dry = f_Volume;   // We multiply by volume already here as an optimisation, this will be the last result of the DSP-chain...
    float resonator_wet = 0.f;
    if (t_ResonatorOn) {
        resonator_dry = (1.f - f_ResonatorDryWet) *
                        f_Volume; // We multiply by volume already here as an optimisation, this will be the last result of the DSP-chain...
        resonator_wet = f_ResonatorDryWet *
                        f_Volume;         // We multiply by volume already here as an optimisation, this will be the last result of the DSP-chain...
    }
    // --- Calculate delay settings ---
    float f_delay_dry = 1.f;
    float f_delay_wet = 0.f;
    if (t_DelayEnable) {
        f_delay_dry = (1.f - f_DelayDryWet);
        f_delay_wet = f_DelayDryWet;
    }
    float f_delay_len = t_DelayTimeShortened ? f_DelayTime * shorterMaxDelayLength : f_DelayTime * maxDelayLength;
    if (f_delay_len < 1.f)
        f_delay_len = 1.f;
    dlyLine.SetLength((uint32_t) f_delay_len);
    dlyLine.SetFeedback(f_DelayFeedback);

    // === Main DSP output loop[s] ===
    float wave_osc_buf[32] = {0.f}; // Beware: for Plaits Wavetable rendering the buffer must be "empty"!
    float wave_osc_buf_c[32] = {0.f}; // Seperate output for OSC C is selected via GUI
    float delay_buf[32] = {0.f};    // Delaybuffer

    // --- Process oscillators and apply MGs and EGs to them if required ---
    if (isWaveTableGood_A)
        wt_osc_A.Render(f_freq_A, f_Vol_A * vol_eg_A, f_ScanWavTblA, wavetables_A, wave_osc_buf, bufSz);

    if (isWaveTableGood_B)
        wt_osc_B.Render(f_freq_B, f_Vol_B * vol_eg_B, f_ScanWavTblB, wavetables_B, wave_osc_buf, bufSz);

    if (t_CoutOnRightCh) // Send OSC C seperately to right output
    {
        if (isWaveTableGood_C)   // If pitch of OSC is meant to be generated but currently invalid, we will simply omit generating the audio for it
            wt_osc_C.Render(f_freq_C, f_Vol_C * vol_eg_C, f_ScanWavTblC, wavetables_C, wave_osc_buf_c, bufSz);
    } else    // Mix all Wavetable oscillators for output
    {
        if (isWaveTableGood_C)   // If pitch of OSC is meant to be generated but currently invalid, we will simply omit generating the audio for it
            wt_osc_C.Render(f_freq_C, f_Vol_C * vol_eg_C, f_ScanWavTblC, wavetables_C, wave_osc_buf, bufSz);
    }
    // --- Remember one sample of unprocessed external input level for possibly later modulation usage ---
    f_externalVal_ = averageExternal.dejitter(fabsf(data.buf[0]));

    // --- Additional data and processing for resonator ---
    f_ResonatorFreq /= 44100.f;
    float reso_buf[32]{0};
    if (t_AddDelayAfterResonator) {
        float external_signal_wet = 0.f;
        float oscillators_signal = 0.f;
        // --- Output oscillators with delay, processed by resonator now! ---
        for (uint32_t i = 0; i < 32; i++) {
            external_signal_wet = data.buf[i * 2] * f_ExternalWet;
            oscillators_signal = wave_osc_buf[i];
            reso_buf[i] = Fold_do(oscillators_signal + external_signal_wet, f_WaveShaperDryWet);
            wave_osc_buf[i] = (oscillators_signal + external_signal_wet) *
                              resonator_dry;   // Apply Resonator now and let original signal through partly if activated
        }
        resonator.Process(f_ResonatorFreq, f_ResonatorStructure, f_ResonatorBrightness, f_ResonatorDamping, reso_buf,
                          reso_buf, bufSz);
        for (uint32_t i = 0; i < 32; i++)
            wave_osc_buf[i] = wave_osc_buf[i] + reso_buf[i] *
                                                resonator_wet;   // Apply Resonator now and let original signal through partly if activated

        memcpy(delay_buf, wave_osc_buf, bufSz);
        dlyLine.Process(delay_buf, 0, 1, bufSz);         // Add delay

        // --- Main output loop -> please note: there is some redundant code here, but we found out that having no if-clouses inside the loop speeds up output massively ;-) ---
        if (t_CoutOnRightCh) // Output for processed OSC C on the right, unprocessed OSC A+OSC B+external in on the left
        {
            for (uint32_t bufId = 0; bufId < bufSz; bufId++) {
                data.buf[bufId * 2] = delay_buf[bufId] * f_delay_wet + wave_osc_buf[bufId] * f_delay_dry +
                                      data.buf[bufId * 2] * f_ExternalDry;
                data.buf[bufId * 2 + 1] = wave_osc_buf_c[bufId];
            }
            // === Postprocessing ===
            f_accentVal_ = averageAccent.dejitter((fabsf(data.buf[30]) + fabsf(data.buf[31])) /
                                                  2.f);  // Remember average volume of last completely processed value to maybe apply as Accent Bend or "Wavescan" in next round...
        } else  // Output everything to the left channel
        {
            for (uint32_t bufId = 0; bufId < bufSz; bufId++)
                data.buf[bufId * 2] = delay_buf[bufId] * f_delay_wet + wave_osc_buf[bufId] * f_delay_dry +
                                      data.buf[bufId * 2] * f_ExternalDry;
            // === Postprocessing ===
            f_accentVal_ = averageAccent.dejitter(
                    fabsf(data.buf[30]));  // Remember average volume of last completely processed value to maybe apply as Accent Bend or "Wavescan" in next round...
        }
    } else // Add Delay before Resonator
    {
        // --- Add delay (before resonator)---
        memcpy(delay_buf, wave_osc_buf, bufSz);
        for (uint32_t bufId = 0; bufId < bufSz; bufId++)
            delay_buf[bufId * 2] += data.buf[bufId * 2] * f_ExternalWet;
        dlyLine.Process(delay_buf, 0, 1, bufSz);         // Add delay

        // --- Output oscillators with delay, processed by resonator now! (please note: there is some redundant code here, but we found out that having no if-clouses inside the loop speeds up output massively ;-) ---
        if (t_CoutOnRightCh) // Output for processed OSC C on the right, unprocessed OSC A+OSC B+external in on the left
        {
            for (uint32_t bufId = 0; bufId < bufSz; bufId++) {
                delay_buf[bufId] = delay_buf[bufId] * f_delay_wet + wave_osc_buf[bufId] *
                                                                    f_delay_dry;     // Add Wavefolder+delay as set by Dry/Wet Slider
                reso_buf[bufId] = Fold_do(delay_buf[bufId], f_WaveShaperDryWet);
            }
            resonator.Process(f_ResonatorFreq, f_ResonatorStructure, f_ResonatorBrightness, f_ResonatorDamping,
                              reso_buf, reso_buf, bufSz);
            for (uint32_t bufId = 0; bufId < bufSz; bufId++) {
                data.buf[bufId * 2] = data.buf[bufId * 2] * f_ExternalDry + delay_buf[bufId] * resonator_dry +
                                      reso_buf[bufId] *
                                      resonator_wet;   // Apply Resonator now and let original signal through partly if activated
                data.buf[bufId * 2 + 1] = wave_osc_buf_c[bufId];
            }
            // === Postprocessing ===
            f_accentVal_ = averageAccent.dejitter((fabsf(data.buf[30]) + fabsf(data.buf[31])) /
                                                  2.f);  // Remember average volume of last completely processed value to maybe apply as Accent Bend or "Wavescan" in next round...
        } else    // Output everything to the left channel
        {
            for (uint32_t bufId = 0; bufId < bufSz; bufId++) {
                delay_buf[bufId] = delay_buf[bufId] * f_delay_wet + wave_osc_buf[bufId] *
                                                                    f_delay_dry;     // Add Wavefolder+delay as set by Dry/Wet Slider
                reso_buf[bufId] = Fold_do(delay_buf[bufId], f_WaveShaperDryWet);
            }
            resonator.Process(f_ResonatorFreq, f_ResonatorStructure, f_ResonatorBrightness, f_ResonatorDamping,
                              reso_buf, reso_buf, bufSz);
            for (uint32_t bufId = 0; bufId < bufSz; bufId++) {
                data.buf[bufId * 2] = data.buf[bufId * 2] * f_ExternalDry + delay_buf[bufId] * resonator_dry +
                                      reso_buf[bufId] *
                                      resonator_wet;   // Apply Resonator now and let original signal through partly if activated
                data.buf[bufId * 2 + 1] = 0.f;  // Make sure that we "leave no garbage" on the unused channel
            }
            // === Postprocessing ===
            f_accentVal_ = averageAccent.dejitter(
                    fabsf(data.buf[30]));  // Remember average volume of last completely processed value to maybe apply as Accent Bend or "Wavescan" in next round...
        }
    }
}

void ctagSoundProcessorFreakwaves::Init(std::size_t blockSize, void *blockPtr) {
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    // --- Init LFOs ---
    lfoPitchC.SetSampleRate(44100.f / bufSz);
    lfoPitchC.SetFrequency(1.f);

    for (int i = 0; i < NUM_OF_LFOS_FW; i++) {
        lfo[i].SetSampleRate(44100.f /
                             bufSz);   // Please note: because the LFO is applied already outside the DSP-loop we reduce it's frequency in a manner to fit
        lfo[i].SetFrequency(1.f);
    }

    int totalBlockSzRequired = 260 * 64 * 2 + 512 * 4;
    totalBlockSzRequired *= 3;
    assert(totalBlockSzRequired < blockSize);

    // Alloc mem for one wavetable (Oscillator A)
    buffer_A = (int16_t *) blockPtr;
    blockPtr = static_cast<uint8_t *>(blockPtr) + 260 * 64 * 2;
    memset(buffer_A, 0, 260 * 64 * 2);
    fbuffer_A = (float *) blockPtr;
    blockPtr = static_cast<uint8_t *>(blockPtr) + 512 * 2;
    memset(fbuffer_A, 0, 512 * 4);
    wt_osc_A.Init();


    // Alloc mem for one wavetable (Oscillator B)
    buffer_B = (int16_t *) blockPtr;
    blockPtr = static_cast<uint8_t *>(blockPtr) + 260 * 64 * 2;
    memset(buffer_B, 0, 260 * 64 * 2);
    fbuffer_B = (float *) blockPtr;
    blockPtr = static_cast<uint8_t *>(blockPtr) + 512 * 2;
    memset(fbuffer_B, 0, 512 * 4);
    wt_osc_B.Init();

    // Alloc mem for one wavetable (Oscillator C)
    buffer_C = (int16_t *) blockPtr;
    blockPtr = static_cast<uint8_t *>(blockPtr) + 260 * 64 * 2;
    memset(buffer_C, 0, 260 * 64 * 2);
    fbuffer_C = (float *) blockPtr;
    blockPtr = static_cast<uint8_t *>(blockPtr) + 512 * 2;
    memset(fbuffer_C, 0, 512 * 4);
    wt_osc_C.Init();

    env_A.SetSampleRate(44100.f /
                        bufSz);       // ADSR-EG: sync Env with our audio-processing, but update only once outside the main DSP-Loop
    env_A.SetModeExp();                             // Logarithmic scaling
    env_B.SetSampleRate(44100.f /
                        bufSz);       // ADSR-EG: sync Env with our audio-processing, but update only once outside the main DSP-Loop
    env_B.SetModeExp();                             // Logarithmic scaling
    env_C.SetSampleRate(44100.f /
                        bufSz);       // ADSR-EG: sync Env with our audio-processing, but update only once outside the main DSP-Loop
    env_C.SetModeExp();                             // Logarithmic scaling

    resonator.Init(0.5f, 4);  // Position of phase (0-1), resolution(4-24): quality vs. speed, samplerate
    // --- Scale quantizers for Oscillator A and B ---
    quantiz_A.Init();
    quantiz_B.Init();
}

ctagSoundProcessorFreakwaves::~ctagSoundProcessorFreakwaves() {
}

void ctagSoundProcessorFreakwaves::knowYourself() {
    // autogenerated code here
    // sectionCpp0
    pMapPar.emplace("GateA", [&](const int val) { GateA = val; });
    pMapTrig.emplace("GateA", [&](const int val) { trig_GateA = val; });
    pMapPar.emplace("GateB", [&](const int val) { GateB = val; });
    pMapTrig.emplace("GateB", [&](const int val) { trig_GateB = val; });
    pMapPar.emplace("MasterPitch", [&](const int val) { MasterPitch = val; });
    pMapCv.emplace("MasterPitch", [&](const int val) { cv_MasterPitch = val; });
    pMapPar.emplace("MasterTune", [&](const int val) { MasterTune = val; });
    pMapCv.emplace("MasterTune", [&](const int val) { cv_MasterTune = val; });
    pMapPar.emplace("CoutOnRightCh", [&](const int val) { CoutOnRightCh = val; });
    pMapTrig.emplace("CoutOnRightCh", [&](const int val) { trig_CoutOnRightCh = val; });
    pMapPar.emplace("QuantInputA", [&](const int val) { QuantInputA = val; });
    pMapTrig.emplace("QuantInputA", [&](const int val) { trig_QuantInputA = val; });
    pMapPar.emplace("Qscale_A", [&](const int val) { Qscale_A = val; });
    pMapCv.emplace("Qscale_A", [&](const int val) { cv_Qscale_A = val; });
    pMapPar.emplace("QuantInputB", [&](const int val) { QuantInputB = val; });
    pMapTrig.emplace("QuantInputB", [&](const int val) { trig_QuantInputB = val; });
    pMapPar.emplace("Qscale_B", [&](const int val) { Qscale_B = val; });
    pMapCv.emplace("Qscale_B", [&](const int val) { cv_Qscale_B = val; });
    pMapPar.emplace("Qscale_C", [&](const int val) { Qscale_C = val; });
    pMapCv.emplace("Qscale_C", [&](const int val) { cv_Qscale_C = val; });
    pMapPar.emplace("PortaA", [&](const int val) { PortaA = val; });
    pMapTrig.emplace("PortaA", [&](const int val) { trig_PortaA = val; });
    pMapPar.emplace("PortamentoTimeA", [&](const int val) { PortamentoTimeA = val; });
    pMapCv.emplace("PortamentoTimeA", [&](const int val) { cv_PortamentoTimeA = val; });
    pMapPar.emplace("PortaB", [&](const int val) { PortaB = val; });
    pMapTrig.emplace("PortaB", [&](const int val) { trig_PortaB = val; });
    pMapPar.emplace("PortamentoTimeB", [&](const int val) { PortamentoTimeB = val; });
    pMapCv.emplace("PortamentoTimeB", [&](const int val) { cv_PortamentoTimeB = val; });
    pMapPar.emplace("PortaC", [&](const int val) { PortaC = val; });
    pMapTrig.emplace("PortaC", [&](const int val) { trig_PortaC = val; });
    pMapPar.emplace("PortamentoTimeC", [&](const int val) { PortamentoTimeC = val; });
    pMapCv.emplace("PortamentoTimeC", [&](const int val) { cv_PortamentoTimeC = val; });
    pMapPar.emplace("Vol_A", [&](const int val) { Vol_A = val; });
    pMapCv.emplace("Vol_A", [&](const int val) { cv_Vol_A = val; });
    pMapPar.emplace("Vol_B", [&](const int val) { Vol_B = val; });
    pMapCv.emplace("Vol_B", [&](const int val) { cv_Vol_B = val; });
    pMapPar.emplace("Vol_C", [&](const int val) { Vol_C = val; });
    pMapCv.emplace("Vol_C", [&](const int val) { cv_Vol_C = val; });
    pMapPar.emplace("Vol_Ext", [&](const int val) { Vol_Ext = val; });
    pMapCv.emplace("Vol_Ext", [&](const int val) { cv_Vol_Ext = val; });
    pMapPar.emplace("ExternalWet", [&](const int val) { ExternalWet = val; });
    pMapCv.emplace("ExternalWet", [&](const int val) { cv_ExternalWet = val; });
    pMapPar.emplace("BalanceAB_C", [&](const int val) { BalanceAB_C = val; });
    pMapCv.emplace("BalanceAB_C", [&](const int val) { cv_BalanceAB_C = val; });
    pMapPar.emplace("Volume", [&](const int val) { Volume = val; });
    pMapCv.emplace("Volume", [&](const int val) { cv_Volume = val; });
    pMapPar.emplace("WaveTblA", [&](const int val) { WaveTblA = val; });
    pMapCv.emplace("WaveTblA", [&](const int val) { cv_WaveTblA = val; });
    pMapPar.emplace("ScanWavTblA", [&](const int val) { ScanWavTblA = val; });
    pMapCv.emplace("ScanWavTblA", [&](const int val) { cv_ScanWavTblA = val; });
    pMapPar.emplace("pitch_A", [&](const int val) { pitch_A = val; });
    pMapCv.emplace("pitch_A", [&](const int val) { cv_pitch_A = val; });
    pMapPar.emplace("tune_A", [&](const int val) { tune_A = val; });
    pMapCv.emplace("tune_A", [&](const int val) { cv_tune_A = val; });
    pMapPar.emplace("WaveTblB", [&](const int val) { WaveTblB = val; });
    pMapCv.emplace("WaveTblB", [&](const int val) { cv_WaveTblB = val; });
    pMapPar.emplace("ScanWavTblB", [&](const int val) { ScanWavTblB = val; });
    pMapCv.emplace("ScanWavTblB", [&](const int val) { cv_ScanWavTblB = val; });
    pMapPar.emplace("pitch_B", [&](const int val) { pitch_B = val; });
    pMapCv.emplace("pitch_B", [&](const int val) { cv_pitch_B = val; });
    pMapPar.emplace("tune_B", [&](const int val) { tune_B = val; });
    pMapCv.emplace("tune_B", [&](const int val) { cv_tune_B = val; });
    pMapPar.emplace("WaveTblC", [&](const int val) { WaveTblC = val; });
    pMapCv.emplace("WaveTblC", [&](const int val) { cv_WaveTblC = val; });
    pMapPar.emplace("ScanWavTblC", [&](const int val) { ScanWavTblC = val; });
    pMapCv.emplace("ScanWavTblC", [&](const int val) { cv_ScanWavTblC = val; });
    pMapPar.emplace("relative_tune_C", [&](const int val) { relative_tune_C = val; });
    pMapCv.emplace("relative_tune_C", [&](const int val) { cv_relative_tune_C = val; });
    pMapPar.emplace("GeneratePitchC", [&](const int val) { GeneratePitchC = val; });
    pMapTrig.emplace("GeneratePitchC", [&](const int val) { trig_GeneratePitchC = val; });
    pMapPar.emplace("CnotAorB", [&](const int val) { CnotAorB = val; });
    pMapTrig.emplace("CnotAorB", [&](const int val) { trig_CnotAorB = val; });
    pMapPar.emplace("ChooseBforNot", [&](const int val) { ChooseBforNot = val; });
    pMapTrig.emplace("ChooseBforNot", [&](const int val) { trig_ChooseBforNot = val; });
    pMapPar.emplace("UseABGate", [&](const int val) { UseABGate = val; });
    pMapTrig.emplace("UseABGate", [&](const int val) { trig_UseABGate = val; });
    pMapPar.emplace("GateForCisB", [&](const int val) { GateForCisB = val; });
    pMapTrig.emplace("GateForCisB", [&](const int val) { trig_GateForCisB = val; });
    pMapPar.emplace("LFOspeedC", [&](const int val) { LFOspeedC = val; });
    pMapCv.emplace("LFOspeedC", [&](const int val) { cv_LFOspeedC = val; });
    pMapPar.emplace("LFOamountC", [&](const int val) { LFOamountC = val; });
    pMapCv.emplace("LFOamountC", [&](const int val) { cv_LFOamountC = val; });
    pMapPar.emplace("ExtModCisOn", [&](const int val) { ExtModCisOn = val; });
    pMapTrig.emplace("ExtModCisOn", [&](const int val) { trig_ExtModCisOn = val; });
    pMapPar.emplace("ExtModGain", [&](const int val) { ExtModGain = val; });
    pMapCv.emplace("ExtModGain", [&](const int val) { cv_ExtModGain = val; });
    pMapPar.emplace("AmountForExternalMod", [&](const int val) { AmountForExternalMod = val; });
    pMapCv.emplace("AmountForExternalMod", [&](const int val) { cv_AmountForExternalMod = val; });
    pMapPar.emplace("ExternalDetectToRelease", [&](const int val) { ExternalDetectToRelease = val; });
    pMapTrig.emplace("ExternalDetectToRelease", [&](const int val) { trig_ExternalDetectToRelease = val; });
    pMapPar.emplace("AccentModCisOn", [&](const int val) { AccentModCisOn = val; });
    pMapTrig.emplace("AccentModCisOn", [&](const int val) { trig_AccentModCisOn = val; });
    pMapPar.emplace("DetectAccentLevel", [&](const int val) { DetectAccentLevel = val; });
    pMapCv.emplace("DetectAccentLevel", [&](const int val) { cv_DetectAccentLevel = val; });
    pMapPar.emplace("AmountForAccentMod", [&](const int val) { AmountForAccentMod = val; });
    pMapCv.emplace("AmountForAccentMod", [&](const int val) { cv_AmountForAccentMod = val; });
    pMapPar.emplace("AccentToScanC", [&](const int val) { AccentToScanC = val; });
    pMapTrig.emplace("AccentToScanC", [&](const int val) { trig_AccentToScanC = val; });
    pMapPar.emplace("KeytrackModCisOn", [&](const int val) { KeytrackModCisOn = val; });
    pMapTrig.emplace("KeytrackModCisOn", [&](const int val) { trig_KeytrackModCisOn = val; });
    pMapPar.emplace("KeytrackingLevel", [&](const int val) { KeytrackingLevel = val; });
    pMapCv.emplace("KeytrackingLevel", [&](const int val) { cv_KeytrackingLevel = val; });
    pMapPar.emplace("KeytrackSlew", [&](const int val) { KeytrackSlew = val; });
    pMapTrig.emplace("KeytrackSlew", [&](const int val) { trig_KeytrackSlew = val; });
    pMapPar.emplace("KeytrackingToEcho", [&](const int val) { KeytrackingToEcho = val; });
    pMapTrig.emplace("KeytrackingToEcho", [&](const int val) { trig_KeytrackingToEcho = val; });
    pMapPar.emplace("EGvolActive_A", [&](const int val) { EGvolActive_A = val; });
    pMapTrig.emplace("EGvolActive_A", [&](const int val) { trig_EGvolActive_A = val; });
    pMapPar.emplace("EGvolSlow_A", [&](const int val) { EGvolSlow_A = val; });
    pMapTrig.emplace("EGvolSlow_A", [&](const int val) { trig_EGvolSlow_A = val; });
    pMapPar.emplace("AttackVol_A", [&](const int val) { AttackVol_A = val; });
    pMapCv.emplace("AttackVol_A", [&](const int val) { cv_AttackVol_A = val; });
    pMapPar.emplace("DecayVol_A", [&](const int val) { DecayVol_A = val; });
    pMapCv.emplace("DecayVol_A", [&](const int val) { cv_DecayVol_A = val; });
    pMapPar.emplace("SustainVol_A", [&](const int val) { SustainVol_A = val; });
    pMapCv.emplace("SustainVol_A", [&](const int val) { cv_SustainVol_A = val; });
    pMapPar.emplace("ReleaseVol_A", [&](const int val) { ReleaseVol_A = val; });
    pMapCv.emplace("ReleaseVol_A", [&](const int val) { cv_ReleaseVol_A = val; });
    pMapPar.emplace("EGvolActive_B", [&](const int val) { EGvolActive_B = val; });
    pMapTrig.emplace("EGvolActive_B", [&](const int val) { trig_EGvolActive_B = val; });
    pMapPar.emplace("EGvolSlow_B", [&](const int val) { EGvolSlow_B = val; });
    pMapTrig.emplace("EGvolSlow_B", [&](const int val) { trig_EGvolSlow_B = val; });
    pMapPar.emplace("AttackVol_B", [&](const int val) { AttackVol_B = val; });
    pMapCv.emplace("AttackVol_B", [&](const int val) { cv_AttackVol_B = val; });
    pMapPar.emplace("DecayVol_B", [&](const int val) { DecayVol_B = val; });
    pMapCv.emplace("DecayVol_B", [&](const int val) { cv_DecayVol_B = val; });
    pMapPar.emplace("SustainVol_B", [&](const int val) { SustainVol_B = val; });
    pMapCv.emplace("SustainVol_B", [&](const int val) { cv_SustainVol_B = val; });
    pMapPar.emplace("ReleaseVol_B", [&](const int val) { ReleaseVol_B = val; });
    pMapCv.emplace("ReleaseVol_B", [&](const int val) { cv_ReleaseVol_B = val; });
    pMapPar.emplace("EGvolActive_C", [&](const int val) { EGvolActive_C = val; });
    pMapTrig.emplace("EGvolActive_C", [&](const int val) { trig_EGvolActive_C = val; });
    pMapPar.emplace("EGvolSlow_C", [&](const int val) { EGvolSlow_C = val; });
    pMapTrig.emplace("EGvolSlow_C", [&](const int val) { trig_EGvolSlow_C = val; });
    pMapPar.emplace("AttackVol_C", [&](const int val) { AttackVol_C = val; });
    pMapCv.emplace("AttackVol_C", [&](const int val) { cv_AttackVol_C = val; });
    pMapPar.emplace("DecayVol_C", [&](const int val) { DecayVol_C = val; });
    pMapCv.emplace("DecayVol_C", [&](const int val) { cv_DecayVol_C = val; });
    pMapPar.emplace("SustainVol_C", [&](const int val) { SustainVol_C = val; });
    pMapCv.emplace("SustainVol_C", [&](const int val) { cv_SustainVol_C = val; });
    pMapPar.emplace("ReleaseVol_C", [&](const int val) { ReleaseVol_C = val; });
    pMapCv.emplace("ReleaseVol_C", [&](const int val) { cv_ReleaseVol_C = val; });
    pMapPar.emplace("DelayEnable", [&](const int val) { DelayEnable = val; });
    pMapTrig.emplace("DelayEnable", [&](const int val) { trig_DelayEnable = val; });
    pMapPar.emplace("AddDelayAfterResonator", [&](const int val) { AddDelayAfterResonator = val; });
    pMapTrig.emplace("AddDelayAfterResonator", [&](const int val) { trig_AddDelayAfterResonator = val; });
    pMapPar.emplace("DelayDryWet", [&](const int val) { DelayDryWet = val; });
    pMapCv.emplace("DelayDryWet", [&](const int val) { cv_DelayDryWet = val; });
    pMapPar.emplace("DelayTimeShortened", [&](const int val) { DelayTimeShortened = val; });
    pMapTrig.emplace("DelayTimeShortened", [&](const int val) { trig_DelayTimeShortened = val; });
    pMapPar.emplace("DelayTime", [&](const int val) { DelayTime = val; });
    pMapCv.emplace("DelayTime", [&](const int val) { cv_DelayTime = val; });
    pMapPar.emplace("DelayFeedback", [&](const int val) { DelayFeedback = val; });
    pMapCv.emplace("DelayFeedback", [&](const int val) { cv_DelayFeedback = val; });
    pMapPar.emplace("ResonatorOn", [&](const int val) { ResonatorOn = val; });
    pMapTrig.emplace("ResonatorOn", [&](const int val) { trig_ResonatorOn = val; });
    pMapPar.emplace("ResonatorDryWet", [&](const int val) { ResonatorDryWet = val; });
    pMapCv.emplace("ResonatorDryWet", [&](const int val) { cv_ResonatorDryWet = val; });
    pMapPar.emplace("WaveShaperDryWet", [&](const int val) { WaveShaperDryWet = val; });
    pMapCv.emplace("WaveShaperDryWet", [&](const int val) { cv_WaveShaperDryWet = val; });
    pMapPar.emplace("ResonatorPosition", [&](const int val) { ResonatorPosition = val; });
    pMapCv.emplace("ResonatorPosition", [&](const int val) { cv_ResonatorPosition = val; });
    pMapPar.emplace("ResonatorFreq", [&](const int val) { ResonatorFreq = val; });
    pMapCv.emplace("ResonatorFreq", [&](const int val) { cv_ResonatorFreq = val; });
    pMapPar.emplace("ResonatorStructure", [&](const int val) { ResonatorStructure = val; });
    pMapCv.emplace("ResonatorStructure", [&](const int val) { cv_ResonatorStructure = val; });
    pMapPar.emplace("ResonatorBrightness", [&](const int val) { ResonatorBrightness = val; });
    pMapCv.emplace("ResonatorBrightness", [&](const int val) { cv_ResonatorBrightness = val; });
    pMapPar.emplace("ResonatorDamping", [&](const int val) { ResonatorDamping = val; });
    pMapCv.emplace("ResonatorDamping", [&](const int val) { cv_ResonatorDamping = val; });
    pMapPar.emplace("lfoActive_1", [&](const int val) { lfoActive_1 = val; });
    pMapTrig.emplace("lfoActive_1", [&](const int val) { trig_lfoActive_1 = val; });
    pMapPar.emplace("lfoDestination_1", [&](const int val) { lfoDestination_1 = val; });
    pMapCv.emplace("lfoDestination_1", [&](const int val) { cv_lfoDestination_1 = val; });
    pMapPar.emplace("lfoType_1", [&](const int val) { lfoType_1 = val; });
    pMapCv.emplace("lfoType_1", [&](const int val) { cv_lfoType_1 = val; });
    pMapPar.emplace("lfoSpeed_1", [&](const int val) { lfoSpeed_1 = val; });
    pMapCv.emplace("lfoSpeed_1", [&](const int val) { cv_lfoSpeed_1 = val; });
    pMapPar.emplace("lfoAmnt_1", [&](const int val) { lfoAmnt_1 = val; });
    pMapCv.emplace("lfoAmnt_1", [&](const int val) { cv_lfoAmnt_1 = val; });
    pMapPar.emplace("lfoActive_2", [&](const int val) { lfoActive_2 = val; });
    pMapTrig.emplace("lfoActive_2", [&](const int val) { trig_lfoActive_2 = val; });
    pMapPar.emplace("lfoDestination_2", [&](const int val) { lfoDestination_2 = val; });
    pMapCv.emplace("lfoDestination_2", [&](const int val) { cv_lfoDestination_2 = val; });
    pMapPar.emplace("lfoType_2", [&](const int val) { lfoType_2 = val; });
    pMapCv.emplace("lfoType_2", [&](const int val) { cv_lfoType_2 = val; });
    pMapPar.emplace("lfoSpeed_2", [&](const int val) { lfoSpeed_2 = val; });
    pMapCv.emplace("lfoSpeed_2", [&](const int val) { cv_lfoSpeed_2 = val; });
    pMapPar.emplace("lfoAmnt_2", [&](const int val) { lfoAmnt_2 = val; });
    pMapCv.emplace("lfoAmnt_2", [&](const int val) { cv_lfoAmnt_2 = val; });
    pMapPar.emplace("lfoActive_3", [&](const int val) { lfoActive_3 = val; });
    pMapTrig.emplace("lfoActive_3", [&](const int val) { trig_lfoActive_3 = val; });
    pMapPar.emplace("lfoDestination_3", [&](const int val) { lfoDestination_3 = val; });
    pMapCv.emplace("lfoDestination_3", [&](const int val) { cv_lfoDestination_3 = val; });
    pMapPar.emplace("lfoType_3", [&](const int val) { lfoType_3 = val; });
    pMapCv.emplace("lfoType_3", [&](const int val) { cv_lfoType_3 = val; });
    pMapPar.emplace("lfoSpeed_3", [&](const int val) { lfoSpeed_3 = val; });
    pMapCv.emplace("lfoSpeed_3", [&](const int val) { cv_lfoSpeed_3 = val; });
    pMapPar.emplace("lfoAmnt_3", [&](const int val) { lfoAmnt_3 = val; });
    pMapCv.emplace("lfoAmnt_3", [&](const int val) { cv_lfoAmnt_3 = val; });
    pMapPar.emplace("lfoActive_4", [&](const int val) { lfoActive_4 = val; });
    pMapTrig.emplace("lfoActive_4", [&](const int val) { trig_lfoActive_4 = val; });
    pMapPar.emplace("lfoDestination_4", [&](const int val) { lfoDestination_4 = val; });
    pMapCv.emplace("lfoDestination_4", [&](const int val) { cv_lfoDestination_4 = val; });
    pMapPar.emplace("lfoType_4", [&](const int val) { lfoType_4 = val; });
    pMapCv.emplace("lfoType_4", [&](const int val) { cv_lfoType_4 = val; });
    pMapPar.emplace("lfoSpeed_4", [&](const int val) { lfoSpeed_4 = val; });
    pMapCv.emplace("lfoSpeed_4", [&](const int val) { cv_lfoSpeed_4 = val; });
    pMapPar.emplace("lfoAmnt_4", [&](const int val) { lfoAmnt_4 = val; });
    pMapCv.emplace("lfoAmnt_4", [&](const int val) { cv_lfoAmnt_4 = val; });
    pMapPar.emplace("lfoActive_5", [&](const int val) { lfoActive_5 = val; });
    pMapTrig.emplace("lfoActive_5", [&](const int val) { trig_lfoActive_5 = val; });
    pMapPar.emplace("lfoDestination_5", [&](const int val) { lfoDestination_5 = val; });
    pMapCv.emplace("lfoDestination_5", [&](const int val) { cv_lfoDestination_5 = val; });
    pMapPar.emplace("lfoType_5", [&](const int val) { lfoType_5 = val; });
    pMapCv.emplace("lfoType_5", [&](const int val) { cv_lfoType_5 = val; });
    pMapPar.emplace("lfoSpeed_5", [&](const int val) { lfoSpeed_5 = val; });
    pMapCv.emplace("lfoSpeed_5", [&](const int val) { cv_lfoSpeed_5 = val; });
    pMapPar.emplace("lfoAmnt_5", [&](const int val) { lfoAmnt_5 = val; });
    pMapCv.emplace("lfoAmnt_5", [&](const int val) { cv_lfoAmnt_5 = val; });
    pMapPar.emplace("lfoActive_6", [&](const int val) { lfoActive_6 = val; });
    pMapTrig.emplace("lfoActive_6", [&](const int val) { trig_lfoActive_6 = val; });
    pMapPar.emplace("lfoDestination_6", [&](const int val) { lfoDestination_6 = val; });
    pMapCv.emplace("lfoDestination_6", [&](const int val) { cv_lfoDestination_6 = val; });
    pMapPar.emplace("lfoType_6", [&](const int val) { lfoType_6 = val; });
    pMapCv.emplace("lfoType_6", [&](const int val) { cv_lfoType_6 = val; });
    pMapPar.emplace("lfoSpeed_6", [&](const int val) { lfoSpeed_6 = val; });
    pMapCv.emplace("lfoSpeed_6", [&](const int val) { cv_lfoSpeed_6 = val; });
    pMapPar.emplace("lfoAmnt_6", [&](const int val) { lfoAmnt_6 = val; });
    pMapCv.emplace("lfoAmnt_6", [&](const int val) { cv_lfoAmnt_6 = val; });
    pMapPar.emplace("lfoActive_7", [&](const int val) { lfoActive_7 = val; });
    pMapTrig.emplace("lfoActive_7", [&](const int val) { trig_lfoActive_7 = val; });
    pMapPar.emplace("lfoDestination_7", [&](const int val) { lfoDestination_7 = val; });
    pMapCv.emplace("lfoDestination_7", [&](const int val) { cv_lfoDestination_7 = val; });
    pMapPar.emplace("lfoType_7", [&](const int val) { lfoType_7 = val; });
    pMapCv.emplace("lfoType_7", [&](const int val) { cv_lfoType_7 = val; });
    pMapPar.emplace("lfoSpeed_7", [&](const int val) { lfoSpeed_7 = val; });
    pMapCv.emplace("lfoSpeed_7", [&](const int val) { cv_lfoSpeed_7 = val; });
    pMapPar.emplace("lfoAmnt_7", [&](const int val) { lfoAmnt_7 = val; });
    pMapCv.emplace("lfoAmnt_7", [&](const int val) { cv_lfoAmnt_7 = val; });
    isStereo = true;
    id = "Freakwaves";
    // sectionCpp0
}