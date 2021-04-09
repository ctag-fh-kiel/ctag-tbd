/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020/2021 by Robert Manzke. All rights reserved.

(c) 2021 for the "VctrSnt"-Plugin by Mathias Brüssel
VctrSnt stands for "Vector Synthesizer" and is inspired by the legendary Sequential Circuits Prophet VS http://www.vintagesynth.com/sci/pvs.php
This TBD synth is stereo when the optional auto-panner effect is activated, the sound generation itself is monophonic, though.
When only one audio-output is used and panning is on it will be audible as a kind of tremolo-effect instead, which makes it mono-compatible,
this may be welcome when combined with other modules in your rack's signal chain.
Very much like the Prophet VS, VctrSnt can blend sound between four oscillators, that's what the name vector-synthesis mainly stands for.
In contrast to its role model this is not done with a joystick but with separate controllers/CV per axis.
As with the VS the Oscillators are named A,B,C,D. On the X-axis the wavetables can be crossfaded as A<->C,
on the Y-axis samples (as an enhancement in contrast of using wavetables only) can be crossfaded as D<->B.
Similar to the PPG and other wavetable-based instruments, the waves within the table can be changed manually or via an LFO and is called "Z-scanning" here.
The volume envelope and the panner can be turned off completely, this is important in case you want to use a true analogue filter behind the digital
oscillators, which is one of the strong points of vintage wavetable synths like the Prophet VS and PPG 2 or early samplers like the EMU II.

VctrSnt borrows a fair amount of code from the TBD-Plugins WTOsc and Rompler, big thanks to Robert Manzke for implementing those!

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "ctagSoundProcessorVctrSnt.hpp"
#include "esp_heap_caps.h"
#include "helpers/ctagNumUtil.hpp"
#include "plaits/dsp/engine/engine.h"

using namespace CTAG::SP;

#define GATE_HIGH_NEW       2
#define GATE_HIGH           1
#define GATE_LOW            0

#define IDX_OSC_B           1         // Sample-Oscillator on top of Y-Axix for "Vector-Stick"
#define IDX_OSC_D           0         // Sample-Oscillator on bottom of Y-Axix for "Vector-Stick"

// --- Replace function-call of frequency-conversion with macro for increasing speed just a bit ---
#define noteToFreq(incoming_note) (HELPERS::fastpow2 ((incoming_note - 69.f) / 12.f) *440.f)

// --- Additional Macro for automated parameter evaluations ---
#define MK_TRIG_PAR(outname, inname) int outname = process_param_trig(data, trig_##inname, inname, e_##inname);
#define MK_GATE_PAR(outname, inname) bool outname = (bool)process_param_trig(data, trig_##inname, inname, e_##inname, 1);
#define MK_ADEG_PAR(outname, inname) int outname = process_param_trig(data, trig_##inname, inname, e_##inname, 2);

// --- Modify sine-wave for Squarewave/PWM or various modulations (including Pitch-Mod, Filter-Mod, Z-Scan and Vector-Modulation) ---
#define SINE_TO_SQUARE(sine_val)                      sine_val = (sine_val >= 0) ? 1.f : -1.f;

// --- Morph sinewave to square, pseudo triangle, sample and hold and similar ---
float ctagSoundProcessorVctrSnt::morph_sine_wave(float sine_val, int morph_mode, int enum_sine)
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
float ctagSoundProcessorVctrSnt::applySnH(float sine_lfo_val, int enum_val)
{
  if(sine_lfo_val > 0.5f || sine_lfo_val < -0.5f )   // we use this so that we can have equal frequency as with a spared value
  {
    if (hold_trigger[enum_val])      // Index relies on the member "enum snh_members", we use arrays for the elements needed instead of dynamically instanciating several objects
    {
      saved_sample[enum_val] = oscSnH[enum_val].Process();  // values -1.f ... +1.f
    }
    hold_trigger[enum_val] = false;
  }
  else
    hold_trigger[enum_val] = true;

  return(saved_sample[enum_val]);
}

// --- Process trigger signals and keep their state internally ---
inline int ctagSoundProcessorVctrSnt::process_param_trig(const ProcessData &data, int trig_myparm, int my_parm, int enum_trig_state_id, int gate_type = 0 )
{
  int trig_status = 0;

  if(trig_myparm != -1)       // Trigger given via CV/Gate or button?
  {
    trig_status = (data.trig[trig_myparm]==0); // HIGH is 0, so we negate for boolean logic
    if( gate_type == 1 )          // Gate-type
      return(trig_status);

    if(trig_status)    // Statuschange from HIGH to LOW or LOW to HIGH? Startup-Status for prev_trig_state is -1, so first change is always new
    {
      if( low_reached[enum_trig_state_id] )    // We had a trigger low before the new trigger high
      {
        if (prev_trig_state[enum_trig_state_id] == GATE_LOW || gate_type==2 )   // Toggle or AD EG Trigger...
        {
          prev_trig_state[enum_trig_state_id] = GATE_HIGH;       // Remember status for next round
          low_reached[enum_trig_state_id] = false;
          return (GATE_HIGH_NEW);           // New trigger
        }
        else        // previous status was high!
        {
          prev_trig_state[enum_trig_state_id] = GATE_LOW;       // Remember status for next round
          low_reached[enum_trig_state_id] = false;
          return (GATE_LOW);           // New trigger
        }
      }
    }
    else
      low_reached[enum_trig_state_id] = true;
  }
  else                        // We may have a trigger set by activating the button via the GUI
  {
    if (my_parm != prev_trig_state[enum_trig_state_id])    // Statuschange from HIGH to LOW or LOW to HIGH?
    {
      prev_trig_state[enum_trig_state_id] = my_parm;       // Remember status
      if(my_parm != 0)                   // LOW if 0
        return (GATE_HIGH_NEW);          // New trigger
      else
        return (GATE_LOW);           // Trigger released
    }
  }
  return(prev_trig_state[enum_trig_state_id]);            // No change (1 for active, 0 for inactive)
}

// --- Transform wavetables from the format as to be found on https://waveeditonline.com to Mutable Instruments format, so that we can use the Plaits WT-Oscillator ---
void ctagSoundProcessorVctrSnt::prepareWavetable( const int16_t **wavetables, int currentBank, bool *isWaveTableGood, float **fbuffer, int16_t **ibuffer )
{
  // Precalculates wavetable data according to https://www.dafx12.york.ac.uk/papers/dafx12_submission_69.pdf
  // plaits uses integrated wavetable synthesis, i.e. integrated wavetables, order K=1 (one integration), N=1 (linear interpolation)
  // check if sample rom seems to have current bank
  if(!sample_rom.HasSliceGroup(currentBank * 64, currentBank * 64 + 63))
  {
    *isWaveTableGood = false;
    return;
  }
  int size = sample_rom.GetSliceGroupSize(currentBank * 64, currentBank * 64 + 63);
  if(size != 256*64)
  {
    *isWaveTableGood = false;
    return;
  }
  int bankOffset = currentBank*64*256;
  int bufferOffset = 4*64; // load sample data into buffer at offset, due to pre-calculation each wave will be 260 words long
  sample_rom.Read(&((*ibuffer)[bufferOffset]), bankOffset, 256*64);
  // --- Start conversion of data, 64 wavetables per bank ---
  int c = 0;
  for(int i=0;i<64;i++)   // iterate all waves
  {
    int startOffset = bufferOffset + i*256; // which wave
    // prepare long array, i.e. x = numpy.array(list(wave) * 2 + wave[0] + wave[1] + wave[2] + wave[3])
    float sum4 = (*ibuffer)[startOffset] + (*ibuffer)[startOffset+1] + (*ibuffer)[startOffset+2] + (*ibuffer)[startOffset+3]; // add dc
    for(int j=0;j<512;j++)
      (*fbuffer)[j] = (*ibuffer)[startOffset + (j%256)] + sum4;
    removeMeanOfFloatArray(*fbuffer, 512);      // x -= x.mean()
    scaleFloatArrayToAbsMax(*fbuffer, 512);     // x /= numpy.abs(x).max()
    accumulateFloatArray(*fbuffer, 512);        // x = numpy.cumsum(x)
    removeMeanOfFloatArray(*fbuffer, 512);      // x -= x.mean()
    wavetables[i] = &((*ibuffer)[c]);                // create pointer map
    for(int j=512-256-4;j<512;j++)
    {
      int16_t v = static_cast<int16_t >(roundf((*fbuffer)[j] * 4.f * 32768.f / 256.f)); // x = numpy.round(x * (4 * 32768.0 / WAVETABLE_SIZE)
      (*ibuffer)[c++] = v;
    }
  }
  *isWaveTableGood = true;
}

