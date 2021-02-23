/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020/2021 by Robert Manzke. All rights reserved.

(c) 2021 for the "Formantor"-Plugin by Mathias Brüssel
The idea of Formantor is to to have a simple yet fun to control synthesizer combining a Phase Distortion oscillator with a vowel-filter and an envelope.
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

// --- Rescale randome values from 0...1 to needed output ---
#define RESCALE_FLT_MIN_MAX(inname, out_min, out_max) (inname * (out_max-out_min)+out_min)

// --- Additional Macro for automated parameter evaluations ---
#define MK_TRIG_PAR(outname, inname) int outname = process_param_trig(data, trig_##inname, inname, e_##inname);
#define MK_GATE_PAR(outname, inname) bool outname = (bool)process_param_trig(data, trig_##inname, inname, e_##inname, 1);
#define MK_ADEG_PAR(outname, inname) int outname = process_param_trig(data, trig_##inname, inname, e_##inname, 2);

// --- Modify sine-wave for Squarewave/PWM or various modulations (including Pitch-Mod, Filter-Mod, Z-Scan and Vector-Modulation) ---
#define SINE_TO_SQUARE(sine_val)                      sine_val = (sine_val >= 0) ? 1.f : -1.f;


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
void ctagSoundProcessorFormantor::random_bp_filter_settings()
{
  for( int i=0; i<5; i++)   // Process all 5 formants
  {
    // --- Set Cutoff frequencies for 3 Bandpass filters for each formant ---
    f_CutOffXarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 0.2f, 0.4f);      // 0.4f, 0.8f);
    f_CutOffYarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 0.3f, 0.6f);
    f_CutOffZarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 0.5f, 0.8f);

    // --- Set Resonance for 3 Bandpass filters for each formant ---
    f_ResoXarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 3.f, 4.5f);
    f_ResoYarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 2.f, 4.f);       // 2.f, 4.5f);
    f_ResoZarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 3.5f, 5.f);

    // --- Set Volume for 3 Bandpass filters for each formant ---
    f_FltAmntXarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 0.8f, 1.4f);
    f_FltAmntYarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 0.6f, 1.2f); // 0.6f, 1.8f);
    f_FltAmntZarray[i] = RESCALE_FLT_MIN_MAX(rndVal.Process(), 0.9f, 1.8f);
  }
}

