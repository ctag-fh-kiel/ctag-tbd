/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020 by Robert Manzke. All rights reserved.

(c) 2021 for the "RecNplay"-Plugin by Mathias Brüssel
RecNplay is a simple synth combined with an optional minimalistic sequencer that allows recording of steps via realtime- or step-by-step recording.
For each step recorded all current parameter-settings of the synth will be stored. Direct play or step playback can make use of an Attack/Decay envelope.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include <tbd/sounds/ctagSoundProcessorRecNPlay.hpp>

#include <stmlib/stmlib.h>

using namespace CTAG::SP;

// --- Trigger/Gate values ---
#define GATE_HIGH_NEW       2
#define GATE_HIGH           1
#define GATE_LOW            0

#define MAX_CUTOFF_FREQ     14000.f

#define EG_ATTACK_MAX       8.f
#define EG_DECAY_MAX        20.f

// --- Additional macros for oscillator and GUI-parameter processing ---
#define MK_PITCH_PAR(outname, inname)     float outname = inname; if(cv_##inname != -1) outname += data.cv[cv_##inname]*60.f;

#define MK_TRIG_PAR(outname, inname) int outname = process_param_trig(data, trig_##inname, inname, e_##inname);
#define MK_GATE_PAR(outname, inname) bool outname = (bool)process_param_trig(data, trig_##inname, inname, e_##inname, 1);
#define MK_ADEG_PAR(outname, inname) int outname = process_param_trig(data, trig_##inname, inname, e_##inname, 2);

#define MK_INT_CONSTRAIN_1(outname, inname, factor); MK_INT_PAR_ABS(outname, inname, (float)(factor+1)); CONSTRAIN(outname, 1, factor);
#define MK_INT_CONSTRAIN_0(outname, inname, factor); MK_INT_PAR_ABS(outname, inname, (float)(factor+1)); CONSTRAIN(outname, 0, factor);