// --- Main processing routine for VctrSnt ---
void ctagSoundProcessorVctrSnt::Process(const ProcessData &data)
{
  // --- DSP calculation results ---
  float f_val_result = 0.f;

  // === Read Buttons from GUI or Trigger/Gate and Sliders from GUI or CV and scale the data if required, "order in way of apperance" on GUI ===
  // Trigger variables will be named like the given parameter with an t_-Prefix but stay integers, CV-Variables are floats and thus f_*
  // --- Voice / Volume ---
  MK_GATE_PAR(g_Gate, Gate);
  MK_ADEG_PAR(t_Gate, Gate);

  MK_TRIG_PAR(t_EGvolGate, EGvolGate);
  if (t_Gate == GATE_HIGH_NEW && !t_EGvolGate)   // We have two modes for "on trigger": reset playing of samples to start y/n
  {
    romplers[IDX_OSC_B]->Reset();     // Reset Sample-OSCs to play from beginning and so in if new triggered
    romplers[IDX_OSC_D]->Reset();     // Please note: once triggered, all Voices will stay active (i.e. also samples will play "forever" if looping is active)
  }
  float f_MasterPitch = MasterPitch;  // Range is -48...48 as "MIDI notes"...

  MK_FLT_PAR_ABS_SFT(f_MasterTune, MasterTune, 1200.f, 1.f);
  f_MasterPitch += f_MasterTune*12;                         // Please note: that this tuning in combination with quantize can be used for error-correction of CV, too!

  float f_MasterPitch_CV = f_MasterPitch;
  if (cv_MasterPitch != -1)                                 // Please note: we save the CV-assciated pitch here seperately, this may be excluded per Oscillator-Group via an option!
    f_MasterPitch_CV = f_MasterPitch+data.cv[cv_MasterPitch] * 60.f;    // We expect "MIDI"-notes 0-59 for 5 octaves (5*12) which fit into 0...+5V with 1V/Oct logic!

  MK_TRIG_PAR(t_QuantizePitch, QuantizePitch);
  if( t_QuantizePitch )
    f_MasterPitch_CV = (float)((int)f_MasterPitch_CV);     // We get rid of the values behind the decimal point if quantize is on

  MK_FLT_PAR_ABS(f_Volume, Volume, 4095.f, 4.f);

  // --- Check if Oscillator-Groups should be excluded from CV pitch-tracking ---
  MK_TRIG_PAR(t_ExclSubOSCmasterPitch, ExclSubOSCmasterPitch);
  float f_MasterPitch_SubOSC = t_ExclSubOSCmasterPitch ? f_MasterPitch : f_MasterPitch_CV;   // We either add the CV-masterpitch for groups of oscillators or ignore it!
  MK_TRIG_PAR(t_ExclWTmasterPitch, ExclWTmasterPitch);
  float f_MasterPitch_WT = t_ExclWTmasterPitch ? f_MasterPitch : f_MasterPitch_CV;
  MK_TRIG_PAR(t_ExclSMPmasterPitch, ExclSMPmasterPitch);
  float f_MasterPitch_SMP = t_ExclSMPmasterPitch ? f_MasterPitch: f_MasterPitch_CV;

  // --- VectorSpace (we mix the signals of the various oscillators later, here) ---
  MK_FLT_PAR_ABS( f_PWMintensity, PWMintensity, 4095.f, 1.f);
  MK_FLT_PAR_ABS_MIN_MAX( f_PWMspeed, PWMspeed, 4095.f, 0.05f, 20.f);
  lfoPWM.SetFrequency(f_PWMspeed);
  float f_LFO_pwm = lfoPWM.Process();   // This is a "free running" LFO, we don't use other dependancies to not complicate things, even if it's unused

  MK_TRIG_PAR(t_SubOscPWM_A, SubOscPWM_A);
  MK_TRIG_PAR(t_SubOsc2VCF_A, SubOsc2VCF_A);
  MK_FLT_PAR_ABS(f_SubOscFade_A, SubOscFade_A, 4095.f, 1.f);
  float f_PitchSubOsc_A = PitchSubOsc_A/100.f;                    // Range is -3600...3600 as "MIDI notes"...
  if (cv_PitchSubOsc_A != -1)
    f_PitchSubOsc_A += data.cv[cv_PitchSubOsc_A] * 60.f;          // We expect "MIDI"-notes 0-59 for 5 octaves (5*12) which fit into 0...+5V with 1V/Oct logic!

  MK_TRIG_PAR(t_SubOscPWM_C, SubOscPWM_C);
  MK_TRIG_PAR(t_SubOsc2VCF_C, SubOsc2VCF_C);
  MK_FLT_PAR_ABS(f_SubOscFade_C, SubOscFade_C, 4095.f, 1.f);
  float f_PitchSubOsc_C = PitchSubOsc_C/100.f;                    // Range is -3600...3600 as "MIDI notes"...
  if (cv_PitchSubOsc_C != -1)
    f_PitchSubOsc_C += data.cv[cv_PitchSubOsc_C] * 60.f;          // We expect "MIDI"-notes 0-59 for 5 octaves (5*12) which fit into 0...+5V with 1V/Oct logic!

  MK_FLT_PAR_ABS(f_VolWT_A, VolWT_A, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_VolOsc_B, VolOsc_B, 4095.f,2.f);     // Samples may be recorded at lower volume, so we leave a headroom option by scaling by 2
  MK_FLT_PAR_ABS(f_VolWT_C, VolWT_C, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_VolOsc_D, VolOsc_D, 4095.f,2.f);     // Samples may be recorded at lower volume, so we leave a headroom option by scaling by 2

  MK_TRIG_PAR(t_StereoSplit, StereoSplit);                          // If activated the oscillator-groups will be panned left/right
  MK_FLT_PAR_ABS_PAN(f_XfadeWaveTbls, XfadeWaveTbls, 2048.f,1.f);     // We have a middle-centered scale, but we need 0-1.0 for mixing
  MK_FLT_PAR_ABS_PAN(f_XfadeSamples, XfadeSamples, 2048.f, 1.f);

  // --- Vector Modulators WaveTables ---
  MK_TRIG_PAR(t_LfoWTxFadeActive_1, LfoWTxFadeActive_1);
  MK_INT_PAR_ABS(i_LfoTypeWTxFade_1, LfoTypeWTxFade_1, 7.f)
  CONSTRAIN(i_LfoTypeWTxFade_1, 0, 6);      // 7 possible types of LFOs
  MK_FLT_PAR_ABS(f_LfoWTxFadeRange_1, LfoWTxFadeRange_1, 4095.f, 1.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_LfoWTxFadeSpeed_1, LfoWTxFadeSpeed_1, 4095.f, 0.05f, 15.f);
  lfoXfadeWT_1.SetFrequency(f_LfoWTxFadeSpeed_1);

  MK_TRIG_PAR(t_LfoWTxFadeActive_2, LfoWTxFadeActive_2);
  MK_INT_PAR_ABS(i_LfoTypeWTxFade_2, LfoTypeWTxFade_2, 7.f)
  CONSTRAIN(i_LfoTypeWTxFade_2, 0, 6);      // 7 possible types of LFOs
  MK_FLT_PAR_ABS(f_LfoWTxFadeRange_2, LfoWTxFadeRange_2, 4095.f, 1.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_LfoWTxFadeSpeed_2, LfoWTxFadeSpeed_2, 4095.f, 0.05f, 15.f);
  lfoXfadeWT_2.SetFrequency(f_LfoWTxFadeSpeed_2);

  // --- Vector Modulators Samples ---
  MK_TRIG_PAR(t_LfoSAMPxFadeActive_1, LfoSAMPxFadeActive_1);
  MK_INT_PAR_ABS(i_LfoTypeSAMPxFade_1, LfoTypeSAMPxFade_1, 7.f)
  CONSTRAIN(i_LfoTypeSAMPxFade_1, 0, 6);      // 7 possible types of LFOs
  MK_FLT_PAR_ABS(f_LfoSAMPxFadeRange_1, LfoSAMPxFadeRange_1, 4095.f, 1.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_LfoSAMPxFadeSpeed_1, LfoSAMPxFadeSpeed_1, 4095.f, 0.05f, 15.f);
  lfoXfadeSample_1.SetFrequency(f_LfoSAMPxFadeSpeed_1);

  MK_TRIG_PAR(t_LfoSAMPxFadeActive_2, LfoSAMPxFadeActive_2);
  MK_INT_PAR_ABS(i_LfoTypeSAMPxFade_2, LfoTypeSAMPxFade_2, 7.f)
  CONSTRAIN(i_LfoTypeSAMPxFade_2, 0, 6);      // 7 possible types of LFOs
  MK_FLT_PAR_ABS(f_LfoSAMPxFadeRange_2, LfoSAMPxFadeRange_2, 4095.f, 1.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_LfoSAMPxFadeSpeed_2, LfoSAMPxFadeSpeed_2, 4095.f, 0.05f, 15.f);
  lfoXfadeSample_2.SetFrequency(f_LfoSAMPxFadeSpeed_2);

  // --- WaveTable Scan Modulators ---
  MK_FLT_PAR_ABS(f_ScanWavTblA, ScanWavTblA, 4095.f, 1.f);
  MK_TRIG_PAR(t_ScanWavTbl_A, ScanWavTbl_A);
  MK_TRIG_PAR(t_ModulateSubOscXfade_A, ModulateSubOscXfade_A);
  MK_INT_PAR_ABS(i_LFOzScanType_A, LFOzScanType_A, 7.f)
  CONSTRAIN(i_LFOzScanType_A, 0, 6);      // 7 possible types of LFOs
  MK_FLT_PAR_ABS(f_LFOzScanAmt_A, LFOzScanAmt_A, 4095.f, 1.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_LFOzScanSpeed_A, LFOzScanSpeed_A, 4095.f, 0.01f, 20.f);

  MK_FLT_PAR_ABS(f_ScanWavTblC, ScanWavTblC, 4095.f, 1.f);
  MK_TRIG_PAR(t_ScanWavTbl_C, ScanWavTbl_C);
  MK_TRIG_PAR(t_ModulateSubOscXfade_C, ModulateSubOscXfade_C);
  MK_INT_PAR_ABS(i_LFOzScanType_C, LFOzScanType_C, 7.f)
  CONSTRAIN(i_LFOzScanType_C, 0, 6);      // 7 possible types of LFOs
  MK_FLT_PAR_ABS(f_LFOzScanAmt_C, LFOzScanAmt_C, 4095.f, 1.f);
  MK_FLT_PAR_ABS_MIN_MAX(f_LFOzScanSpeed_C, LFOzScanSpeed_C, 4095.f, 0.01f, 20.f);

  // === Frequency Modulation (Please note: these controls are located further down in the GUI, but we need the infos to set the oscillators already here!) ===
  float f_PitchMod = 0.f;
  MK_TRIG_PAR(t_FreqModActive, FreqModActive);
  MK_TRIG_PAR(t_FreqModExclSubOSC, FreqModExclSubOSC);
  MK_TRIG_PAR(t_FreqModExclWT, FreqModExclWT);
  MK_TRIG_PAR(t_FreqModExclSample, FreqModExclSample);
  MK_INT_PAR_ABS(i_FreqModType, FreqModType, 7.f)
  CONSTRAIN(i_FreqModType, 0, 6);      // 7 possible types of LFOs
  MK_FLT_PAR_ABS(f_FreqModAmnt, FreqModAmnt, 4095.f, 7.f);    // Apply max a 5th of modulation
  MK_FLT_PAR_ABS_MIN_MAX(f_FreqModSpeed, FreqModSpeed, 4095.f, 0.01f, 20.f);   // LFO will have frequencies from 0.05 to 100 Hz
  MK_FLT_PAR_ABS(f_FreqModAnalog, FreqModAnalog, 4095.f, 1.f);

  // --- Calculate values for optional frequency modulation ---
  float f_PitchMod_SubOsc = 0.f;  // We have to define values for each category of oscillator, we may simply add values of 0 lateron if no modulation took place!
  float f_PitchMod_WT = 0.f;
  float f_PitchMod_Sample = 0.f;
  if(t_FreqModActive)
  {
    if( f_FreqModAnalog != 0.f ) // Apply "analogue / random variation of FreqMod-Amount?
    {
      f_FreqModAmnt += 3.5f * lfoRandom.Process() * f_FreqModAnalog;    // Within range randomly modify frequency modulation amount
      CONSTRAIN(f_FreqModAmnt, 0.f, 7.f);
      f_FreqModSpeed += 10.f * lfoRandom.Process() * f_FreqModAnalog;   // Within range randomly modify frequency modulation speed
      CONSTRAIN(f_FreqModSpeed, 0.01f, 20.f);
    }
    lfoPitch.SetFrequency(f_FreqModSpeed);
    float pitchVal = lfoPitch.Process();
    pitchVal = morph_sine_wave(pitchVal, i_FreqModType, e_PitchMod);    // Convert sine-wave to square-wave, pseudo-triangle, S&H and so on if desired
    f_PitchMod = pitchVal * f_FreqModAmnt;    // We will add this to each Oscillator, the value is either 0 (if inactive) or the current offset.

    // --- Check per category of oscillators if PitchMod should be applied (if not set, we will simply add 0 lateron for no effect) ---
    if( !t_FreqModExclSubOSC )
      f_PitchMod_SubOsc = f_PitchMod;
    if( !t_FreqModExclWT )
      f_PitchMod_WT = f_PitchMod;
    if( !t_FreqModExclSample )
      f_PitchMod_Sample = f_PitchMod;
  }
  // === Wave-Table oscillator A ===
  // --- Wave select A ---
  currentBank_A = WaveTblA;
  if (lastBank_A != currentBank_A)
  { // this is slow, hence not modulated by CV
    prepareWavetable((const int16_t **) &wavetables_A, currentBank_A, &isWaveTableGood_A, &fbuffer_A, &buffer_A);
    lastBank_A = currentBank_A;
  }
  // --- Pitch / Tune Wavetable Oscillator A ---
  float f_pitch_A = pitch_A;
  if (cv_pitch_A != -1)
    f_pitch_A += data.cv[cv_pitch_A] * 12.f * 5.f;
  MK_FLT_PAR_ABS_SFT(f_tune_A, tune_A, 1200.f, 1.f);
  const float f_freq_A = plaits::NoteToFrequency(60 + f_tune_A*12.f + f_pitch_A+f_MasterPitch_WT+f_PitchMod_WT) * 0.998f;

  // --- Filter settings for wavetable A ---
  MK_FLT_PAR_ABS(fLFOFMFilt_A, lfo2filtfm_A, 4095.f, 1.f);
  MK_FLT_PAR_ABS(fCut_A, fcut_A, 4095.f, 1.f)
  MK_FLT_PAR_ABS_MIN_MAX(fReso_A, freso_A, 4095.f, 1.f, 20.f);

  // --- Optional filter modulation WT A ---
  MK_TRIG_PAR(t_FilterLFOon_A, FilterLFOon_A);
  if( t_FilterLFOon_A )
  {
    MK_INT_PAR_ABS(i_LFOfilterType_A, LFOfilterType_A, 7.f)
    CONSTRAIN(i_LFOfilterType_A, 0, 6);      // 7 possible types of LFOs
    MK_FLT_PAR_ABS_MIN_MAX(f_LFOSpeed_A, lfospeed_A, 4095.f, 0.05f, 20.f);
    lfo_A.SetFrequency(f_LFOSpeed_A);
    float f_filterLFO_A = lfo_A.Process();
    f_filterLFO_A = morph_sine_wave(f_filterLFO_A, i_LFOfilterType_A, e_filterLFO_A);    // Convert sine-wave to square-wave, pseudo-triangle, S&H and so on if desired
    fCut_A += f_filterLFO_A * fLFOFMFilt_A;   // Filter modulation
    CONSTRAIN(fCut_A, 0.f, 1.f);    // limit values
  }
  fCut_A = 20.f * stmlib::SemitonesToRatio(fCut_A * 120.f);
  svf_A.set_f_q<stmlib::FREQUENCY_FAST>(fCut_A / 44100.f, fReso_A);
  MK_INT_PAR_ABS(iFType_A, fmode_A, 4.f)
  CONSTRAIN(iFType_A, 0, 3);

  // --- Check for Wavetable Z-axis modulation ---
  lfo_WT_A.SetFrequency(f_LFOzScanSpeed_A);
  float f_LFO_WT_A = lfo_WT_A.Process();
  if( t_ScanWavTbl_A)
  {
    f_LFO_WT_A = morph_sine_wave(f_LFO_WT_A, i_LFOzScanType_A, e_WT_A);   // Convert sine-wave to square-wave, pseudo-triangle, S&H and so on if desired
    f_ScanWavTblA += f_LFO_WT_A * f_LFOzScanAmt_A;
    CONSTRAIN(f_ScanWavTblA, 0.f, 1.f)
  }
  // --- Render A: Calc wave and apply filter ---
  float out_A[32] = {0.f};
  if(isWaveTableGood_A)
  {
    wt_osc_A.Render(f_freq_A, f_VolWT_A, f_ScanWavTblA, wavetables_A, out_A, bufSz);
    if(t_SubOscPWM_A)   // PWM modulated square-wave as sub-oscillator?
      oscSub_A.SetFrequency(noteToFreq(f_MasterPitch_SubOSC+f_PitchSubOsc_A+f_PitchMod_SubOsc) );

    float subOscVal = 0.f;
    float lfo_Xfade_SubOSC_A = 1.f;
    if(t_ModulateSubOscXfade_A && t_LfoWTxFadeActive_2)   // Modulate SubOSC a Xfade with LFO 2 of WT XFade?
    {
      lfo_Xfade_SubOSC_A = lfoXfadeWT_2.Process(); // We apply auto-xfades in the final loop, to gain results as smooth as possible!
      lfo_Xfade_SubOSC_A = morph_sine_wave(lfo_Xfade_SubOSC_A, i_LfoTypeWTxFade_2, e_XfadeWT_2);   // Convert sine-wave to square-wave, pseudo-triangle, S&H and so on if desired
      lfo_Xfade_SubOSC_A *= f_LfoWTxFadeRange_2;
    }
    if( !t_SubOsc2VCF_A )    // Apply Filter _before_ mix of SubOSC with Wavetable
    {
      switch (iFType_A)    // 0 if filter is off!
      {
        case 1: svf_A.Process<stmlib::FILTER_MODE_LOW_PASS>(out_A, out_A, bufSz); break;
        case 2: svf_A.Process<stmlib::FILTER_MODE_BAND_PASS>(out_A, out_A, bufSz); break;
        case 3: svf_A.Process<stmlib::FILTER_MODE_HIGH_PASS>(out_A, out_A, bufSz); break;
      }
    }
    for( int i=0; i<bufSz; i++)
    {
      if(t_SubOscPWM_A)   // PWM modulated square-wave as sub-oscillator?
      {
        subOscVal = oscSub_A.Process();
        subOscVal += f_LFO_pwm*f_PWMintensity; // Add "PWM-offset", we take the amount and LFO-speed for our sub-oscillator from the settings of the table-Z-scan!
        SINE_TO_SQUARE(subOscVal);    // This by nature contains a constrain, avoiding value overflow!
      }
      else                  // Noise as sub-oscillator
      {
        if( f_PitchSubOsc_A < 0.f )    // Pitchslider below middle-Position? => Pink Noise
          subOscVal = oscPnoise_A.Process();
        else    // White Noise
          subOscVal = oscWnoise_A.Process();
      }
      out_A[i] = out_A[i]*(1.f-f_SubOscFade_A)*lfo_Xfade_SubOSC_A + subOscVal*f_VolWT_A*f_SubOscFade_A*lfo_Xfade_SubOSC_A;    // Crossfade the Wavetable and its suboscillator
    }
    if( t_SubOsc2VCF_A )    // Apply Filter _after_ mix of SubOSC with Wavetable
    {
      switch (iFType_A)    // 0 if filter is off!
      {
        case 1: svf_A.Process<stmlib::FILTER_MODE_LOW_PASS>(out_A, out_A, bufSz); break;
        case 2: svf_A.Process<stmlib::FILTER_MODE_BAND_PASS>(out_A, out_A, bufSz); break;
        case 3: svf_A.Process<stmlib::FILTER_MODE_HIGH_PASS>(out_A, out_A, bufSz); break;
      }
    }
  }
  // === Wave-Table oscillator C ===
  // --- Wave select C ---
  currentBank_C = WaveTblC;
  if (lastBank_C != currentBank_C)
  { // this is slow, hence not modulated by CV
    prepareWavetable((const int16_t **) &wavetables_C, currentBank_C, &isWaveTableGood_C, &fbuffer_C, &buffer_C);
    lastBank_C = currentBank_C;
  }
  // --- Pitch / Tune Wavetable Oscillator C ---
  float f_pitch_C = pitch_C;
  if (cv_pitch_C != -1)
    f_pitch_C += data.cv[cv_pitch_C] * 12.f * 5.f;
  MK_FLT_PAR_ABS_SFT(f_tune_C, tune_C, 1200.f, 1.f);
  const float f_freq_C = plaits::NoteToFrequency(60 + f_tune_C*12.f + f_pitch_C+f_MasterPitch_WT+f_PitchMod_WT) * 0.998f;

  // --- Filter settings for wavetable C ---
  MK_FLT_PAR_ABS(fLFOFMFilt_C, lfo2filtfm_C, 4095.f, 1.f);
  MK_FLT_PAR_ABS(fCut_C, fcut_C, 4095.f, 1.f)
  MK_FLT_PAR_ABS_MIN_MAX(fReso_C, freso_C, 4095.f, 1.f, 20.f);

  // --- Optional filter modulation WT A ---
  MK_TRIG_PAR(t_FilterLFOon_C, FilterLFOon_C);
  if( t_FilterLFOon_C )
  {
    MK_INT_PAR_ABS(i_LFOfilterType_C, LFOfilterType_C, 7.f)
    CONSTRAIN(i_LFOfilterType_C, 0, 6);      // 7 possible types of LFOs
    MK_FLT_PAR_ABS_MIN_MAX(f_LFOSpeed_C, lfospeed_C, 4095.f, 0.05f, 20.f);
    lfo_A.SetFrequency(f_LFOSpeed_C);
    float f_filterLFO_C = lfo_C.Process();
    f_filterLFO_C = morph_sine_wave(f_filterLFO_C, i_LFOfilterType_C, e_filterLFO_C);    // Convert sine-wave to square-wave, pseudo-triangle, S&H and so on if desired
    fCut_C += f_filterLFO_C * fLFOFMFilt_C;   // Filter modulation
    CONSTRAIN(fCut_C, 0.f, 1.f);    // limit values
  }
  fCut_C = 20.f * stmlib::SemitonesToRatio(fCut_C * 120.f);
  svf_C.set_f_q<stmlib::FREQUENCY_FAST>(fCut_C / 44100.f, fReso_C);
  MK_INT_PAR_ABS(iFType_C, fmode_C, 4.f)
  CONSTRAIN(iFType_C, 0, 3);

  // --- Check for Wavetable Z-axis modulation ---
  lfo_WT_C.SetFrequency(f_LFOzScanSpeed_C);
  float f_LFO_WT_C = lfo_WT_C.Process();
  if( t_ScanWavTbl_C)
  {
    // MORPH_SINE(f_LFO_WT_C, i_LFOzScanType_C, e_WT_C);   // Convert sine-wave to square-wave, pseudo-triangle, S&H and so on if desired
    f_LFO_WT_C = morph_sine_wave(f_LFO_WT_C, i_LFOzScanType_C, e_WT_C);
    f_ScanWavTblC += f_LFO_WT_C * f_LFOzScanAmt_C;
    CONSTRAIN(f_ScanWavTblC, 0.f, 1.f)
  }
  // --- Render C: Calc wave and apply filter ---
  float out_C[32] = {0.f};
  if(isWaveTableGood_C)
  {
    wt_osc_C.Render(f_freq_C, f_VolWT_C, f_ScanWavTblC, wavetables_C, out_C, bufSz);
    if(t_SubOscPWM_C)   // PWM modulated square-wave as sub-oscillator?
      oscSub_C.SetFrequency(noteToFreq(f_MasterPitch_SubOSC+f_PitchSubOsc_C+f_PitchMod_SubOsc) );

    float subOscVal = 0.f;
    float lfo_Xfade_SubOSC_C = 1.f;
    if(t_ModulateSubOscXfade_C && t_LfoSAMPxFadeActive_2)   // Modulate SubOSC C Xfade with LFO 2 of Sample XFade?
    {
      lfo_Xfade_SubOSC_C = lfoXfadeSample_2.Process(); // We apply auto-xfades in the final loop, to gain results as smooth as possible!
      lfo_Xfade_SubOSC_C = morph_sine_wave(lfo_Xfade_SubOSC_C, i_LfoTypeSAMPxFade_2, e_XfadeSAMP_2);   // Convert sine-wave to square-wave, pseudo-triangle, S&H and so on if desired
      lfo_Xfade_SubOSC_C *= f_LfoSAMPxFadeRange_2;
    }
    if(!t_SubOsc2VCF_C)    // Apply filter _before_ mix of SubOSC to wavetable
    {
      switch (iFType_C)    // 0 if filter is off!
      {
        case 1: svf_C.Process<stmlib::FILTER_MODE_LOW_PASS>(out_C, out_C, bufSz); break;
        case 2: svf_C.Process<stmlib::FILTER_MODE_BAND_PASS>(out_C, out_C, bufSz);break;
        case 3: svf_C.Process<stmlib::FILTER_MODE_HIGH_PASS>(out_C, out_C, bufSz);break;
      }
    }
    for( int i=0; i<bufSz; i++)
    {
      if(t_SubOscPWM_C)   // PWM modulated square-wave as sub-oscillator?
      {
        subOscVal = oscSub_C.Process();
        subOscVal += f_LFO_pwm*f_PWMintensity; // Add "PWM-offset", we take the amount and LFO-speed for our sub-oscillator from the settings of the table-Z-scan!
        SINE_TO_SQUARE(subOscVal);    // This by nature contains a constrain, avoiding value overflow!
        subOscVal *= f_VolWT_C;
      }
      else                  // Noise as sub-oscillator
      {
        if( f_PitchSubOsc_C < 0.f )   // Pitch-Slider below Middle? => Pink Noise
          subOscVal = oscPnoise_C.Process()*f_VolWT_C;
        else // White noise
          subOscVal = oscWnoise_C.Process()*f_VolWT_C;
      }
      out_C[i] = out_C[i]*(1.f-f_SubOscFade_C)*lfo_Xfade_SubOSC_C + subOscVal*f_SubOscFade_C*lfo_Xfade_SubOSC_C;    // Crossfade the Wavetable and its suboscillator
    }
    if(t_SubOsc2VCF_C)    // Apply filter _after_ mix of SubOSC to wavetable
    {
      switch (iFType_C)    // 0 if filter is off!
      {
        case 1: svf_C.Process<stmlib::FILTER_MODE_LOW_PASS>(out_C, out_C, bufSz); break;
        case 2: svf_C.Process<stmlib::FILTER_MODE_BAND_PASS>(out_C, out_C, bufSz);break;
        case 3: svf_C.Process<stmlib::FILTER_MODE_HIGH_PASS>(out_C, out_C, bufSz);break;
      }
    }
  }
  // === Sample oscillator B ===
  romplers[IDX_OSC_B]->params.gate = true;
  MK_INT_PAR_ABS(i_Bank_B, bank_B, 32.f)
  CONSTRAIN(i_Bank_B, 0, 31)
  MK_INT_PAR_ABS(i_Slice_B, slice_B, 32.f)
  CONSTRAIN(i_Slice_B, 0, 31)
  i_Slice_B = i_Bank_B * 32 + i_Slice_B + wtSliceOffset;
  romplers[IDX_OSC_B]->params.slice = i_Slice_B;

  // --- Sample B pitch related items ---
  MK_FLT_PAR_ABS_SFT(f_Speed_B, speed_B, 2048.f, 2.f)
  romplers[IDX_OSC_B]->params.playbackSpeed = f_Speed_B;
  float f_Pitch_B = pitch_B;
  if (cv_pitch_B != -1)
    f_Pitch_B += data.cv[cv_pitch_B] * 12.f * 5.f;
  romplers[IDX_OSC_B]->params.pitch = f_MasterPitch_SMP+f_Pitch_B+f_PitchMod_Sample;
  MK_FLT_PAR_ABS_SFT(f_Tune_B, tune_B, 1200.f, 12.f)
  romplers[IDX_OSC_B]->params.tune = f_Tune_B;

  // --- Sample Length and Loop-related stuff ---
  MK_FLT_PAR_ABS(f_start_B, start_B, 1048576.f, 1.f);
  romplers[IDX_OSC_B]->params.startOffsetRelative = f_start_B;
  MK_FLT_PAR_ABS(f_length_B, length_B, 1048576.f, 1.f);
  romplers[IDX_OSC_B]->params.lengthRelative = f_length_B;
  MK_FLT_PAR_ABS(f_lpstart_B, lpstart_B, 1048576.f, 1.f)
  romplers[IDX_OSC_B]->params.loopMarker = f_lpstart_B;
  MK_TRIG_PAR(t_loop_B, loop_B)
  romplers[IDX_OSC_B]->params.loop = t_loop_B;
  MK_TRIG_PAR(t_loop_pipo_B, loop_pipo_B)
  romplers[IDX_OSC_B]->params.loopPiPo = t_loop_pipo_B;
  romplers[IDX_OSC_B]->params.gain = f_VolOsc_B;

  // --- Filter params OSC B ---
  MK_FLT_PAR_ABS(f_Cut_B, fcut_B, 4095.f, 1.f)
  romplers[IDX_OSC_B]->params.cutoff = f_Cut_B;
  MK_FLT_PAR_ABS(f_Reso_B, freso_B, 4095.f, 20.f)
  romplers[IDX_OSC_B]->params.resonance = f_Reso_B;
  MK_INT_PAR_ABS(i_FType_B, fmode_B, 4.f)
  CONSTRAIN(i_FType_B, 0, 3);
  romplers[IDX_OSC_B]->params.filterType = static_cast<RomplerVoice::FilterType>(i_FType_B);

  // --- Filter modulation LFO OSC B ---
  MK_FLT_PAR_ABS_MIN_MAX(f_LFOSpeed_B, lfospeed_B, 4095.f, 0.05f, 20.f);
  romplers[IDX_OSC_B]->params.lfoSpeed = f_LFOSpeed_B;
  MK_FLT_PAR_ABS(f_LFOFMFilt_B, lfo2filtfm_B, 4095.f, 1.f)
  MK_TRIG_PAR(t_FilterLFOon_B, FilterLFOon_B);
  romplers[IDX_OSC_B]->params.lfoFMFilter = t_FilterLFOon_B ? f_LFOFMFilt_B : 0.f;

  // --- Render and buffer Sample oscillator B ---
  romplers[IDX_OSC_B]->Process(sample_buf_B, bufSz);

  // === Sample oscillator D ===
  romplers[IDX_OSC_D]->params.gate = true;
  MK_INT_PAR_ABS(i_Bank_D, bank_D, 32.f)
  CONSTRAIN(i_Bank_D, 0, 31)
  MK_INT_PAR_ABS(i_Slice_D, slice_D, 32.f)
  CONSTRAIN(i_Slice_D, 0, 31)
  i_Slice_D = i_Bank_D * 32 + i_Slice_D + wtSliceOffset;
  romplers[IDX_OSC_D]->params.slice = i_Slice_D;
  // --- Sample D pitch related items ---
  MK_FLT_PAR_ABS_SFT(f_Speed_D, speed_D, 2048.f, 2.f)
  romplers[IDX_OSC_D]->params.playbackSpeed = f_Speed_D;
  float f_Pitch_D = pitch_D;
  if (cv_pitch_D != -1)
    f_Pitch_D += data.cv[cv_pitch_D] * 12.f * 5.f;
  romplers[IDX_OSC_D]->params.pitch = f_MasterPitch_SMP+f_Pitch_D+f_PitchMod_Sample;
  MK_FLT_PAR_ABS_SFT(f_Tune_D, tune_D, 2048.f, 12.f)
  romplers[IDX_OSC_D]->params.tune = f_Tune_D;

  // --- Sample Length and Loop-related stuff ---
  MK_FLT_PAR_ABS(f_start_D, start_D, 1048576.f, 1.f);
  romplers[IDX_OSC_D]->params.startOffsetRelative = f_start_D;
  MK_FLT_PAR_ABS(f_length_D, length_D, 1048576.f, 1.f);
  romplers[IDX_OSC_D]->params.lengthRelative = f_length_D;
  MK_FLT_PAR_ABS(f_lpstart_D, lpstart_D, 1048576.f, 1.f)
  romplers[IDX_OSC_D]->params.loopMarker = f_lpstart_D;
  MK_TRIG_PAR(t_loop_D, loop_D)
  romplers[IDX_OSC_D]->params.loop = t_loop_D;
  MK_TRIG_PAR(t_loop_pipo_D, loop_pipo_D)
  romplers[IDX_OSC_D]->params.loopPiPo = t_loop_pipo_D;
  romplers[IDX_OSC_D]->params.gain = f_VolOsc_D;

  // --- Filter params OSC D ---
  MK_FLT_PAR_ABS(f_Cut_D, fcut_D, 4095.f, 1.f);
  romplers[IDX_OSC_D]->params.cutoff = f_Cut_D;
  MK_FLT_PAR_ABS_MIN_MAX(f_Reso_D, freso_D, 4095.f, 0.05f, 20.f);
  romplers[IDX_OSC_D]->params.resonance = f_Reso_D;
  MK_INT_PAR_ABS(i_FType_D, fmode_D, 4.f)
  CONSTRAIN(i_FType_D, 0, 3);
  romplers[IDX_OSC_D]->params.filterType = static_cast<RomplerVoice::FilterType>(i_FType_D);

  // --- Filter modulation LFO OSC D ---
  MK_FLT_PAR_ABS_MIN_MAX(f_lfospeed_D, lfospeed_D, 4095.f, 0.05f, 20.f);
  romplers[IDX_OSC_D]->params.lfoSpeed = f_lfospeed_D;
  MK_FLT_PAR_ABS(f_lfo2filtfm_D, lfo2filtfm_D, 4095.f, 1.f);
  MK_TRIG_PAR(t_FilterLFOon_D, FilterLFOon_D);
  romplers[IDX_OSC_D]->params.lfoFMFilter = t_FilterLFOon_D ? f_lfo2filtfm_D : 0.f;

  // --- Render and buffer Sample oscillator D ---
  romplers[IDX_OSC_D]->Process(sample_buf_D, bufSz);

  // === Volume Envelope ===
  float vol_eg_process = 0.f; // If active we also "precalculate" the Process() values for our main loop (Vol EG first) - this might be slightly less accurate, but uses about 1/32 less performance for these functions
  MK_TRIG_PAR(t_EGvolActive, EGvolActive);
  if( t_EGvolActive )     // Is the envelope activated anyways?
  {
    MK_TRIG_PAR(t_EGvolSlow, EGvolSlow);
    MK_TRIG_PAR(t_EGvolADSRon, EGvolADSRon);
    MK_FLT_PAR_ABS(f_AttackVol, AttackVol, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_DecayVol, DecayVol, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_SustainVol, SustainVol, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_ReleaseVol, ReleaseVol, 4095.f, 10.f);
    if(t_EGvolSlow)   // We extend the EG-times (for AD _and_ ADSR) if slow envelope is selected!
    {
      f_AttackVol *= 30.f;
      f_DecayVol *= 30.f;
      f_ReleaseVol *= 30.f;
    }
    // --- Decide if to use ADSR or AD envelope
    if(t_EGvolADSRon)   // ADSR mode
    {
      vol_eg_adsr.SetAttack(f_AttackVol);
      vol_eg_adsr.SetDecay(f_DecayVol);
      vol_eg_adsr.SetSustain(f_SustainVol);
      vol_eg_adsr.SetRelease(f_ReleaseVol);

      vol_eg_adsr.Gate(g_Gate);
      vol_eg_process = vol_eg_adsr.Process();   // Precalculate current Volume EG, it will be added in the "main" DSP-loop below
    }
    else  // AD mode
    {
      vol_eg.SetAttack(f_AttackVol);
      vol_eg.SetDecay(f_DecayVol);

      if (t_Gate == GATE_HIGH_NEW)    // New trigger encountered?
        vol_eg.Trigger();
      vol_eg_process = vol_eg.Process();   // Precalculate current Volume EG, it will be added in the "main" DSP-loop below
    }
  }
  else    // EGs (AD or ADSR) are not active, we Reset the ADSR!
    vol_eg_adsr.Reset();

  // === Panner/Tremolo ===
  MK_TRIG_PAR(t_PannerOn, PannerOn);
  MK_FLT_PAR_ABS(f_PanAmnt_HI, PanAmnt, 4095.f, 1.f);
  float f_PanAmnt_LO = 1.f-f_PanAmnt_HI;
  MK_FLT_PAR_ABS_MIN_MAX(f_PanFreq, PanFreq, 4095.f, 0.05f, 15.f);
  lfoPanner.SetFrequency(f_PanFreq);



  // --- Precalculate values for Panner ---
  float panValue = (fastsin(lfoPanner.Process())+1.f) * M_PI / 4.f;             // Panning based on the algorithm found here: https://audioordeal.co.uk/how-to-build-a-vst-lesson-2-autopanner/
  float fastcos_panValue = fastcos(panValue);
  float fastsin_panValue = fastsin(panValue);

  // --- Precalculate Vector XFades LFOs for automated Wavetable Xfades ---
  if(t_LfoWTxFadeActive_1)
  {
    float f_lfo_value_Xfade_WT_1 = lfoXfadeWT_1.Process(); // We apply auto-xfades in the final loop, to gain results as smooth as possible!
    f_lfo_value_Xfade_WT_1 = morph_sine_wave(f_lfo_value_Xfade_WT_1, i_LfoTypeWTxFade_1, e_XfadeWT_1);   // Convert sine-wave to square-wave, pseudo-triangle, S&H and so on if desired
    f_XfadeWaveTbls += f_LfoWTxFadeRange_1 * f_lfo_value_Xfade_WT_1;
  }
  if(t_LfoWTxFadeActive_2 && !t_ModulateSubOscXfade_C )   // Modulate SubOSC Xfade instead?
  {
    float f_lfo_value_Xfade_WT_2 = lfoXfadeWT_2.Process(); // We apply auto-xfades in the final loop, to gain results as smooth as possible!
    f_lfo_value_Xfade_WT_2 = morph_sine_wave(f_lfo_value_Xfade_WT_2, i_LfoTypeWTxFade_2, e_XfadeWT_2);   // Convert sine-wave to square-wave, pseudo-triangle, S&H and so on if desired
    f_XfadeWaveTbls += f_LfoWTxFadeRange_2 * f_lfo_value_Xfade_WT_2;
  }
  CONSTRAIN(f_XfadeWaveTbls,0.f, 1.f);

  // --- Precalculate Vector XFades LFOs for automated Sample Xfades ---
  if(t_LfoSAMPxFadeActive_1)
  {
    float f_lfo_value_Xfade_SMP_1 = lfoXfadeSample_1.Process(); // We apply auto-xfades in the final loop, to gain results as smooth as possible!
    f_lfo_value_Xfade_SMP_1 = morph_sine_wave(f_lfo_value_Xfade_SMP_1, i_LfoTypeSAMPxFade_1, e_XfadeSAMP_1);   // Convert sine-wave to square-wave, pseudo-triangle, S&H and so on if desired
    f_XfadeSamples += f_LfoSAMPxFadeRange_1 * f_lfo_value_Xfade_SMP_1;
  }
  if(t_LfoSAMPxFadeActive_2 && !t_ModulateSubOscXfade_C )   // Modulate SubOSC Xfade instead?
  {
    float f_lfo_value_Xfade_SMP_2 = lfoXfadeSample_2.Process(); // We apply auto-xfades in the final loop, to gain results as smooth as possible!
    f_lfo_value_Xfade_SMP_2 = morph_sine_wave(f_lfo_value_Xfade_SMP_2, i_LfoTypeSAMPxFade_2, e_XfadeSAMP_2);   // Convert sine-wave to square-wave, pseudo-triangle, S&H and so on if desired
    f_XfadeSamples += f_LfoSAMPxFadeRange_2 * f_lfo_value_Xfade_SMP_2;
  }
  CONSTRAIN(f_XfadeSamples, 0.f, 1.f);

  // --- Precalculate elements for Vector-Space mixer including Mastervolume ---
  float f_mix_factor = t_StereoSplit ? 0.5f : 0.25f;    // We mix 4 inputs mono or 2 inputs per stereo-channel
  float f_XfadeWaveTbls_A = (1.f-f_XfadeWaveTbls) * f_mix_factor * f_Volume;
  float f_XfadeWaveTbls_C = (f_XfadeWaveTbls) * f_mix_factor * f_Volume;
  float f_XfadeSamples_D = (1.f-f_XfadeSamples) * f_mix_factor * f_Volume;
  float f_XfadeSamples_B = (f_XfadeSamples)  * f_mix_factor * f_Volume;
  float f_val_result_l = 0.f;
  float f_val_result_r = 0.f;

  // === DSP-output: This is the loop where the audio-output is written ===
  for (uint32_t i = 0; i < bufSz; i++)
  {
    // --- Decide if we have to pan groups of oscillators to left/right ---
    if(t_StereoSplit)
    {
      f_val_result_l = out_A[i] * f_XfadeWaveTbls_A;        // Get precalculated data from wavetable-voice A for output
      f_val_result_r = out_C[i] * f_XfadeWaveTbls_C;       // Mix with precalculated data from wavetable-voice B for output
      f_val_result_l += sample_buf_D[i] * f_XfadeSamples_D;      // Crossfade samples and mix with wavetables
      f_val_result_r += sample_buf_B[i] * f_XfadeSamples_B;

      if(t_EGvolActive)                                 // Apply volume EG?
      {
        f_val_result_l *= vol_eg_process;
        f_val_result_r *= vol_eg_process;
      }
      if (t_PannerOn && f_PanAmnt_HI != 0.f)            // Autopanner is on / has amount > 0
      {
        float f_val_result_l_dry = f_val_result_l;
        float f_val_result_r_dry = f_val_result_r;
        f_val_result_l *= fastcos_panValue; // fastcos(panValue);
        f_val_result_r *= fastsin_panValue; // fastsin(panValue);

        f_val_result_l = f_val_result_l_dry*f_PanAmnt_LO + f_val_result_l*f_PanAmnt_HI;  // Crossfade with original signal, depending on needed amount of panner
        CONSTRAIN(f_val_result_l, -1.f, 1.f);         // Limit result to max. audio-level
        data.buf[i * 2] = f_val_result_l;         // Left channel (sound in principle is mono, without the right channel we will have a tremelo instead of an auto-panner effect!

        f_val_result_r = f_val_result_r_dry * f_PanAmnt_LO + f_val_result_r * f_PanAmnt_HI;  // Crossfade with original signal, depending on needed amount of panner
        CONSTRAIN(f_val_result_r, -1.f, 1.f);         // Limit result to max. audio-level
        data.buf[i * 2 + 1] = f_val_result_r;       // Right channel output

      }
      else  // Channels are split, but no auto-panner is active
      {
        CONSTRAIN(f_val_result_l, -1.f, 1.f);     // Limit result to max. audio-level
        data.buf[i * 2] = f_val_result_l;            // Left channel

        CONSTRAIN(f_val_result_r, -1.f, 1.f);     // Limit result to max. audio-level
        data.buf[i * 2 + 1] = f_val_result_r;          // Right channel output
      }
    }
    else      // XFades are not panned Left/Right, but we still may have an auto-panner effect on the mono-signal
    {
      // --- Oscillator-Mix and Mastervolume (truncate audio in case of clipping) ---
      f_val_result = out_A[i] * f_XfadeWaveTbls_A;        // Get precalculated data from wavetable-voice A for output
      f_val_result += out_C[i] * f_XfadeWaveTbls_C;       // Mix with precalculated data from wavetable-voice B for output
      f_val_result += sample_buf_D[i] * f_XfadeSamples_D;      // Crossfade samples and mix with wavetables
      f_val_result += sample_buf_B[i] * f_XfadeSamples_B;

      if (t_EGvolActive)                                 // Apply volume EG?
        f_val_result *= vol_eg_process;

      // --- Output of DSP-results, add Autopanner if needed (Please note: we apply panning in the final loop, to gain results as smooth as possible) ---
      if (t_PannerOn && f_PanAmnt_HI != 0.f)       // Autopanner is on / has amount > 0
      {
        f_val_result_l = f_val_result * fastcos_panValue; // fastcos(panValue);
        f_val_result_r = f_val_result * fastsin_panValue; // fastsin(panValue);

        f_val_result_l = f_val_result * f_PanAmnt_LO + f_val_result_l * f_PanAmnt_HI;  // Crossfade with original signal, depending on needed amount of panner
        CONSTRAIN(f_val_result_l, -1.f, 1.f);         // Limit result to max. audio-level
        data.buf[i * 2] = f_val_result_l;         // Left channel (sound in principle is mono, without the right channel we will have a tremelo instead of an auto-panner effect!

        f_val_result_r = f_val_result * f_PanAmnt_LO + f_val_result_r * f_PanAmnt_HI;  // Crossfade with original signal, depending on needed amount of panner
        CONSTRAIN(f_val_result_r, -1.f, 1.f);         // Limit result to max. audio-level
        data.buf[i * 2 + 1] = f_val_result_r;       // Right channel output
      }
      else    // No channel-split or auto-panning
      {
        CONSTRAIN(f_val_result, -1.f, 1.f);     // Limit result to max. audio-level
        data.buf[i * 2] = f_val_result;            // Left channel
        data.buf[i * 2 + 1] = f_val_result;          // Right channel output
      }
    }
  }
}

