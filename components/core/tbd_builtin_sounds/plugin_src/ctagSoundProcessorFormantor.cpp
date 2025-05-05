/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020/2021 by Robert Manzke. All rights reserved.

(c) 2021 for the "Formantor"-Plugin by Mathias Brüssel
The idea of Formantor is to have a simple yet fun to control synthesizer combining a Phase Distortion oscillator, an additional oscillators,
a vowel-filter, a resonator, a tremolo and an envelope.
There are different options for playing and controlling the formant filters, for more details please look here:
https://docs.google.com/document/d/1c8mjxWjdiJNP0xpkU2CxRUp9av6V4W39wARJf3_SMSo
Formantor uses filters and a Phase Distortion synth by Carlos Laguna Ruiz implemented in his VULT language, the synth can be found here:
https://github.com/modlfo/teensy-vult-example
The code originally was intended as an add-on to the Teensy Audio library and got modified to be used with the TBD along with other filters from VULT-examples
by using the VULT compiler. For more details on the topic please look here: https://github.com/modlfo/vult
For the formant-filter Open Source code by alex@smartelectronix.com got used, as found here: https://www.musicdsp.org/en/latest/Filters/110-formant-filter.html
Also interpolation extensions by Autodave got used: https://github.com/antoniograzioli/Autodafe

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include <tbd/sounds/SoundProcessorFormantor.hpp>
#include <tbd/sound_utils/ctagNumUtil.hpp>

#include "plaits/dsp/engine/engine.h"

// --- VULT "Library for TBD" ---

// FIXME: why are the source files included here?
// #include <tbd/sound_utils/vult/vult_formantor.cpp>
// #include <tbd/sound_utils/vult/vultin.cpp"

using namespace tbd::sounds;

#define GATE_HIGH_NEW       2
#define GATE_HIGH           1
#define GATE_LOW            0

// --- Replace function-call of frequency-conversion with macro for increasing speed just a bit ---
#define noteToFreq(incoming_note) (sound_utils::fastpow2 ((incoming_note - 69.f) / 12.f) *440.f)

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

// --- Process trigger signals and keep their state internally ---
inline int SoundProcessorFormantor::process_param_trig(const audio::ProcessData&data, int trig_myparm, int my_parm, int enum_trigger_id, int gate_type = 0 )
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
        return (GATE_LOW);           // Trigger released
    }
  }
  return(prev_trig_state[enum_trigger_id]);            // No change (1 for active, 0 for inactive)
}

// --- Sample & Hold LFO ---
float SoundProcessorFormantor::applySnH(float sine_lfo_val)
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
float SoundProcessorFormantor::formant_filter(float in)     // Vowel IDs are 0...4: A, E, I, O, U
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
void SoundProcessorFormantor::random_bp_filter_settings(int set_num)
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

// --- Set one set of bandpass-Filters to given values ---
void SoundProcessorFormantor::set3BPfilters(int id, float cut1, float cut2, float cut3, float res1, float res2, float res3, float amp1, float amp2, float amp3)
{
  f_CutOffXarray[id] = cut1; // Cutoff frequency values for  formants with 3 BP filters
  f_CutOffYarray[id] = cut2;
  f_CutOffZarray[id] = cut3;

  f_ResoXarray[id] = res1;  // Resonance values for  formants with 3 BP filters
  f_ResoYarray[id] = res2;
  f_ResoZarray[id] = res3;

  f_FltAmntXarray[id] = amp1;  // Volume values for  formants with 3 BP filters
  f_FltAmntYarray[id] = amp2;
  f_FltAmntZarray[id] = amp3;
}