// --- Process trigger signals and keep their state internally ---
inline int ctagSoundProcessorRecNPlay::process_param_trig(const ProcessData &data, int trig_myparm, int my_parm, int enum_trigger_id, int gate_type = 0 )
{
  int trig_status = 0;

  if(trig_myparm != -1)       // Trigger given via CV/Gate or button?
  {
    trig_status = (data.trig[trig_myparm]==0); // HIGH is 0, so we negate for boolean logic
    if( gate_type == 1 )    // Use Gate-signal
    {
      if(trig_status)
      {
        if(low_reached[enum_trigger_id])   // We have a positive gate and last time it was negative)
        {
          low_reached[enum_trigger_id] = false;
          return(GATE_HIGH_NEW);           // New trigger
        }
        else
          return (GATE_HIGH);
      }
      else
      {
        low_reached[enum_trigger_id] = true;
        return(GATE_LOW);
      }
    }
    // --- Non-Gate triggers, if we reach here: toggle-trigger or AD-Trigger ---
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

void ctagSoundProcessorRecNPlay::Process(const ProcessData &data)
{
float f_val_saw = 0.f;
float f_val_result = 0.f;

  // === Process all parameters from the GUI ===
  // --- VolEG ---
  MK_FLT_PAR_ABS(f_Volume, Volume, 4095.f, 2.f);   // We use a high value to have headroom, so lower the volume normally
  MK_TRIG_PAR(t_EGactiv, EGactiv);
  MK_FLT_PAR_ABS(f_Attack, Attack, 4095.f, EG_ATTACK_MAX);
  MK_FLT_PAR_ABS(f_Decay, Decay, 4095.f, EG_DECAY_MAX);
  MK_ADEG_PAR(t_StepAndEGtrigger, StepAndEGtrigger);

  // --- Voice ---
  MK_FLT_PAR_ABS(f_VoiceVol, VoiceVol, 4095.f, 1.f);
  MK_PITCH_PAR(f_PitchSaw, PitchSaw);
  MK_FLT_PAR_ABS(f_ExtSawMix, ExtSawMix, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SawNoiseMix, SawNoiseMix, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_Cutoff, Cutoff, 4095.f, MAX_CUTOFF_FREQ);
  MK_FLT_PAR_ABS_MIN_MAX(f_Resonance, Resonance, 4095.f, 0.0002f, 1.99f);
  MK_FLT_PAR_ABS_MIN_MAX(f_Drive, Drive, 4095.f, 0.899f, 8.99f);

  // --- FX ---
  MK_INT_CONSTRAIN_0(i_BitsToCrush, BitsToCrush, 16);
  MK_FLT_PAR_ABS(f_DownsampleFactor, DownsampleFactor, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SnHamount, SnHamount, 4095.f, 1.f);

  // --- Stepper ---
  MK_TRIG_PAR(t_StepperActive, StepperActive);
  MK_TRIG_PAR(t_RecPlay, RecPlay);
  MK_INT_CONSTRAIN_1(i_CurrentStep, CurrentStep, 32);
  MK_INT_CONSTRAIN_1(i_NumberOfSteps, NumberOfSteps, 32);
  MK_FLT_PAR_ABS(f_BPMint, BPMint, 19200.f, 1.f);

  // === Process stepper including memorizing steps if needed ===
  if(t_StepperActive)
  {
    // --- Check if we have to change the "current step" (used to "overdub" current recording or modify playback sequence) ---
    if( i_CurrentStep != current_step_mem_ )
    {
      step_id = i_CurrentStep-1;  // We count from 1 to n externally and from 0 to n-1 internally
      CONSTRAIN(step_id, 0, i_NumberOfSteps-1);
      current_step_mem_ = i_CurrentStep;
    }
    // --- Record current step if recording is activated for it ---
    if( t_RecPlay == GATE_LOW )  // RecordingMode => We process with Gate-Logic, so we don't miss anything, if it is actually activated after the first trigger for the next step
    {
      if (last_step_mode_play) // We increment by one below and so start over again with index one on record
      {
        last_step_mode_play = false;
        step_id = 0;
      }
      // --- Next Step to record? ---
      if(t_StepAndEGtrigger == GATE_HIGH_NEW)
      {
        stepMem[step_id][s_VoiceVol] = f_VoiceVol;
        stepMem[step_id][s_PitchSaw] = f_PitchSaw;
        stepMem[step_id][s_ExtSawMix] = f_ExtSawMix;
        stepMem[step_id][s_SawNoiseMix] = f_SawNoiseMix;
        stepMem[step_id][s_Cutoff] = f_Cutoff;
        stepMem[step_id][s_Resonance] = f_Resonance;
        stepMem[step_id][s_Drive] = f_Drive;
      }
    }
    else // --- Playback mode ---
    {
      // --- Check if we have to reset our step-sequence ---
      if (t_RecPlay == GATE_HIGH_NEW) // We increment by one below and so start over again with index one on play
      {
        last_step_mode_play = true;   // If we change to recorde-mode again, we know "where we came from", then ;-)
        step_id = 0;
        metro.SetFrequencyPhase(f_BPMint*32.f, 0.f);  // Reset oscillator for sequencer
      }
      // --- Apply metro if active and count Vibration-changes to generate trigger internally if needed ---
      if( f_BPMint > 0.f )
      {
        if( f_BPMint != f_BPMint_mem)
          metro.SetFrequency(f_BPMint*32.f);      // (==1920.f/60.f equivalent to 1/32 at 60BPM)...
        f_BPMint_mem = f_BPMint;                        // ...We transfer "Triggers per Minute" to "Vibrations per Second", aka Frequency in Hz

        float metro_val = metro.Process();              // Get next sample from sinus to check for "rising edge"
        if( clockSinusWasLow && (metro_val >= 0.f) )    // We have a "rising edge"!
        {
          clockSinusWasLow = false;
          t_StepAndEGtrigger = GATE_HIGH_NEW;       // Generate a regular trigger, but internally
        }
        else                                        // We have a "miss"
          t_StepAndEGtrigger = GATE_LOW;            // Generate a regular "no trigger detected"-event, but internally

        if( metro_val < 0.f )                       // Falling edge?
          clockSinusWasLow = true;                  // Remember our frequency became negative to check for rising edge again with next round
      }
      // --- On trigger playback previously recorded step (volume may be 0 for muting) ---
      if (t_StepAndEGtrigger == GATE_HIGH_NEW)
      {
        f_VoiceVol_mem = stepMem[step_id][s_VoiceVol];
        f_PitchSaw_mem = stepMem[step_id][s_PitchSaw];
        f_ExtSawMix_mem = stepMem[step_id][s_ExtSawMix];
        f_SawNoiseMix_mem = stepMem[step_id][s_SawNoiseMix];
        f_Cutoff_mem = stepMem[step_id][s_Cutoff];
        f_Resonance_mem = stepMem[step_id][s_Resonance];
        f_Drive_mem = stepMem[step_id][s_Drive];
        // --- Increment to next step if triggered, with wraparound ---
      }
      // --- Restore previously buffered values for playback (we keep them until next trigger or returning to record or "direct play" ---
      f_VoiceVol = f_VoiceVol_mem;      // Because we use the same variables as those associated to the GUI-sliders or CV...
      f_PitchSaw = f_PitchSaw_mem;
      f_ExtSawMix = f_ExtSawMix_mem;
      f_SawNoiseMix = f_SawNoiseMix_mem;      // ...we have to overwrite those regular values with stepper-values we remembered with the last step!
      f_Cutoff = f_Cutoff_mem;
      f_Resonance = f_Resonance_mem;
      f_Drive = f_Drive_mem;
    }
    // --- Increment step on trigger for record _or_ playback of stepper ---
    if (t_StepAndEGtrigger == GATE_HIGH_NEW)
    {
      step_id++;
      step_id %= i_NumberOfSteps;
    }
  }
  // === Precalculate parameters for DSP-Stuff ===
  // --- Set EG ---
  vol_env.SetAttack(f_Attack);
  vol_env.SetDecay(f_Decay);

  // --- Sample and Hold for Decimator ---
  if( t_StepperActive && t_RecPlay )    // Stepper active in Play-Mode?
  {
    if (t_StepAndEGtrigger == GATE_HIGH_NEW)
    {

      f_DownsampleFactor += wNoiseSnH * f_SnHamount;  // Modulate downsampling-amount on trigger or step, according to S&H amount, we take a White Noise event we saved earlier already now
      CONSTRAIN(f_DownsampleFactor, 0.f, 1.f);
      SnHlatched_ = true;    // Hold Sample for SnH now!
      f_DownsampleFactorMem_ = f_DownsampleFactor;
    }
  }
  else                    // Stepper inactive or in Record-Mode!
    SnHlatched_ = false;    // No random BitCrushing intensity stored via SnH (anymore)

  // --- Decimator ---
  decimator.SetBitsToCrush(i_BitsToCrush);
  if(SnHlatched_)          // We remembered randomized BitCrushing-Intensity
    decimator.SetDownsampleFactor(f_DownsampleFactorMem_);
  else
    decimator.SetDownsampleFactor(f_DownsampleFactor);

  // --- Sequencer is was active or inactive, check if VOL-EG is active (alone) and maybe newly triggered, modify overall volume accordingly ---
  if (t_EGactiv)
  {
    if (t_StepAndEGtrigger == GATE_HIGH_NEW)
      vol_env.Trigger();

    f_Volume *= vol_env.Process();
  }
  // --- Calculate "left" part of balanced values (possible from restored values from stepper) ---
  float f_ExternalVol = 1.f - f_ExtSawMix;
  float f_SawVol = f_ExtSawMix - f_SawNoiseMix;
  CONSTRAIN(f_SawVol, 0.f, 1.f);
  float f_NoiseVol = f_SawNoiseMix;

  // --- Apply filter-parameters (may be set directly or via stepper if active) ---
  wpKorg35.SetSaturation(f_Drive);
  wpKorg35.SetCutoff(f_Cutoff);
  wpKorg35.SetResonance(f_Resonance);

  // --- Pitch ---
  f_PitchSaw /= 120.f;  // VULT-oscillators are scaled from 0.0 to 1.0 for values between "MIDI"-Note 0...127

  // === Main DSP-Loop ===
  float whiteNoise = 0.f;
  for (int i = 0; i < bufSz; i++)
  {
    // --- Calculate oscillators and mix them with external input ---
    whiteNoise = wNoise.Process();
    f_val_saw = Saw_eptr_process(saw_data, f_PitchSaw);
    f_val_result = whiteNoise*f_NoiseVol + f_val_saw*f_SawVol + data.buf[i*2+processCh]*f_ExternalVol;

    // --- Filter mix and apply bitcrusher if needed ---
    f_val_result = wpKorg35.Process(f_val_result)*f_VoiceVol;
    if( i_BitsToCrush > 0 )   // Use BitCrusher?
      data.buf[i * 2 + processCh] = decimator.Process(f_val_result) * f_Volume;
    else
      data.buf[i * 2 + processCh] = f_val_result * f_Volume;
  }
  // === Post-processing (of main loop) ===
  wNoiseSnH = whiteNoise; // We sample one White Noise event in case we need it to modulate the BitCrusher later on
}

void ctagSoundProcessorRecNPlay::Init(std::size_t blockSize, void *blockPtr)
{
    // construct internal data model
    knowYourself();
    model = std::make_unique<SoundProcessorParams>(id, isStereo);
    LoadPreset(0);

    // --- Inialize DSP-relevant objects ---
    metro.SetSampleRate(44100.f / bufSz);    // bufSz currently is 32 => 1378,125 Hz
    metro.SetFrequency(120.f / 60.f );       // equivalent to 2 Triggers per Second at 120 BPM

    Saw_eptr_process_init(saw_data);      // VULT Saw wave

    decimator.Init();
    memset(&stepMem, 0, sizeof(stepMem));
    vol_env.SetSampleRate(44100.f / bufSz);   // We calculate the EG outside the main loop already, so we have a reduced refresh-rate!
}

ctagSoundProcessorRecNPlay::~ctagSoundProcessorRecNPlay()
{
}

void ctagSoundProcessorRecNPlay::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("Volume", [&](const int val){ Volume = val;});
	pMapCv.emplace("Volume", [&](const int val){ cv_Volume = val;});
	pMapPar.emplace("EGactiv", [&](const int val){ EGactiv = val;});
	pMapTrig.emplace("EGactiv", [&](const int val){ trig_EGactiv = val;});
	pMapPar.emplace("Attack", [&](const int val){ Attack = val;});
	pMapCv.emplace("Attack", [&](const int val){ cv_Attack = val;});
	pMapPar.emplace("Decay", [&](const int val){ Decay = val;});
	pMapCv.emplace("Decay", [&](const int val){ cv_Decay = val;});
	pMapPar.emplace("StepAndEGtrigger", [&](const int val){ StepAndEGtrigger = val;});
	pMapTrig.emplace("StepAndEGtrigger", [&](const int val){ trig_StepAndEGtrigger = val;});
	pMapPar.emplace("VoiceVol", [&](const int val){ VoiceVol = val;});
	pMapCv.emplace("VoiceVol", [&](const int val){ cv_VoiceVol = val;});
	pMapPar.emplace("PitchSaw", [&](const int val){ PitchSaw = val;});
	pMapCv.emplace("PitchSaw", [&](const int val){ cv_PitchSaw = val;});
	pMapPar.emplace("ExtSawMix", [&](const int val){ ExtSawMix = val;});
	pMapCv.emplace("ExtSawMix", [&](const int val){ cv_ExtSawMix = val;});
	pMapPar.emplace("SawNoiseMix", [&](const int val){ SawNoiseMix = val;});
	pMapCv.emplace("SawNoiseMix", [&](const int val){ cv_SawNoiseMix = val;});
	pMapPar.emplace("Cutoff", [&](const int val){ Cutoff = val;});
	pMapCv.emplace("Cutoff", [&](const int val){ cv_Cutoff = val;});
	pMapPar.emplace("Resonance", [&](const int val){ Resonance = val;});
	pMapCv.emplace("Resonance", [&](const int val){ cv_Resonance = val;});
	pMapPar.emplace("Drive", [&](const int val){ Drive = val;});
	pMapCv.emplace("Drive", [&](const int val){ cv_Drive = val;});
	pMapPar.emplace("BitsToCrush", [&](const int val){ BitsToCrush = val;});
	pMapCv.emplace("BitsToCrush", [&](const int val){ cv_BitsToCrush = val;});
	pMapPar.emplace("DownsampleFactor", [&](const int val){ DownsampleFactor = val;});
	pMapCv.emplace("DownsampleFactor", [&](const int val){ cv_DownsampleFactor = val;});
	pMapPar.emplace("SnHamount", [&](const int val){ SnHamount = val;});
	pMapCv.emplace("SnHamount", [&](const int val){ cv_SnHamount = val;});
	pMapPar.emplace("StepperActive", [&](const int val){ StepperActive = val;});
	pMapTrig.emplace("StepperActive", [&](const int val){ trig_StepperActive = val;});
	pMapPar.emplace("RecPlay", [&](const int val){ RecPlay = val;});
	pMapTrig.emplace("RecPlay", [&](const int val){ trig_RecPlay = val;});
	pMapPar.emplace("CurrentStep", [&](const int val){ CurrentStep = val;});
	pMapCv.emplace("CurrentStep", [&](const int val){ cv_CurrentStep = val;});
	pMapPar.emplace("NumberOfSteps", [&](const int val){ NumberOfSteps = val;});
	pMapCv.emplace("NumberOfSteps", [&](const int val){ cv_NumberOfSteps = val;});
	pMapPar.emplace("BPMint", [&](const int val){ BPMint = val;});
	pMapCv.emplace("BPMint", [&](const int val){ cv_BPMint = val;});
	isStereo = false;
	id = "RecNPlay";
	// sectionCpp0
}