/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020/2021 by Robert Manzke. All rights reserved.

(c) 2021 for the "SpaceFX"-Plugin by Mathias Brüssel
The idea of SpaceFX is to have a simple yet flexible to control white noise and modulated filter-selfoscillation generator.
SpaceFX among other things tries to pay tribute to early space-rock bands like Gong, Hawkwind, Far east family band and others.
The main inspiration being Tim Blake's usage of the VCS 3 for such "space noises". But also more modern ambient and glitch-like textures are possible!
The plugin uses a diode-ladder filter as well as a sawwave oscillator by Carlos Laguna Ruiz implemented in his VULT language,
for more details on the topic please look here: https://github.com/modlfo/vult

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
#include "ctagSoundProcessorSpaceFX.hpp"
#include "plaits/dsp/engine/engine.h"

// --- VULT "Library for TBD" ---
// #include "./vult/vult_formantor.cpp"  // Already defined in ctagSoundProcessorFormantor, so to avoid double defines we omit it here!
// #include "./vult/vultin.cpp"          // Already defined in ctagSoundProcessorFormantor, so to avoid double defines we omit it here!

using namespace CTAG::SP;

// --- Trigger/Gate values ---
#define GATE_HIGH_NEW       2
#define GATE_HIGH           1
#define GATE_LOW            0

// --- Frequency Ranges for LFOs that can be "pitch-shifted" ---
#define PULSE_MIN_FREQ      0.05f
#define PULSE_MAX_FREQ      60.f
#define SINE_MIN_FREQ       0.05f
#define SINE_MAX_FREQ       60.f
#define SQUARE_MIN_FREQ     0.05f
#define SQUARE_MAX_FREQ     100.f

// --- Additional macros for oscillator and GUI-parameter processing ---
#define SINE_TO_SQUARE(sine_val)                sine_val = (sine_val >= 0) ? 1.f : -1.f;
#define MK_PITCH_PAR(outname, inname)           float outname = inname; if(cv_##inname != -1) outname += data.cv[cv_##inname]*60.f;
#define MK_TRIG_PAR(outname, inname)            int outname = process_param_trig(data, trig_##inname, inname, e_##inname);
#define MK_ADEG_PAR(outname, inname)            int outname = process_param_trig(data, trig_##inname, inname, e_##inname, 2);
#define MK_GATE_PAR(outname, inname, gate_var)  bool outname = (bool)process_param_trig(data, trig_##inname, inname, gate_var, 1);
#define MK_FLT_PAR_HYSTERESIS(outname, inname, hysteresis_memory, norm, scale) \
        float outname = inname / norm * scale; \
        if(cv_##inname != -1) { outname = 0.05f * fabsf(data.cv[cv_##inname])*scale + 0.95f * hysteresis_memory; \
        hysteresis_memory = outname; }

// --- Process trigger signals and keep their state internally ---
inline int ctagSoundProcessorSpaceFX::process_param_trig(const ProcessData &data, int trig_myparm, int my_parm, int enum_trigger_id, int gate_type = 0 )
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