// --- Main processing function for Formantor ---
void SoundProcessorFormantor::Process(const audio::ProcessData&data)
{
// --- Main processing output ---
float f_val_result = 0.f;
float f_val_pd = 0.f;   // Results for PD Osc
float f_val_sqw = 0.f;  // Results for SQW Osc

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
  MK_PITCH_PAR(f_MasterPitch, MasterPitch);
  MK_FLT_PAR_ABS_SFT(f_MasterTune, MasterTune, 1200.f, 1.f);
  MK_TRIG_PAR(t_QuantizePitch , QuantizePitch);
  MK_FLT_PAR_ABS(f_Volume, Volume, 4095.f, 2.f);
  float f_current_note = f_MasterPitch+f_MasterTune;
  int i_current_note = (int)cvPitch.dejitter(f_current_note);   // Round and cast to nearest note, we need this for pitch quantize and/or formant selection by key
  if( t_QuantizePitch )
    f_current_note = (float)i_current_note;

  // === Voice section ===
  MK_TRIG_PAR(t_VoicesDirectOut, VoicesDirectOut);
  MK_FLT_PAR_ABS(f_PDamount, PDamount, 4095.f, 1.f);
  // --- SQW Osc / PWM ---
  MK_PITCH_PAR(f_PDPitch, PDPitch);
  MK_FLT_PAR_ABS_SFT(f_PDTune, PDTune, 1200.f, 1.f);
  MK_FLT_PAR_ABS_SFT(f_SQWTune, SQWTune, 1200.f, 1.f);
  MK_PITCH_PAR(f_SQWPitch, SQWPitch);
  MK_FLT_PAR_ABS(f_PDpitchMod, PDpitchMod, 4095.f, 1.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_PWMspeed, PWMspeed, 4095.f, 0.05f, 20.f);
  MK_FLT_PAR_ABS(f_PWMintensity, PWMintensity, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_WaveFolder, WaveFolder, 4095.f, 1.f);

  // === Mixer/Xmod section ===
  MK_TRIG_PAR(t_AMon, AMon);
  MK_FLT_PAR_ABS(f_PDaMod, PDaMod, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SQWaMod, SQWaMod, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_PDvol, PDvol, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SQWvol, SQWvol, 4095.f, 1.f);

  // === Formant section ===
  MK_ADEG_PAR(t_FormantRndNew, FormantRndNew);
  MK_INT_PAR_ABS(i_FormantSelect, FormantSelect,  12.f);
  i_FormantSelect--;    // GUI shows values 1-12 for formants a,e,i,o,u (plus interpolated variants) we use 0-11 internally!
  CONSTRAIN(i_FormantSelect, 0, 11);  // We use 11 values to make them playable by 12 notes per octave as well ;-)
  MK_FLT_PAR_ABS(f_FormantAmount, FormantAmount, 4095.f, 1.f);
  MK_TRIG_PAR(t_FormantFilterOn, FormantFilterOn);
  // --- Check for formant-selection via black keys ---
  MK_TRIG_PAR(t_KeyLogic, KeyLogic);
  int formant_selected = 0;
  if( t_KeyLogic )    // Select formants via notes on a keyboard or the slider on the GUI
    formant_selected = i_current_note%12;    // We have max 12 formants, one associated to every key, regardless of its octave
  else    // No key logic
    formant_selected = i_FormantSelect;         // Take formants from the slider / CV instead
  CONSTRAIN( formant_selected, 0, 11); // We had a weird segmentation violation, which should be preventable, just in case...
  if( t_FormantRndNew == GATE_HIGH_NEW)
    random_bp_filter_settings(formant_selected);

  // === Resonator (Resonant comb filter) ===
  MK_TRIG_PAR(t_ResCombOn, ResCombOn);
  MK_FLT_PAR_ABS(f_ResFreq, ResFreq, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_ResTone, ResTone, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_ResQ, ResQ, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_ResAmount, ResAmount,4095.f, 1.f);

  // === Tremolo ===
  MK_TRIG_PAR(t_TremoloActive, TremoloActive);
  MK_TRIG_PAR(t_TremoloGateTrigger, TremoloGateTrigger);
  MK_TRIG_PAR(t_TremoloIsSQW, TremoloIsSQW);
  MK_TRIG_PAR(t_TremoloAfterResonator, TremoloAfterResonator);
  MK_FLT_PAR_ABS(f_TremoloAttack, TremoloAttack, 4095.f, 20.f);
  MK_FLT_PAR_ABS(f_TremoloRelease, TremoloRelease, 4095.f, 20.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_TremoloSpeed, TremoloSpeed, 4095.f, 0.05f, 20.f);
  MK_FLT_PAR_ABS(f_TremoloAmount,TremoloAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_TremoloPDAmount,TremoloPDAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_TremoloResAmount,TremoloResAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_TremoloSnHpd, TremoloSnHpd, 4095.f, 1.f);
  MK_TRIG_PAR(t_TremoloPDisSQW, TremoloPDisSQW);

  // === Volume Envelope section ===
  float vol_eg_process = 1.f;              // Set to max, in case if EG is unused this will work, too!
  MK_TRIG_PAR(t_EGvolActive, EGvolActive); // If active we also "precalculate" the Process() values for our main loop (Vol EG first)
  MK_FLT_PAR_ABS(f_EnvPDamount, EnvPDamount, 4095.f, 1.f);
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
      vowel_factor = 0.4f;   // Formant A is much louder, we lower the volume! Else, the factor simply will be 1.
    if (formant_selected == 3)
      vowel_factor = 1.1f;   // Formant I is much quieter, we increase the volume! Else, the factor simply will be 1.

    formant_filter_set_formant(formant_selected); // We set the selected formant to be used as member-variable for runtime-optimisation for main loop
  }
  // --- Set LFO for PWM ---
  lfoPWM.SetFrequency(f_PWMspeed);
  float f_LFO_pwm = lfoPWM.Process();   // This is a "free running" LFO, we don't use other dependancies to not complicate things, even if it's unused

  // --- Set tremolo ---
  float f_LFO_tremolo = 0.f;
  float f_LFO_tremolo_pd = 0.f;
  float f_SnH = 0.f;
  float f_tremolo_eg = 0.f;

  if(t_TremoloGateTrigger)
  {
    if( t_TremoloActive && !g_Gate )    // We combine the Global Gate as used for the ADSR to enable the tremolo
      t_TremoloActive = false;          // Tremolo will only be activated as long as the gate is on in this case...
  }
  if( t_TremoloActive )
  {
    tremolo_adsr.SetAttack(f_TremoloAttack);
    tremolo_adsr.SetDecay(0.f);
    tremolo_adsr.SetSustain(1.f);
    tremolo_adsr.SetRelease(f_TremoloRelease);
    f_trem_release_active = true;
    tremolo_adsr.Gate(true);
  }
  else
  {
    tremolo_adsr.SetRelease(f_TremoloRelease);
    tremolo_adsr.Gate(false);              // Reset Attack-phase of tremolo if effect is off
  }
  if(f_trem_release_active)                     // If we come here after the end of a tremolo we optimize the performance by preventing computation of the tremolo
  {
    f_tremolo_eg = tremolo_adsr.Process();      // If Tremolo is off, the effect will fade out until the Tremolo-EG is zero!
    if( !f_tremolo_eg )
      f_trem_release_active = false;            // We do this for perfomance-optimisation, so that after the release-phase no computation of the tremolo is necessary anymore...
    else
    {
      lfoTremolo.SetFrequency(f_TremoloSpeed);
      f_LFO_tremolo = f_LFO_tremolo_pd = lfoTremolo.Process();
      f_SnH = applySnH(f_LFO_tremolo);        // We may need this to modulate the Resonator Tone Amount
      if (t_TremoloIsSQW)
        SINE_TO_SQUARE(f_LFO_tremolo);
      if(t_TremoloPDisSQW)
        SINE_TO_SQUARE(f_LFO_tremolo_pd);
    }
  }
  // --- Set values for PD-synth (we modded the MIDI example, because that's an easy way to set pitch and PD-amount _before_ the main loop to save performance... ---
  if( t_EGvolActive && vol_eg_process && f_EnvPDamount)
    f_PDamount = vol_eg_process*f_PDamount*f_EnvPDamount;     // Modulate Phase for PD-OSC by EG if active
  if(f_trem_release_active)                // Modulate Phase for PD-OSC by Tremolo (Sine-Wave) if active
    f_PDamount += f_LFO_tremolo_pd*f_TremoloPDAmount*f_tremolo_eg;
  f_PDamount += f_SnH*f_TremoloSnHpd*f_tremolo_eg;    // S&H Modulation on PD-OSC Phase
  CONSTRAIN(f_PDamount, 0.f, 1.f);
  Phasedist_real_controlChange(pd_data, 31, f_PDamount, 0);     // In the given example PD was mapped to MIDI CC 31
  Phasedist_real_noteOn(pd_data, f_current_note+f_PDPitch+f_PDTune+(7.f*f_LFO_pwm*f_PDpitchMod), 110, 0);   // Notes seemingly are notated one note lower for Phasedist_real_noteOn()

  // --- Set values for additional PWM oscillator ---
  oscPWM.SetFrequency(noteToFreq(f_current_note+f_SQWPitch+f_SQWTune) );

  // === Realtime DSP output loop ===
  float f_sine_tmp = 0.f;
  float f_right_direct_out = 0.f;
  float f_rescomb_process = 0.f;
  for(uint32_t i = 0; i < bufSz; i++)
  {
    // --- Calculate all oscillators (PD, SQW & PWM) ---
    f_val_pd = Phasedist_real_process(pd_data,0);   // The input-parameter is ignored by this VULT algorithm with the MIDI-example!
    f_val_pd = Fold_do(f_val_pd, f_WaveFolder);   // Apply wavefolder to PD-OSC
    f_val_sqw = f_sine_tmp = oscPWM.Process();
    f_val_sqw += f_LFO_pwm*f_PWMintensity; // Add "PWM-offset"
    SINE_TO_SQUARE(f_val_sqw);  // This by nature contains a constrain, avoiding value overflow (possibly due to PWM-mechanism)!

    // --- Calculate values for amplitude-modulations (PD, SQW/PWM) ---
    if(t_AMon)
    {
      float f_val_pd_tmp = f_val_pd;
      f_val_pd = X_FADE(f_PDaMod, f_val_pd, f_val_pd*f_sine_tmp );
      f_val_sqw = X_FADE(f_SQWaMod, f_val_sqw, f_val_sqw * f_val_pd_tmp);
    }
    // --- Mixer --- // Please note: We also save the f_rescomb_processvalue for S&H tremolo in case the resonator is not active and value for direct out of right channel
    f_val_result = f_right_direct_out = f_rescomb_process = (f_val_pd*f_PDvol+f_val_sqw*f_SQWvol) / 2.f;    

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
    // --- Tremolo before Resonator? ---
    if( !t_TremoloAfterResonator )
      f_val_result = X_FADE(f_TremoloAmount*f_tremolo_eg, f_val_result, f_LFO_tremolo*f_val_result); // XFade will be all left it tremolo-EG is finished!

    // --- Apply resonator if active  ---
    if( t_ResCombOn )
    {
      f_rescomb_process = Rescomb_process(rescomb_data, f_val_result, f_ResFreq, f_ResTone, f_ResQ);
      f_val_result = X_FADE(f_ResAmount, f_val_result, f_rescomb_process );
    }
    // --- Tremolo after resonator? ---
    if( t_TremoloAfterResonator )
      f_val_result = X_FADE(f_TremoloAmount*f_tremolo_eg, f_val_result, f_LFO_tremolo*f_val_result); // XFade will be all left it tremolo-EG is finished!

    // --- Fade in Resonator S&H result with Tremolo if activated ---
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
void SoundProcessorFormantor::Init(std::size_t blockSize, void *blockPtr)
{
  // --- Additional oscillator (PWM) ---
  oscPWM.SetSampleRate(44100.f);
  oscPWM.SetFrequency(1.f);

  // --- VULT ---
  Phasedist_real_process_init(pd_data);
  Phasedist_real_default(pd_data);              // Enable default settings for PD-Synth

  Rescomb_process_init(rescomb_data);           // Modified to use heaps::malloc()
  assert(blockSize >= sizeof(float)*675);
  rescomb_data._inst179._inst47a.bufferptr = (float*)blockPtr;
  memset(rescomb_data._inst179._inst47a.bufferptr, 0, sizeof(float)*675);

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

  // --- Set initial [prior randomly chosen] formants for 3 Bandpass filters ---
  set3BPfilters(0, 0.350134, 0.442857, 0.302735,3.871680, 3.772821, 3.358482,1.005863, 0.947466, 0.090034 );
  set3BPfilters(1, 0.265362, 0.496865, 0.212996,1.655421, 2.887684, 2.983210,0.526202, 0.882908, 1.544251);
  set3BPfilters(2, 0.267909, 0.509398, 0.252759, 4.143388, 0.561378, 4.309381, 1.306037, 1.170296, 1.735694);
  set3BPfilters(3,  0.350134, 0.442857, 0.302735, 3.871680, 3.772821, 3.358482, 1.005863, 0.947466, 0.090034);
  set3BPfilters(4, 0.265362, 0.496865, 0.212996, 1.655421, 2.887684, 2.983210, 0.526202, 0.882908, 1.544251);
  set3BPfilters(5, 0.336168, 0.573984, 0.346148, 4.073148, 2.539462, 2.549362, 1.052410, 0.854977, 1.185147);
  set3BPfilters(6, 0.374319, 0.576136, 0.525537, 4.042409, 1.695248, 2.023940, 0.346494, 0.931837, 0.273987);
  set3BPfilters(7, 0.336168, 0.573984, 0.346148, 4.073148, 2.539462, 2.549362, 1.052410, 0.854977, 1.185147);
  set3BPfilters(8, 0.394112, 0.352247, 0.215574, 2.220654, 1.369757, 3.133960, 1.187745, 0.235485, 0.283078);
  set3BPfilters(9, 0.383531, 0.003512, 0.419062, 2.394067, 3.441839, 4.235986, 1.086089, 0.491692, 1.002079);
  set3BPfilters(10, 0.323634, 0.182159, 0.542681, 1.721589, 1.657328, 4.037016, 0.249820, 0.917323, 1.561672);
  set3BPfilters(11, 0.357433, 0.271803, 0.600643, 3.492126, 2.210455, 4.336462, 0.963865, 0.687047, 1.209702);

  // --- LFO for PWM of SQW Oscillator ---
  lfoPWM.SetSampleRate(44100.f / bufSz);
  lfoPWM.SetFrequency(1.f);

  // --- LFO for Tremolo / Panner ---
  lfoTremolo.SetSampleRate(44100.f / bufSz);
  lfoTremolo.SetFrequency(1.f);

  tremolo_adsr.SetSampleRate(44100.f/ bufSz);
  tremolo_adsr.SetModeExp();
  tremolo_adsr.Reset();
}

// --- Formantor Destructor ---
SoundProcessorFormantor::~SoundProcessorFormantor()
{
}
