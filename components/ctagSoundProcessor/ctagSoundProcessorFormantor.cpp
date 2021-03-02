/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020/2021 by Robert Manzke. All rights reserved.

(c) 2021 for the "Formantor"-Plugin by Mathias Brüssel
The idea of Formantor is to to have a simple yet fun to control synthesizer combining a Phase Distortion oscillator, some additional osciallators,
a vowel-filter, a resonator, a tremolo and an envelope.
There are different options for playing and controlling the formant filters, for more details please look here:
https://docs.google.com/document/d/1c8mjxWjdiJNP0xpkU2CxRUp9av6V4W39wARJf3_SMSo
Formantor uses filters and a Phase Distortion synth by Carlos Laguna Ruiz implemented in his VULT language, the synth can be found here:
https://github.com/modlfo/teensy-vult-example
The code originally was intended as an add-on to the Teensy Audio library and got modified to be used with the TBD along with other filters from VULT-examples
by using the VULT compiler. For more details on the topic please look here: https://github.com/modlfo/vult
For the formant-filter Open Source code by alex@smartelectronix.com got used, as found here: https://www.musicdsp.org/en/latest/Filters/110-formant-filter.html

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "ctagSoundProcessorFormantor.hpp"
#include "helpers/ctagNumUtil.hpp"

#include "plaits/dsp/engine/engine.h"

using namespace CTAG::SP;

#define GATE_HIGH_NEW       2
#define GATE_HIGH           1
#define GATE_LOW            0

// --- Replace function-call of frequency-conversion with macro for increasing speed just a bit ---
#define noteToFreq(incoming_note) (HELPERS::fastpow2 ((incoming_note - 69.f) / 12.f) *440.f)

// --- Rescale random values from 0...1 to needed output ---
#define RESCALE_FLT_MIN_MAX(inname, out_min, out_max) (inname * (out_max-out_min)+out_min)

// --- Additional Macro for automated parameter evaluations ---
#define MK_PITCH_PAR(outname, inname)     float outname = inname; if(cv_##inname != -1) outname += data.cv[cv_##inname]*60.f;

#define MK_TRIG_PAR(outname, inname) int outname = process_param_trig(data, trig_##inname, inname, e_##inname);
#define MK_GATE_PAR(outname, inname) bool outname = (bool)process_param_trig(data, trig_##inname, inname, e_##inname, 1);
#define MK_ADEG_PAR(outname, inname) int outname = process_param_trig(data, trig_##inname, inname, e_##inname, 2);

// --- Modify sine-wave for Squarewave/PWM or various modulations (including Pitch-Mod, Filter-Mod, Z-Scan and Vector-Modulation) ---
#define SINE_TO_SQUARE(sine_val)                      sine_val = (sine_val >= 0) ? 1.f : -1.f;
#define X_FADE(fader, val_left, val_right)            ((1.f-fader)*val_left + fader*val_right)

// --- VULT "Library for TBD" ---
#include "../vult/vultin.cpp"
#include "./vult/vult_formantor.cpp"

// --- Process trigger signals and keep their state internally ---
inline int ctagSoundProcessorFormantor::process_param_trig(const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id, int gate_type = 0 )
{
 int trig_status = 0;
  
  if(trig_myparm != -1)       // Trigger given via CV/Gate or button?
  {
    trig_status = !data.trig[trig_myparm]; // HIGH is 0, so we negate for boolean logic
    if( gate_type == 1 )
      return(trig_status);

    if(trig_status)    // Statuschange from HIGH to LOW or LOW to HIGH? Startup-Status for prev_trig_state is -1, so first change is always new
    {
      if( low_reached[trig_myparm] )    // We had a trigger low before the new trigger high
      {
        if (prev_trig_state[prev_trig_state_id] == GATE_LOW || gate_type==2 )   // Toggle or AD EG Trigger...
        {
          prev_trig_state[prev_trig_state_id] = GATE_HIGH;       // Remember status for next round
          low_reached[trig_myparm] = false;
          return (GATE_HIGH_NEW);           // New trigger
        }
        else        // previous status was high!
        {
          prev_trig_state[prev_trig_state_id] = GATE_LOW;       // Remember status for next round
          low_reached[trig_myparm] = false;
          return (GATE_LOW);           // New trigger
        }
      }
    }
    else
      low_reached[trig_myparm] = true;
  }
  else                        // We may have a trigger set by activating the button via the GUI
  {
    if (my_parm != prev_trig_state[prev_trig_state_id])    // Statuschange from HIGH to LOW or LOW to HIGH?
    {
      prev_trig_state[prev_trig_state_id] = my_parm;       // Remember status
      if(my_parm != 0)                   // LOW if 0
        return (GATE_HIGH_NEW);          // New trigger
      else
        return (GATE_LOW);           // Trigger released
    }
  }
  return(prev_trig_state[prev_trig_state_id]);            // No change (1 for active, 0 for inactive)
}

