/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020/2021 by Robert Manzke. All rights reserved.

(c) 2021 for the "Subbotnik"-Plugin by Mathias Brüssel
Subbotnik originates as a term for a volunteer workday on a Saturday in the Soviet Union, also known under that name in the former DDR / Eastern Germany.
TBD-Subbotnik uses "Eastcoast" building-blocks in a more "Westcoast"/Buchla-synths inspired way. Its name is meant as a play on words, paying tribute to Morton Subotnick composer and friend of Don Buchla,
famous for his composition "Silver Apples of the Moon", using a Buchla 100, said to be the first electronic work available on record to the public.

Among other things Subbotnik uses two clones of Mutable Instruments Braids and examples for the VULT-language by Carlos Laguna Ruiz, including a Diode Ladder Filer
for more details on the topics please look here: https://mutable-instruments.net/modules/braids/ and here https://github.com/modlfo/vult

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

// --- Module dependant includes ---
#include "ctagSoundProcessorSubbotnik.hpp"
#include <iostream>
#include <cmath>
#include "braids/quantizer_scales.h"

// --- VULT "Library for TBD" ---
// #include "./vult/vult_formantor.cpp"  // Already defined in ctagSoundProcessorFormantor, so to avoid double defines we omit it here!
// #include "./vult/vultin.cpp"          // Already defined in ctagSoundProcessorFormantor, so to avoid double defines we omit it here!

using namespace CTAG::SP;

// --- Trigger/Gate values ---
#define GATE_HIGH_NEW       2
#define GATE_HIGH           1
#define GATE_LOW            0

// --- Additional macros for oscillator and GUI-parameter processing ---
#define MK_TRIG_PAR(outname, inname)            int outname = process_param_trig(data, trig_##inname, inname, e_##inname);
#define MK_TRIG_PAR_LOC(outname, inname)        outname = process_param_trig(data, trig_##inname, inname, e_##inname);
#define MK_GATE_PAR(outname, inname)            bool outname = (bool)process_param_trig(data, trig_##inname, inname, e_##inname, 1);
#define MK_ADEG_PAR(outname, inname)            int outname = process_param_trig(data, trig_##inname, inname, e_##inname, 2);
#define MK_FLT_PAR_ABS_MIN_MAX_LOC(outname, inname, norm, out_min, out_max) \
        outname = inname/norm * (out_max-out_min)+out_min; \
        if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]) * (out_max-out_min)+out_min;
#define MK_FLT_PAR_ABS_LOC(outname, inname, norm, scale) \
        outname = inname / norm * scale;\
        if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]) * scale;
#define MK_INT_CONSTRAIN(outname, inname, factor); MK_INT_PAR_ABS(outname, inname, (float)(factor+1)); CONSTRAIN(outname, 0, factor);