// --- Constructor for VctrSnt ---
ctagSoundProcessorVctrSnt::ctagSoundProcessorVctrSnt()
{
  // construct internal data model
  knowYourself();
  model = std::make_unique<ctagSPDataModel>(id, isStereo);
  LoadPreset(0);

  // Alloc mem for one wavetable (Oscillator A)
  buffer_A = (int16_t*)heap_caps_malloc(260*64*2, MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT); // 260 = wavetable size after prep, 64 wavetables, 2 bytes per sample (int16)
  assert(buffer_A != NULL);
  memset(buffer_A, 0, 260*64*2);
  fbuffer_A = (float*)heap_caps_malloc(512*4, MALLOC_CAP_8BIT|MALLOC_CAP_INTERNAL); // buffer for wavetable prep computations
  assert(fbuffer_A != NULL);
  memset(fbuffer_A, 0, 512*4);
  wt_osc_A.Init();
  svf_A.Init();

  // Alloc mem for one wavetable (Oscillator C)
  buffer_C = (int16_t*)heap_caps_malloc(260*64*2, MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT); // 260 = wavetable size after prep, 64 wavetables, 2 bytes per sample (int16)
  assert(buffer_C != NULL);
  memset(buffer_C, 0, 260*64*2);
  fbuffer_C = (float*)heap_caps_malloc(512*4, MALLOC_CAP_8BIT|MALLOC_CAP_INTERNAL); // buffer for wavetable prep computations
  assert(fbuffer_C != NULL);
  memset(fbuffer_C, 0, 512*4);
  wt_osc_C.Init();
  svf_C.Init();

  // Get offset of samples that are no wavetables
  wtSliceOffset = sampleRom.GetFirstNonWaveTableSlice();

  // Create sample-oscillators / Rompler-voices and initialize Rompler voices regarding parameters not actively used here ---
  for( int i=0; i < 2; i++)
  {
    romplers[i] = std::make_unique<RomplerVoice>();
    romplers[i]->params.s = 1.f;          // Set Sustain to max, "just in case"
    romplers[i]->params.resonance = 1.f;  // Set Resonance to 1 (should not be 0), "just in case"
  }
  // --- Initialize Volume Envelope ---
  vol_eg.SetSampleRate(44100.f/ bufSz);    // Sync Env with our audio-processing
  vol_eg.SetModeExp();  // Logarithmic scaling
  vol_eg_adsr.SetSampleRate(44100.f/ bufSz);    // Optional ADSR-EG: sync Env with our audio-processing
  vol_eg_adsr.SetModeExp();                     // Logarithmic scaling
  vol_eg_adsr.Reset();

  // --- Suboscillators (PWM, set outside main loop) ---
  oscSub_A.SetSampleRate(44100.f);      // MB ### 20210223 changed from: (44100.f / bufSz); because Process() is called in main loop!
  oscSub_A.SetFrequency(1.f);
  oscSub_C.SetSampleRate(44100.f);      // MB ### 20210223 changed from: (44100.f / bufSz); because Process() is called in main loop!
  oscSub_C.SetFrequency(1.f);

  // --- LFO to modulate the pitch of the Oscillators ---
  lfoPitch.SetSampleRate(44100.f / bufSz);   // Please note: because the LFO is applied already outside the DSP-loop we reduce it's frequency in a manner to fit
  lfoPitch.SetFrequency(1.f);

  // --- LFO for autopanning effect ---
  lfoPanner.SetSampleRate(44100.f / bufSz);
  lfoPanner.SetFrequency(1.f);

  // --- LFO for PWM of (optional) Sub Oscillators ---
  lfoPWM.SetSampleRate(44100.f / bufSz);
  lfoPWM.SetFrequency(1.f);

  // --- LFOs for filter-mod of Wavetables ---
  lfo_A.SetSampleRate(44100.f / bufSz);
  lfo_A.SetFrequency(1.f);
  lfo_C.SetSampleRate(44100.f / bufSz);
  lfo_C.SetFrequency(1.f);

  // --- LFOs for Vector Xfades of Wavetables ---
  lfoXfadeWT_1.SetSampleRate(44100.f / bufSz);
  lfoXfadeWT_1.SetFrequency(1.f);
  lfoXfadeWT_2.SetSampleRate(44100.f / bufSz);
  lfoXfadeWT_2.SetFrequency(1.f);

  // --- LFOs for Vector Xfades of Samples ---
  lfoXfadeSample_1.SetSampleRate(44100.f / bufSz);
  lfoXfadeSample_1.SetFrequency(1.f);
  lfoXfadeSample_2.SetSampleRate(44100.f / bufSz);
  lfoXfadeSample_2.SetFrequency(1.f);

  // --- LFOs for Z-Scan-mod of Wavetables ---
  lfo_WT_A.SetSampleRate(44100.f / bufSz);
  lfo_WT_A.SetFrequency(1.f);
  lfo_WT_C.SetSampleRate(44100.f / bufSz);
  lfo_WT_C.SetFrequency(1.f);

  Gate = false;   // Init Gate to off for instanziation of object, so that we wait for a trigger
}