// --- Sample & Hold LFO ---
float ctagSoundProcessorFormantor::applySnH(float sine_lfo_val)
{
  if(sine_lfo_val > 0.5f || sine_lfo_val < -0.5f )   // we use this so that we can have equal frequency as with a spared value
  {
    if (hold_triggerSnH)
      saved_SnHsample = oscSnH.Process();  // values -1.f ... +1.f
    hold_triggerSnH = false;
  }
  else
    hold_triggerSnH = true;

  return(saved_SnHsample);
}

// --- Formant filter function, based on method by alex@smartelectronix.com ---
float ctagSoundProcessorFormantor::formant_filter(float in)     // Vowel IDs are 0...4: A, E, I, O, U
{
  float res = (float) ( coeff_cur[0] * in + // Current coeficient for current formant "coeff_cur" has to be set by formant_filter_set_formant() before!
                        coeff_cur[1] * vowel_mem[0] + coeff_cur[2] * vowel_mem[1] + coeff_cur[3] * vowel_mem[2] +
                        coeff_cur[4] * vowel_mem[3] + coeff_cur[5] * vowel_mem[4] + coeff_cur[6] * vowel_mem[5] +
                        coeff_cur[7] * vowel_mem[6] + coeff_cur[8] * vowel_mem[7] + coeff_cur[9] * vowel_mem[8] + coeff_cur[10] * vowel_mem[9] );

  vowel_mem[9] = vowel_mem[8]; vowel_mem[8] = vowel_mem[7]; vowel_mem[7] = vowel_mem[6]; vowel_mem[6] = vowel_mem[5]; vowel_mem[5] = vowel_mem[4];
  vowel_mem[4] = vowel_mem[3]; vowel_mem[3] = vowel_mem[2]; vowel_mem[2] = vowel_mem[1]; vowel_mem[1] = vowel_mem[0];

  vowel_mem[0] = res;
  return res;
}

// --- Find random formants by setting parameters for 5*3 BP-filters ---
void ctagSoundProcessorFormantor::random_bp_filter_settings(int set_num)
{
int lo=0; int hi=0;

  // --- Decide which mode is needed to set BP "formants" ---
  if( set_num < 0 )   // Random all (upon initialisation)
  {
    lo = 0;
    hi = 11;
  }
  else                // Random one
  {
    lo = set_num;
    hi = set_num;
  }
  // ### printf("random_bp_filter_settings(%d, %d)\n", hi, lo );
  // --- Either set one or all required random bandpass combinations to result in new formant filtering ---
  for( int i=lo; i <= hi; i++)   // Process all 5 formants
  {
    // --- Set Cutoff frequencies for 3 Bandpass filters for each formant ---
    f_CutOffXarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 0.2f, 0.4f);
    f_CutOffYarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 0.3f, 0.6f);
    f_CutOffZarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 0.5f, 0.8f);

    // --- Set Resonance for 3 Bandpass filters for each formant ---
    f_ResoXarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 3.f, 4.5f);
    f_ResoYarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 2.f, 4.f);
    f_ResoZarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 3.5f, 5.f);

    // --- Set Volume for 3 Bandpass filters for each formant ---
    f_FltAmntXarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 0.8f, 1.4f);
    f_FltAmntYarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 0.6f, 1.2f);
    f_FltAmntZarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 0.9f, 1.8f);
  }
}