// -> Selecable from 0 to 128000 for 128 notes with 100 cent/semitone, one semitone in "Braids-notation" is 128 integer, 5V range => 12 * 5 * 128 == 7680 the factor 0.1279921875f is equivalent to inname/128000*16383
#define MK_BRAIDS_PITCH(outname, inname); \
        float outname = inname*0.1279921875f; \
        if(cv_##inname != -1) outname += fabsf(data.cv[cv_##inname])*7680.f;

// -> Range of one octave in "braids-notation" is 128*12 == 1536, the factor 1.28 is equivalent to inname/1200*1536
#define MK_BRAIDS_TUNE(outname, inname); \
        float outname = inname*1.28f; \
        if(cv_##inname != -1) outname = (fabsf(data.cv[cv_##inname])-0.5f)*1536.f;

#define MK_INT_PAR_ABS_LOC(outname, inname, scale) outname = inname;  if(cv_##inname != -1) outname = static_cast<int>(fabsf(data.cv[cv_##inname]) * scale);
#define MK_INT_CONSTRAIN_LOC(outname, inname, factor); MK_INT_PAR_ABS_LOC(outname, inname, (float)(factor+1)); CONSTRAIN(outname, 0, factor);
#define MK_INT_CONSTRAIN_MIN_MAX_LOC(outname, inname, out_min, out_max) \
        outname = (int)inname; \
        if(cv_##inname != -1) outname = (int)(fabsf(data.cv[cv_##inname]) * (out_max-out_min)+out_min); \
        CONSTRAIN(outname, out_min, out_max);

#define DSP_INT2FLOAT(in_val)         ((float)(in_val/32767.f))
#define DSP_FLOAT2INT(in_val)         ((int)(in_val*32767.f))
#define DSP_FLOAT2INT_PITCH(in_val)   ((int)(in_val*16383.f))

#define MIN_LFO_SPEED     0.05f
#define MAX_LFO_SPEED     20.f
#define MIN_AD_ATTACK     0.f
#define MAX_AD_ATTACK     8.f
#define MIN_AD_DECAY      1.f
#define MAX_AD_DECAY      10.f

#define MAX_ADSR_ATTACK   8.f
#define MAX_ADSR_DECAY    10.f
#define MAX_ADSR_RELEASE  10.f

// --- Morph sinewave to square, pseudo triangle, sample and hold and similar ---
float ctagSoundProcessorSubbotnik::morph_sine_wave(float sine_val, int morph_mode, int enum_sine)
{
  switch (morph_mode)
  {
    case 0:                                       // SINE_WAVE
      return(sine_val);                           // leave unchanged!
    case 1:
      return((sine_val>=0.f) ? 1.f : -1.f);       // SINE_TO_SQUARE
    case 2:
      return (fabsf(sine_val));                   // SINE_MOD_RIGHT
    case 3:
      return (-fabsf(sine_val));                  // SINE_MOD_LEFT
    case 4:
      return(1.0f-fabsf(sine_val));               // SINE_TO_TRI_MOD_RIGH
    case 5:
      return(-(1.0f-fabsf(sine_val)));            // SINE_TO_TRI_MOD_LEFT
    case 6:
      return(applySnH(sine_val, enum_sine));      // Sample & Hold, values -1.f ... +1.f
    default:                                      // unexpected value
      return(sine_val);                           // leave unchanged!
  }
}

// --- Sample & Hold LFOs ---
float ctagSoundProcessorSubbotnik::applySnH(float sine_lfo_val, int enum_val)
{
  if(sine_lfo_val >= 0.f )   // positive half-wave after a previous negative one will trigger a new random value to be held
  {
    if(previous_sine_val[enum_val] < 0.f )      // Index relies on the member "enum snh_members", we use arrays for the elements needed instead of dynamically instanciating several objects
      saved_sample[enum_val] = oscSnH[enum_val].Process();  // values -1.f ... +1.f
  }
  previous_sine_val[enum_val] = sine_lfo_val;
  return(saved_sample[enum_val]);
}

// --- Process trigger signals and keep their state internally ---
inline int ctagSoundProcessorSubbotnik::process_param_trig(const ProcessData &data, int trig_myparm, int my_parm, int enum_trigger_id, int gate_type = 0 )
{
  int trig_status = 0;

  if(trig_myparm != -1)       // Trigger given via CV/Gate or button?
  {
    trig_status = (data.trig[trig_myparm]==0); // HIGH is 0, so we negate for boolean logic
    if( gate_type == 1 )
      return(trig_status);

    if(trig_status)    // Statuschange from HIGH to LOW or LOW to HIGH? Startup-Status for prev_trig_state is -1, so first change is always new
    {
      if( low_reached[enum_trigger_id] )    // We had a trigger low before the new trigger high
      {
        if (prev_trig_state[enum_trigger_id] == GATE_LOW || gate_type==2 )   // Toggle or AD EG Trigger...
        {
          prev_trig_state[enum_trigger_id] = GATE_HIGH;       // Remember status for next round
          low_reached[enum_trigger_id] = false;
          return (GATE_HIGH_NEW);           // New trigger
        }
        else        // previous status was high!
        {
          prev_trig_state[enum_trigger_id] = GATE_LOW;       // Remember status for next round
          low_reached[enum_trigger_id] = false;
          return (GATE_LOW);           // New trigger
        }
      }
    }
    else
      low_reached[enum_trigger_id] = true;
  }
  else                        // We may have a trigger set by activating the button via the GUI
  {
    if (my_parm != prev_trig_state[enum_trigger_id])    // Statuschange from HIGH to LOW or LOW to HIGH?
    {
      prev_trig_state[enum_trigger_id] = my_parm;       // Remember status
      if(my_parm != 0)                   // LOW if 0
        return (GATE_HIGH_NEW);          // New trigger
      else
        return (GATE_LOW);              // Trigger released
    }
  }
  return(prev_trig_state[enum_trigger_id]);            // No change (1 for active, 0 for inactive)
}


void ctagSoundProcessorSubbotnik::Process(const ProcessData &data)
{
  // === Process all parameters from the GUI ---
  // --- Global settings ---
  MK_ADEG_PAR(t_Gate, Gate);      // We may have a trigger if AD EG (be sure to check for ADEG first, because the gates have a common status variable)
  MK_GATE_PAR(g_Gate, Gate);      // And/Or a gate if ADSR EG
  MK_FLT_PAR_ABS(f_Volume, Volume, 4095.f, 4.f);   // We use a high value to have headroom, so lower the volume normally
  MK_FLT_PAR_ABS_PAN(f_xFadeOscAoscB, xFadeOscAoscB, 2047.f, 1.f);

  // --- Macro Oscillator A (MI Braids Clone) Parameter ---
  MK_TRIG_PAR(t_osc_active_A, osc_active_A);
  MK_INT_CONSTRAIN(i_shape_A, shape_A, 46);
  MK_FLT_PAR_ABS(f_vol_A, vol_A, 4095.f, 1.f);
  MK_BRAIDS_PITCH(f_pitch_A, pitch_A);
  MK_BRAIDS_TUNE(f_tune_A, tune_A);
  MK_INT_CONSTRAIN(i_timbre_A, timbre_A, 32767);
  MK_INT_CONSTRAIN(i_color_A, color_A, 32767);

  MK_TRIG_PAR(t_xModActive_A, xModActive_A);
  MK_FLT_PAR_ABS(f_am_xModB_A, am_xModB_A, 4095.f, 1.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_freqX_factor_A, freqX_factor_A, 4095.f, 1.f, 10.f);
  MK_FLT_PAR_ABS(f_freq_xModB_A, freq_xModB_A, 4095.f, 0.1f*f_freqX_factor_A);
  MK_FLT_PAR_ABS(f_timbre_xModB_A, timbre_xModB_A, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_color_xModB_A, color_xModB_A, 4095.f, 1.f);

  // --- Macro Oscillator B (MI Braids Clone) Parameter ---
  MK_TRIG_PAR(t_osc_active_B, osc_active_B);
  MK_INT_CONSTRAIN(i_shape_B, shape_B, 46);
  MK_FLT_PAR_ABS(f_vol_B, vol_B, 4095.f, 1.f);
  MK_BRAIDS_PITCH(f_pitch_B, pitch_B);
  MK_BRAIDS_TUNE(f_tune_B, tune_B);
  MK_INT_CONSTRAIN(i_timbre_B, timbre_B, 32767);
  MK_INT_CONSTRAIN(i_color_B, color_B, 32767);

  MK_TRIG_PAR(t_xModActive_B, xModActive_B);
  MK_FLT_PAR_ABS(f_am_xModA_B, am_xModA_B, 4095.f, 1.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_freqX_factor_B, freqX_factor_B, 4095.f, 1.f, 10.f);
  MK_FLT_PAR_ABS(f_freq_xModA_B, freq_xModA_B, 4095.f, 0.1f*f_freqX_factor_B);
  MK_FLT_PAR_ABS(f_timbre_xModA_B, timbre_xModA_B, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_color_xModA_B, color_xModA_B, 4095.f, 1.f);

  // --- Filter ---
  MK_TRIG_PAR(t_enableFilter, enableFilter);
  MK_TRIG_PAR(t_filterIsSVF, filterIsSVF);
  MK_FLT_PAR_ABS(f_WavefolderAmnt, WavefolderAmnt, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_Cutoff, Cutoff, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_Resonance, Resonance, 4095.f, 5.f);
  MK_FLT_PAR_ABS(f_ADfilterAmnt, ADfilterAmnt, 4095.f, 1.f);

  // --- AD envelope and loop ---
  MK_TRIG_PAR(t_enableADeg, enableADeg);
  MK_TRIG_PAR(t_ADegIsLowpassGate, ADegIsLowpassGate);
  MK_FLT_PAR_ABS(f_attackADeg, attackADeg, 4095.f, MAX_AD_ATTACK);
  MK_FLT_PAR_ABS(f_decayADeg, decayADeg, 4095.f, MAX_AD_DECAY);
  MK_TRIG_PAR(t_loopADeg, loopADeg);

  // --- ADSR envelope and loop ---
  MK_TRIG_PAR(t_enableVolADSReg, enableVolADSReg);
  MK_FLT_PAR_ABS(f_attackVolADSReg, attackVolADSReg, 4095.f, MAX_ADSR_ATTACK);
  MK_FLT_PAR_ABS(f_decayVolADSReg, decayVolADSReg, 4095.f, MAX_ADSR_DECAY);
  MK_FLT_PAR_ABS(f_sustainVolADSReg, sustainVolADSReg, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_releaseVolADSReg, releaseVolADSReg, 4095.f, MAX_ADSR_RELEASE);

  // --- LFOs ---
  MK_TRIG_PAR_LOC(t_lfoActive[5], lfoActive_1);
  MK_INT_CONSTRAIN_LOC(i_lfoDestination[5], lfoDestination_1, 10);
  MK_INT_CONSTRAIN_LOC(i_lfoType[5], lfoType_1, 6);
  MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[5], lfoSpeed_1, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
  MK_FLT_PAR_ABS_LOC(f_lfoAmnt[5], lfoAmnt_1, 4095.f, 1.f);

  MK_TRIG_PAR_LOC(t_lfoActive[6], lfoActive_2);
  MK_INT_CONSTRAIN_LOC(i_lfoDestination[6], lfoDestination_2, 10);
  MK_INT_CONSTRAIN_LOC(i_lfoType[6], lfoType_2, 6);
  MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[6], lfoSpeed_2, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
  MK_FLT_PAR_ABS_LOC(f_lfoAmnt[6], lfoAmnt_2, 4095.f, 1.f);

  MK_TRIG_PAR_LOC(t_lfoActive[7], lfoActive_3);
  MK_INT_CONSTRAIN_LOC(i_lfoDestination[7], lfoDestination_3, 10);
  MK_INT_CONSTRAIN_LOC(i_lfoType[7], lfoType_3, 6);
  MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[7], lfoSpeed_3, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
  MK_FLT_PAR_ABS_LOC(f_lfoAmnt[7], lfoAmnt_3, 4095.f, 1.f);

  MK_TRIG_PAR_LOC(t_lfoActive[8], lfoActive_4);
  MK_INT_CONSTRAIN_LOC(i_lfoDestination[8], lfoDestination_4, 10);
  MK_INT_CONSTRAIN_LOC(i_lfoType[8], lfoType_4, 6);
  MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[8], lfoSpeed_4, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
  MK_FLT_PAR_ABS_LOC(f_lfoAmnt[8], lfoAmnt_4, 4095.f, 1.f);

  MK_TRIG_PAR_LOC(t_lfoActive[9], lfoActive_5);
  MK_INT_CONSTRAIN_LOC(i_lfoDestination[9], lfoDestination_5, 10);
  MK_INT_CONSTRAIN_LOC(i_lfoType[9], lfoType_5, 6);
  MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[9], lfoSpeed_5, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
  MK_FLT_PAR_ABS_LOC(f_lfoAmnt[9], lfoAmnt_5, 4095.f, 1.f);

  // --- S&H Speed Modulators (LFOs with S&H-"waveform" internally) ---
  MK_TRIG_PAR_LOC(t_lfoActive[0], lfoppActive_1);
  MK_INT_CONSTRAIN_LOC(i_lfoDestination[0], lfoppDestination_1, 17);    // LFO-wavetype i_lfoType[x] is fixed to 6 == S&H and initialized already
  MK_INT_CONSTRAIN_LOC(i_lfoType[0], lfoppType_1, 6);
  MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[0], lfoppSpeed_1, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
  MK_FLT_PAR_ABS_LOC(f_lfoAmnt[0], lfoppAmnt_1, 4095.f, 1.f);

  MK_TRIG_PAR_LOC(t_lfoActive[1], lfoppActive_2);
  MK_INT_CONSTRAIN_LOC(i_lfoDestination[1], lfoppDestination_2, 17);    // LFO-wavetype i_lfoType[x] is fixed to 6 == S&H and initialized already
  MK_INT_CONSTRAIN_LOC(i_lfoType[1], lfoppType_2, 6);
  MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[1], lfoppSpeed_2, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
  MK_FLT_PAR_ABS_LOC(f_lfoAmnt[1], lfoppAmnt_2, 4095.f, 1.f);

  MK_TRIG_PAR_LOC(t_lfoActive[2], lfoppActive_3);
  MK_INT_CONSTRAIN_LOC(i_lfoDestination[2], lfoppDestination_3, 17);    // LFO-wavetype i_lfoType[x] is fixed to 6 == S&H and initialized already
  MK_INT_CONSTRAIN_LOC(i_lfoType[2], lfoppType_3, 6);
  MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[2], lfoppSpeed_3, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
  MK_FLT_PAR_ABS_LOC(f_lfoAmnt[2], lfoppAmnt_3, 4095.f, 1.f);

  MK_TRIG_PAR_LOC(t_lfoActive[3], lfoppActive_4);
  MK_INT_CONSTRAIN_LOC(i_lfoDestination[3], lfoppDestination_4, 17);    // LFO-wavetype i_lfoType[x] is fixed to 6 == S&H and initialized already
  MK_INT_CONSTRAIN_LOC(i_lfoType[3], lfoppType_4, 6);
  MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[3], lfoppSpeed_4, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
  MK_FLT_PAR_ABS_LOC(f_lfoAmnt[3], lfoppAmnt_4, 4095.f, 1.f);

  MK_TRIG_PAR_LOC(t_lfoActive[4], lfoppActive_5);
  MK_INT_CONSTRAIN_LOC(i_lfoDestination[4], lfoppDestination_5, 17);    // LFO-wavetype i_lfoType[x] is fixed to 6 == S&H and initialized already
  MK_INT_CONSTRAIN_LOC(i_lfoType[4], lfoppType_5, 6);
  MK_FLT_PAR_ABS_MIN_MAX_LOC(f_lfoSpeed[4], lfoppSpeed_5, 4095.f, MIN_LFO_SPEED, MAX_LFO_SPEED);
  MK_FLT_PAR_ABS_LOC(f_lfoAmnt[4], lfoppAmnt_5, 4095.f, 1.f);

  // === Preprocess all parameters before main DSP-loop[s] ---
  // --- Calculate final pitch as integer ---
  int i_pitch_A = (int)(f_pitch_A+f_tune_A);    // f_pitch has a range of 5 octaves, f_tune max +-1 octave
  CONSTRAIN(i_pitch_A,0, 16383);
  int i_pitch_B = (int)(f_pitch_B+f_tune_B);
  CONSTRAIN(i_pitch_B,0, 16383);

  // --- Check if we have to substitute the diode ladder filter with a SVF for performance-reasons ---
  if( (t_osc_active_A && t_osc_active_B) && (shape_is_critical(i_shape_A) || shape_is_critical(i_shape_B)) )
    t_filterIsSVF = true;

  // --- Calculate LFOs (we do this first, because it may change other values we will use as we go on) ---
  for( int i=0; i<NUM_OF_LFOS; i++)
  {
    if( t_lfoActive[i] )  // Please note: we process the S&H LFOs (IDs 0-3) first, because they can change speeds of LFOs 1-4 with IDs 4-7 as well!
    {
      lfo[i].SetFrequency(f_lfoSpeed[i]);
      f_lfo_val[i] = morph_sine_wave(lfo[i].Process(), i_lfoType[i], i)*f_lfoAmnt[i];      // (float sine_val, int morph_mode, int enum_sine)
      switch (i_lfoDestination[i])
      {
        case 0:
          f_xFadeOscAoscB += f_lfo_val[i];
          CONSTRAIN(f_xFadeOscAoscB, 0.f, 1.f);
          break;
        case 1:
          f_vol_A += f_lfo_val[i];
          CONSTRAIN(f_vol_A, 0.f, 1.f);
          break;
        case 2:
          i_pitch_A += DSP_FLOAT2INT_PITCH(f_lfo_val[i]);
          CONSTRAIN(i_pitch_A, 0, 16383);
          break;
        case 3:
          i_timbre_A += DSP_FLOAT2INT(f_lfo_val[i]);
          CONSTRAIN(i_timbre_A, 0, 32767);
          break;
        case 4:
          i_color_A += DSP_FLOAT2INT(f_lfo_val[i]);
          CONSTRAIN(i_color_A, 0, 32767);
          break;
        case 5:
          f_vol_B += f_lfo_val[i];
          CONSTRAIN(f_vol_B, 0.f, 1.f);
          break;
        case 6:
          i_pitch_B += DSP_FLOAT2INT_PITCH(f_lfo_val[i]);
          CONSTRAIN(i_pitch_B, 0, 16383);
          break;
        case 7:
          i_timbre_B += DSP_FLOAT2INT(f_lfo_val[i]);
          CONSTRAIN(i_timbre_B, 0, 32767);
          break;
        case 8:
          i_color_B += DSP_FLOAT2INT(f_lfo_val[i]);
          CONSTRAIN(i_color_B, 0, 32767);
          break;
        case 9:
          f_WavefolderAmnt += f_lfo_val[i];
          CONSTRAIN(f_WavefolderAmnt, 0.f, 1.f);
          break;
        case 10:
          f_Cutoff += f_lfo_val[i];
          CONSTRAIN(f_Cutoff, 0.f, 1.f);
          break;
        case 11:
          f_lfoSpeed[5] += f_lfo_val[i]*MAX_LFO_SPEED;         // LFO 1 has ID 4, because we process S&H speed-modifiers first, which are LFOs [0] to [3] internally
          CONSTRAIN(f_lfoSpeed[5], MIN_LFO_SPEED, MAX_LFO_SPEED);
          break;
        case 12:
          f_lfoSpeed[6] += f_lfo_val[i]*MAX_LFO_SPEED;         // LFO 1 has ID 4, because we process S&H speed-modifiers first, which are LFOs [0] to [3] internally
          CONSTRAIN(f_lfoSpeed[6], MIN_LFO_SPEED, MAX_LFO_SPEED);
          break;
        case 13:
          f_lfoSpeed[7] += f_lfo_val[i]*MAX_LFO_SPEED;         // LFO 1 has ID 4, because we process S&H speed-modifiers first, which are LFOs [0] to [3] internally
          CONSTRAIN(f_lfoSpeed[7], MIN_LFO_SPEED, MAX_LFO_SPEED);
          break;
        case 14:
          f_lfoSpeed[8] += f_lfo_val[i]*MAX_LFO_SPEED;         // LFO 1 has ID 4, because we process S&H speed-modifiers first, which are LFOs [0] to [3] internally
          CONSTRAIN(f_lfoSpeed[8], MIN_LFO_SPEED, MAX_LFO_SPEED);
          break;
        case 15:
          f_lfoSpeed[9] += f_lfo_val[i]*MAX_LFO_SPEED;         // LFO 1 has ID 4, because we process S&H speed-modifiers first, which are LFOs [0] to [3] internally
          CONSTRAIN(f_lfoSpeed[9], MIN_LFO_SPEED, MAX_LFO_SPEED);
          break;
        case 16:
          f_attackADeg += f_lfo_val[i]*MAX_AD_ATTACK;         // LFO 1 has ID 4, because we process S&H speed-modifiers first, which are LFOs [0] to [3] internally
          CONSTRAIN(f_attackADeg, MIN_AD_ATTACK, MAX_AD_ATTACK);
          break;
        case 17:
          f_decayADeg += f_lfo_val[i]*MAX_AD_DECAY;         // LFO 1 has ID 4, because we process S&H speed-modifiers first, which are LFOs [0] to [3] internally
          CONSTRAIN(f_decayADeg, MIN_AD_DECAY, MAX_AD_DECAY);
          break;
      }
    }
  }
  // --- EGs: Set AD-EG (drives volume or filter) ---
  ad_eg.SetAttack(f_attackADeg);
  ad_eg.SetDecay(f_decayADeg);

  // --- EGs: Set Volume ADSR-EG ---
  vol_eg_adsr.SetAttack(f_attackVolADSReg);
  vol_eg_adsr.SetDecay(f_decayVolADSReg);
  vol_eg_adsr.SetSustain(f_sustainVolADSReg);
  vol_eg_adsr.SetRelease(f_releaseVolADSReg);
  vol_eg_adsr.Gate(g_Gate);

  // --- EGs: Process the volume ADSR-EG ---
  float vol_adsr = 0.f;
  if( t_enableVolADSReg )
    vol_adsr = vol_eg_adsr.Process();

  // --- EGs: Check if we have to trigger the AD-EG or our macro-oscillator, especially important for mallet or plucked kind of sounds ---
  if(t_Gate == GATE_HIGH_NEW)   // We have two modes for "on trigger": reset playing of macro-osc to start y/n
  {
    osc_A.Strike();
    osc_B.Strike();
    ad_eg.Trigger();
  }
  // --- Process AD-EG ---
  if(t_enableADeg)
  {
    if(t_Gate && t_loopADeg && !ad_eg.GetIsRunning())  // Instead of using ad_eg.SetLoop() we trigger on request, so that we can [re]enable Strike() for the OSCs
    {
      osc_A.Strike();
      osc_B.Strike();
      ad_eg.Trigger();
    }
    if( t_ADegIsLowpassGate && t_enableFilter )        // AD-EG is set to filter instead of volume (if filter-EG is selected, but filter is off we ignore it!)
    {
      f_Cutoff += ad_eg.Process()*f_ADfilterAmnt;
      if( t_enableVolADSReg )
        f_Volume *= vol_adsr;             // Apply volume ADSR alone?
    }
    else    // AD-EG in volume mode
    {
      if( t_enableVolADSReg )   // We may have to combine it or substitute with the Volume ADSR
      {
        if(!vol_eg_adsr.IsIdle())
          f_Volume *= ad_eg.Process() + vol_adsr;    // the f_Volume variable acts as our VCA and will be applied to the DSP-result after the filter (before possible effects)
        else
          f_Volume = 0.f;
      }
      else    // Process volume AD-EG alone
        f_Volume *= ad_eg.Process();
    }
  }
  else  // AD-EG is inactive
  {
    if( t_enableVolADSReg ) // We may have to apply the volume ADSR alone
      f_Volume *= vol_adsr;
  }
  // --- Set Shape, Timbre, Colour and Pitch for Macro Oscillator A ---
  osc_A.set_shape((braids::MacroOscillatorShape)i_shape_A);
  osc_A.set_pitch(i_pitch_A);
  osc_A.set_parameters(i_timbre_A, i_color_A);

  // --- Set Shape, Timbre, Colour and Pitch for Macro Oscillator B ---
  osc_B.set_shape((braids::MacroOscillatorShape)i_shape_B);
  osc_B.set_pitch(i_pitch_B);
  osc_B.set_parameters(i_timbre_B, i_color_B);

  // --- Check if crossfades for Osc A are disabled ---
  if(!t_xModActive_A || !t_osc_active_B)    // Please note: we won't have crossmodulation, if Osc B is inactive
  {
    f_am_xModB_A = 0.f;
    f_freq_xModB_A = 0.f;
    f_timbre_xModB_A = 0.f;
    f_color_xModB_A = 0.f;
  }
  // --- Check if crossfades for Osc B are disabled ---
  if(!t_xModActive_B || !t_osc_active_A)    // Please note: we won't have crossmodulation, if Osc A is inactive
  {
    f_am_xModA_B = 0.f;
    f_freq_xModA_B = 0.f;
    f_timbre_xModA_B = 0.f;
    f_color_xModA_B = 0.f;
  }
  // --- Check if a Mac-Osc is disabled  ---
  if( !t_osc_active_A )
    f_vol_A = 0.f;  // We "fake" the disabling by setting the volume to 0, because it would be worse in terms of performance to check for activity in main loop
  if( !t_osc_active_B )
    f_vol_B = 0.f;
  // --- Apply crossfader to oscillator volumes ---
  f_vol_A *= (1.f-f_xFadeOscAoscB);
  f_vol_B *= f_xFadeOscAoscB;

  // === Main DSP Loops including DA output ===
  if( t_enableFilter ) // Please note: we have partly redundant code for DSP-loop with or without (different)filter[s], because it turned out to be much faster than have the distinction inside the main-loop!
  {
    if(t_filterIsSVF)     // Use SFV if selected or forced because of complex Oscillator shapes to gain performance?
    {
      for (int i = 0; i < bufSz; i++)
      {
        if ((i % 2) == 0)                      // We need an audio-buffer of at least two byte, so we render two bytes, give out one or only give out the second in exchange...
        {
          // --- Render OSC A and remember results ---
          if (osc_active_A)
          {
            // --- Apply values for Cross-modulation (if any) from OSC B to OSC A ---
            i_pitch_A += DSP_FLOAT2INT(f_val_result_1_B * f_freq_xModB_A);
            CONSTRAIN(i_pitch_A, 0, 16383);
            i_timbre_A += DSP_FLOAT2INT(f_val_result_1_B * f_timbre_xModB_A);
            CONSTRAIN(i_timbre_A, 0, 32767);
            i_color_A += DSP_FLOAT2INT(f_val_result_1_B * f_color_xModB_A);
            CONSTRAIN(i_color_A, 0, 32767);
            osc_A.set_pitch(i_pitch_A);
            osc_A.set_parameters(i_timbre_A, i_color_A);
            m_vol_A_incl_am = f_vol_A + f_val_result_1_B * f_am_xModB_A;

            osc_A.Render(sync_A, buffer_A, 2);
            f_val_result_1_A = DSP_INT2FLOAT(buffer_A[0]);
            f_val_result_2_A = DSP_INT2FLOAT(buffer_A[1]);
          }
          else
          {
            f_val_result_1_A = 0.f;
            f_val_result_2_A = 0.f;
          }
          // --- Render OSC A and remember results ---
          if (osc_active_B)
          {
            // --- Apply values for Cross-modulation (if any) from OSC A to OSC B ---
            i_pitch_B += DSP_FLOAT2INT(f_val_result_1_A * f_freq_xModA_B);
            CONSTRAIN(i_pitch_B, 0, 16383);
            i_timbre_B += DSP_FLOAT2INT(f_val_result_1_A * f_timbre_xModA_B);
            CONSTRAIN(i_timbre_B, 0, 32767);
            i_color_B += DSP_FLOAT2INT(f_val_result_1_A * f_color_xModA_B);
            CONSTRAIN(i_color_B, 0, 32767);
            osc_B.set_pitch(i_pitch_B);
            osc_B.set_parameters(i_timbre_B, i_color_B);
            m_vol_B_incl_am = f_vol_B + f_val_result_1_A * f_am_xModA_B;

            osc_B.Render(sync_B, buffer_B, 2);
            f_val_result_1_B = DSP_INT2FLOAT(buffer_B[0]);
            f_val_result_2_B = DSP_INT2FLOAT(buffer_B[1]);
          }
          else
          {
            f_val_result_1_B = 0.f;
            f_val_result_2_B = 0.f;
          }
          f_val_result = Svf_process(svf_data,Fold_do(f_val_result_1_A * m_vol_A_incl_am + f_val_result_1_B * m_vol_B_incl_am, f_WavefolderAmnt), f_Cutoff, f_Resonance, 0) * f_Volume;
          data.buf[i*2] = data.buf[i*2+1] = f_val_result;     // give out first result of renedering on both channels in even-indexed DSP-loop
        }
        else
        {
          f_val_result = Svf_process(svf_data,Fold_do(f_val_result_2_A * m_vol_A_incl_am + f_val_result_2_B * m_vol_B_incl_am, f_WavefolderAmnt), f_Cutoff, f_Resonance, 0) * f_Volume;
          data.buf[i*2] = data.buf[i*2+1] = f_val_result;    // give out second result of renedering on both channels in odd-indexed DSP-loop
        }
      }
    }
    else    // Use the standard diode ladder filter, performance should be sufficiant in combination with the Oscillator shapes encountered!
    {
      for (int i = 0; i < bufSz; i++)
      {
        if ((i % 2) == 0)                      // We need an audio-buffer of at least two byte, so we render two bytes, give out one or only give out the second in exchange...
        {
          // --- Render OSC A and remember results ---
          if (osc_active_A)
          {
            // --- Apply values for Cross-modulation (if any) from OSC B to OSC A ---
            i_pitch_A += DSP_FLOAT2INT(f_val_result_1_B * f_freq_xModB_A);
            CONSTRAIN(i_pitch_A, 0, 16383);
            i_timbre_A += DSP_FLOAT2INT(f_val_result_1_B * f_timbre_xModB_A);
            CONSTRAIN(i_timbre_A, 0, 32767);
            i_color_A += DSP_FLOAT2INT(f_val_result_1_B * f_color_xModB_A);
            CONSTRAIN(i_color_A, 0, 32767);
            osc_A.set_pitch(i_pitch_A);
            osc_A.set_parameters(i_timbre_A, i_color_A);
            m_vol_A_incl_am = f_vol_A + f_val_result_1_B * f_am_xModB_A;

            osc_A.Render(sync_A, buffer_A, 2);
            f_val_result_1_A = DSP_INT2FLOAT(buffer_A[0]);
            f_val_result_2_A = DSP_INT2FLOAT(buffer_A[1]);
          }
          else
          {
            f_val_result_1_A = 0.f;
            f_val_result_2_A = 0.f;
          }
          // --- Render OSC A and remember results ---
          if (osc_active_B)
          {
            // --- Apply values for Cross-modulation (if any) from OSC A to OSC B ---
            i_pitch_B += DSP_FLOAT2INT(f_val_result_1_A * f_freq_xModA_B);
            CONSTRAIN(i_pitch_B, 0, 16383);
            i_timbre_B += DSP_FLOAT2INT(f_val_result_1_A * f_timbre_xModA_B);
            CONSTRAIN(i_timbre_B, 0, 32767);
            i_color_B += DSP_FLOAT2INT(f_val_result_1_A * f_color_xModA_B);
            CONSTRAIN(i_color_B, 0, 32767);
            osc_B.set_pitch(i_pitch_B);
            osc_B.set_parameters(i_timbre_B, i_color_B);
            m_vol_B_incl_am = f_vol_B + f_val_result_1_A * f_am_xModA_B;

            osc_B.Render(sync_B, buffer_B, 2);
            f_val_result_1_B = DSP_INT2FLOAT(buffer_B[0]);
            f_val_result_2_B = DSP_INT2FLOAT(buffer_B[1]);
          }
          else
          {
            f_val_result_1_B = 0.f;
            f_val_result_2_B = 0.f;
          }
          f_val_result = Ladder_process(ladder_data, Fold_do(f_val_result_1_A*m_vol_A_incl_am + f_val_result_1_B*m_vol_B_incl_am, f_WavefolderAmnt), f_Cutoff, f_Resonance)*f_Volume;
          data.buf[i*2] = data.buf[i*2+1] = f_val_result;     // give out first result of renedering on both channels in even-indexed DSP-loop
        }
        else
        {
          f_val_result = Ladder_process(ladder_data, Fold_do(f_val_result_2_A*m_vol_A_incl_am + f_val_result_2_B*m_vol_B_incl_am, f_WavefolderAmnt), f_Cutoff, f_Resonance)*f_Volume;
          data.buf[i*2] = data.buf[i*2+1] = f_val_result;    // give out second result of renedering on both channels in odd-indexed DSP-loop
        }
      }

    }
  }
  else    // Filter is disabled
  {
    for (int i = 0; i < bufSz; i++)
    {
      if ((i % 2) ==  0)                      // We need an audio-buffer of at least two byte, so we render two bytes, give out one or only give out the second in exchange...
      {
        // --- Apply values for Cross-modulation (if any) from OSC B to OSC A ---
        i_pitch_A += DSP_FLOAT2INT(f_val_result_1_B * f_freq_xModB_A);
        CONSTRAIN(i_pitch_A, 0, 16383);
        i_timbre_A += DSP_FLOAT2INT(f_val_result_1_B * f_timbre_xModB_A);
        CONSTRAIN(i_timbre_A, 0, 32767);
        i_color_A += DSP_FLOAT2INT(f_val_result_1_B * f_color_xModB_A);
        CONSTRAIN(i_color_A, 0, 32767);
        osc_A.set_pitch(i_pitch_A);
        osc_A.set_parameters(i_timbre_A, i_color_A);
        m_vol_A_incl_am = f_vol_A + f_val_result_1_B*f_am_xModB_A;

        // --- Render OSC A and remember results ---
        osc_A.Render(sync_A, buffer_A, 2);
        f_val_result_1_A = DSP_INT2FLOAT(buffer_A[0]);
        f_val_result_2_A = DSP_INT2FLOAT(buffer_A[1]);

        // --- Apply values for Cross-modulation (if any) from OSC A to OSC B ---
        i_pitch_B += DSP_FLOAT2INT(f_val_result_1_A * f_freq_xModA_B);
        CONSTRAIN(i_pitch_B, 0, 16383);
        i_timbre_B += DSP_FLOAT2INT(f_val_result_1_A * f_timbre_xModA_B);
        CONSTRAIN(i_timbre_B, 0, 32767);
        i_color_B += DSP_FLOAT2INT(f_val_result_1_A * f_color_xModA_B);
        CONSTRAIN(i_color_B, 0, 32767);
        osc_B.set_pitch(i_pitch_B);
        osc_B.set_parameters(i_timbre_B, i_color_B);
        m_vol_B_incl_am = f_vol_B + f_val_result_1_A*f_am_xModA_B;

        // --- Render OSC A and remember results ---
        osc_B.Render(sync_B, buffer_B, 2);
        f_val_result_1_B = DSP_INT2FLOAT(buffer_B[0]);
        f_val_result_2_B = DSP_INT2FLOAT(buffer_B[1]);

        f_val_result = (f_val_result_1_A*m_vol_A_incl_am + f_val_result_1_B*m_vol_B_incl_am)*f_Volume;
        data.buf[i*2] = data.buf[i*2+1] = f_val_result;    // give out first result of renedering on both channels in even-indexed DSP-loop
      }
      else
      {
        f_val_result = (f_val_result_2_A*m_vol_A_incl_am + f_val_result_2_B*m_vol_B_incl_am)*f_Volume;
        data.buf[i*2] = data.buf[i*2+1] = f_val_result;    // give out second result of renedering on both channels in odd-indexed DSP-loop
      }
    }
  }
}

void ctagSoundProcessorSubbotnik::Init(std::size_t blockSize, void *blockPtr)
{
  // construct internal data model
  knowYourself();
  model = std::make_unique<ctagSPDataModel>(id, isStereo);
  LoadPreset(0);

  // --- Initalize Braids Macro Oscillators ---
  osc_A.Init();
  osc_A.set_pitch(100);
  osc_A.set_shape(braids::MacroOscillatorShape::MACRO_OSC_SHAPE_CSAW);

  osc_B.Init();
  osc_B.set_pitch(100);
  osc_B.set_shape(braids::MacroOscillatorShape::MACRO_OSC_SHAPE_CSAW);

  // --- Init EGs ---
  ad_eg.SetSampleRate(44100.f / bufSz);
  ad_eg.SetModeExp();

  vol_eg_adsr.SetSampleRate(44100.f / bufSz);
  vol_eg_adsr.SetModeExp();

  // --- Init LFOs ---
  for(int i=0; i<NUM_OF_LFOS; i++)
  {
    lfo[i].SetSampleRate(44100.f / bufSz);   // Please note: because the LFO is applied already outside the DSP-loop we reduce it's frequency in a manner to fit
    lfo[i].SetFrequency(1.f);
  }
  // --- Initialize VULT stuff ---
  Ladder_process_init(ladder_data);     // Diode Ladder Filter with Heun based resonance frequency smoothing
  Svf__ctx_type_4_init(svf_data);
}

ctagSoundProcessorSubbotnik::~ctagSoundProcessorSubbotnik()
{
}

void ctagSoundProcessorSubbotnik::knowYourself()
{
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("Gate", [&](const int val){ Gate = val;});
	pMapTrig.emplace("Gate", [&](const int val){ trig_Gate = val;});
	pMapPar.emplace("Volume", [&](const int val){ Volume = val;});
	pMapCv.emplace("Volume", [&](const int val){ cv_Volume = val;});
	pMapPar.emplace("xFadeOscAoscB", [&](const int val){ xFadeOscAoscB = val;});
	pMapCv.emplace("xFadeOscAoscB", [&](const int val){ cv_xFadeOscAoscB = val;});
	pMapPar.emplace("osc_active_A", [&](const int val){ osc_active_A = val;});
	pMapTrig.emplace("osc_active_A", [&](const int val){ trig_osc_active_A = val;});
	pMapPar.emplace("shape_A", [&](const int val){ shape_A = val;});
	pMapCv.emplace("shape_A", [&](const int val){ cv_shape_A = val;});
	pMapPar.emplace("vol_A", [&](const int val){ vol_A = val;});
	pMapCv.emplace("vol_A", [&](const int val){ cv_vol_A = val;});
	pMapPar.emplace("pitch_A", [&](const int val){ pitch_A = val;});
	pMapCv.emplace("pitch_A", [&](const int val){ cv_pitch_A = val;});
	pMapPar.emplace("tune_A", [&](const int val){ tune_A = val;});
	pMapCv.emplace("tune_A", [&](const int val){ cv_tune_A = val;});
	pMapPar.emplace("timbre_A", [&](const int val){ timbre_A = val;});
	pMapCv.emplace("timbre_A", [&](const int val){ cv_timbre_A = val;});
	pMapPar.emplace("color_A", [&](const int val){ color_A = val;});
	pMapCv.emplace("color_A", [&](const int val){ cv_color_A = val;});
	pMapPar.emplace("xModActive_A", [&](const int val){ xModActive_A = val;});
	pMapTrig.emplace("xModActive_A", [&](const int val){ trig_xModActive_A = val;});
	pMapPar.emplace("am_xModB_A", [&](const int val){ am_xModB_A = val;});
	pMapCv.emplace("am_xModB_A", [&](const int val){ cv_am_xModB_A = val;});
	pMapPar.emplace("freqX_factor_A", [&](const int val){ freqX_factor_A = val;});
	pMapCv.emplace("freqX_factor_A", [&](const int val){ cv_freqX_factor_A = val;});
	pMapPar.emplace("freq_xModB_A", [&](const int val){ freq_xModB_A = val;});
	pMapCv.emplace("freq_xModB_A", [&](const int val){ cv_freq_xModB_A = val;});
	pMapPar.emplace("timbre_xModB_A", [&](const int val){ timbre_xModB_A = val;});
	pMapCv.emplace("timbre_xModB_A", [&](const int val){ cv_timbre_xModB_A = val;});
	pMapPar.emplace("color_xModB_A", [&](const int val){ color_xModB_A = val;});
	pMapCv.emplace("color_xModB_A", [&](const int val){ cv_color_xModB_A = val;});
	pMapPar.emplace("osc_active_B", [&](const int val){ osc_active_B = val;});
	pMapTrig.emplace("osc_active_B", [&](const int val){ trig_osc_active_B = val;});
	pMapPar.emplace("shape_B", [&](const int val){ shape_B = val;});
	pMapCv.emplace("shape_B", [&](const int val){ cv_shape_B = val;});
	pMapPar.emplace("vol_B", [&](const int val){ vol_B = val;});
	pMapCv.emplace("vol_B", [&](const int val){ cv_vol_B = val;});
	pMapPar.emplace("pitch_B", [&](const int val){ pitch_B = val;});
	pMapCv.emplace("pitch_B", [&](const int val){ cv_pitch_B = val;});
	pMapPar.emplace("tune_B", [&](const int val){ tune_B = val;});
	pMapCv.emplace("tune_B", [&](const int val){ cv_tune_B = val;});
	pMapPar.emplace("timbre_B", [&](const int val){ timbre_B = val;});
	pMapCv.emplace("timbre_B", [&](const int val){ cv_timbre_B = val;});
	pMapPar.emplace("color_B", [&](const int val){ color_B = val;});
	pMapCv.emplace("color_B", [&](const int val){ cv_color_B = val;});
	pMapPar.emplace("xModActive_B", [&](const int val){ xModActive_B = val;});
	pMapTrig.emplace("xModActive_B", [&](const int val){ trig_xModActive_B = val;});
	pMapPar.emplace("am_xModA_B", [&](const int val){ am_xModA_B = val;});
	pMapCv.emplace("am_xModA_B", [&](const int val){ cv_am_xModA_B = val;});
	pMapPar.emplace("freqX_factor_B", [&](const int val){ freqX_factor_B = val;});
	pMapCv.emplace("freqX_factor_B", [&](const int val){ cv_freqX_factor_B = val;});
	pMapPar.emplace("freq_xModA_B", [&](const int val){ freq_xModA_B = val;});
	pMapCv.emplace("freq_xModA_B", [&](const int val){ cv_freq_xModA_B = val;});
	pMapPar.emplace("timbre_xModA_B", [&](const int val){ timbre_xModA_B = val;});
	pMapCv.emplace("timbre_xModA_B", [&](const int val){ cv_timbre_xModA_B = val;});
	pMapPar.emplace("color_xModA_B", [&](const int val){ color_xModA_B = val;});
	pMapCv.emplace("color_xModA_B", [&](const int val){ cv_color_xModA_B = val;});
	pMapPar.emplace("enableFilter", [&](const int val){ enableFilter = val;});
	pMapTrig.emplace("enableFilter", [&](const int val){ trig_enableFilter = val;});
	pMapPar.emplace("filterIsSVF", [&](const int val){ filterIsSVF = val;});
	pMapTrig.emplace("filterIsSVF", [&](const int val){ trig_filterIsSVF = val;});
	pMapPar.emplace("WavefolderAmnt", [&](const int val){ WavefolderAmnt = val;});
	pMapCv.emplace("WavefolderAmnt", [&](const int val){ cv_WavefolderAmnt = val;});
	pMapPar.emplace("Cutoff", [&](const int val){ Cutoff = val;});
	pMapCv.emplace("Cutoff", [&](const int val){ cv_Cutoff = val;});
	pMapPar.emplace("Resonance", [&](const int val){ Resonance = val;});
	pMapCv.emplace("Resonance", [&](const int val){ cv_Resonance = val;});
	pMapPar.emplace("ADfilterAmnt", [&](const int val){ ADfilterAmnt = val;});
	pMapCv.emplace("ADfilterAmnt", [&](const int val){ cv_ADfilterAmnt = val;});
	pMapPar.emplace("enableADeg", [&](const int val){ enableADeg = val;});
	pMapTrig.emplace("enableADeg", [&](const int val){ trig_enableADeg = val;});
	pMapPar.emplace("ADegIsLowpassGate", [&](const int val){ ADegIsLowpassGate = val;});
	pMapTrig.emplace("ADegIsLowpassGate", [&](const int val){ trig_ADegIsLowpassGate = val;});
	pMapPar.emplace("attackADeg", [&](const int val){ attackADeg = val;});
	pMapCv.emplace("attackADeg", [&](const int val){ cv_attackADeg = val;});
	pMapPar.emplace("decayADeg", [&](const int val){ decayADeg = val;});
	pMapCv.emplace("decayADeg", [&](const int val){ cv_decayADeg = val;});
	pMapPar.emplace("loopADeg", [&](const int val){ loopADeg = val;});
	pMapTrig.emplace("loopADeg", [&](const int val){ trig_loopADeg = val;});
	pMapPar.emplace("enableVolADSReg", [&](const int val){ enableVolADSReg = val;});
	pMapTrig.emplace("enableVolADSReg", [&](const int val){ trig_enableVolADSReg = val;});
	pMapPar.emplace("attackVolADSReg", [&](const int val){ attackVolADSReg = val;});
	pMapCv.emplace("attackVolADSReg", [&](const int val){ cv_attackVolADSReg = val;});
	pMapPar.emplace("decayVolADSReg", [&](const int val){ decayVolADSReg = val;});
	pMapCv.emplace("decayVolADSReg", [&](const int val){ cv_decayVolADSReg = val;});
	pMapPar.emplace("sustainVolADSReg", [&](const int val){ sustainVolADSReg = val;});
	pMapCv.emplace("sustainVolADSReg", [&](const int val){ cv_sustainVolADSReg = val;});
	pMapPar.emplace("releaseVolADSReg", [&](const int val){ releaseVolADSReg = val;});
	pMapCv.emplace("releaseVolADSReg", [&](const int val){ cv_releaseVolADSReg = val;});
	pMapPar.emplace("lfoActive_1", [&](const int val){ lfoActive_1 = val;});
	pMapTrig.emplace("lfoActive_1", [&](const int val){ trig_lfoActive_1 = val;});
	pMapPar.emplace("lfoDestination_1", [&](const int val){ lfoDestination_1 = val;});
	pMapCv.emplace("lfoDestination_1", [&](const int val){ cv_lfoDestination_1 = val;});
	pMapPar.emplace("lfoType_1", [&](const int val){ lfoType_1 = val;});
	pMapCv.emplace("lfoType_1", [&](const int val){ cv_lfoType_1 = val;});
	pMapPar.emplace("lfoSpeed_1", [&](const int val){ lfoSpeed_1 = val;});
	pMapCv.emplace("lfoSpeed_1", [&](const int val){ cv_lfoSpeed_1 = val;});
	pMapPar.emplace("lfoAmnt_1", [&](const int val){ lfoAmnt_1 = val;});
	pMapCv.emplace("lfoAmnt_1", [&](const int val){ cv_lfoAmnt_1 = val;});
	pMapPar.emplace("lfoActive_2", [&](const int val){ lfoActive_2 = val;});
	pMapTrig.emplace("lfoActive_2", [&](const int val){ trig_lfoActive_2 = val;});
	pMapPar.emplace("lfoDestination_2", [&](const int val){ lfoDestination_2 = val;});
	pMapCv.emplace("lfoDestination_2", [&](const int val){ cv_lfoDestination_2 = val;});
	pMapPar.emplace("lfoType_2", [&](const int val){ lfoType_2 = val;});
	pMapCv.emplace("lfoType_2", [&](const int val){ cv_lfoType_2 = val;});
	pMapPar.emplace("lfoSpeed_2", [&](const int val){ lfoSpeed_2 = val;});
	pMapCv.emplace("lfoSpeed_2", [&](const int val){ cv_lfoSpeed_2 = val;});
	pMapPar.emplace("lfoAmnt_2", [&](const int val){ lfoAmnt_2 = val;});
	pMapCv.emplace("lfoAmnt_2", [&](const int val){ cv_lfoAmnt_2 = val;});
	pMapPar.emplace("lfoActive_3", [&](const int val){ lfoActive_3 = val;});
	pMapTrig.emplace("lfoActive_3", [&](const int val){ trig_lfoActive_3 = val;});
	pMapPar.emplace("lfoDestination_3", [&](const int val){ lfoDestination_3 = val;});
	pMapCv.emplace("lfoDestination_3", [&](const int val){ cv_lfoDestination_3 = val;});
	pMapPar.emplace("lfoType_3", [&](const int val){ lfoType_3 = val;});
	pMapCv.emplace("lfoType_3", [&](const int val){ cv_lfoType_3 = val;});
	pMapPar.emplace("lfoSpeed_3", [&](const int val){ lfoSpeed_3 = val;});
	pMapCv.emplace("lfoSpeed_3", [&](const int val){ cv_lfoSpeed_3 = val;});
	pMapPar.emplace("lfoAmnt_3", [&](const int val){ lfoAmnt_3 = val;});
	pMapCv.emplace("lfoAmnt_3", [&](const int val){ cv_lfoAmnt_3 = val;});
	pMapPar.emplace("lfoActive_4", [&](const int val){ lfoActive_4 = val;});
	pMapTrig.emplace("lfoActive_4", [&](const int val){ trig_lfoActive_4 = val;});
	pMapPar.emplace("lfoDestination_4", [&](const int val){ lfoDestination_4 = val;});
	pMapCv.emplace("lfoDestination_4", [&](const int val){ cv_lfoDestination_4 = val;});
	pMapPar.emplace("lfoType_4", [&](const int val){ lfoType_4 = val;});
	pMapCv.emplace("lfoType_4", [&](const int val){ cv_lfoType_4 = val;});
	pMapPar.emplace("lfoSpeed_4", [&](const int val){ lfoSpeed_4 = val;});
	pMapCv.emplace("lfoSpeed_4", [&](const int val){ cv_lfoSpeed_4 = val;});
	pMapPar.emplace("lfoAmnt_4", [&](const int val){ lfoAmnt_4 = val;});
	pMapCv.emplace("lfoAmnt_4", [&](const int val){ cv_lfoAmnt_4 = val;});
	pMapPar.emplace("lfoActive_5", [&](const int val){ lfoActive_5 = val;});
	pMapTrig.emplace("lfoActive_5", [&](const int val){ trig_lfoActive_5 = val;});
	pMapPar.emplace("lfoDestination_5", [&](const int val){ lfoDestination_5 = val;});
	pMapCv.emplace("lfoDestination_5", [&](const int val){ cv_lfoDestination_5 = val;});
	pMapPar.emplace("lfoType_5", [&](const int val){ lfoType_5 = val;});
	pMapCv.emplace("lfoType_5", [&](const int val){ cv_lfoType_5 = val;});
	pMapPar.emplace("lfoSpeed_5", [&](const int val){ lfoSpeed_5 = val;});
	pMapCv.emplace("lfoSpeed_5", [&](const int val){ cv_lfoSpeed_5 = val;});
	pMapPar.emplace("lfoAmnt_5", [&](const int val){ lfoAmnt_5 = val;});
	pMapCv.emplace("lfoAmnt_5", [&](const int val){ cv_lfoAmnt_5 = val;});
	pMapPar.emplace("lfoppActive_1", [&](const int val){ lfoppActive_1 = val;});
	pMapTrig.emplace("lfoppActive_1", [&](const int val){ trig_lfoppActive_1 = val;});
	pMapPar.emplace("lfoppDestination_1", [&](const int val){ lfoppDestination_1 = val;});
	pMapCv.emplace("lfoppDestination_1", [&](const int val){ cv_lfoppDestination_1 = val;});
	pMapPar.emplace("lfoppType_1", [&](const int val){ lfoppType_1 = val;});
	pMapCv.emplace("lfoppType_1", [&](const int val){ cv_lfoppType_1 = val;});
	pMapPar.emplace("lfoppSpeed_1", [&](const int val){ lfoppSpeed_1 = val;});
	pMapCv.emplace("lfoppSpeed_1", [&](const int val){ cv_lfoppSpeed_1 = val;});
	pMapPar.emplace("lfoppAmnt_1", [&](const int val){ lfoppAmnt_1 = val;});
	pMapCv.emplace("lfoppAmnt_1", [&](const int val){ cv_lfoppAmnt_1 = val;});
	pMapPar.emplace("lfoppActive_2", [&](const int val){ lfoppActive_2 = val;});
	pMapTrig.emplace("lfoppActive_2", [&](const int val){ trig_lfoppActive_2 = val;});
	pMapPar.emplace("lfoppDestination_2", [&](const int val){ lfoppDestination_2 = val;});
	pMapCv.emplace("lfoppDestination_2", [&](const int val){ cv_lfoppDestination_2 = val;});
	pMapPar.emplace("lfoppType_2", [&](const int val){ lfoppType_2 = val;});
	pMapCv.emplace("lfoppType_2", [&](const int val){ cv_lfoppType_2 = val;});
	pMapPar.emplace("lfoppSpeed_2", [&](const int val){ lfoppSpeed_2 = val;});
	pMapCv.emplace("lfoppSpeed_2", [&](const int val){ cv_lfoppSpeed_2 = val;});
	pMapPar.emplace("lfoppAmnt_2", [&](const int val){ lfoppAmnt_2 = val;});
	pMapCv.emplace("lfoppAmnt_2", [&](const int val){ cv_lfoppAmnt_2 = val;});
	pMapPar.emplace("lfoppActive_3", [&](const int val){ lfoppActive_3 = val;});
	pMapTrig.emplace("lfoppActive_3", [&](const int val){ trig_lfoppActive_3 = val;});
	pMapPar.emplace("lfoppDestination_3", [&](const int val){ lfoppDestination_3 = val;});
	pMapCv.emplace("lfoppDestination_3", [&](const int val){ cv_lfoppDestination_3 = val;});
	pMapPar.emplace("lfoppType_3", [&](const int val){ lfoppType_3 = val;});
	pMapCv.emplace("lfoppType_3", [&](const int val){ cv_lfoppType_3 = val;});
	pMapPar.emplace("lfoppSpeed_3", [&](const int val){ lfoppSpeed_3 = val;});
	pMapCv.emplace("lfoppSpeed_3", [&](const int val){ cv_lfoppSpeed_3 = val;});
	pMapPar.emplace("lfoppAmnt_3", [&](const int val){ lfoppAmnt_3 = val;});
	pMapCv.emplace("lfoppAmnt_3", [&](const int val){ cv_lfoppAmnt_3 = val;});
	pMapPar.emplace("lfoppActive_4", [&](const int val){ lfoppActive_4 = val;});
	pMapTrig.emplace("lfoppActive_4", [&](const int val){ trig_lfoppActive_4 = val;});
	pMapPar.emplace("lfoppDestination_4", [&](const int val){ lfoppDestination_4 = val;});
	pMapCv.emplace("lfoppDestination_4", [&](const int val){ cv_lfoppDestination_4 = val;});
	pMapPar.emplace("lfoppType_4", [&](const int val){ lfoppType_4 = val;});
	pMapCv.emplace("lfoppType_4", [&](const int val){ cv_lfoppType_4 = val;});
	pMapPar.emplace("lfoppSpeed_4", [&](const int val){ lfoppSpeed_4 = val;});
	pMapCv.emplace("lfoppSpeed_4", [&](const int val){ cv_lfoppSpeed_4 = val;});
	pMapPar.emplace("lfoppAmnt_4", [&](const int val){ lfoppAmnt_4 = val;});
	pMapCv.emplace("lfoppAmnt_4", [&](const int val){ cv_lfoppAmnt_4 = val;});
	pMapPar.emplace("lfoppActive_5", [&](const int val){ lfoppActive_5 = val;});
	pMapTrig.emplace("lfoppActive_5", [&](const int val){ trig_lfoppActive_5 = val;});
	pMapPar.emplace("lfoppDestination_5", [&](const int val){ lfoppDestination_5 = val;});
	pMapCv.emplace("lfoppDestination_5", [&](const int val){ cv_lfoppDestination_5 = val;});
	pMapPar.emplace("lfoppType_5", [&](const int val){ lfoppType_5 = val;});
	pMapCv.emplace("lfoppType_5", [&](const int val){ cv_lfoppType_5 = val;});
	pMapPar.emplace("lfoppSpeed_5", [&](const int val){ lfoppSpeed_5 = val;});
	pMapCv.emplace("lfoppSpeed_5", [&](const int val){ cv_lfoppSpeed_5 = val;});
	pMapPar.emplace("lfoppAmnt_5", [&](const int val){ lfoppAmnt_5 = val;});
	pMapCv.emplace("lfoppAmnt_5", [&](const int val){ cv_lfoppAmnt_5 = val;});
	isStereo = true;
	id = "Subbotnik";
	// sectionCpp0
}