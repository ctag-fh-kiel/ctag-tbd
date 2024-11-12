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


#include <tbd/sounds/ctagSoundProcessorAPCpp.hpp>
#include <cstdlib>

#define GATE_HIGH_NEW       2
#define GATE_HIGH           1
#define GATE_LOW            0

using namespace CTAG::SP;

// --- Process trigger signals and keep their state internally ---
inline int ctagSoundProcessorAPCpp::process_param_trig(const ProcessData &data, int trig_myparm, int my_parm, int enum_trigger_id, int gate_type = 0 )
{
  int trig_status = 0;

  if(trig_myparm != -1)       // Trigger given via CV/Gate or button?
  {
    trig_status = (data.trig[trig_myparm]==0); // HIGH is 0, we assign true or false to trig_status, without changine the value of the reference to the data.trig-array!
    if( gate_type == 1 )      // We have an on/off gate
      return(trig_status);    // And return true if the gate is on, false if off

    if(trig_status)    // Statuschange (for toggles) from HIGH to LOW or LOW to HIGH? 
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

// --- Helper function: rescale CV or Pot to float 0...1.0 (CV is already in correct format, we still keep it inside this method for convenience ---
inline float ctagSoundProcessorAPCpp::process_param_float(const ProcessData &data, int cv_myparm, int my_parm, float out_min, float out_max, bool exponential )
{
  if(cv_myparm != -1)
  {
    if (data.cv[cv_myparm] >= 0.0f)     // This is a bypass solution to avoid negative values in rare cases
    {
      if (exponential)                  // Typically this is used for volume envelopes
        return data.cv[cv_myparm] * data.cv[cv_myparm] * out_max;     // Avoid negative values by self-multiplication, apply exponatial-type of scaling, ignore lower limit for range
      else                              // Standard processing for all other usecases
        return data.cv[cv_myparm] * (out_max - out_min) + out_min;     // Rescale float of 0.0..1.0 to min...max output-range
    }
    else
      return out_min;                   // Unexpected, return minimum valid value
  }
  else
  {
    if (exponential)                    // Typically this is used for volume envelopes,
      return (my_parm/4095.f) * (my_parm/4095.f) * out_max;       // Avoid negative values by self-multiplication, apply exponatial-type of scaling, ignore lower limit for range
    else                                // Standard processing for all other usecases
      return (my_parm/4095.f) * (out_max - out_min) + out_min;    // Convert to float of 0.0..1.0 and scale to min..max output-range
  }
}

void ctagSoundProcessorAPCpp::Process(const ProcessData &data)
{
  // --- Read and buffer triggers/options for APC ---
  // Use sinus or square for pitch-modulation of each of the two oscillators
  fm1_is_square = process_param_trig(data, trig_FreqmodSquare_active_1, FreqmodSquare_active_1, e_FreqmodSquare_active_1);
  fm2_is_square = process_param_trig(data, trig_FreqmodSquare_active_2, FreqmodSquare_active_2, e_FreqmodSquare_active_2);
  // Sine waves instead of squares?
  smooth_it_1 = process_param_trig(data, trig_SmoothOSC_1, SmoothOSC_1, e_SmoothOSC_1);
  smooth_it_2 = process_param_trig(data, trig_SmoothOSC_2, SmoothOSC_2, e_SmoothOSC_2);
  // PWM instead of AM?
  pwm_mod_1 = process_param_trig(data, trig_MOD_is_PWM_1, MOD_is_PWM_1, e_MOD_is_PWM_1);
  pwm_mod_2 = process_param_trig(data, trig_MOD_is_PWM_2, MOD_is_PWM_2, e_MOD_is_PWM_2);
  // MOD for OSCs active?
  mod1_on = process_param_trig(data, trig_MOD_active_1, MOD_active_1, e_MOD_active_1);  // Allow PWM for Osc1?
  mod2_on = process_param_trig(data, trig_MOD_active_2, MOD_active_2, e_MOD_active_2);  // Allow PWM for Osc2?
  // Volume Envelope active and/or triggered?
  env_active = process_param_trig(data, trig_Env_active, Env_active, e_Env_active);
  env_trigger = process_param_trig(data, trig_Trigger_env, Trigger_env, e_Trigger_env, 2);  // Variant 2: trigger always, not (only) toggled!
  env_loop = process_param_trig(data, trig_Env_loop_active, Env_loop_active, e_Env_loop_active);

  // --- Read and buffer controllers for APC and frequencies required for sound-generation lateron ---
  f_midi_note_1 = process_param_float(data, cv_Freq_1, Freq_1) * 60.f;   // buffer Frequency for Oscillator 1 compatible with MIDI-notes as CV
  f_midi_note_2 = process_param_float(data, cv_Freq_2, Freq_2) * 60.f;   // buffer Frequency for Oscillator 2 compatible with MIDI-notes as CV
  // PMW frequencies
  pwm_freq_1 = process_param_float(data, cv_MOD_freq_1, MOD_freq_1, 0.05f, 25.f );
  pwm_freq_2 = process_param_float(data, cv_MOD_freq_2, MOD_freq_2, 0.05f, 25.f );
  // FM frequencies
  fm_freq_1 = process_param_float(data, cv_Freqmod_freq_1, Freqmod_freq_1, 0.05f, 100.f );
  fm_freq_2 = process_param_float(data, cv_Freqmod_freq_2, Freqmod_freq_2, 0.05f, 100.f );
  // FM amount
  fm_amnt_1 = process_param_float(data, cv_Freqmod_amount_1, Freqmod_amount_1 );
  fm_amnt_2 = process_param_float(data, cv_Freqmod_amount_2, Freqmod_amount_2 );
  // Volume for "VCA" (and max. vol for envelope)
  volume = process_param_float(data, cv_Vol_amount, Vol_amount);
  // Envelope parameters
  vol_attack = process_param_float(data, cv_Env_Attack, Env_Attack, 0.f, 2.f, true );  // Important: set optional last parameter to exponential scaling!
  vol_decay = process_param_float(data, cv_Env_Decay, Env_Decay, 0.f, 10.f, true );   // Important: set optional last parameter to exponential scaling!

  // --- Set values for Envelope Generator ---
  vol_env.SetAttack(vol_attack);
  vol_env.SetDecay(vol_decay);
  vol_env.SetLoop( (bool)env_loop );

  // --- DSP related stuff that can be handled already before the main DSP-loop ---
  osc_freq_1 = noteToFreq(f_midi_note_1+36.f);      // set Frequency of Oscillator 1 according to GUI or CV, allow 1V/Oct logic for CV
  osc_freq_7_up_1 = osc_freq_1 / 2.f * 3.f;                     // perfect fifth above current pitch, we use this as max +- 7 semitones for pitchmod
  osc_freq_2 = noteToFreq(f_midi_note_2+36.f);      // set Frequency of Oscillator 2 according to GUI or CV, allow 1V/Oct logic for CV
  osc_freq_7_up_2 = osc_freq_2 / 2.f * 3.f;                     // perfect fifth above current pitch, we use this as max +- 7 semitones for pitchmod

  if(!fm_amnt_1 )
    osc_1.SetFrequency(osc_freq_1);                             // Set Frequency of Osc1 already here if no FM
  else        // Oscillator-Frequency is modulated => SetFrequency in main loop
    fm_1.SetFrequency(fm_freq_1);                               // Set Frequencies of LFOs for Frequency Modulation for Osc1

  if(!fm_amnt_2 )
    osc_2.SetFrequency(osc_freq_2);                             // Set Frequency of Osc1 already here if no FM
  else        // Oscillator-Frequency is modulated => SetFrequency in main loop
    fm_2.SetFrequency(fm_freq_2);                               // Set Frequencies of LFOs for Frequency Modulation for Osc1

  if( mod1_on )
    pwm_1.SetFrequency(pwm_freq_1);                             // Set Frequency for PWM of Osc1
  if( mod2_on )
    pwm_2.SetFrequency(pwm_freq_2);                             // Set Frequency for PWM of Osc2

  if(smooth_it_1 && !smooth_it_2)              // Amplify mastervolume a bit in case if smoothed operation-mode is selected
    smoothed_amp_factor = 1.1f;
  else if(!smooth_it_1 && smooth_it_2)              // Amplify mastervolume a bit in case if smoothed operation-mode is selected
    smoothed_amp_factor = 1.5f;
  else if(smooth_it_1 && smooth_it_2)               // Both Osc are Sinewaves - Amplify mastervolume massively in case if smoothed operation-mode is selected
    smoothed_amp_factor = 4.f;

  if( env_active && env_trigger==GATE_HIGH_NEW )
    vol_env.Trigger();

  // --- This is our main loop, where the generation of the APC-tones takes place ---
  for (uint32_t i = 0; i < bufSz; i++)
  {
    if (fm_amnt_1)       // Pitch-Modulation / "fm" is active, else the frequency has been set outside of the main loop already!
    {
      fm_freq_1 = fm_1.Process();
      if (fm1_is_square)
        (fm_freq_1 >= 0.f) ? fm_freq_1 = 1.f : fm_freq_1 = -1.f;   // Conversion of sinus to square
      osc_1.SetFrequency(osc_freq_1 + osc_freq_7_up_1 * fm_freq_1 * fm_amnt_1);   // We modulate +- a fifth max.
    }
    if (fm_amnt_2)       // Pitch-Modulation / "fm" is active, else the frequency has been set outside of the main loop already!
    {
      fm_freq_2 = fm_2.Process();
      if (fm2_is_square)
        (fm_freq_2 >= 0.f) ? fm_freq_2 = 1.f : fm_freq_2 = -1.f;   // Conversion of sinus to square
      osc_2.SetFrequency(osc_freq_2 + osc_freq_7_up_2 * fm_freq_2 * fm_amnt_2);   // We modulate +- a fifth max.
    }
    f_valA = osc_1.Process();   // Get next sample from (sine) oscillator 1
    f_valB = osc_2.Process();   // Get next sample from (sine) oscillator 2

    if (mod1_on)     // If MOD is activate modulate Pulsewidth of Oscillator 1 or add sines when smoothed
    {
      if (pwm_mod_1)                                      // Use pseude PWM (also on Sine)
      {
        f_valA += pwm_1.Process();                        // Shift wave resulting in a PWM-effect
        if (f_valA > 1.f) f_valA = 1.f;
        if (f_valA < -1.f) f_valA = -1.f;
      }
      else
        f_valA *= ((pwm_1.Process() >= 0) ? 1.f : -1.f);     // Amplitude-modulation with squarewave
    }
    if (mod2_on)     // If MOD is activate modulate Pulsewidth of Oscillator 2 or add sines when smoothed
    {
      if (pwm_mod_2)                                      // Use pseude PWM (also on Sine)
      {
        f_valB += pwm_2.Process();                        // Shift wave resulting in a PWM-effect
        if (f_valB > 1.f) f_valB = 1.f;
        if (f_valB < -1.f) f_valB = -1.f;
      }
      else
        f_valB *= ((pwm_2.Process() >= 0) ? 1.f : -1.f);    // Amplitude-modulation with squarewave
    }
    // --- Calculate pitched PulseWaves (from Sinusgenerators) or "smoothed" waves, Pulse Width Modulation and so on ---
    if(smooth_it_1 || smooth_it_2)    // Use sinewaves instead of squarewaves...
    {
      if( smooth_it_1 )   // Set OSC 1 to sine
      {
        if (f_valA > 1.f) f_valA = 1.f;   // Truncate in case if "PWM" got too far
        if (f_valA < -1.f) f_valA = -1.f; // Truncate in case if "PWM" got too far
      }
      else
      {
        if (f_valA >= 0.f) f_valA = 1.f;  // Convert osc1 to square
        if (f_valA < 0.f) f_valA = -1.f;  // Convert osc1 to square
      }
      if( smooth_it_2 )   // Set OSC 2 to sine
      {
        if (f_valB > 1.f) f_valB = 1.f;   // Truncate in case if "PWM" got too far
        if (f_valB < -1.f) f_valB = -1.f; // Truncate in case if "PWM" got too far
      }
      else
      {
        if (f_valB >= 0.f) f_valB = 1.f;  // Convert osc1 to square
        if (f_valB < 0.f) f_valB = -1.f;  // Convert osc1 to square
      }
      f_val_result = f_valA * f_valB;   // "Ringmod"/AM instead of NAND-operation when smoothed
    }
    else  // Standard: square-wave based APC operation
      f_val_result = ((f_valA >= 0.f) & (f_valB < 0.f)) ?  1.f : -1.f;  // NAND operation => squarewave...

    // --- Adjust Mastervolume incl. Volume envelope and truncate in case of clipping ---
    f_val_result *= volume * smoothed_amp_factor;     // If we are in smoothed-operation-mode we amplify the volume just a bit...
    if( env_active )
      f_val_result *= vol_env.Process();            // Apply Volume Envelope if required
    if( f_val_result > 1.f)
      f_val_result = 1.f;
    if( f_val_result < -1.f)
      f_val_result = -1.f;

    // --- Output of DSP-results ---
    data.buf[i * 2 + processCh] = f_val_result;
  }
}

void ctagSoundProcessorAPCpp::Init(std::size_t blockSize, void *blockPtr)
{
  // construct internal data model
  knowYourself();
  model = std::make_unique<ctagSPDataModel>(id, isStereo);
  LoadPreset(0);

  // --- Initialize Oscillators and LFOs ---
  osc_1.SetSampleRate(44100.f);
  osc_1.SetFrequency(120.f);
  pwm_1.SetSampleRate(44100.f);
  pwm_1.SetFrequency(6.f);

  osc_2.SetSampleRate(44100.f);
  osc_2.SetFrequency(440.f);
  pwm_2.SetSampleRate(44100.f);
  pwm_2.SetFrequency(6.f);

  fm_1.SetSampleRate(44100.f);
  fm_1.SetFrequency(6.f);
  fm_2.SetSampleRate(44100.f);
  fm_2.SetFrequency(6.f);

  // --- Initialize Volume Envelope ---
  vol_env.SetSampleRate(44100.f);    // Sync Env with our audio-processing
  vol_env.SetModeExp();                   // Logarithmic scaling
}

ctagSoundProcessorAPCpp::~ctagSoundProcessorAPCpp()
{
}

void ctagSoundProcessorAPCpp::knowYourself(){
    // autogenerated code here
    // sectionCpp0
    pMapPar.emplace("MOD_freq_1", [&](const int val){ MOD_freq_1 = val;});
    pMapCv.emplace("MOD_freq_1", [&](const int val){ cv_MOD_freq_1 = val;});
    pMapPar.emplace("Freq_1", [&](const int val){ Freq_1 = val;});
    pMapCv.emplace("Freq_1", [&](const int val){ cv_Freq_1 = val;});
    pMapPar.emplace("MOD_active_1", [&](const int val){ MOD_active_1 = val;});
    pMapTrig.emplace("MOD_active_1", [&](const int val){ trig_MOD_active_1 = val;});
    pMapPar.emplace("MOD_is_PWM_1", [&](const int val){ MOD_is_PWM_1 = val;});
    pMapTrig.emplace("MOD_is_PWM_1", [&](const int val){ trig_MOD_is_PWM_1 = val;});
    pMapPar.emplace("SmoothOSC_1", [&](const int val){ SmoothOSC_1 = val;});
    pMapTrig.emplace("SmoothOSC_1", [&](const int val){ trig_SmoothOSC_1 = val;});
    pMapPar.emplace("SmoothOSC_2", [&](const int val){ SmoothOSC_2 = val;});
    pMapTrig.emplace("SmoothOSC_2", [&](const int val){ trig_SmoothOSC_2 = val;});
    pMapPar.emplace("MOD_is_PWM_2", [&](const int val){ MOD_is_PWM_2 = val;});
    pMapTrig.emplace("MOD_is_PWM_2", [&](const int val){ trig_MOD_is_PWM_2 = val;});
    pMapPar.emplace("MOD_active_2", [&](const int val){ MOD_active_2 = val;});
    pMapTrig.emplace("MOD_active_2", [&](const int val){ trig_MOD_active_2 = val;});
    pMapPar.emplace("Freq_2", [&](const int val){ Freq_2 = val;});
    pMapCv.emplace("Freq_2", [&](const int val){ cv_Freq_2 = val;});
    pMapPar.emplace("MOD_freq_2", [&](const int val){ MOD_freq_2 = val;});
    pMapCv.emplace("MOD_freq_2", [&](const int val){ cv_MOD_freq_2 = val;});
    pMapPar.emplace("Freqmod_amount_1", [&](const int val){ Freqmod_amount_1 = val;});
    pMapCv.emplace("Freqmod_amount_1", [&](const int val){ cv_Freqmod_amount_1 = val;});
    pMapPar.emplace("Freqmod_freq_1", [&](const int val){ Freqmod_freq_1 = val;});
    pMapCv.emplace("Freqmod_freq_1", [&](const int val){ cv_Freqmod_freq_1 = val;});
    pMapPar.emplace("FreqmodSquare_active_1", [&](const int val){ FreqmodSquare_active_1 = val;});
    pMapTrig.emplace("FreqmodSquare_active_1", [&](const int val){ trig_FreqmodSquare_active_1 = val;});
    pMapPar.emplace("FreqmodSquare_active_2", [&](const int val){ FreqmodSquare_active_2 = val;});
    pMapTrig.emplace("FreqmodSquare_active_2", [&](const int val){ trig_FreqmodSquare_active_2 = val;});
    pMapPar.emplace("Freqmod_freq_2", [&](const int val){ Freqmod_freq_2 = val;});
    pMapCv.emplace("Freqmod_freq_2", [&](const int val){ cv_Freqmod_freq_2 = val;});
    pMapPar.emplace("Freqmod_amount_2", [&](const int val){ Freqmod_amount_2 = val;});
    pMapCv.emplace("Freqmod_amount_2", [&](const int val){ cv_Freqmod_amount_2 = val;});
    pMapPar.emplace("Vol_amount", [&](const int val){ Vol_amount = val;});
    pMapCv.emplace("Vol_amount", [&](const int val){ cv_Vol_amount = val;});
    pMapPar.emplace("Env_active", [&](const int val){ Env_active = val;});
    pMapTrig.emplace("Env_active", [&](const int val){ trig_Env_active = val;});
    pMapPar.emplace("Trigger_env", [&](const int val){ Trigger_env = val;});
    pMapTrig.emplace("Trigger_env", [&](const int val){ trig_Trigger_env = val;});
    pMapPar.emplace("Env_Attack", [&](const int val){ Env_Attack = val;});
    pMapCv.emplace("Env_Attack", [&](const int val){ cv_Env_Attack = val;});
    pMapPar.emplace("Env_Decay", [&](const int val){ Env_Decay = val;});
    pMapCv.emplace("Env_Decay", [&](const int val){ cv_Env_Decay = val;});
    pMapPar.emplace("Env_loop_active", [&](const int val){ Env_loop_active = val;});
    pMapTrig.emplace("Env_loop_active", [&](const int val){ trig_Env_loop_active = val;});
    isStereo = false;
    id = "APCpp";
    // sectionCpp0
}