// --- Main processing function for Formantor ---
void ctagSoundProcessorFormantor::Process(const ProcessData &data)
{
// --- Main processing output ---
float f_val_result = 0.f;
float f_val_pd = 0.f;   // Results for PD Osc
float f_val_sqw = 0.f;  // Results for SQW Osc
float f_val_saw = 0.f;  // Results for SAW Osc

// --- Formant values for BP formants ---
float f_formant_x = 0.f;
float f_formant_y = 0.f;
float f_formant_z = 0.f;
float f_CutOffX = 0.f;
float f_CutOffY = 0.f;
float f_CutOffZ = 0.f;
float f_ResoX = 0.f;
float f_ResoY = 0.f;
float f_ResoZ = 0.f;
float f_FltAmntX = 0.f;
float f_FltAmntY = 0.f;
float f_FltAmntZ = 0.f;

// --- Formant values for fix formant filter ---
float vowel_factor = 1.f;
bool b_use_fix_formants = true;

  // === Global section ===
  MK_ADEG_PAR(t_Gate, Gate);      // We may have a trigger if AD EG
  MK_GATE_PAR(g_Gate, Gate);      // Or a gate if ADSR EG

  MK_FLT_PAR_ABS(f_Volume, Volume, 4095.f, 2.f);

  // === Voice section ===
  MK_TRIG_PAR(t_VoicesDirectOut, VoicesDirectOut);
  MK_ADEG_PAR(t_FormantRndNew, FormantRndNew);
  if( t_FormantRndNew == GATE_HIGH_NEW)
    random_bp_filter_settings();

  MK_TRIG_PAR(t_FormantFilterOn, FormantFilterOn);

  MK_PITCH_PAR(f_MasterPitch, MasterPitch);
  MK_TRIG_PAR(t_QuantizePitch , QuantizePitch);
  float f_current_note = f_MasterPitch;
  int i_current_note = (int)(f_current_note+0.49f);   // Round and cast to nearest note, we need this for pitch quantize and/or formant selection by key
  if( t_QuantizePitch )
    f_current_note = (float)i_current_note;

  MK_FLT_PAR_ABS(f_PDamount, PDamount, 4095.f, 1.f);

  // --- SQW Osc / PWM ---
  MK_PITCH_PAR(f_SQWPitch, SQWPitch);
  MK_FLT_PAR_ABS_SFT(f_SQWTune, SQWTune, 1200.f, 1.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_PWMspeed, PWMspeed, 4095.f, 0.05f, 20.f);
  MK_FLT_PAR_ABS(f_PWMintensity, PWMintensity, 4095.f, 1.f);

  // === Mixer/Xmod section ===
  MK_TRIG_PAR(t_AMon, AMon);
  MK_FLT_PAR_ABS(f_PDaMod, PDaMod, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SQWaMod, SQWaMod, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SAWaMod, SAWaMod, 4095.f, 1.f);

  MK_FLT_PAR_ABS(f_PDvol, PDvol, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SQWvol, SQWvol, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SAWvol, SAWvol, 4095.f, 1.f);

  // === Formant section ===
  MK_INT_PAR_ABS(i_FormantSelect, FormantSelect,  12.f);
  i_FormantSelect--;    // GUI shows values 1-12 for formants a,e,i,o,u (plus interpolated variants) we use 0-11 internally!
  CONSTRAIN(i_FormantSelect, 0, 11);  // We use 11 values to make them playable by 12 notes per octave as well ;-)

  MK_FLT_PAR_ABS(f_FormantAmount, FormantAmount, 4095.f, 1.f);

  // --- Check for formant-selection via black keys ---
  MK_TRIG_PAR( t_KeyLogic, KeyLogic );
  int formant_selected = 0;
  if( t_KeyLogic )    // Select formants via notes on a keyboard or the slider on the GUI
    formant_selected = i_current_note%12;       // We have max 12 formants, one associated to every key, regardless of its octave
  else    // No key logic
    formant_selected = i_FormantSelect;         // Take formants from the slider / CV instead
  CONSTRAIN( formant_selected, 0, 11); // We had a weird segmentation violation, which should be preventable, just in case...

  // === Resonator (Resonant comb filter) ===
  MK_TRIG_PAR(t_ResCombOn, ResCombOn);
  MK_TRIG_PAR(t_ResCombBeforeFormants, ResCombBeforeFormants);
  MK_FLT_PAR_ABS(f_ResFreq, ResFreq, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_ResTone, ResTone, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_ResQ, ResQ, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_ResAmount, ResAmount,4095.f, 1.f);

  // === Tremolo ===
  MK_TRIG_PAR(t_TremoloActive, TremoloActive);
  MK_TRIG_PAR(t_TremoloIsSQW, TremoloIsSQW);
  MK_TRIG_PAR(t_TremoloAfterFormant, TremoloAfterFormant);
  MK_FLT_PAR_ABS(f_TremoloAttack, TremoloAttack, 4095.f, 20.f);
  MK_FLT_PAR_ABS(f_TremoloRelease, TremoloRelease, 4095.f, 20.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_TremoloSpeed, TremoloSpeed, 4095.f, 0.05f, 20.f);
  MK_FLT_PAR_ABS(f_TremoloAmount,TremoloAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_TremoloResAmount,TremoloResAmount, 4095.f, 1.f);
  tremolo_adsr.SetAttack(f_TremoloAttack);

  // === Volume Envelope section ===
  float vol_eg_process = 1.f;              // Set to max, in case if EG is unused this will work, too!
  MK_TRIG_PAR(t_EGvolActive, EGvolActive); // If active we also "precalculate" the Process() values for our main loop (Vol EG first)
  if( t_EGvolActive )     // Is the envelope activated anyways?
  {
    MK_TRIG_PAR(t_EGvolSlow, EGvolSlow);
    MK_TRIG_PAR(t_ADSRon, ADSRon);
    MK_FLT_PAR_ABS(f_Attack, Attack, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_Decay, Decay, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_Sustain, Sustain, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_Release, Release, 4095.f, 10.f);
    if (t_EGvolSlow)   // We extend the EG-times (for AD _and_ ADSR) if slow envelope is selected!
    {
      f_Attack *= 30.f;
      f_Decay *= 30.f;
      f_Release *= 30.f;
    }
    // --- Decide if to use ADSR or AD envelope
    if (t_ADSRon)   // ADSR mode for volume EG
    {
      vol_eg_adsr.SetAttack(f_Attack);
      vol_eg_adsr.SetDecay(f_Decay);
      vol_eg_adsr.SetSustain(f_Sustain);
      vol_eg_adsr.SetRelease(f_Release);
      vol_eg_adsr.Gate(g_Gate);
      vol_eg_process = vol_eg_adsr.Process();   // Precalculate current Volume EG, it will be added in the "main" DSP-loop below
    }
    else  // AD mode for volume EG
    {
      vol_eg_ad.SetAttack(f_Attack);
      vol_eg_ad.SetDecay(f_Decay);
      if (t_Gate == GATE_HIGH_NEW)    // New trigger encountered?
        vol_eg_ad.Trigger();
      vol_eg_process = vol_eg_ad.Process();   // Precalculate current Volume EG, it will be added in the "main" DSP-loop below
    }
  }
  // === Precalculation for realtime DSP loop ===
  // --- Find out what formant-related settings have to be made before main loop ---
  if( t_ResCombOn )      // With resonator for performance reasons (on the ESP) only random formants are to be used
  {
    b_use_fix_formants = false;
    f_CutOffX = f_CutOffXarray[formant_selected]; // Used with random formants for bandpass function later: Svf_process(Svf__ctx_type_4 &_ctx, float x, float cv, float q, int sel) - sel==2 for bandpass
    f_CutOffY = f_CutOffXarray[formant_selected];
    f_CutOffZ = f_CutOffXarray[formant_selected];
    f_ResoX = f_ResoXarray[formant_selected];
    f_ResoY = f_ResoYarray[formant_selected];
    f_ResoZ = f_ResoZarray[formant_selected];
    f_FltAmntX = f_FltAmntXarray[formant_selected];
    f_FltAmntY = f_FltAmntYarray[formant_selected];
    f_FltAmntZ = f_FltAmntZarray[formant_selected];
  }
  else // Use fix formants
  {
    if (formant_selected == 0)
      vowel_factor = 0.6f;   // Formant A is much louder, we lower the volume! Else, the factor simply will be 1.
    if (formant_selected == 3)
      vowel_factor = 1.1f;   // Formant I is much quieter, we increase the volume! Else, the factor simply will be 1.

    formant_filter_set_formant(formant_selected); // We set the selected formant to be used as member-variable for runtime-optimisation for main loop
  }
  // --- Set values for PD-synth (we modded the MIDI example, because that's an easy way to set pitch and PD-amount _before_ the main loop to save performance... ---
  Phasedist_real_controlChange(pd_data, 31, f_PDamount, 0);     // In the given example PD was mapped to MIDI CC 31
  Phasedist_real_noteOn(pd_data, f_current_note, 110, 0);

  // --- Set LFOs (PWM and Tremolo) ---
  lfoPWM.SetFrequency(f_PWMspeed);
  float f_LFO_pwm = lfoPWM.Process();   // This is a "free running" LFO, we don't use other dependancies to not complicate things, even if it's unused

  // --- Set tremolo ---
  lfoTremolo.SetFrequency(f_TremoloSpeed);
  float f_LFO_tremolo = lfoTremolo.Process();
  float f_SnH = applySnH(f_LFO_tremolo);        // We may need this to modulate the Resonator Tone Amount
  if (t_TremoloIsSQW)
    SINE_TO_SQUARE(f_LFO_tremolo);
  tremolo_adsr.SetAttack(f_TremoloAttack);
  tremolo_adsr.SetDecay(0.f);
  tremolo_adsr.SetSustain(1.f);
  tremolo_adsr.SetRelease(f_TremoloRelease);
  if( t_TremoloActive )
    tremolo_adsr.Gate(true);
  else
    tremolo_adsr.Gate(false);              // Reset Attack-phase of tremolo if effect is off
  float f_tremolo_eg = tremolo_adsr.Process();  // If Tremolo is off, the effect will fade out until the Tremolo-EG is zero!

  // --- Set values for additional PWM oscillator ---
  oscPWM.SetFrequency(noteToFreq(f_current_note+f_SQWPitch+f_SQWTune) );

  // === Realtime DSP output loop ===
  float sawNote = f_current_note / 127.f;    // Saw_eptr_process() is scaled to 0 ... 1.0 for 5 octaves of "MIDI"-notes as float-value...
  float f_sine_tmp = 0.f;
  float f_right_direct_out = 0.f;
  float f_rescomb_process = 0.f;
  for(uint32_t i = 0; i < bufSz; i++)
  {
    // --- Calculate all oscillators (PD, SAW, SQW/PWM) ---
    f_val_pd = Phasedist_real_process(pd_data,0);   // The input-parameter is ignored by this VULT algorithm with the MIDI-example!
    f_val_saw = 0.f; // Performancetest! Saw_eptr_process( saw_data, sawNote );
    f_val_sqw = f_sine_tmp = oscPWM.Process();
    f_val_sqw += f_LFO_pwm*f_PWMintensity; // Add "PWM-offset"
    SINE_TO_SQUARE(f_val_sqw);  // This by nature contains a constrain, avoiding value overflow (possibly due to PWM-mechanism)!

    // --- Calculate values for crossmodulations (PD, SAW, SQW/PWM) ---
    if(t_AMon)
    {
      float f_val_pd_tmp = f_val_pd;
      f_val_pd = X_FADE(f_PDaMod, f_val_pd, f_val_pd * f_val_saw);
      f_val_saw = X_FADE(f_SAWaMod, f_val_saw, f_val_saw * f_sine_tmp);
      f_val_sqw = X_FADE(f_SQWaMod, f_val_sqw, f_val_sqw * f_val_pd_tmp);
    }
    // --- Mixer --- // Please note: We also save the f_rescomb_processvalue for S&H tremolo in case the resonator is not active and value for direct out of right channel
    f_val_result = f_right_direct_out = f_rescomb_process = (f_val_pd*f_PDvol + f_val_sqw*f_SQWvol + f_val_saw*f_SAWvol) / 3.f;    // Check if we already devide here? ###

    // --- Tremolo before Formant-Filter/Rescomb? ---
    if( !t_TremoloAfterFormant && t_TremoloActive)    // ### to check t_TremoloActive here is a hack, because normally we need the EG for smooth transitions!
      f_val_result = X_FADE(f_TremoloAmount*f_tremolo_eg, f_val_result, f_LFO_tremolo*f_val_result); // XFade will be all left it tremolo-EG is finished!

    /*  ###
    if( t_ResCombOn && t_ResCombBeforeFormants)     // ### Drop this option ???
    {
      f_rescomb_process = Rescomb_process(rescomb_data, f_val_result, f_ResFreq, f_ResTone, f_ResQ);
      f_val_result = X_FADE(f_ResAmount, f_val_result, f_rescomb_process );
    }
    static int count_me = 0;  // ###
    if( (count_me%1000)==0)  // ###
      printf("t_ResCombBeforeFormants: %d\n",t_ResCombBeforeFormants); // ###
    count_me++; // ###
    ### */

    // --- Formant-Filter with fix or random values ---
    if( t_FormantFilterOn )
    {
      float f_val_formants = 0.f;
      if( b_use_fix_formants )
        f_val_formants = formant_filter(f_val_result * vowel_factor);
      else
      {
        f_formant_x = Svf_process(svf_data_x, f_val_result, f_CutOffX, f_ResoX, 2) * f_FltAmntX;
        f_formant_y = Svf_process(svf_data_y, f_val_result, f_CutOffY, f_ResoY, 2) * f_FltAmntY;
        f_formant_z = Svf_process(svf_data_z, f_val_result, f_CutOffZ, f_ResoZ, 2) * f_FltAmntZ;
        f_val_formants = (f_formant_x + f_formant_y + f_formant_z)/3.f;
      }
      f_rescomb_process = f_val_formants;   // We save the value for S&H tremolo in case the resonator is not active
      f_val_result = X_FADE(f_FormantAmount, f_val_result, f_val_formants);
    }
    // --- Apply resonator if active (and located after formant-filter) ---
    if( t_ResCombOn ) // ###  && !t_ResCombBeforeFormants)
    {
      f_rescomb_process = Rescomb_process(rescomb_data, f_val_result, f_ResFreq, f_ResTone, f_ResQ);
      f_val_result = X_FADE(f_ResAmount, f_val_result, f_rescomb_process );
    }
    // --- Tremolo after Formant-Filter/Resonator? ---
    if( t_TremoloAfterFormant && t_TremoloActive)    // ### to check t_TremoloActive here is a hack, because normally we need the EG for smooth transitions!
      f_val_result = X_FADE(f_TremoloAmount*f_tremolo_eg, f_val_result, f_LFO_tremolo*f_val_result); // XFade will be all left it tremolo-EG is finished!

    // --- Fade in Resonator S&H result with Tremolo if activated ---
    if( f_TremoloResAmount >= 0.1f) // ### Hack to check performance!
      f_val_result = X_FADE(f_TremoloResAmount*f_tremolo_eg, f_val_result, f_SnH*f_rescomb_process);

    // --- Apply AD[SR] if active and send out DSP-result ---
    f_val_result *= vol_eg_process * f_Volume;      // Apply AD or ADSR volume shaping to audio (is 1.0 if EG is inactive), adjust master-volume
    CONSTRAIN(f_val_result, -1.f, 1.f );
    if(t_VoicesDirectOut )
    {
      f_right_direct_out *= f_Volume;
      CONSTRAIN(f_right_direct_out, -1.f, 1.f);  // Limit right-channel result to max. audio-level
      data.buf[i * 2] = f_val_result;                     // Left channel
      data.buf[i * 2 + 1] = f_right_direct_out;           // Right channel output
    }
    else
    {
      data.buf[i * 2] = f_val_result;                     // Left channel
      data.buf[i * 2 + 1] = f_val_result;                 // Right channel output
    }
  }
}

