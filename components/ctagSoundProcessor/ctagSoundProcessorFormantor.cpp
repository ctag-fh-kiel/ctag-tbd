/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020/2021 by Robert Manzke. All rights reserved.

(c) 2021 for the "Formantor"-Plugin by Mathias Brüssel
The idea of Formantor is to to have a simple yet fun to control synthesizer combining a Phase Distortion oscillator with a vowel-filter and an envelope.
There are different options for playing and controlling the formant filters, for more details please look here:
https://docs.google.com/document/d/1c8mjxWjdiJNP0xpkU2CxRUp9av6V4W39wARJf3_SMSo
Formantor uses a Phase Distortion synth by Carlos Laguna Ruiz implemented in his VULT language, to be found here:
https://github.com/modlfo/teensy-vult-example
The code originally was intended as an add-on to the Teensy Audio library and got modified to be used with the TBD along with other VULT-examples
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
#define GATE_LOW_NEW        -1

// --- Replace function-call of frequency-conversion with macro for increasing speed just a bit ---
#define noteToFreq(incoming_note) (HELPERS::fastpow2 ((incoming_note - 69.f) / 12.f) *440.f)

// --- Additional Macro for automated parameter evaluations ---
#define MK_TRIG_PAR(outname, inname) int outname = process_param_trig(data, trig_##inname, inname, e_##inname);

// --- Modify sine-wave for Squarewave/PWM or various modulations (including Pitch-Mod, Filter-Mod, Z-Scan and Vector-Modulation) ---
#define SINE_TO_SQUARE(sine_val)                      sine_val = (sine_val >= 0) ? 1.f : -1.f;

// --- VULT "Library for TBD" ---
#include "vult_formantor.cpp"
#include "vultin.cpp"


// --- Process trigger signals and keep their state internally ---
inline int ctagSoundProcessorFormantor::process_param_trig(const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id )
{
  if(trig_myparm != -1)       // Trigger given via CV/Gate or button?
  {
    if((!data.trig[trig_myparm]) != prev_trig_state[prev_trig_state_id])    // Statuschange from HIGH to LOW or LOW to HIGH?
    {
      prev_trig_state[prev_trig_state_id] = !data.trig[trig_myparm];       // Remember status
      if (data.trig[trig_myparm] == 0)                      // HIGH if 0
        return (GATE_HIGH_NEW);          // New trigger
      else
        return (GATE_LOW_NEW);           // Trigger released
    }
  }
  else                        // We may have a trigger set by activating the button via the GUI
  {
    if (my_parm != prev_trig_state[prev_trig_state_id])    // Statuschange from HIGH to LOW or LOW to HIGH?
    {
      prev_trig_state[prev_trig_state_id] = my_parm;       // Remember status
      if(my_parm != 0)                   // LOW if 0
        return (GATE_HIGH_NEW);          // New trigger
      else
        return (GATE_LOW_NEW);           // Trigger released
    }
  }
  return(prev_trig_state[prev_trig_state_id]);            // No change (1 for active, 0 for inactive)
}

// --- Main processing function for Formantor ---
void ctagSoundProcessorFormantor::Process(const ProcessData &data)
{
  float f_val_result = 0.f;
  float f_formant_l = 0.f; // one formant lower to current
  float f_formant_h = 0.f; // one formant higher to current

  // === Global section ===
  MK_TRIG_PAR(t_Gate, Gate);
  MK_FLT_PAR_ABS(f_Volume, Volume, 4095.f, 1.f);

  // === Voice section ===
  MK_TRIG_PAR(t_FormantFilterOn, FormantFilterOn);

  float f_MasterPitch = MasterPitch;  // Range is -48...48 as "MIDI notes"...
  float f_MasterPitch_CV = f_MasterPitch;
  if (cv_MasterPitch != -1)                                 // Please note: we save the CV-assciated pitch here seperately, this may be excluded per Oscillator-Group via an option!
    f_MasterPitch_CV = f_MasterPitch+data.cv[cv_MasterPitch] * 60.f;    // We expect "MIDI"-notes 0-59 for 5 octaves (5*12) which fit into 0...+5V with 1V/Oct logic!

  float current_note = f_MasterPitch_CV+48.f;
  int current_note_int = (int)current_note;

  MK_FLT_PAR_ABS(f_PDamount, PDamount, 4095.f, 1.f);

  MK_INT_PAR_ABS(i_FormantSelect, FormantSelect,  6.f);
  i_FormantSelect--;    // GUI shows values 1-5 for formants a,e,i,o,u, we use 0-4 internally!
  CONSTRAIN(i_FormantSelect, 0, 4);

  MK_TRIG_PAR(t_FormantLock, FormantLock );

  // --- Check for formant-selection via black keys ---
  MK_TRIG_PAR( t_BlackKeyLogic, BlackKeyLogic );
  if( t_BlackKeyLogic )    // We may encounter a black key for formant change
  {
    int my_formant = formant_trigger[current_note_int%12];
    if( my_formant != -1)     // We found a new formant via a black key
    {
      formant_selected = my_formant;
      current_note_int = note_save;   // We had a black key, so reset the current note to the one we remembered before.
    }
    else
      note_save = current_note_int;   // We had no black key, so remember it for later!
  }
  else    // No black key logic
    formant_selected = i_FormantSelect;   // Take formants from the slider / CV instead

  // --- If active: only allow new formants when a new note is triggered! ---
  if( t_FormantLock )
  {
    if( t_Gate == GATE_HIGH_NEW )
      i_FormantSelect_save = formant_selected;   // The new formant will be used now with the newly triggered note
    else
      formant_selected = i_FormantSelect_save;   // We reset the selected formant to the one that had been saved when the last note got triggered
  }
  else
    i_FormantSelect_save = formant_selected;     // We save the current formant in case formant-selection gets locked to triggering lateron!


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

      if (t_Gate > 0)      // values range from -1...+2
        vol_eg_adsr.Gate(true);
      else
        vol_eg_adsr.Gate(false);
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

  // --- Precalculation for realtime DSP loop ---
  Phasedist_real_controlChange(pd_data, 31, f_PDamount, 0);
  Phasedist_real_noteOn(pd_data, current_note, 110, 0);

  int vowel_id = formant_selected;  // values 1-5 on the GUI and maybe 0 or 1-n from a keyboard


  // --- Realtime DSP output loop ---
  for(uint32_t i = 0; i < bufSz; i++)
  {
    f_val_result = Phasedist_real_process(pd_data,0);
    if( t_FormantFilterOn )
      f_val_result = Rescomb_process(rescomb_data, f_val_result, 0.5, 0.4f, 0.6f); // Pitch, Tone, Resonance
      ; // To be implemented ###
    
    f_val_result *= vol_eg_process;                 // Apply AD or ADSR volume shaping to audio (is 1.0 if EG is inactive)
    data.buf[i * 2 + processCh] = f_val_result;     // Mono channel output for plugin in slot 1 and/or in slot 2
  }
}