// --- Plugin runtime entry point (process CV/Gate and audio) ---
void ctagSoundProcessorSpaceFX::Process(const ProcessData &data) 
{
float f_val_result = 0.f;             // Sum of results for DSP-output
float f_val_saw = 0.f;                // Saw wave

  // === Get settings from GUI and/or CV/Gate (pots/buttons) ===
  // --- Global ---
  MK_ADEG_PAR(t_Trigger, Trigger);            // We may have a new trigger
  MK_GATE_PAR(t_Gate, Trigger, e_Gate);       // Or a gate if ADSR EG
  MK_PITCH_PAR(f_MasterPitch, MasterPitch);
  MK_FLT_PAR_ABS_SFT(f_MasterTune, MasterTune, 1200.f, 1.f);
  MK_FLT_PAR_ABS_SFT(f_FilterTracking, FilterTracking, 2047.f, 1.f);
  float f_current_note_to_filter = (f_MasterPitch+f_MasterTune)/60.f*f_FilterTracking;   // Calculate a value to "play" the resonating filter by keys
  MK_FLT_PAR_ABS(f_Volume, Volume, 4095.f, 4.f);   // We use a high value to have headroom, so lower the volume normally

  // --- Voice / Noise section ---
  MK_TRIG_PAR(t_EnableNoise, EnableNoise);
  MK_FLT_PAR_ABS(f_NoiseColour, NoiseColour, 4095.f, 1.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_MixedNoiseLevel, MixedNoiseLevel, 4095.f, 0.0002f, 3.5f); // Avoid stopping of filter-selfoscillation by at least low noise
  MK_TRIG_PAR(t_EnableNoiseMod, EnableNoiseMod);
  MK_FLT_PAR_ABS_MIN_MAX(f_NoiseColourModSpeed, NoiseColourModSpeed, 4095.f, 0.05f, 100.f);
  MK_FLT_PAR_ABS(f_NoiseColourModAmount, NoiseColourModAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_NoiseLevelModSpeed, NoiseLevelModSpeed, 4095.f, 0.05f, 100.f);
  MK_FLT_PAR_ABS(f_NoiseLevelModAmount, NoiseLevelModAmount, 4095.f, 1.f);

  // --- Voice / Filter section ---
  MK_TRIG_PAR(t_PitchToCutoff, PitchToCutoff);
  MK_FLT_PAR_ABS(f_Cutoff, Cutoff, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_Resonance, Resonance, 4095.f, 5.f);
  MK_TRIG_PAR(t_VintageResonance, VintageResonance);
  MK_FLT_PAR_ABS(f_CutoffModulationAmount, CutoffModulationAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_ResonanceModulationAmount, ResonanceModulationAmount, 4095.f, 5.f);
  MK_FLT_PAR_ABS(f_AmplitudeModulationAmount, AmplitudeModulationAmount, 4095.f, 1.f);

  // --- AD Envelope section ---
  MK_TRIG_PAR(t_ADenvEnable, ADenvEnable);
  MK_TRIG_PAR(t_ADenvToVCA, ADenvToVCA);
  MK_TRIG_PAR(t_ADenvNegative, ADenvNegative);
  MK_FLT_PAR_ABS(f_ADattack, ADattack, 4095.f, 8.f);
  MK_FLT_PAR_ABS(f_ADdecay, ADdecay, 4095.f, 25.f);
  MK_TRIG_PAR(t_ADegLoop, ADegLoop);
  MK_FLT_PAR_ABS_SFT(f_ADenvelopeCutoffAmount, ADenvelopeCutoffAmount, 2047.f, 1.f);
  MK_FLT_PAR_ABS_SFT(f_ADenvelopeResonanceAmount, ADenvelopeResonanceAmount, 2047.f, 1.f);
  MK_TRIG_PAR(t_ADtoSawLFOfrequ, ADtoSawLFOfrequ);
  MK_FLT_PAR_ABS_SFT(f_ADtoSawSpeedLevel, ADtoSawSpeedLevel, 2047.f, 1.f);

  // --- ADSR Envelope section ---
  MK_TRIG_PAR(t_ADSRenvEnable, ADSRenvEnable);
  MK_TRIG_PAR(t_ADSRenvToVCA, ADSRenvToVCA);
  MK_TRIG_PAR(t_ADSRenvNegative, ADSRenvNegative);
  MK_FLT_PAR_ABS(f_ADSRattack, ADSRattack, 4095.f, 8.f);
  MK_FLT_PAR_ABS(f_ADSRdecay, ADSRdecay, 4095.f, 10.f);
  MK_FLT_PAR_ABS(f_ADSRsustain, ADSRsustain, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_ADSRrelease, ADSRrelease, 4095.f, 20.f);
  MK_FLT_PAR_ABS_SFT(f_ADSRenvelopeCutoffAmount, ADSRenvelopeCutoffAmount, 2047.f, 1.f);
  MK_FLT_PAR_ABS_SFT(f_ADSRenvelopeResonanceAmount, ADSRenvelopeResonanceAmount, 2047.f, 1.f);
  MK_TRIG_PAR(t_ADSRtoSineLFOfrequ, ADSRtoSineLFOfrequ);
  MK_FLT_PAR_ABS_SFT(f_ADSRtoSineSpeedLevel, ADSRtoSineSpeedLevel, 2047.f, 1.f);

  // --- SOU Mixer section ---
  MK_TRIG_PAR(t_SnH1Enable, SnH1Enable);
  MK_FLT_PAR_ABS_MIN_MAX(f_SnH1Freq, SnH1Freq, 4095.f, 0.5f, 200.f);
  MK_FLT_PAR_ABS(f_SnH1CutoffLevel, SnH1CutoffLevel, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SnH1ResonanceLevel, SnH1ResonanceLevel, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SnH1AMlevel, SnH1AMlevel, 4095.f, 1.f);
  MK_FLT_PAR_ABS_SFT(f_SnH1FrequSnH2Change, SnH1FrequSnH2Change, 2047.f, 1.f);
  MK_FLT_PAR_ABS_SFT(f_SnH1FrequSquareChange, SnH1FrequSquareChange, 2047.f, 1.f);
  MK_FLT_PAR_ABS_SFT(f_SnH1ADattackChange, SnH1ADattackChange, 2047.f, 1.f);

  MK_TRIG_PAR(t_SnH2Enable, SnH2Enable);
  MK_FLT_PAR_ABS_MIN_MAX(f_SnH2Freq, SnH2Freq, 4095.f, 0.5f, 200.f);
  MK_FLT_PAR_ABS(f_SnH2CutoffLevel, SnH2CutoffLevel, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SnH2ResonanceLevel, SnH2ResonanceLevel, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SnH2AMlevel, SnH2AMlevel, 4095.f, 1.f);
  MK_FLT_PAR_ABS_SFT(f_SnH2FrequSnH1Change, SnH2FrequSnH1Change, 2047.f, 1.f);
  MK_FLT_PAR_ABS_SFT(f_SnH2FrequPulseChange, SnH2FrequPulseChange, 2047.f, 1.f);
  MK_FLT_PAR_ABS_SFT(f_SnH2ADdecayChange, SnH2ADdecayChange, 2047.f, 1.f);

  // --- LFO Modulation Mixer section ---
  MK_TRIG_PAR(t_SineEnable, SineEnable);
  MK_FLT_PAR_ABS_MIN_MAX(f_SineFreq, SineFreq, 4095.f, SINE_MIN_FREQ, SINE_MAX_FREQ);
  MK_FLT_PAR_ABS(f_SineCutoffAmount, SineCutoffAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SineResonanceAmount, SineResonanceAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SineAMamount, SineAMamount, 4095.f, 1.f);

  MK_TRIG_PAR(t_PulseEnable, PulseEnable);
  MK_FLT_PAR_ABS_MIN_MAX(f_PulseFreq, PulseFreq, 4095.f, PULSE_MIN_FREQ, PULSE_MAX_FREQ);
  MK_FLT_PAR_ABS(f_PulseWidth, PulseWidth, 4095.f, 0.248f);   // This is an odd value we found via trial-and-error to avoid possible overdrive during PWM resulting in no modulation at all
  MK_FLT_PAR_ABS_MIN_MAX(f_PWMspeed, PWMspeed, 4095.f, 0.005f, 10.f); // This has to be very slow, because we modulate the PWM of an LFO here!
  MK_FLT_PAR_ABS(f_PWMamount, PWMamount, 4095.f, 0.9f);       // We limit the max. PWM just a bit to avoid overdrive, resulting in no signal at all
  MK_FLT_PAR_ABS(f_PulseCutoffAmount, PulseCutoffAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_PulseResonanceAmount, PulseResonanceAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_PulseAMamount, PulseAMamount, 4095.f, 1.f);

  MK_TRIG_PAR(t_SawEnable, SawEnable);
  MK_FLT_PAR_ABS_MIN_MAX(f_SawFreq, SawFreq, 4095.f, 0.01f, 1.f);     // Vult Oscillators are scaled from 0 to 1 for Notes 0...120
  MK_FLT_PAR_ABS(f_SawCutoffAmount, SawCutoffAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SawResonanceAmount, SawResonanceAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SawAMamount, SawAMamount, 4095.f, 1.f);

  MK_TRIG_PAR(t_SquareEnable, SquareEnable);
  MK_FLT_PAR_ABS_MIN_MAX(f_SquareFreq, SquareFreq, 4095.f, 0.05f, 200.f);
  MK_FLT_PAR_ABS(f_SquareCutoffAmount, SquareCutoffAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SquareResonanceAmount, SquareResonanceAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_SquareAMamount, SquareAMamount, 4095.f, 1.f);

  // --- Delay ---
  MK_TRIG_PAR(t_DelayEnable, DelayEnable);
  MK_TRIG_PAR(t_DelayTimeShortened, DelayTimeShortened);
  MK_FLT_PAR_HYSTERESIS(f_DelayTime, DelayTime, m_DelayTime, 4095.f, 1.f);
  MK_FLT_PAR_HYSTERESIS(f_DelayFeedback, DelayFeedback, m_DelayFeedback, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_DelayDryWet, DelayDryWet, 4095.f, 1.f);

  // --- Reverb ---
  MK_TRIG_PAR(t_ReverbEnable, ReverbEnable);
  MK_TRIG_PAR(t_ReverbIsMono, ReverbIsMono);
  MK_FLT_PAR_ABS(f_RevInputGain, RevInputGain, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_RevAmount, RevAmount, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_RevDiffusion, RevDiffusion, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_RevTime, RevTime, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_RevLowpass, RevLowpass, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_RevLFO1, RevLFO1, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_RevLFO2, RevLFO2, 4095.f, 1.f);

  // === Precalculations for DSP-Loop ===
  // --- Noise: mixture and optional modulation of noise-sources ---
  float f_WhiteNoise = 0.0002f;   // Avoid stopping of filter-selfoscillation by at least low  noise
  float f_PinkNoise = 0.0002f;
  noiseColourMod.SetFrequency(f_NoiseColourModSpeed);
  noiseLevelMod.SetFrequency(f_NoiseLevelModSpeed);
  if(t_EnableNoiseMod) // Calculate modulation for Noise-mix and level
  {
    f_NoiseColourModAmount = noiseColourMod.Process();
    f_NoiseLevelModAmount = noiseLevelMod.Process();
  }
  if(t_EnableNoise)    // Calculate factors for white and pink noise to be applied in main DSP-loop, based on [mix-]levels and optional modulation
  {
    f_WhiteNoise = (f_NoiseColour+f_NoiseColourModAmount) * (f_MixedNoiseLevel+f_NoiseLevelModAmount);
    f_PinkNoise = (1.f - (f_NoiseColour+f_NoiseColourModAmount) ) * (f_MixedNoiseLevel+f_NoiseLevelModAmount);
  }
  // --- SOU ("Source of Uncertainty") adjustments ---
  if(t_SnH1Enable)     // SnH1 Engine changes AD-Attack-Length, and/or SnH2-Speed and/or Square-speed if activated
  {
    f_ADattack += snh_1*f_SnH1ADattackChange*8.f;
    CONSTRAIN(f_ADattack, 0.f, 8.f );
    f_SquareFreq += snh_1*f_SnH1FrequSquareChange*SQUARE_MAX_FREQ;
    CONSTRAIN(f_SquareFreq,SQUARE_MIN_FREQ, SQUARE_MAX_FREQ);
    f_SnH2Freq += snh_1*f_SnH1FrequSnH2Change*200.f;
    CONSTRAIN(f_SnH2Freq, 0.5f, 200.f);
  }
  if(t_SnH2Enable)      // SnH2 Engine changes AD-Decay-Length and/or SnH1-Speed and/or PulseTri-speed if activated
  {
    f_ADdecay += snh_2*f_SnH2ADdecayChange*25.f;
    CONSTRAIN(f_ADdecay, 0.f, 25.f );
    f_PulseFreq += snh_2*f_SnH2FrequPulseChange*PULSE_MAX_FREQ;
    CONSTRAIN(f_PulseFreq,PULSE_MIN_FREQ, PULSE_MAX_FREQ);
    f_SnH1Freq += snh_2*f_SnH2FrequSnH1Change*200.f;
    CONSTRAIN(f_SnH1Freq, 0.5f, 200.f);
  }
  // --- AD-Envelope ---
  eg_ad.SetAttack(f_ADattack);
  eg_ad.SetDecay(f_ADdecay);
  eg_ad.SetLoop((bool)t_ADegLoop);
  if(t_Trigger == GATE_HIGH_NEW)    // New trigger encountered?
    eg_ad.Trigger();

  // --- ADSR-Envelope ---
  float f_EGvolume = 0.f;       // This is the future result of the main volume modified by both EGs (if any of them is active)
  float eg_ad_process = 0.f;    // Remember EG-value for possible modification of Cutoff and/or Resonance lateron
  float eg_adsr_process = 0.f;  // Remember EG-value for possible modification of Cutoff and/or Resonance lateron

  eg_adsr.SetAttack(f_ADSRattack);
  eg_adsr.SetDecay(f_ADSRdecay);
  eg_adsr.SetSustain(f_ADSRsustain);
  eg_adsr.SetRelease(f_ADSRrelease);
  eg_adsr.Gate(t_Gate);

  // --- Calculate possible VCA-modification for both EGs ---
  if( t_ADenvEnable )
  {
    eg_ad_process = eg_ad.Process();
    if( t_ADenvToVCA )
      f_EGvolume = t_ADenvNegative ? (1.f-eg_ad_process) * f_Volume : eg_ad_process*f_Volume;
  }
  if( t_ADSRenvEnable )
  {
    eg_adsr_process = eg_adsr.Process();
    if( t_ADSRenvToVCA )
      f_EGvolume = t_ADSRenvNegative ? (1.f-eg_adsr_process)*f_Volume + f_EGvolume : eg_adsr_process*f_Volume + f_EGvolume;
    else
    {
      if( !t_ADenvEnable || !t_ADenvToVCA)
        f_EGvolume = f_Volume;
    }
  }
  else
  {
    if (!t_ADenvEnable || !t_ADenvToVCA)
      f_EGvolume = f_Volume;
  }
  // --- Calculate possible LFO-Speed-modification for both EGs ---
  if( t_ADenvEnable && t_ADtoSawLFOfrequ )
  {
    f_SawFreq += eg_ad_process*f_ADtoSawSpeedLevel;
    CONSTRAIN(f_SawFreq, 0.01f, 1.f);
  }
  if( t_ADSRenvEnable && t_ADSRtoSineLFOfrequ )
  {
    f_SineFreq += eg_adsr_process*f_ADSRtoSineSpeedLevel*100.f;
    CONSTRAIN(f_SineFreq, 0.05f, 100.f);
  }
  // --- Set LFO frequencies ---
  sine.SetFrequency(f_SineFreq);
  square.SetFrequency(f_SquareFreq);
  pulse.SetFrequency(f_PulseFreq);
  lfoPWM.SetFrequency(f_PWMspeed);

  snh_sine_1.SetFrequency(f_SnH1Freq);
  snh_sine_2.SetFrequency(f_SnH2Freq);

  // --- Process LFO-waves to be calculated outside of main DSP-loop already ---
  float sine_val = sine.Process();        // Remember current value[s] of LFO[s]
  float pulse_val = pulse.Process();
  pulse_val += f_PulseWidth+lfoPWM.Process()*f_PWMamount; // Shift Pulsewidth and add "PWM-offset", if any
  SINE_TO_SQUARE(pulse_val);  // This by nature contains a constrain, avoiding value overflow (possibly due to PWM-mechanism)!
  f_val_saw = Saw_eptr_process_slow(saw_data, f_SawFreq);      // This is a hack for now: beware that you can't use Saw_eptr_process_slow() and Saw_eptr_process() in the same application for now...
  float square_val = square.Process();
  SINE_TO_SQUARE(square_val);

  // --- Process Sample & Hold results for S&H1 and S&H2 ---
  if( snh_1_hold == false && snh_sine_1.Process() >= 0 )    // We have not found a valid random sample yet or again, wait for next timed trigger
  {
    snh_1 = snh_noise_1.Process();    // Trigger reached => We fetch a new random sample value
    snh_1_hold = true;                // Hold it!
  }
  if( snh_1_hold == true && snh_sine_1.Process() < 0 )    // We are on hold but the holding-time is over
    snh_1_hold = false;                                   // Prepare to fetch and hold next random sample-value

  if( snh_2_hold == false && snh_sine_2.Process() >= 0 )
  {
    snh_2 = snh_noise_2.Process();
    snh_2_hold = true;
  }
  if( snh_2_hold == true && snh_sine_2.Process() < 0 )
    snh_2_hold = false;

  // --- Adjust levels for cutoff or resonance if individual modulators are not disabled ---
  if( !t_PitchToCutoff )
    f_current_note_to_filter = 0.f;

  if( !t_SineEnable )
  {
    f_SineCutoffAmount = 0.f;
    f_SineResonanceAmount = 0.f;
    f_SineAMamount = 0.f;
  }
  if( !t_PulseEnable )
  {
    f_PulseCutoffAmount = 0.f;
    f_PulseResonanceAmount = 0.f;
    f_PulseAMamount = 0.f;
  }
  if( !t_SawEnable )
  {
    f_SawCutoffAmount = 0.f;
    f_SawResonanceAmount = 0.f;
    f_SawAMamount = 0.f;
  }
  if( !t_SquareEnable )
  {
    f_SquareCutoffAmount = 0.f;
    f_SquareResonanceAmount = 0.f;
    f_SquareAMamount = 0.f;
  }
  if( !t_SnH1Enable )
  {
    f_SnH1CutoffLevel = 0.f;
    f_SnH1ResonanceLevel = 0.f;
    f_SnH1AMlevel = 0.f;
  }
  if( !t_SnH2Enable )
  {
    f_SnH2CutoffLevel = 0.f;
    f_SnH2ResonanceLevel = 0.f;
    f_SnH2AMlevel = 0.f;
  }
  // --- Calculate overall Cutoff-modulation (LFOs+EGs) ---
  f_Cutoff += ( f_current_note_to_filter + sine_val*f_SineCutoffAmount + pulse_val*f_PulseCutoffAmount + square_val*f_SquareCutoffAmount + f_val_saw*f_SawCutoffAmount +
                snh_1*f_SnH1CutoffLevel + snh_2*f_SnH2CutoffLevel)*f_CutoffModulationAmount + eg_ad_process*f_ADenvelopeCutoffAmount + eg_adsr_process*f_ADSRenvelopeCutoffAmount;   // Add EGs to LFOs
  CONSTRAIN(f_Cutoff, 0.f, 1.f);

  // --- Calculate overall Resonance-modulation (LFOs+EGs) ---
  f_Resonance += (sine_val*f_SineResonanceAmount + pulse_val*f_PulseResonanceAmount + square_val*f_SquareResonanceAmount + f_val_saw*f_SawResonanceAmount + snh_1*f_SnH1ResonanceLevel +
                  snh_2*f_SnH2ResonanceLevel)*f_ResonanceModulationAmount + eg_ad_process*f_ADenvelopeResonanceAmount + eg_adsr_process*f_ADSRenvelopeResonanceAmount; // Add EGs to LFOs
  CONSTRAIN(f_Resonance, 0.f, 5.f);

  // --- Calculate overall Amplitude-modulation (LFOs+EGs) ---
  float f_am_mod = (  sine_val*f_SineAMamount +  pulse_val*f_PulseAMamount + square_val*f_SquareAMamount + f_val_saw*f_SawAMamount       // Sum of modulations for amplitude
                    + snh_1*f_SnH1AMlevel + snh_2*f_SnH2AMlevel ) * f_AmplitudeModulationAmount;
  f_am_mod = f_am_mod ? f_am_mod*f_EGvolume*3.f : f_EGvolume;   // We either amplify the AM by the Volume (maybe EG-driven) or set it to the current volume if no AM is applied...

  // --- Calculate delay settings ---
  float f_dry = (1.f-f_DelayDryWet);
  float f_delay_len = t_DelayTimeShortened ? f_DelayTime*shorterMaxDelayLength : f_DelayTime*maxDelayLength;
  if( f_delay_len < 1.f )
    f_delay_len = 1.f;
  dlyLine.SetLength((uint32_t)f_delay_len);
  dlyLine.SetFeedback(f_DelayFeedback);

  // --- Calculate reverb settings ---
  reverb.set_input_gain(f_RevInputGain);
  reverb.set_amount(f_RevAmount);
  reverb.set_diffusion(f_RevDiffusion);
  reverb.set_time(f_RevTime);
  reverb.set_lp(f_RevLowpass);
  reverb.set_lfo1_freq(f_RevLFO1);
  reverb.set_lfo2_freq(f_RevLFO2);

  // === Main DSP-Loop[s] ===
  float left[bufSz], right[bufSz];
  float delay_buffer[bufSz*2];                    // The delaybuffer has the same structure as a TBD data-buffer!
  float row_buffer[bufSz];
  if( t_DelayEnable )                            // We either have normal delay processing or try to reset it once
  {
    if( t_VintageResonance )     // Check for normal vs. "vintage filter resonance"
    {
      for (uint32_t i = 0; i < bufSz; i++)
      {
        f_val_result = pNoise.Process() * f_PinkNoise + wNoise.Process() * f_WhiteNoise;
        f_val_result = Ladder_process(ladder_data, f_val_result, f_Cutoff, f_Resonance) * f_am_mod;               // Heun-logic for cutoff-pitch
        delay_buffer[i * 2] = row_buffer[i] = f_val_result;           // Put (so to say) left channel to delay buffer
      }
    }
    else  // "vintage resonance chosen"
    {
      for (uint32_t i = 0; i < bufSz; i++)
      {
        f_val_result = pNoise.Process() * f_PinkNoise + wNoise.Process() * f_WhiteNoise;
        f_val_result = Ladder_process_euler(ladder_vintage_data, f_val_result, f_Cutoff, f_Resonance) * f_am_mod;
        delay_buffer[i * 2] = row_buffer[i] = f_val_result;           // Put (so to say) left channel to delay buffer
      }
    }
    dlyLine.Process(delay_buffer, 0, 2, 64);

    for (uint32_t i = 0; i < bufSz; i++)
      left[i] = right[i] = f_dry*row_buffer[i] + f_DelayDryWet*delay_buffer[i*2];     // Copy delay buffer to both output channels
  }
  else                                            // We do the preprocessing without delay!
  {
    if( t_VintageResonance )     // Check for normal vs. "vintage filter resonance"
    {
      for (uint32_t i = 0; i < bufSz; i++)
      {
        f_val_result = pNoise.Process() * f_PinkNoise + wNoise.Process() * f_WhiteNoise;
        f_val_result = Ladder_process(ladder_data, f_val_result, f_Cutoff, f_Resonance) * f_am_mod;               // Heun-logic for cutoff-pitch
        left[i] = right[i] = f_val_result;
      }
    }
    else  // "vintage resonance chosen"
    {
      for (uint32_t i = 0; i < bufSz; i++)
      {
        f_val_result = pNoise.Process() * f_PinkNoise + wNoise.Process() * f_WhiteNoise;
        f_val_result = Ladder_process_euler(ladder_vintage_data, f_val_result, f_Cutoff, f_Resonance) * f_am_mod;
        left[i] = right[i] = f_val_result;
      }
    }
  }
  if( t_ReverbEnable )
    reverb.Process(left, right, bufSz);           // Add reverb if selected

  // --- Output to DA-converter ---
  if( t_ReverbIsMono && t_ReverbEnable )
  {
    for (int i = 0; i < bufSz; i++)
      data.buf[i*2] = data.buf[i*2+1] = (left[i]+right[i])/2.f;
  }
  else    // Reverb either is stereo or we have to give out the dry signal
  {
    for (int i = 0; i < bufSz; i++)
    {
      data.buf[i * 2] = left[i];
      data.buf[i * 2 + 1] = right[i];
    }
  }
}