// --- Formantor Constructor ---
ctagSoundProcessorFormantor::ctagSoundProcessorFormantor()
{
  // construct internal data model
  knowYourself();
  model = std::make_unique<ctagSPDataModel>(id, isStereo);
  LoadPreset(0);

  // --- Additional oscillator (PWM) ---
  oscPWM.SetSampleRate(44100.f);
  oscPWM.SetFrequency(1.f);

  // --- VULT ---
  Phasedist_real_process_init(pd_data);
  Phasedist_real_default(pd_data);              // Enable default settings for PD-Synth

  Rescomb_process_init(rescomb_data);           // Modified to use heap_caps_malloc()

  Saw_eptr_process_init(saw_data);          // SawWave

  // State Variable Filter (Bandpass) init (svf_data_x,y,z... for 3 bandpasses);
  Svf__ctx_type_4_init(svf_data_x);
  Svf__ctx_type_4_init(svf_data_y);
  Svf__ctx_type_4_init(svf_data_z);

  // --- Initialize Volume Envelope ---
  vol_eg_ad.SetSampleRate(44100.f/ bufSz);    // Sync Env with our audio-processing
  vol_eg_ad.SetModeExp();  // Logarithmic scaling
  vol_eg_adsr.SetSampleRate(44100.f/ bufSz);    // Optional ADSR-EG: sync Env with our audio-processing
  vol_eg_adsr.SetModeExp();                     // Logarithmic scaling
  vol_eg_adsr.Reset();

  // --- Set random formants for 3 Bandpass filters ---
  random_bp_filter_settings();

  // --- LFO for PWM of SQW Oscillator ---
  lfoPWM.SetSampleRate(44100.f / bufSz);
  lfoPWM.SetFrequency(1.f);

  // --- LFO for Tremolo / Panner ---
  lfoTremolo.SetSampleRate(44100.f / bufSz);
  lfoTremolo.SetFrequency(1.f);

  tremolo_adsr.SetSampleRate(44100.f/ bufSz);
  tremolo_adsr.SetModeExp();
  tremolo_adsr.Reset();
  tremolo_adsr.SetDecay(0.f); // We only use the Attack to delay the tremolo-effect, so the rest can be set statically here already
  tremolo_adsr.SetSustain(1.f);
  tremolo_adsr.SetRelease(0.f);
}