// --- Formantor Constructor ---
ctagSoundProcessorFormantor::ctagSoundProcessorFormantor()
{
  // construct internal data model
  knowYourself();
  model = std::make_unique<ctagSPDataModel>(id, isStereo);
  LoadPreset(0);

  // --- VULT ---
  Phasedist_real_process_init(pd_data);
  Phasedist_real_default(pd_data);              // Enable default settings for PD-Synth

  Rescomb_process_init(rescomb_data);

  // --- Initialize Volume Envelope ---
  vol_eg_ad.SetSampleRate(44100.f/ bufSz);    // Sync Env with our audio-processing
  vol_eg_ad.SetModeExp();  // Logarithmic scaling
  vol_eg_adsr.SetSampleRate(44100.f/ bufSz);    // Optional ADSR-EG: sync Env with our audio-processing
  vol_eg_adsr.SetModeExp();                     // Logarithmic scaling
  vol_eg_adsr.Reset();
}

// --- Formantor Destructor ---
ctagSoundProcessorFormantor::~ctagSoundProcessorFormantor()
{
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
	pMapPar.emplace("Volume", [&](const int val){ Volume = val;});
	pMapCv.emplace("Volume", [&](const int val){ cv_Volume = val;});
	pMapPar.emplace("PDamount", [&](const int val){ PDamount = val;});
	pMapCv.emplace("PDamount", [&](const int val){ cv_PDamount = val;});
	pMapPar.emplace("FormantFilterOn", [&](const int val){ FormantFilterOn = val;});
	pMapTrig.emplace("FormantFilterOn", [&](const int val){ trig_FormantFilterOn = val;});
	pMapPar.emplace("BlackKeyLogic", [&](const int val){ BlackKeyLogic = val;});
	pMapTrig.emplace("BlackKeyLogic", [&](const int val){ trig_BlackKeyLogic = val;});
	pMapPar.emplace("FormantLock", [&](const int val){ FormantLock = val;});
	pMapTrig.emplace("FormantLock", [&](const int val){ trig_FormantLock = val;});
	pMapPar.emplace("FormantSelect", [&](const int val){ FormantSelect = val;});
	pMapCv.emplace("FormantSelect", [&](const int val){ cv_FormantSelect = val;});
	pMapPar.emplace("TremoloActive", [&](const int val){ TremoloActive = val;});
	pMapTrig.emplace("TremoloActive", [&](const int val){ trig_TremoloActive = val;});
	pMapPar.emplace("TremoloAfterFormant", [&](const int val){ TremoloAfterFormant = val;});
	pMapTrig.emplace("TremoloAfterFormant", [&](const int val){ trig_TremoloAfterFormant = val;});
	pMapPar.emplace("TremoloAttack", [&](const int val){ TremoloAttack = val;});
	pMapCv.emplace("TremoloAttack", [&](const int val){ cv_TremoloAttack = val;});
	pMapPar.emplace("TremoloSpeed", [&](const int val){ TremoloSpeed = val;});
	pMapCv.emplace("TremoloSpeed", [&](const int val){ cv_TremoloSpeed = val;});
	pMapPar.emplace("TremoloAmount", [&](const int val){ TremoloAmount = val;});
	pMapCv.emplace("TremoloAmount", [&](const int val){ cv_TremoloAmount = val;});
	pMapPar.emplace("EGvolActive", [&](const int val){ EGvolActive = val;});
	pMapTrig.emplace("EGvolActive", [&](const int val){ trig_EGvolActive = val;});
	pMapPar.emplace("EGvolSlow", [&](const int val){ EGvolSlow = val;});
	pMapTrig.emplace("EGvolSlow", [&](const int val){ trig_EGvolSlow = val;});
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
	isStereo = true;
	id = "Formantor";
	// sectionCpp0
}