// --- Constructor (initalize factory pattern and all DSP-related stuff) ---
void ctagSoundProcessorSpaceFX::Init(std::size_t const &blockSize, void *const blockPtr)
{
  // construct internal data model
  knowYourself();
  model = std::make_unique<ctagSPDataModel>(id, isStereo);
  LoadPreset(0);

  // --- Initialize Envelopes ---
  eg_ad.SetSampleRate(44100.f/ bufSz);    // Sync Env with our audio-processing
  eg_ad.SetModeExp();  // Logarithmic scaling
  eg_adsr.SetSampleRate(44100.f/ bufSz);    // Sync Env with our audio-processing
  eg_adsr.SetModeExp();  // Logarithmic scaling
  eg_adsr.Reset();

  // --- Initialize MI Verb ---
  reverb_buffer = (float *) heap_caps_malloc(32768 * sizeof(float), MALLOC_CAP_SPIRAM);     // MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT
  if(!reverb_buffer)
    ESP_LOGE("MIVerb", "Could not allocate shared buffer!");
  else
  {
    reverb.Init(reverb_buffer);
    reverb.set_amount(0.7f);
    reverb.set_lp(0.7f);
    reverb.set_lfo2_freq(0.1f);
    reverb.set_lfo1_freq(0.2f);
    reverb.set_time(0.6f);
    reverb.set_diffusion(0.85f);
  }
  // --- Initalize LFOs ---
  sine.SetSampleRate(44100.f / bufSz);    // bufSz currently is 32 => 1378,125 Hz
  sine.SetFrequency(1.f);
  pulse.SetSampleRate(44100.f / bufSz);    // bufSz currently is 32 => 1378,125 Hz
  pulse.SetFrequency(1.f);
  lfoPWM.SetSampleRate(44100.f / bufSz);    // bufSz currently is 32 => 1378,125 Hz
  lfoPWM.SetFrequency(1.f);
  square.SetSampleRate(44100.f / bufSz);    // bufSz currently is 32 => 1378,125 Hz
  square.SetFrequency(1.f);

  noiseColourMod.SetSampleRate(44100.f / bufSz);    // bufSz currently is 32 => 1378,125 Hz
  noiseColourMod.SetFrequency(1.f);
  noiseLevelMod.SetSampleRate(44100.f / bufSz);    // bufSz currently is 32 => 1378,125 Hz
  noiseLevelMod.SetFrequency(1.f);

  snh_sine_1.SetSampleRate(44100.f / bufSz);    // bufSz currently is 32 => 1378,125 Hz
  snh_sine_1.SetFrequency(1.f);
  snh_sine_2.SetSampleRate(44100.f / bufSz);    // bufSz currently is 32 => 1378,125 Hz
  snh_sine_2.SetFrequency(1.f);

  // --- Initialize VULT stuff ---
  Ladder_process_init(ladder_data);     // Diode Ladder Filter with Heun based resonance frequency smoothing
  Ladder_process(ladder_data, 0.5f, 0.25f, 4.f);    // Initialize ladder_data, so that we can start with a self-oscillating filter
  Ladder_process_euler_init(ladder_vintage_data); // Diode Ladder Filter with Euler based resonance frequency smoothing
  Ladder_process_euler(ladder_vintage_data, 0.5f, 0.25f, 4.f); // Initialize ladder_vintage_data, so that we can start with a self-oscillating filter
  Saw_eptr_process_init(saw_data);        // Saw wave
}