// --- Formantor Destructor ---
ctagSoundProcessorFormantor::~ctagSoundProcessorFormantor()
{
  heap_caps_free(rescomb_data._inst179._inst47a.bufferptr);   // Free delay buffer of Resonant comb filter!
}

// --- Formantor Initializer for factory design pattern ---
void ctagSoundProcessorFormantor::knowYourself()
{
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("Gate", [&](const int val){ Gate = val;});
	pMapTrig.emplace("Gate", [&](const int val){ trig_Gate = val;});
	pMapPar.emplace("MasterPitch", [&](const int val){ MasterPitch = val;});
	pMapCv.emplace("MasterPitch", [&](const int val){ cv_MasterPitch = val;});
	pMapPar.emplace("MasterTune", [&](const int val){ MasterTune = val;});
	pMapCv.emplace("MasterTune", [&](const int val){ cv_MasterTune = val;});
	pMapPar.emplace("QuantizePitch", [&](const int val){ QuantizePitch = val;});
	pMapTrig.emplace("QuantizePitch", [&](const int val){ trig_QuantizePitch = val;});
	pMapPar.emplace("Volume", [&](const int val){ Volume = val;});
	pMapCv.emplace("Volume", [&](const int val){ cv_Volume = val;});
	pMapPar.emplace("VoicesDirectOut", [&](const int val){ VoicesDirectOut = val;});
	pMapTrig.emplace("VoicesDirectOut", [&](const int val){ trig_VoicesDirectOut = val;});
	pMapPar.emplace("PDPitch", [&](const int val){ PDPitch = val;});
	pMapCv.emplace("PDPitch", [&](const int val){ cv_PDPitch = val;});
	pMapPar.emplace("PDTune", [&](const int val){ PDTune = val;});
	pMapCv.emplace("PDTune", [&](const int val){ cv_PDTune = val;});
	pMapPar.emplace("PDamount", [&](const int val){ PDamount = val;});
	pMapCv.emplace("PDamount", [&](const int val){ cv_PDamount = val;});
	pMapPar.emplace("SAWPitch", [&](const int val){ SAWPitch = val;});
	pMapCv.emplace("SAWPitch", [&](const int val){ cv_SAWPitch = val;});
	pMapPar.emplace("SAWTune", [&](const int val){ SAWTune = val;});
	pMapCv.emplace("SAWTune", [&](const int val){ cv_SAWTune = val;});
	pMapPar.emplace("SQWPitch", [&](const int val){ SQWPitch = val;});
	pMapCv.emplace("SQWPitch", [&](const int val){ cv_SQWPitch = val;});
	pMapPar.emplace("SQWTune", [&](const int val){ SQWTune = val;});
	pMapCv.emplace("SQWTune", [&](const int val){ cv_SQWTune = val;});
	pMapPar.emplace("PWMspeed", [&](const int val){ PWMspeed = val;});
	pMapCv.emplace("PWMspeed", [&](const int val){ cv_PWMspeed = val;});
	pMapPar.emplace("PWMintensity", [&](const int val){ PWMintensity = val;});
	pMapCv.emplace("PWMintensity", [&](const int val){ cv_PWMintensity = val;});
	pMapPar.emplace("AMon", [&](const int val){ AMon = val;});
	pMapTrig.emplace("AMon", [&](const int val){ trig_AMon = val;});
	pMapPar.emplace("PDaMod", [&](const int val){ PDaMod = val;});
	pMapCv.emplace("PDaMod", [&](const int val){ cv_PDaMod = val;});
	pMapPar.emplace("SAWaMod", [&](const int val){ SAWaMod = val;});
	pMapCv.emplace("SAWaMod", [&](const int val){ cv_SAWaMod = val;});
	pMapPar.emplace("SQWaMod", [&](const int val){ SQWaMod = val;});
	pMapCv.emplace("SQWaMod", [&](const int val){ cv_SQWaMod = val;});
	pMapPar.emplace("PDvol", [&](const int val){ PDvol = val;});
	pMapCv.emplace("PDvol", [&](const int val){ cv_PDvol = val;});
	pMapPar.emplace("SAWvol", [&](const int val){ SAWvol = val;});
	pMapCv.emplace("SAWvol", [&](const int val){ cv_SAWvol = val;});
	pMapPar.emplace("SQWvol", [&](const int val){ SQWvol = val;});
	pMapCv.emplace("SQWvol", [&](const int val){ cv_SQWvol = val;});
	pMapPar.emplace("FormantFilterOn", [&](const int val){ FormantFilterOn = val;});
	pMapTrig.emplace("FormantFilterOn", [&](const int val){ trig_FormantFilterOn = val;});
	pMapPar.emplace("KeyLogic", [&](const int val){ KeyLogic = val;});
	pMapTrig.emplace("KeyLogic", [&](const int val){ trig_KeyLogic = val;});
	pMapPar.emplace("FormantRndNew", [&](const int val){ FormantRndNew = val;});
	pMapTrig.emplace("FormantRndNew", [&](const int val){ trig_FormantRndNew = val;});
	pMapPar.emplace("FormantSelect", [&](const int val){ FormantSelect = val;});
	pMapCv.emplace("FormantSelect", [&](const int val){ cv_FormantSelect = val;});
	pMapPar.emplace("FormantAmount", [&](const int val){ FormantAmount = val;});
	pMapCv.emplace("FormantAmount", [&](const int val){ cv_FormantAmount = val;});
	pMapPar.emplace("ResCombOn", [&](const int val){ ResCombOn = val;});
	pMapTrig.emplace("ResCombOn", [&](const int val){ trig_ResCombOn = val;});
	pMapPar.emplace("ResCombBeforeFormants", [&](const int val){ ResCombBeforeFormants = val;});
	pMapTrig.emplace("ResCombBeforeFormants", [&](const int val){ trig_ResCombBeforeFormants = val;});
	pMapPar.emplace("ResFreq", [&](const int val){ ResFreq = val;});
	pMapCv.emplace("ResFreq", [&](const int val){ cv_ResFreq = val;});
	pMapPar.emplace("ResTone", [&](const int val){ ResTone = val;});
	pMapCv.emplace("ResTone", [&](const int val){ cv_ResTone = val;});
	pMapPar.emplace("ResQ", [&](const int val){ ResQ = val;});
	pMapCv.emplace("ResQ", [&](const int val){ cv_ResQ = val;});
	pMapPar.emplace("ResAmount", [&](const int val){ ResAmount = val;});
	pMapCv.emplace("ResAmount", [&](const int val){ cv_ResAmount = val;});
	pMapPar.emplace("TremoloActive", [&](const int val){ TremoloActive = val;});
	pMapTrig.emplace("TremoloActive", [&](const int val){ trig_TremoloActive = val;});
	pMapPar.emplace("TremoloAttack", [&](const int val){ TremoloAttack = val;});
	pMapCv.emplace("TremoloAttack", [&](const int val){ cv_TremoloAttack = val;});
	pMapPar.emplace("TremoloRelease", [&](const int val){ TremoloRelease = val;});
	pMapCv.emplace("TremoloRelease", [&](const int val){ cv_TremoloRelease = val;});
	pMapPar.emplace("TremoloIsSQW", [&](const int val){ TremoloIsSQW = val;});
	pMapTrig.emplace("TremoloIsSQW", [&](const int val){ trig_TremoloIsSQW = val;});
	pMapPar.emplace("TremoloSpeed", [&](const int val){ TremoloSpeed = val;});
	pMapCv.emplace("TremoloSpeed", [&](const int val){ cv_TremoloSpeed = val;});
	pMapPar.emplace("TremoloAmount", [&](const int val){ TremoloAmount = val;});
	pMapCv.emplace("TremoloAmount", [&](const int val){ cv_TremoloAmount = val;});
	pMapPar.emplace("TremoloAfterFormant", [&](const int val){ TremoloAfterFormant = val;});
	pMapTrig.emplace("TremoloAfterFormant", [&](const int val){ trig_TremoloAfterFormant = val;});
	pMapPar.emplace("TremoloResAmount", [&](const int val){ TremoloResAmount = val;});
	pMapCv.emplace("TremoloResAmount", [&](const int val){ cv_TremoloResAmount = val;});
	pMapPar.emplace("EGvolActive", [&](const int val){ EGvolActive = val;});
	pMapTrig.emplace("EGvolActive", [&](const int val){ trig_EGvolActive = val;});
	pMapPar.emplace("Attack", [&](const int val){ Attack = val;});
	pMapCv.emplace("Attack", [&](const int val){ cv_Attack = val;});
	pMapPar.emplace("Decay", [&](const int val){ Decay = val;});
	pMapCv.emplace("Decay", [&](const int val){ cv_Decay = val;});
	pMapPar.emplace("ADSRon", [&](const int val){ ADSRon = val;});
	pMapTrig.emplace("ADSRon", [&](const int val){ trig_ADSRon = val;});
	pMapPar.emplace("Sustain", [&](const int val){ Sustain = val;});
	pMapCv.emplace("Sustain", [&](const int val){ cv_Sustain = val;});
	pMapPar.emplace("Release", [&](const int val){ Release = val;});
	pMapCv.emplace("Release", [&](const int val){ cv_Release = val;});
	pMapPar.emplace("EGvolSlow", [&](const int val){ EGvolSlow = val;});
	pMapTrig.emplace("EGvolSlow", [&](const int val){ trig_EGvolSlow = val;});
	pMapPar.emplace("EnvPDamount", [&](const int val){ EnvPDamount = val;});
	pMapCv.emplace("EnvPDamount", [&](const int val){ cv_EnvPDamount = val;});
	isStereo = true;
	id = "Formantor";
	// sectionCpp0
}