// --- Main processing function for Formantor ---
void ctagSoundProcessorFormantor::Process(const ProcessData &data)
{
// --- Main processing output ---
float f_val_result = 0.f;
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
  MK_TRIG_PAR(t_FormantRndNew, FormantRndNew);
  if( t_FormantRndNew == GATE_HIGH_NEW)
  {
    random_bp_filter_settings();  // ### experimental !!! to be optimized in logic later!
    // ### printf("new setting of random BP formants...\n");
  }
  MK_TRIG_PAR(t_FormantFilterOn, FormantFilterOn);

  float f_MasterPitch = (float)(MasterPitch+48);  // Range is -48...48 as "MIDI notes"...
  float f_MasterPitch_CV = f_MasterPitch;
  if (cv_MasterPitch != -1)                                 // Please note: we save the CV-assciated pitch here seperately, this may be excluded per Oscillator-Group via an option!
    f_MasterPitch_CV = f_MasterPitch+data.cv[cv_MasterPitch]*60.f;    // We expect "MIDI"-notes 0-59 for 5 octaves (5*12) which fit into 0...+5V with 1V/Oct logic!

  float f_current_note = f_MasterPitch_CV;
  int i_current_note = (int)f_current_note;

  MK_FLT_PAR_ABS(f_PDamount, PDamount, 4095.f, 1.f);

  MK_INT_PAR_ABS(i_FormantSelect, FormantSelect,  11.f);
  i_FormantSelect--;    // GUI shows values 1-5 for formants a,e,i,o,u, we use 0-4 internally!
  CONSTRAIN(i_FormantSelect, 0, 9);

  MK_TRIG_PAR(t_FormantLock, FormantLock );

  // --- Check for formant-selection via black keys ---
  MK_TRIG_PAR( t_BlackKeyLogic, BlackKeyLogic );
  if( t_BlackKeyLogic )    // We may encounter a black key for formant change
  {
    int my_formant = formant_trigger[i_current_note%24];    // We have max 10 formants, connected to 10 black keys, changing every 2 octaves...
    if( my_formant != -1)     // We found a new formant via a black key
    {
      formant_selected = my_formant;
      i_current_note = i_note_save;   // We had a black key, so reset the current note to the one we remembered before.
      f_current_note = f_note_save;
    }
    else
    {
      i_note_save = i_current_note;   // We had no black key, so remember it for later!
      f_note_save = f_current_note;
    }
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
  // === Resonator (Resonant comb filter) ===
  MK_TRIG_PAR(t_ResCombOn, ResCombOn);
  MK_FLT_PAR_ABS(f_ResFreq, ResFreq, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_ResTone, ResTone, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_ResQ, ResQ, 4095.f, 1.f);

  // === Precalculation for realtime DSP loop ===
  // --- Find out what formant-related settings have to be made before main loop ---
  if( formant_selected > 4 )      // Random formants to be used
  {
    b_use_fix_formants = false;
    formant_selected -= 5;
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
      vowel_factor = 0.7f;   // Formant A is much louder, we lower the volume! Else, the factor simply will be 1.
    formant_filter_set_formant(formant_selected); // We set the selected formant to be used as member-variable for runtime-optimisation for main loop
  }
  // --- Set values for PD-synth ---
  Phasedist_real_controlChange(pd_data, 31, f_PDamount, 0);
  Phasedist_real_noteOn(pd_data, f_current_note, 110, 0);

  // === Realtime DSP output loop ===
  for(uint32_t i = 0; i < bufSz; i++)
  {
    f_val_result = Phasedist_real_process(pd_data,0);
    if( t_ResCombOn )
      f_val_result = Rescomb_process(rescomb_data, f_val_result, f_ResFreq, f_ResTone, f_ResQ);
    if( t_FormantFilterOn )
    {
      if( b_use_fix_formants )
        f_val_result = formant_filter(f_val_result) * vowel_factor;
      else
      {
        f_formant_x = Svf_process(svf_data_x, f_val_result, f_CutOffX, f_ResoX, 2) * f_FltAmntX;
        f_formant_y = Svf_process(svf_data_y, f_val_result, f_CutOffY, f_ResoY, 2) * f_FltAmntY;
        f_formant_z = Svf_process(svf_data_z, f_val_result, f_CutOffZ, f_ResoZ, 2) * f_FltAmntZ;
        f_val_result = f_formant_x + f_formant_y + f_formant_z;
      }
    }
    if( t_BlackKeyLogic )    // ### to be optimized later! ###
      f_val_result = Ladder_process(ladder_data, f_val_result, 0.8f, 3.5f );  // Ladder__ctx_type_8 &_ctx, float input, float cut, float res){

    f_val_result *= vol_eg_process * f_Volume;      // Apply AD or ADSR volume shaping to audio (is 1.0 if EG is inactive), adjust master-volume
    CONSTRAIN(f_val_result, -1.f, 1.f );
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

  // heap_caps_malloc()
  Rescomb_process_init(rescomb_data);

  Ladder_process_init(ladder_data);

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
	pMapPar.emplace("Volume", [&](const int val){ Volume = val;});
	pMapCv.emplace("Volume", [&](const int val){ cv_Volume = val;});
	pMapPar.emplace("PDamount", [&](const int val){ PDamount = val;});
	pMapCv.emplace("PDamount", [&](const int val){ cv_PDamount = val;});
	pMapPar.emplace("FormantFilterOn", [&](const int val){ FormantFilterOn = val;});
	pMapTrig.emplace("FormantFilterOn", [&](const int val){ trig_FormantFilterOn = val;});
	pMapPar.emplace("FormantRndNew", [&](const int val){ FormantRndNew = val;});
	pMapTrig.emplace("FormantRndNew", [&](const int val){ trig_FormantRndNew = val;});
	pMapPar.emplace("BlackKeyLogic", [&](const int val){ BlackKeyLogic = val;});
	pMapTrig.emplace("BlackKeyLogic", [&](const int val){ trig_BlackKeyLogic = val;});
	pMapPar.emplace("FormantLock", [&](const int val){ FormantLock = val;});
	pMapTrig.emplace("FormantLock", [&](const int val){ trig_FormantLock = val;});
	pMapPar.emplace("FormantSelect", [&](const int val){ FormantSelect = val;});
	pMapCv.emplace("FormantSelect", [&](const int val){ cv_FormantSelect = val;});
	pMapPar.emplace("ResCombOn", [&](const int val){ ResCombOn = val;});
	pMapTrig.emplace("ResCombOn", [&](const int val){ trig_ResCombOn = val;});
	pMapPar.emplace("ResFreq", [&](const int val){ ResFreq = val;});
	pMapCv.emplace("ResFreq", [&](const int val){ cv_ResFreq = val;});
	pMapPar.emplace("ResTone", [&](const int val){ ResTone = val;});
	pMapCv.emplace("ResTone", [&](const int val){ cv_ResTone = val;});
	pMapPar.emplace("ResQ", [&](const int val){ ResQ = val;});
	pMapCv.emplace("ResQ", [&](const int val){ cv_ResQ = val;});
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
	isStereo = false;
	id = "Formantor";
	// sectionCpp0
}