ctagSoundProcessorSpaceFX::~ctagSoundProcessorSpaceFX() 
{
    if(nullptr != reverb_buffer){
        heap_caps_free(reverb_buffer);
    }
}

// --- Attach parameters from GUI ---
void ctagSoundProcessorSpaceFX::knowYourself()
{
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("Trigger", [&](const int val){ Trigger = val;});
	pMapTrig.emplace("Trigger", [&](const int val){ trig_Trigger = val;});
	pMapPar.emplace("MasterPitch", [&](const int val){ MasterPitch = val;});
	pMapCv.emplace("MasterPitch", [&](const int val){ cv_MasterPitch = val;});
	pMapPar.emplace("MasterTune", [&](const int val){ MasterTune = val;});
	pMapCv.emplace("MasterTune", [&](const int val){ cv_MasterTune = val;});
	pMapPar.emplace("FilterTracking", [&](const int val){ FilterTracking = val;});
	pMapCv.emplace("FilterTracking", [&](const int val){ cv_FilterTracking = val;});
	pMapPar.emplace("Volume", [&](const int val){ Volume = val;});
	pMapCv.emplace("Volume", [&](const int val){ cv_Volume = val;});
	pMapPar.emplace("EnableNoise", [&](const int val){ EnableNoise = val;});
	pMapTrig.emplace("EnableNoise", [&](const int val){ trig_EnableNoise = val;});
	pMapPar.emplace("NoiseColour", [&](const int val){ NoiseColour = val;});
	pMapCv.emplace("NoiseColour", [&](const int val){ cv_NoiseColour = val;});
	pMapPar.emplace("MixedNoiseLevel", [&](const int val){ MixedNoiseLevel = val;});
	pMapCv.emplace("MixedNoiseLevel", [&](const int val){ cv_MixedNoiseLevel = val;});
	pMapPar.emplace("EnableNoiseMod", [&](const int val){ EnableNoiseMod = val;});
	pMapTrig.emplace("EnableNoiseMod", [&](const int val){ trig_EnableNoiseMod = val;});
	pMapPar.emplace("NoiseColourModSpeed", [&](const int val){ NoiseColourModSpeed = val;});
	pMapCv.emplace("NoiseColourModSpeed", [&](const int val){ cv_NoiseColourModSpeed = val;});
	pMapPar.emplace("NoiseColourModAmount", [&](const int val){ NoiseColourModAmount = val;});
	pMapCv.emplace("NoiseColourModAmount", [&](const int val){ cv_NoiseColourModAmount = val;});
	pMapPar.emplace("NoiseLevelModSpeed", [&](const int val){ NoiseLevelModSpeed = val;});
	pMapCv.emplace("NoiseLevelModSpeed", [&](const int val){ cv_NoiseLevelModSpeed = val;});
	pMapPar.emplace("NoiseLevelModAmount", [&](const int val){ NoiseLevelModAmount = val;});
	pMapCv.emplace("NoiseLevelModAmount", [&](const int val){ cv_NoiseLevelModAmount = val;});
	pMapPar.emplace("PitchToCutoff", [&](const int val){ PitchToCutoff = val;});
	pMapTrig.emplace("PitchToCutoff", [&](const int val){ trig_PitchToCutoff = val;});
	pMapPar.emplace("Cutoff", [&](const int val){ Cutoff = val;});
	pMapCv.emplace("Cutoff", [&](const int val){ cv_Cutoff = val;});
	pMapPar.emplace("Resonance", [&](const int val){ Resonance = val;});
	pMapCv.emplace("Resonance", [&](const int val){ cv_Resonance = val;});
	pMapPar.emplace("VintageResonance", [&](const int val){ VintageResonance = val;});
	pMapTrig.emplace("VintageResonance", [&](const int val){ trig_VintageResonance = val;});
	pMapPar.emplace("CutoffModulationAmount", [&](const int val){ CutoffModulationAmount = val;});
	pMapCv.emplace("CutoffModulationAmount", [&](const int val){ cv_CutoffModulationAmount = val;});
	pMapPar.emplace("ResonanceModulationAmount", [&](const int val){ ResonanceModulationAmount = val;});
	pMapCv.emplace("ResonanceModulationAmount", [&](const int val){ cv_ResonanceModulationAmount = val;});
	pMapPar.emplace("AmplitudeModulationAmount", [&](const int val){ AmplitudeModulationAmount = val;});
	pMapCv.emplace("AmplitudeModulationAmount", [&](const int val){ cv_AmplitudeModulationAmount = val;});
	pMapPar.emplace("ADenvEnable", [&](const int val){ ADenvEnable = val;});
	pMapTrig.emplace("ADenvEnable", [&](const int val){ trig_ADenvEnable = val;});
	pMapPar.emplace("ADenvToVCA", [&](const int val){ ADenvToVCA = val;});
	pMapTrig.emplace("ADenvToVCA", [&](const int val){ trig_ADenvToVCA = val;});
	pMapPar.emplace("ADattack", [&](const int val){ ADattack = val;});
	pMapCv.emplace("ADattack", [&](const int val){ cv_ADattack = val;});
	pMapPar.emplace("ADdecay", [&](const int val){ ADdecay = val;});
	pMapCv.emplace("ADdecay", [&](const int val){ cv_ADdecay = val;});
	pMapPar.emplace("ADegLoop", [&](const int val){ ADegLoop = val;});
	pMapTrig.emplace("ADegLoop", [&](const int val){ trig_ADegLoop = val;});
	pMapPar.emplace("ADenvNegative", [&](const int val){ ADenvNegative = val;});
	pMapTrig.emplace("ADenvNegative", [&](const int val){ trig_ADenvNegative = val;});
	pMapPar.emplace("ADenvelopeCutoffAmount", [&](const int val){ ADenvelopeCutoffAmount = val;});
	pMapCv.emplace("ADenvelopeCutoffAmount", [&](const int val){ cv_ADenvelopeCutoffAmount = val;});
	pMapPar.emplace("ADenvelopeResonanceAmount", [&](const int val){ ADenvelopeResonanceAmount = val;});
	pMapCv.emplace("ADenvelopeResonanceAmount", [&](const int val){ cv_ADenvelopeResonanceAmount = val;});
	pMapPar.emplace("ADtoSawLFOfrequ", [&](const int val){ ADtoSawLFOfrequ = val;});
	pMapTrig.emplace("ADtoSawLFOfrequ", [&](const int val){ trig_ADtoSawLFOfrequ = val;});
	pMapPar.emplace("ADtoSawSpeedLevel", [&](const int val){ ADtoSawSpeedLevel = val;});
	pMapCv.emplace("ADtoSawSpeedLevel", [&](const int val){ cv_ADtoSawSpeedLevel = val;});
	pMapPar.emplace("ADSRenvEnable", [&](const int val){ ADSRenvEnable = val;});
	pMapTrig.emplace("ADSRenvEnable", [&](const int val){ trig_ADSRenvEnable = val;});
	pMapPar.emplace("ADSRenvToVCA", [&](const int val){ ADSRenvToVCA = val;});
	pMapTrig.emplace("ADSRenvToVCA", [&](const int val){ trig_ADSRenvToVCA = val;});
	pMapPar.emplace("ADSRattack", [&](const int val){ ADSRattack = val;});
	pMapCv.emplace("ADSRattack", [&](const int val){ cv_ADSRattack = val;});
	pMapPar.emplace("ADSRdecay", [&](const int val){ ADSRdecay = val;});
	pMapCv.emplace("ADSRdecay", [&](const int val){ cv_ADSRdecay = val;});
	pMapPar.emplace("ADSRsustain", [&](const int val){ ADSRsustain = val;});
	pMapCv.emplace("ADSRsustain", [&](const int val){ cv_ADSRsustain = val;});
	pMapPar.emplace("ADSRrelease", [&](const int val){ ADSRrelease = val;});
	pMapCv.emplace("ADSRrelease", [&](const int val){ cv_ADSRrelease = val;});
	pMapPar.emplace("ADSRenvNegative", [&](const int val){ ADSRenvNegative = val;});
	pMapTrig.emplace("ADSRenvNegative", [&](const int val){ trig_ADSRenvNegative = val;});
	pMapPar.emplace("ADSRenvelopeCutoffAmount", [&](const int val){ ADSRenvelopeCutoffAmount = val;});
	pMapCv.emplace("ADSRenvelopeCutoffAmount", [&](const int val){ cv_ADSRenvelopeCutoffAmount = val;});
	pMapPar.emplace("ADSRenvelopeResonanceAmount", [&](const int val){ ADSRenvelopeResonanceAmount = val;});
	pMapCv.emplace("ADSRenvelopeResonanceAmount", [&](const int val){ cv_ADSRenvelopeResonanceAmount = val;});
	pMapPar.emplace("ADSRtoSineLFOfrequ", [&](const int val){ ADSRtoSineLFOfrequ = val;});
	pMapTrig.emplace("ADSRtoSineLFOfrequ", [&](const int val){ trig_ADSRtoSineLFOfrequ = val;});
	pMapPar.emplace("ADSRtoSineSpeedLevel", [&](const int val){ ADSRtoSineSpeedLevel = val;});
	pMapCv.emplace("ADSRtoSineSpeedLevel", [&](const int val){ cv_ADSRtoSineSpeedLevel = val;});
	pMapPar.emplace("SnH1Enable", [&](const int val){ SnH1Enable = val;});
	pMapTrig.emplace("SnH1Enable", [&](const int val){ trig_SnH1Enable = val;});
	pMapPar.emplace("SnH1Freq", [&](const int val){ SnH1Freq = val;});
	pMapCv.emplace("SnH1Freq", [&](const int val){ cv_SnH1Freq = val;});
	pMapPar.emplace("SnH1CutoffLevel", [&](const int val){ SnH1CutoffLevel = val;});
	pMapCv.emplace("SnH1CutoffLevel", [&](const int val){ cv_SnH1CutoffLevel = val;});
	pMapPar.emplace("SnH1ResonanceLevel", [&](const int val){ SnH1ResonanceLevel = val;});
	pMapCv.emplace("SnH1ResonanceLevel", [&](const int val){ cv_SnH1ResonanceLevel = val;});
	pMapPar.emplace("SnH1AMlevel", [&](const int val){ SnH1AMlevel = val;});
	pMapCv.emplace("SnH1AMlevel", [&](const int val){ cv_SnH1AMlevel = val;});
	pMapPar.emplace("SnH1FrequSnH2Change", [&](const int val){ SnH1FrequSnH2Change = val;});
	pMapCv.emplace("SnH1FrequSnH2Change", [&](const int val){ cv_SnH1FrequSnH2Change = val;});
	pMapPar.emplace("SnH1FrequSquareChange", [&](const int val){ SnH1FrequSquareChange = val;});
	pMapCv.emplace("SnH1FrequSquareChange", [&](const int val){ cv_SnH1FrequSquareChange = val;});
	pMapPar.emplace("SnH1ADattackChange", [&](const int val){ SnH1ADattackChange = val;});
	pMapCv.emplace("SnH1ADattackChange", [&](const int val){ cv_SnH1ADattackChange = val;});
	pMapPar.emplace("SnH2Enable", [&](const int val){ SnH2Enable = val;});
	pMapTrig.emplace("SnH2Enable", [&](const int val){ trig_SnH2Enable = val;});
	pMapPar.emplace("SnH2Freq", [&](const int val){ SnH2Freq = val;});
	pMapCv.emplace("SnH2Freq", [&](const int val){ cv_SnH2Freq = val;});
	pMapPar.emplace("SnH2CutoffLevel", [&](const int val){ SnH2CutoffLevel = val;});
	pMapCv.emplace("SnH2CutoffLevel", [&](const int val){ cv_SnH2CutoffLevel = val;});
	pMapPar.emplace("SnH2ResonanceLevel", [&](const int val){ SnH2ResonanceLevel = val;});
	pMapCv.emplace("SnH2ResonanceLevel", [&](const int val){ cv_SnH2ResonanceLevel = val;});
	pMapPar.emplace("SnH2AMlevel", [&](const int val){ SnH2AMlevel = val;});
	pMapCv.emplace("SnH2AMlevel", [&](const int val){ cv_SnH2AMlevel = val;});
	pMapPar.emplace("SnH2FrequSnH1Change", [&](const int val){ SnH2FrequSnH1Change = val;});
	pMapCv.emplace("SnH2FrequSnH1Change", [&](const int val){ cv_SnH2FrequSnH1Change = val;});
	pMapPar.emplace("SnH2FrequPulseChange", [&](const int val){ SnH2FrequPulseChange = val;});
	pMapCv.emplace("SnH2FrequPulseChange", [&](const int val){ cv_SnH2FrequPulseChange = val;});
	pMapPar.emplace("SnH2ADdecayChange", [&](const int val){ SnH2ADdecayChange = val;});
	pMapCv.emplace("SnH2ADdecayChange", [&](const int val){ cv_SnH2ADdecayChange = val;});
	pMapPar.emplace("SineEnable", [&](const int val){ SineEnable = val;});
	pMapTrig.emplace("SineEnable", [&](const int val){ trig_SineEnable = val;});
	pMapPar.emplace("SineFreq", [&](const int val){ SineFreq = val;});
	pMapCv.emplace("SineFreq", [&](const int val){ cv_SineFreq = val;});
	pMapPar.emplace("SineCutoffAmount", [&](const int val){ SineCutoffAmount = val;});
	pMapCv.emplace("SineCutoffAmount", [&](const int val){ cv_SineCutoffAmount = val;});
	pMapPar.emplace("SineResonanceAmount", [&](const int val){ SineResonanceAmount = val;});
	pMapCv.emplace("SineResonanceAmount", [&](const int val){ cv_SineResonanceAmount = val;});
	pMapPar.emplace("SineAMamount", [&](const int val){ SineAMamount = val;});
	pMapCv.emplace("SineAMamount", [&](const int val){ cv_SineAMamount = val;});
	pMapPar.emplace("PulseEnable", [&](const int val){ PulseEnable = val;});
	pMapTrig.emplace("PulseEnable", [&](const int val){ trig_PulseEnable = val;});
	pMapPar.emplace("PulseFreq", [&](const int val){ PulseFreq = val;});
	pMapCv.emplace("PulseFreq", [&](const int val){ cv_PulseFreq = val;});
	pMapPar.emplace("PulseWidth", [&](const int val){ PulseWidth = val;});
	pMapCv.emplace("PulseWidth", [&](const int val){ cv_PulseWidth = val;});
	pMapPar.emplace("PWMspeed", [&](const int val){ PWMspeed = val;});
	pMapCv.emplace("PWMspeed", [&](const int val){ cv_PWMspeed = val;});
	pMapPar.emplace("PWMamount", [&](const int val){ PWMamount = val;});
	pMapCv.emplace("PWMamount", [&](const int val){ cv_PWMamount = val;});
	pMapPar.emplace("PulseCutoffAmount", [&](const int val){ PulseCutoffAmount = val;});
	pMapCv.emplace("PulseCutoffAmount", [&](const int val){ cv_PulseCutoffAmount = val;});
	pMapPar.emplace("PulseResonanceAmount", [&](const int val){ PulseResonanceAmount = val;});
	pMapCv.emplace("PulseResonanceAmount", [&](const int val){ cv_PulseResonanceAmount = val;});
	pMapPar.emplace("PulseAMamount", [&](const int val){ PulseAMamount = val;});
	pMapCv.emplace("PulseAMamount", [&](const int val){ cv_PulseAMamount = val;});
	pMapPar.emplace("SawEnable", [&](const int val){ SawEnable = val;});
	pMapTrig.emplace("SawEnable", [&](const int val){ trig_SawEnable = val;});
	pMapPar.emplace("SawFreq", [&](const int val){ SawFreq = val;});
	pMapCv.emplace("SawFreq", [&](const int val){ cv_SawFreq = val;});
	pMapPar.emplace("SawCutoffAmount", [&](const int val){ SawCutoffAmount = val;});
	pMapCv.emplace("SawCutoffAmount", [&](const int val){ cv_SawCutoffAmount = val;});
	pMapPar.emplace("SawResonanceAmount", [&](const int val){ SawResonanceAmount = val;});
	pMapCv.emplace("SawResonanceAmount", [&](const int val){ cv_SawResonanceAmount = val;});
	pMapPar.emplace("SawAMamount", [&](const int val){ SawAMamount = val;});
	pMapCv.emplace("SawAMamount", [&](const int val){ cv_SawAMamount = val;});
	pMapPar.emplace("SquareEnable", [&](const int val){ SquareEnable = val;});
	pMapTrig.emplace("SquareEnable", [&](const int val){ trig_SquareEnable = val;});
	pMapPar.emplace("SquareFreq", [&](const int val){ SquareFreq = val;});
	pMapCv.emplace("SquareFreq", [&](const int val){ cv_SquareFreq = val;});
	pMapPar.emplace("SquareCutoffAmount", [&](const int val){ SquareCutoffAmount = val;});
	pMapCv.emplace("SquareCutoffAmount", [&](const int val){ cv_SquareCutoffAmount = val;});
	pMapPar.emplace("SquareResonanceAmount", [&](const int val){ SquareResonanceAmount = val;});
	pMapCv.emplace("SquareResonanceAmount", [&](const int val){ cv_SquareResonanceAmount = val;});
	pMapPar.emplace("SquareAMamount", [&](const int val){ SquareAMamount = val;});
	pMapCv.emplace("SquareAMamount", [&](const int val){ cv_SquareAMamount = val;});
	pMapPar.emplace("DelayEnable", [&](const int val){ DelayEnable = val;});
	pMapTrig.emplace("DelayEnable", [&](const int val){ trig_DelayEnable = val;});
	pMapPar.emplace("DelayTimeShortened", [&](const int val){ DelayTimeShortened = val;});
	pMapTrig.emplace("DelayTimeShortened", [&](const int val){ trig_DelayTimeShortened = val;});
	pMapPar.emplace("DelayTime", [&](const int val){ DelayTime = val;});
	pMapCv.emplace("DelayTime", [&](const int val){ cv_DelayTime = val;});
	pMapPar.emplace("DelayFeedback", [&](const int val){ DelayFeedback = val;});
	pMapCv.emplace("DelayFeedback", [&](const int val){ cv_DelayFeedback = val;});
	pMapPar.emplace("DelayDryWet", [&](const int val){ DelayDryWet = val;});
	pMapCv.emplace("DelayDryWet", [&](const int val){ cv_DelayDryWet = val;});
	pMapPar.emplace("ReverbEnable", [&](const int val){ ReverbEnable = val;});
	pMapTrig.emplace("ReverbEnable", [&](const int val){ trig_ReverbEnable = val;});
	pMapPar.emplace("ReverbIsMono", [&](const int val){ ReverbIsMono = val;});
	pMapTrig.emplace("ReverbIsMono", [&](const int val){ trig_ReverbIsMono = val;});
	pMapPar.emplace("RevInputGain", [&](const int val){ RevInputGain = val;});
	pMapCv.emplace("RevInputGain", [&](const int val){ cv_RevInputGain = val;});
	pMapPar.emplace("RevDiffusion", [&](const int val){ RevDiffusion = val;});
	pMapCv.emplace("RevDiffusion", [&](const int val){ cv_RevDiffusion = val;});
	pMapPar.emplace("RevTime", [&](const int val){ RevTime = val;});
	pMapCv.emplace("RevTime", [&](const int val){ cv_RevTime = val;});
	pMapPar.emplace("RevLowpass", [&](const int val){ RevLowpass = val;});
	pMapCv.emplace("RevLowpass", [&](const int val){ cv_RevLowpass = val;});
	pMapPar.emplace("RevLFO1", [&](const int val){ RevLFO1 = val;});
	pMapCv.emplace("RevLFO1", [&](const int val){ cv_RevLFO1 = val;});
	pMapPar.emplace("RevLFO2", [&](const int val){ RevLFO2 = val;});
	pMapCv.emplace("RevLFO2", [&](const int val){ cv_RevLFO2 = val;});
	pMapPar.emplace("RevAmount", [&](const int val){ RevAmount = val;});
	pMapCv.emplace("RevAmount", [&](const int val){ cv_RevAmount = val;});
	isStereo = true;
	id = "SpaceFX";
	// sectionCpp0
}