// --- Destructor for VctrSnt (cleaning up is important! ;-) ---
ctagSoundProcessorVctrSnt::~ctagSoundProcessorVctrSnt()
{
  // Free mem for one wavetable (Oscillator A)
  heap_caps_free(buffer_A);
  heap_caps_free(fbuffer_A);
  // Free mem for one wavetable (Oscillator C)
  heap_caps_free(buffer_C);
  heap_caps_free(fbuffer_C);
}

// --- Entrypoint for VctrSnt's parameters, automatically generated by the TBD framework (factory design pattern) ---
void ctagSoundProcessorVctrSnt::knowYourself()
{
  // autogenerated code here
  // sectionCpp0
	pMapPar.emplace("Gate", [&](const int val){ Gate = val;});
	pMapTrig.emplace("Gate", [&](const int val){ trig_Gate = val;});
	pMapPar.emplace("EGvolGate", [&](const int val){ EGvolGate = val;});
	pMapTrig.emplace("EGvolGate", [&](const int val){ trig_EGvolGate = val;});
	pMapPar.emplace("MasterPitch", [&](const int val){ MasterPitch = val;});
	pMapCv.emplace("MasterPitch", [&](const int val){ cv_MasterPitch = val;});
	pMapPar.emplace("MasterTune", [&](const int val){ MasterTune = val;});
	pMapCv.emplace("MasterTune", [&](const int val){ cv_MasterTune = val;});
	pMapPar.emplace("QuantizePitch", [&](const int val){ QuantizePitch = val;});
	pMapTrig.emplace("QuantizePitch", [&](const int val){ trig_QuantizePitch = val;});
	pMapPar.emplace("Volume", [&](const int val){ Volume = val;});
	pMapCv.emplace("Volume", [&](const int val){ cv_Volume = val;});
	pMapPar.emplace("ExclSubOSCmasterPitch", [&](const int val){ ExclSubOSCmasterPitch = val;});
	pMapTrig.emplace("ExclSubOSCmasterPitch", [&](const int val){ trig_ExclSubOSCmasterPitch = val;});
	pMapPar.emplace("ExclWTmasterPitch", [&](const int val){ ExclWTmasterPitch = val;});
	pMapTrig.emplace("ExclWTmasterPitch", [&](const int val){ trig_ExclWTmasterPitch = val;});
	pMapPar.emplace("ExclSMPmasterPitch", [&](const int val){ ExclSMPmasterPitch = val;});
	pMapTrig.emplace("ExclSMPmasterPitch", [&](const int val){ trig_ExclSMPmasterPitch = val;});
	pMapPar.emplace("PWMintensity", [&](const int val){ PWMintensity = val;});
	pMapCv.emplace("PWMintensity", [&](const int val){ cv_PWMintensity = val;});
	pMapPar.emplace("PWMspeed", [&](const int val){ PWMspeed = val;});
	pMapCv.emplace("PWMspeed", [&](const int val){ cv_PWMspeed = val;});
	pMapPar.emplace("SubOscPWM_A", [&](const int val){ SubOscPWM_A = val;});
	pMapTrig.emplace("SubOscPWM_A", [&](const int val){ trig_SubOscPWM_A = val;});
	pMapPar.emplace("SubOsc2VCF_A", [&](const int val){ SubOsc2VCF_A = val;});
	pMapTrig.emplace("SubOsc2VCF_A", [&](const int val){ trig_SubOsc2VCF_A = val;});
	pMapPar.emplace("PitchSubOsc_A", [&](const int val){ PitchSubOsc_A = val;});
	pMapCv.emplace("PitchSubOsc_A", [&](const int val){ cv_PitchSubOsc_A = val;});
	pMapPar.emplace("SubOscFade_A", [&](const int val){ SubOscFade_A = val;});
	pMapCv.emplace("SubOscFade_A", [&](const int val){ cv_SubOscFade_A = val;});
	pMapPar.emplace("SubOscPWM_C", [&](const int val){ SubOscPWM_C = val;});
	pMapTrig.emplace("SubOscPWM_C", [&](const int val){ trig_SubOscPWM_C = val;});
	pMapPar.emplace("SubOsc2VCF_C", [&](const int val){ SubOsc2VCF_C = val;});
	pMapTrig.emplace("SubOsc2VCF_C", [&](const int val){ trig_SubOsc2VCF_C = val;});
	pMapPar.emplace("PitchSubOsc_C", [&](const int val){ PitchSubOsc_C = val;});
	pMapCv.emplace("PitchSubOsc_C", [&](const int val){ cv_PitchSubOsc_C = val;});
	pMapPar.emplace("SubOscFade_C", [&](const int val){ SubOscFade_C = val;});
	pMapCv.emplace("SubOscFade_C", [&](const int val){ cv_SubOscFade_C = val;});
	pMapPar.emplace("VolWT_A", [&](const int val){ VolWT_A = val;});
	pMapCv.emplace("VolWT_A", [&](const int val){ cv_VolWT_A = val;});
	pMapPar.emplace("VolOsc_B", [&](const int val){ VolOsc_B = val;});
	pMapCv.emplace("VolOsc_B", [&](const int val){ cv_VolOsc_B = val;});
	pMapPar.emplace("VolWT_C", [&](const int val){ VolWT_C = val;});
	pMapCv.emplace("VolWT_C", [&](const int val){ cv_VolWT_C = val;});
	pMapPar.emplace("VolOsc_D", [&](const int val){ VolOsc_D = val;});
	pMapCv.emplace("VolOsc_D", [&](const int val){ cv_VolOsc_D = val;});
	pMapPar.emplace("StereoSplit", [&](const int val){ StereoSplit = val;});
	pMapTrig.emplace("StereoSplit", [&](const int val){ trig_StereoSplit = val;});
	pMapPar.emplace("XfadeWaveTbls", [&](const int val){ XfadeWaveTbls = val;});
	pMapCv.emplace("XfadeWaveTbls", [&](const int val){ cv_XfadeWaveTbls = val;});
	pMapPar.emplace("XfadeSamples", [&](const int val){ XfadeSamples = val;});
	pMapCv.emplace("XfadeSamples", [&](const int val){ cv_XfadeSamples = val;});
	pMapPar.emplace("LfoWTxFadeActive_1", [&](const int val){ LfoWTxFadeActive_1 = val;});
	pMapTrig.emplace("LfoWTxFadeActive_1", [&](const int val){ trig_LfoWTxFadeActive_1 = val;});
	pMapPar.emplace("LfoTypeWTxFade_1", [&](const int val){ LfoTypeWTxFade_1 = val;});
	pMapCv.emplace("LfoTypeWTxFade_1", [&](const int val){ cv_LfoTypeWTxFade_1 = val;});
	pMapPar.emplace("LfoWTxFadeRange_1", [&](const int val){ LfoWTxFadeRange_1 = val;});
	pMapCv.emplace("LfoWTxFadeRange_1", [&](const int val){ cv_LfoWTxFadeRange_1 = val;});
	pMapPar.emplace("LfoWTxFadeSpeed_1", [&](const int val){ LfoWTxFadeSpeed_1 = val;});
	pMapCv.emplace("LfoWTxFadeSpeed_1", [&](const int val){ cv_LfoWTxFadeSpeed_1 = val;});
	pMapPar.emplace("LfoWTxFadeActive_2", [&](const int val){ LfoWTxFadeActive_2 = val;});
	pMapTrig.emplace("LfoWTxFadeActive_2", [&](const int val){ trig_LfoWTxFadeActive_2 = val;});
	pMapPar.emplace("ModulateSubOscXfade_A", [&](const int val){ ModulateSubOscXfade_A = val;});
	pMapTrig.emplace("ModulateSubOscXfade_A", [&](const int val){ trig_ModulateSubOscXfade_A = val;});
	pMapPar.emplace("LfoTypeWTxFade_2", [&](const int val){ LfoTypeWTxFade_2 = val;});
	pMapCv.emplace("LfoTypeWTxFade_2", [&](const int val){ cv_LfoTypeWTxFade_2 = val;});
	pMapPar.emplace("LfoWTxFadeRange_2", [&](const int val){ LfoWTxFadeRange_2 = val;});
	pMapCv.emplace("LfoWTxFadeRange_2", [&](const int val){ cv_LfoWTxFadeRange_2 = val;});
	pMapPar.emplace("LfoWTxFadeSpeed_2", [&](const int val){ LfoWTxFadeSpeed_2 = val;});
	pMapCv.emplace("LfoWTxFadeSpeed_2", [&](const int val){ cv_LfoWTxFadeSpeed_2 = val;});
	pMapPar.emplace("LfoSAMPxFadeActive_1", [&](const int val){ LfoSAMPxFadeActive_1 = val;});
	pMapTrig.emplace("LfoSAMPxFadeActive_1", [&](const int val){ trig_LfoSAMPxFadeActive_1 = val;});
	pMapPar.emplace("LfoTypeSAMPxFade_1", [&](const int val){ LfoTypeSAMPxFade_1 = val;});
	pMapCv.emplace("LfoTypeSAMPxFade_1", [&](const int val){ cv_LfoTypeSAMPxFade_1 = val;});
	pMapPar.emplace("LfoSAMPxFadeRange_1", [&](const int val){ LfoSAMPxFadeRange_1 = val;});
	pMapCv.emplace("LfoSAMPxFadeRange_1", [&](const int val){ cv_LfoSAMPxFadeRange_1 = val;});
	pMapPar.emplace("LfoSAMPxFadeSpeed_1", [&](const int val){ LfoSAMPxFadeSpeed_1 = val;});
	pMapCv.emplace("LfoSAMPxFadeSpeed_1", [&](const int val){ cv_LfoSAMPxFadeSpeed_1 = val;});
	pMapPar.emplace("LfoSAMPxFadeActive_2", [&](const int val){ LfoSAMPxFadeActive_2 = val;});
	pMapTrig.emplace("LfoSAMPxFadeActive_2", [&](const int val){ trig_LfoSAMPxFadeActive_2 = val;});
	pMapPar.emplace("ModulateSubOscXfade_C", [&](const int val){ ModulateSubOscXfade_C = val;});
	pMapTrig.emplace("ModulateSubOscXfade_C", [&](const int val){ trig_ModulateSubOscXfade_C = val;});
	pMapPar.emplace("LfoTypeSAMPxFade_2", [&](const int val){ LfoTypeSAMPxFade_2 = val;});
	pMapCv.emplace("LfoTypeSAMPxFade_2", [&](const int val){ cv_LfoTypeSAMPxFade_2 = val;});
	pMapPar.emplace("LfoSAMPxFadeRange_2", [&](const int val){ LfoSAMPxFadeRange_2 = val;});
	pMapCv.emplace("LfoSAMPxFadeRange_2", [&](const int val){ cv_LfoSAMPxFadeRange_2 = val;});
	pMapPar.emplace("LfoSAMPxFadeSpeed_2", [&](const int val){ LfoSAMPxFadeSpeed_2 = val;});
	pMapCv.emplace("LfoSAMPxFadeSpeed_2", [&](const int val){ cv_LfoSAMPxFadeSpeed_2 = val;});
	pMapPar.emplace("WaveTblA", [&](const int val){ WaveTblA = val;});
	pMapCv.emplace("WaveTblA", [&](const int val){ cv_WaveTblA = val;});
	pMapPar.emplace("ScanWavTblA", [&](const int val){ ScanWavTblA = val;});
	pMapCv.emplace("ScanWavTblA", [&](const int val){ cv_ScanWavTblA = val;});
	pMapPar.emplace("pitch_A", [&](const int val){ pitch_A = val;});
	pMapCv.emplace("pitch_A", [&](const int val){ cv_pitch_A = val;});
	pMapPar.emplace("tune_A", [&](const int val){ tune_A = val;});
	pMapCv.emplace("tune_A", [&](const int val){ cv_tune_A = val;});
	pMapPar.emplace("ScanWavTbl_A", [&](const int val){ ScanWavTbl_A = val;});
	pMapTrig.emplace("ScanWavTbl_A", [&](const int val){ trig_ScanWavTbl_A = val;});
	pMapPar.emplace("LFOzScanType_A", [&](const int val){ LFOzScanType_A = val;});
	pMapCv.emplace("LFOzScanType_A", [&](const int val){ cv_LFOzScanType_A = val;});
	pMapPar.emplace("LFOzScanAmt_A", [&](const int val){ LFOzScanAmt_A = val;});
	pMapCv.emplace("LFOzScanAmt_A", [&](const int val){ cv_LFOzScanAmt_A = val;});
	pMapPar.emplace("LFOzScanSpeed_A", [&](const int val){ LFOzScanSpeed_A = val;});
	pMapCv.emplace("LFOzScanSpeed_A", [&](const int val){ cv_LFOzScanSpeed_A = val;});
	pMapPar.emplace("fmode_A", [&](const int val){ fmode_A = val;});
	pMapCv.emplace("fmode_A", [&](const int val){ cv_fmode_A = val;});
	pMapPar.emplace("fcut_A", [&](const int val){ fcut_A = val;});
	pMapCv.emplace("fcut_A", [&](const int val){ cv_fcut_A = val;});
	pMapPar.emplace("freso_A", [&](const int val){ freso_A = val;});
	pMapCv.emplace("freso_A", [&](const int val){ cv_freso_A = val;});
	pMapPar.emplace("FilterLFOon_A", [&](const int val){ FilterLFOon_A = val;});
	pMapTrig.emplace("FilterLFOon_A", [&](const int val){ trig_FilterLFOon_A = val;});
	pMapPar.emplace("LFOfilterType_A", [&](const int val){ LFOfilterType_A = val;});
	pMapCv.emplace("LFOfilterType_A", [&](const int val){ cv_LFOfilterType_A = val;});
	pMapPar.emplace("lfo2filtfm_A", [&](const int val){ lfo2filtfm_A = val;});
	pMapCv.emplace("lfo2filtfm_A", [&](const int val){ cv_lfo2filtfm_A = val;});
	pMapPar.emplace("lfospeed_A", [&](const int val){ lfospeed_A = val;});
	pMapCv.emplace("lfospeed_A", [&](const int val){ cv_lfospeed_A = val;});
	pMapPar.emplace("bank_B", [&](const int val){ bank_B = val;});
	pMapCv.emplace("bank_B", [&](const int val){ cv_bank_B = val;});
	pMapPar.emplace("slice_B", [&](const int val){ slice_B = val;});
	pMapCv.emplace("slice_B", [&](const int val){ cv_slice_B = val;});
	pMapPar.emplace("speed_B", [&](const int val){ speed_B = val;});
	pMapCv.emplace("speed_B", [&](const int val){ cv_speed_B = val;});
	pMapPar.emplace("pitch_B", [&](const int val){ pitch_B = val;});
	pMapCv.emplace("pitch_B", [&](const int val){ cv_pitch_B = val;});
	pMapPar.emplace("tune_B", [&](const int val){ tune_B = val;});
	pMapCv.emplace("tune_B", [&](const int val){ cv_tune_B = val;});
	pMapPar.emplace("start_B", [&](const int val){ start_B = val;});
	pMapCv.emplace("start_B", [&](const int val){ cv_start_B = val;});
	pMapPar.emplace("length_B", [&](const int val){ length_B = val;});
	pMapCv.emplace("length_B", [&](const int val){ cv_length_B = val;});
	pMapPar.emplace("loop_B", [&](const int val){ loop_B = val;});
	pMapTrig.emplace("loop_B", [&](const int val){ trig_loop_B = val;});
	pMapPar.emplace("loop_pipo_B", [&](const int val){ loop_pipo_B = val;});
	pMapTrig.emplace("loop_pipo_B", [&](const int val){ trig_loop_pipo_B = val;});
	pMapPar.emplace("lpstart_B", [&](const int val){ lpstart_B = val;});
	pMapCv.emplace("lpstart_B", [&](const int val){ cv_lpstart_B = val;});
	pMapPar.emplace("fmode_B", [&](const int val){ fmode_B = val;});
	pMapCv.emplace("fmode_B", [&](const int val){ cv_fmode_B = val;});
	pMapPar.emplace("fcut_B", [&](const int val){ fcut_B = val;});
	pMapCv.emplace("fcut_B", [&](const int val){ cv_fcut_B = val;});
	pMapPar.emplace("freso_B", [&](const int val){ freso_B = val;});
	pMapCv.emplace("freso_B", [&](const int val){ cv_freso_B = val;});
	pMapPar.emplace("FilterLFOon_B", [&](const int val){ FilterLFOon_B = val;});
	pMapTrig.emplace("FilterLFOon_B", [&](const int val){ trig_FilterLFOon_B = val;});
	pMapPar.emplace("lfo2filtfm_B", [&](const int val){ lfo2filtfm_B = val;});
	pMapCv.emplace("lfo2filtfm_B", [&](const int val){ cv_lfo2filtfm_B = val;});
	pMapPar.emplace("lfospeed_B", [&](const int val){ lfospeed_B = val;});
	pMapCv.emplace("lfospeed_B", [&](const int val){ cv_lfospeed_B = val;});
	pMapPar.emplace("WaveTblC", [&](const int val){ WaveTblC = val;});
	pMapCv.emplace("WaveTblC", [&](const int val){ cv_WaveTblC = val;});
	pMapPar.emplace("ScanWavTblC", [&](const int val){ ScanWavTblC = val;});
	pMapCv.emplace("ScanWavTblC", [&](const int val){ cv_ScanWavTblC = val;});
	pMapPar.emplace("pitch_C", [&](const int val){ pitch_C = val;});
	pMapCv.emplace("pitch_C", [&](const int val){ cv_pitch_C = val;});
	pMapPar.emplace("tune_C", [&](const int val){ tune_C = val;});
	pMapCv.emplace("tune_C", [&](const int val){ cv_tune_C = val;});
	pMapPar.emplace("ScanWavTbl_C", [&](const int val){ ScanWavTbl_C = val;});
	pMapTrig.emplace("ScanWavTbl_C", [&](const int val){ trig_ScanWavTbl_C = val;});
	pMapPar.emplace("LFOzScanType_C", [&](const int val){ LFOzScanType_C = val;});
	pMapCv.emplace("LFOzScanType_C", [&](const int val){ cv_LFOzScanType_C = val;});
	pMapPar.emplace("LFOzScanAmt_C", [&](const int val){ LFOzScanAmt_C = val;});
	pMapCv.emplace("LFOzScanAmt_C", [&](const int val){ cv_LFOzScanAmt_C = val;});
	pMapPar.emplace("LFOzScanSpeed_C", [&](const int val){ LFOzScanSpeed_C = val;});
	pMapCv.emplace("LFOzScanSpeed_C", [&](const int val){ cv_LFOzScanSpeed_C = val;});
	pMapPar.emplace("fmode_C", [&](const int val){ fmode_C = val;});
	pMapCv.emplace("fmode_C", [&](const int val){ cv_fmode_C = val;});
	pMapPar.emplace("fcut_C", [&](const int val){ fcut_C = val;});
	pMapCv.emplace("fcut_C", [&](const int val){ cv_fcut_C = val;});
	pMapPar.emplace("freso_C", [&](const int val){ freso_C = val;});
	pMapCv.emplace("freso_C", [&](const int val){ cv_freso_C = val;});
	pMapPar.emplace("FilterLFOon_C", [&](const int val){ FilterLFOon_C = val;});
	pMapTrig.emplace("FilterLFOon_C", [&](const int val){ trig_FilterLFOon_C = val;});
	pMapPar.emplace("LFOfilterType_C", [&](const int val){ LFOfilterType_C = val;});
	pMapCv.emplace("LFOfilterType_C", [&](const int val){ cv_LFOfilterType_C = val;});
	pMapPar.emplace("lfo2filtfm_C", [&](const int val){ lfo2filtfm_C = val;});
	pMapCv.emplace("lfo2filtfm_C", [&](const int val){ cv_lfo2filtfm_C = val;});
	pMapPar.emplace("lfospeed_C", [&](const int val){ lfospeed_C = val;});
	pMapCv.emplace("lfospeed_C", [&](const int val){ cv_lfospeed_C = val;});
	pMapPar.emplace("bank_D", [&](const int val){ bank_D = val;});
	pMapCv.emplace("bank_D", [&](const int val){ cv_bank_D = val;});
	pMapPar.emplace("slice_D", [&](const int val){ slice_D = val;});
	pMapCv.emplace("slice_D", [&](const int val){ cv_slice_D = val;});
	pMapPar.emplace("speed_D", [&](const int val){ speed_D = val;});
	pMapCv.emplace("speed_D", [&](const int val){ cv_speed_D = val;});
	pMapPar.emplace("pitch_D", [&](const int val){ pitch_D = val;});
	pMapCv.emplace("pitch_D", [&](const int val){ cv_pitch_D = val;});
	pMapPar.emplace("tune_D", [&](const int val){ tune_D = val;});
	pMapCv.emplace("tune_D", [&](const int val){ cv_tune_D = val;});
	pMapPar.emplace("start_D", [&](const int val){ start_D = val;});
	pMapCv.emplace("start_D", [&](const int val){ cv_start_D = val;});
	pMapPar.emplace("length_D", [&](const int val){ length_D = val;});
	pMapCv.emplace("length_D", [&](const int val){ cv_length_D = val;});
	pMapPar.emplace("loop_D", [&](const int val){ loop_D = val;});
	pMapTrig.emplace("loop_D", [&](const int val){ trig_loop_D = val;});
	pMapPar.emplace("loop_pipo_D", [&](const int val){ loop_pipo_D = val;});
	pMapTrig.emplace("loop_pipo_D", [&](const int val){ trig_loop_pipo_D = val;});
	pMapPar.emplace("lpstart_D", [&](const int val){ lpstart_D = val;});
	pMapCv.emplace("lpstart_D", [&](const int val){ cv_lpstart_D = val;});
	pMapPar.emplace("fmode_D", [&](const int val){ fmode_D = val;});
	pMapCv.emplace("fmode_D", [&](const int val){ cv_fmode_D = val;});
	pMapPar.emplace("fcut_D", [&](const int val){ fcut_D = val;});
	pMapCv.emplace("fcut_D", [&](const int val){ cv_fcut_D = val;});
	pMapPar.emplace("freso_D", [&](const int val){ freso_D = val;});
	pMapCv.emplace("freso_D", [&](const int val){ cv_freso_D = val;});
	pMapPar.emplace("FilterLFOon_D", [&](const int val){ FilterLFOon_D = val;});
	pMapTrig.emplace("FilterLFOon_D", [&](const int val){ trig_FilterLFOon_D = val;});
	pMapPar.emplace("lfo2filtfm_D", [&](const int val){ lfo2filtfm_D = val;});
	pMapCv.emplace("lfo2filtfm_D", [&](const int val){ cv_lfo2filtfm_D = val;});
	pMapPar.emplace("lfospeed_D", [&](const int val){ lfospeed_D = val;});
	pMapCv.emplace("lfospeed_D", [&](const int val){ cv_lfospeed_D = val;});
	pMapPar.emplace("FreqModActive", [&](const int val){ FreqModActive = val;});
	pMapTrig.emplace("FreqModActive", [&](const int val){ trig_FreqModActive = val;});
	pMapPar.emplace("FreqModExclSubOSC", [&](const int val){ FreqModExclSubOSC = val;});
	pMapTrig.emplace("FreqModExclSubOSC", [&](const int val){ trig_FreqModExclSubOSC = val;});
	pMapPar.emplace("FreqModExclWT", [&](const int val){ FreqModExclWT = val;});
	pMapTrig.emplace("FreqModExclWT", [&](const int val){ trig_FreqModExclWT = val;});
	pMapPar.emplace("FreqModExclSample", [&](const int val){ FreqModExclSample = val;});
	pMapTrig.emplace("FreqModExclSample", [&](const int val){ trig_FreqModExclSample = val;});
	pMapPar.emplace("FreqModType", [&](const int val){ FreqModType = val;});
	pMapCv.emplace("FreqModType", [&](const int val){ cv_FreqModType = val;});
	pMapPar.emplace("FreqModAmnt", [&](const int val){ FreqModAmnt = val;});
	pMapCv.emplace("FreqModAmnt", [&](const int val){ cv_FreqModAmnt = val;});
	pMapPar.emplace("FreqModSpeed", [&](const int val){ FreqModSpeed = val;});
	pMapCv.emplace("FreqModSpeed", [&](const int val){ cv_FreqModSpeed = val;});
	pMapPar.emplace("FreqModAnalog", [&](const int val){ FreqModAnalog = val;});
	pMapCv.emplace("FreqModAnalog", [&](const int val){ cv_FreqModAnalog = val;});
	pMapPar.emplace("EGvolActive", [&](const int val){ EGvolActive = val;});
	pMapTrig.emplace("EGvolActive", [&](const int val){ trig_EGvolActive = val;});
	pMapPar.emplace("EGvolSlow", [&](const int val){ EGvolSlow = val;});
	pMapTrig.emplace("EGvolSlow", [&](const int val){ trig_EGvolSlow = val;});
	pMapPar.emplace("AttackVol", [&](const int val){ AttackVol = val;});
	pMapCv.emplace("AttackVol", [&](const int val){ cv_AttackVol = val;});
	pMapPar.emplace("DecayVol", [&](const int val){ DecayVol = val;});
	pMapCv.emplace("DecayVol", [&](const int val){ cv_DecayVol = val;});
	pMapPar.emplace("EGvolADSRon", [&](const int val){ EGvolADSRon = val;});
	pMapTrig.emplace("EGvolADSRon", [&](const int val){ trig_EGvolADSRon = val;});
	pMapPar.emplace("SustainVol", [&](const int val){ SustainVol = val;});
	pMapCv.emplace("SustainVol", [&](const int val){ cv_SustainVol = val;});
	pMapPar.emplace("ReleaseVol", [&](const int val){ ReleaseVol = val;});
	pMapCv.emplace("ReleaseVol", [&](const int val){ cv_ReleaseVol = val;});
	pMapPar.emplace("PannerOn", [&](const int val){ PannerOn = val;});
	pMapTrig.emplace("PannerOn", [&](const int val){ trig_PannerOn = val;});
	pMapPar.emplace("PanAmnt", [&](const int val){ PanAmnt = val;});
	pMapCv.emplace("PanAmnt", [&](const int val){ cv_PanAmnt = val;});
	pMapPar.emplace("PanFreq", [&](const int val){ PanFreq = val;});
	pMapCv.emplace("PanFreq", [&](const int val){ cv_PanFreq = val;});
	isStereo = true;
	id = "VctrSnt";
	// sectionCpp0
}

