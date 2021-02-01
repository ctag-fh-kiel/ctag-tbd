/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020/2021 by Robert Manzke. All rights reserved.

(c) 2021 for the "VctrSnt"-Plugin by Mathias Brüssel
VctrSnt stands for "Vector Synthesizer" and is inspired by the legendary Sequential Circuits Prophet VS http://www.vintagesynth.com/sci/pvs.php
This TBD synth is (optionally) duophonic and uses a stereo panner as effect. The soundgeneration itself is monophonic, though.
When only one audio-output is used and panning is on it will be audible as a kind of tremolo-effect instead.
Very much like the Prophet VS VctrSnt can blend sound between four oscillators, that's what the name vector-synthesis mainly stands for.
In contrast to it's role model this is not done with a joystick but with seperate controllers/CV per axis.
As with the VS the Oscillators are named A,B,C,D. On the X-axis the wavetables can be crossfaded as A<->C,
on the Y-axis samples (as an enhancement in contrast of using wavetables only) can be crossfaded as D<->B.
Simlar to the PPG and other wavetable-based instruments the waves within the table can be changed manually or via an LFO and is called "scanning" here.
The volume envelope and the panner can be turned off completely, this is important in case you want to use a true analogue filter behind the digital
oscillators, which is one of the strong points of wavetable synths like the Prophet VS or PPG 2 or samplers like the EMU II.

 makes use of wavetables, but in contrast to the VS there are only 2 oscillators using those

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
#include <cstdlib>
#include "esp_heap_caps.h"
#include "helpers/ctagNumUtil.hpp"
#include "plaits/dsp/engine/engine.h"

#define GATE_HIGH_NEW       2
#define GATE_HIGH                1
#define GATE_LOW                 0
#define GATE_LOW_NEW        -1

#define IDX_OSC_B           1         // Sample-Oscillator on top of Y-Axix for "Vector-Stick"
#define IDX_OSC_D           0         // Sample-Oscillator on bottom of Y-Axix for "Vector-Stick"

// --- Additional Macros for automated parameter evaluations ---
#define MK_TRIG_PAR(outname, inname) int outname = process_param_trig(data, trig_##inname, inname, e_##inname);
#define MK_FLT_PAR_ABS_PAN(outname, inname, norm, scale)  float outname = ((inname/norm)+1.f)/2.f * scale; if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]) * scale;


// --- Modify sine-wave for Vector or Wavetable-Z-Axis modulation
#define SINE_TO_UPPER_HALF(sine_val)          sine_val = (sine_val >= 0) ? sine_val     : -sine_val;
#define SINE_TO_LOWER_HALF(sine_val)          sine_val = (sine_val >= 0) ? -sine_val    : sine_val;
#define SINE_TO_UPPER_HALF_INV(sine_val)      sine_val = (sine_val >= 0) ? 1.f-sine_val : 0.f;
#define SINE_TO_LOWER_HALF_INV(sine_val)      sine_val = (sine_val >= 0) ? 0.f          : -1.f+sine_val;
#define SINE_TO_SQARE_UPPER_HALF(sine_val)    sine_val = (sine_val >= 0) ? 1.f          : 0.f;
#define SINE_TO_SQARE_LOWER_HALF(sine_val)    sine_val = (sine_val >= 0) ? 0.f          : -1.f;
#define SINE_TO_SQARE(sine_val)               sine_val = (sine_val >= 0) ? 1.f          : -1.f;

using namespace CTAG::SP;

inline int ctagSoundProcessorVctrSnt::process_param_trig(const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id )
{
  if(trig_myparm != -1)       // Trigger given via CV/Gate or button?
  {
    if((!data.trig[trig_myparm]) != prev_trig_state[prev_trig_state_id])    // Statuschange from HIGH to LOW or LOW to HIGH?
    {
      prev_trig_state[prev_trig_state_id] = !data.trig[trig_myparm];       // Remember status
      if (data.trig[trig_myparm] == 0)                      // HIGH if 0
        return (GATE_HIGH_NEW);          // New trigger
      else
        return (GATE_LOW_NEW);         // Trigger released
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

void ctagSoundProcessorVctrSnt::prepareWavetable( const int16_t **wavetables, int currentBank, bool *isWaveTableGood, float **fbuffer, int16_t **ibuffer )
{
  // precalculates wavetable data according to https://www.dafx12.york.ac.uk/papers/dafx12_submission_69.pdf
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
  // start conversion of data, 64 wavetables per bank
  int c = 0;
  for(int i=0;i<64;i++){ // iterate all waves
    int startOffset = bufferOffset + i*256; // which wave
    // prepare long array, i.e. x = numpy.array(list(wave) * 2 + wave[0] + wave[1] + wave[2] + wave[3])
    float sum4 = (*ibuffer)[startOffset] + (*ibuffer)[startOffset+1] + (*ibuffer)[startOffset+2] + (*ibuffer)[startOffset+3]; // add dc
    for(int j=0;j<512;j++)
    {
      (*fbuffer)[j] = (*ibuffer)[startOffset + (j%256)] + sum4;
    }
    // x -= x.mean()
    removeMeanOfFloatArray(*fbuffer, 512);
    // x /= numpy.abs(x).max()
    scaleFloatArrayToAbsMax(*fbuffer, 512);
    // x = numpy.cumsum(x)
    accumulateFloatArray(*fbuffer, 512);
    // x -= x.mean()
    removeMeanOfFloatArray(*fbuffer, 512);
    // create pointer map
    wavetables[i] = &((*ibuffer)[c]);
    // x = numpy.round(x * (4 * 32768.0 / WAVETABLE_SIZE)
    for(int j=512-256-4;j<512;j++)
    {
      int16_t v = static_cast<int16_t >(roundf((*fbuffer)[j] * 4.f * 32768.f / 256.f));
      (*ibuffer)[c++] = v;
    }
  }
  *isWaveTableGood = true;
}

void ctagSoundProcessorVctrSnt::Process(const ProcessData &data)
{
  // --- DSP calculation results ---
  float f_val_result = 0.f;

  // --- Read Buttons from GUI or Trigger/Gate and Sliders from GUI or CV and scale the data if required, "order in way of apperance" on GUI ---
  // Trigger variables will be named like the given parameter with an t_-Prefix but stay integers, CV-Variables are floats and thus f_*

  // --- Voice / Volume ---
  MK_TRIG_PAR(t_Gate, Gate);
  MK_FLT_PAR_ABS(f_Freq, Freq, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_Volume, Volume, 4095.f, 3.f);

  // --- VectorSpace ---
  MK_FLT_PAR_ABS( f_VolWT_A, VolWT_A, 4095.f, 1.f);
  MK_FLT_PAR_ABS( f_VolOsc_B, VolOsc_B, 4095.f, 2.f);     // Samples may be recorded at lower volume, so we leave a headroom option by scaling by 2
  MK_FLT_PAR_ABS( f_VolWT_C, VolWT_C, 4095.f, 1.f);
  MK_FLT_PAR_ABS( f_VolOsc_D, VolOsc_D, 4095.f, 2.f);     // Samples may be recorded at lower volume, so we leave a headroom option by scaling by 2

  MK_FLT_PAR_ABS_PAN(f_XfadeWaveTbls, XfadeWaveTbls, 2048.f, 1.f);     // We have a middle-centered scale, but we need 0-1.0 for mixing
  MK_FLT_PAR_ABS_PAN(f_XfadeSamples, XfadeSamples, 2048.f, 1.f);

  // --- Vector Modulators (Looping EGs start at given offset) ---
  MK_TRIG_PAR(t_LfoWaveTblsXfadeActive, LfoWaveTblsXfadeActive);
  MK_TRIG_PAR(t_LfoWaveTblsXfadeIsSquare, LfoWaveTblsXfadeIsSquare);
  MK_FLT_PAR_ABS_SFT(f_LfoWaveTblsXfadeRange, LfoWaveTblsXfadeRange, 4095.f, 1.f);
  MK_FLT_PAR_ABS_SFT(f_LfoWaveTblsXfadeSpeed, LfoWaveTblsXfadeSpeed, 4095.f, 15.f);
  lfoXfadeWTs.SetFrequency(f_LfoWaveTblsXfadeSpeed);

  MK_TRIG_PAR(t_ModWaveTblsXfade, ModWaveTblsXfade);
  MK_FLT_PAR_ABS_SFT(f_AttackWaveTblsXfd, AttackWaveTblsXfd, 4095.f, 2.f);   // We use exponential scaling for envelopes, so the max effectively will be squared
  MK_FLT_PAR_ABS_SFT(f_DecayWaveTblsXfd, DecayWaveTblsXfd, 4095.f, 10.f);
  xFadeSamples_eg.SetAttack(f_AttackWaveTblsXfd);
  xFadeSamples_eg.SetDecay(f_AttackWaveTblsXfd);

  MK_TRIG_PAR(t_ModSamplesXfade, ModSamplesXfade);
  MK_FLT_PAR_ABS_SFT(f_AttackSamplesXfd, AttackSamplesXfd, 4095.f, 2.f);
  MK_FLT_PAR_ABS_SFT(f_DecaySamplesXfd, DecaySamplesXfd, 4095.f, 10.f);
  xFadeWTs_eg.SetAttack(f_AttackSamplesXfd);
  xFadeWTs_eg.SetDecay(f_DecaySamplesXfd);

  // --- Wave Scanners (Looping EGs start at given offset) ---
  MK_FLT_PAR_ABS(f_ScanWavTblA, ScanWavTblA, 4095.f, 1.f);
  MK_TRIG_PAR(t_ScanWavTblAauto, ScanWavTblAauto);
  MK_FLT_PAR_ABS_SFT(f_AttackScanA, AttackScanA, 4095.f, 2.f);
  MK_FLT_PAR_ABS_SFT(f_DecayScanA, DecayScanA, 4095.f, 10.f);
  MK_FLT_PAR_ABS(f_ScanWavTblC, ScanWavTblC,4095.f, 1.f);
  MK_TRIG_PAR(t_ScanWavTblCauto, ScanWavTblCauto);
  MK_FLT_PAR_ABS_SFT(f_AttackScanC, AttackScanC, 4095.f, 2.f);
  MK_FLT_PAR_ABS_SFT(f_DecayScanC, DecayScanC, 4095.f, 10.f);

  // --- Wave select A ---
  currentBank_A = WaveTblA;
  if(lastBank_A != currentBank_A)
  { // this is slow, hence not modulated by CV
    prepareWavetable( (const int16_t **)&wavetables_A, currentBank_A, &isWaveTableGood_A, &fbuffer_A, &buffer_A );
    lastBank_A = currentBank_A;
  }
  // --- Wave select C ---
  currentBank_C = WaveTblC;
  if(lastBank_C != currentBank_C)
  { // this is slow, hence not modulated by CV
    prepareWavetable( (const int16_t **)&wavetables_C, currentBank_C, &isWaveTableGood_C, &fbuffer_C, &buffer_C );
    lastBank_C = currentBank_C;
  }

  // --- Pitch / Tune Wavetable Oscillator A ---
  float f_pitch_A = pitch_A;
  if(cv_pitch_A != -1)
    f_pitch_A += data.cv[cv_pitch_A] * 12.f * 5.f;
  MK_FLT_PAR_ABS_SFT(f_tune_A, tune_A, 2048.f, 1.f);
  const float f_freq_A = plaits::NoteToFrequency(60 + f_pitch_A + f_tune_A * 12.f) * 0.998f;

  // --- Pitch / Tune Wavetable Oscillator C ---
  float f_pitch_C = pitch_C;
  if(cv_pitch_C != -1)
    f_pitch_C += data.cv[cv_pitch_C] * 12.f * 5.f;
  MK_FLT_PAR_ABS_SFT(f_tune_C, tune_C, 2048.f, 1.f);
  const float f_freq_C = plaits::NoteToFrequency(60 + f_pitch_C + f_tune_C * 12.f) * 0.998f;

  // --- Render and buffer wave-Table oscillators ---
  float out_A[32] = {0.f};  // Initialize (all elements to 0) each time or results may be weird!
  if(isWaveTableGood_A)     // Wavetable A ready?
    wt_osc_A.Render(f_freq_A, f_VolWT_A, f_ScanWavTblA, wavetables_A, out_A, bufSz);

  float out_C[32] = {0.f};  // Initialize (all elements to 0) each time or results may be weird!
  if(isWaveTableGood_C)     // Wavetable C ready?
    wt_osc_C.Render(f_freq_C, f_VolWT_C, f_ScanWavTblC, wavetables_C, out_C, bufSz);

  // === Sample oscillators (aka Rompler Voices) ===
  // --- Sample Oscillator B ---
  romplers[IDX_OSC_B]->params.gate = 1;   // ### To be optimized with real trigger!
  romplers[IDX_OSC_B]->params.sliceLock = false; // ### latch for trigger instead of Gate?
  bool new_trigger_B = false;          // ### To be optimized with real trigger!
  if( new_trigger_B )
    romplers[IDX_OSC_B]->Reset();

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
  if(cv_pitch_B != -1)
    f_Pitch_B += data.cv[cv_pitch_B] * 12.f * 5.f;
  romplers[IDX_OSC_B]->params.pitch = f_Pitch_B;
  MK_FLT_PAR_ABS_SFT(f_Tune_B, tune_B, 2048.f, 12.f)
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
  romplers[IDX_OSC_B]->params.gain = f_VolOsc_B;               // ### change to true trigger here!
  // --- Render and buffer Sample oscillator B ---
  romplers[IDX_OSC_B]->Process(sample_buf_B, bufSz);

  // --- Sample Oscillator D ---
  romplers[IDX_OSC_D]->params.gate = 1;   // ### To be optimized with real trigger!
  romplers[IDX_OSC_D]->params.sliceLock = false; // ### latch for trigger instead of Gate?
  bool new_trigger_D = false;          // ### To be optimized with real trigger!
  if( new_trigger_D )
    romplers[IDX_OSC_D]->Reset();

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
  if(cv_pitch_D != -1)
    f_Pitch_D += data.cv[cv_pitch_D] * 12.f * 5.f;
  romplers[IDX_OSC_D]->params.pitch = f_Pitch_D;
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
  romplers[IDX_OSC_D]->params.gain = f_VolOsc_D;               // ### change to true trigger here!
  // --- Render and buffer Sample oscillator D ---
  romplers[IDX_OSC_D]->Process(sample_buf_D, bufSz);

  // --- Frequency Modulation ---
  MK_TRIG_PAR(t_FreqModActive, FreqModActive);
  MK_TRIG_PAR(t_FreqModSinus, FreqModSinus);
  MK_FLT_PAR_ABS(f_FreqModAmnt, FreqModAmnt, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_FreqModSpeed, FreqModSpeed, 4095.f, 100.f);   // LFO will have frequencies from 0.05 to 100 Hz

  // --- Volume Envelope ---
  MK_TRIG_PAR(t_EGvolActive, EGvolActive);
  MK_TRIG_PAR(t_EGvolGate, EGvolGate);
  MK_FLT_PAR_ABS_SFT(f_AttackVol, AttackVol, 4095.f, 2.f);
  MK_FLT_PAR_ABS_SFT(f_DecayVol, DecayVol, 4095.f, 1.f);
  MK_FLT_PAR_ABS_SFT(f_SustainVol,SustainVol,4095.f, 5.f);
  MK_FLT_PAR_ABS_SFT(f_ReleaseVol, ReleaseVol, 4095.f, 10.f);

  // --- Panner/Tremolo ---
  MK_FLT_PAR_ABS(f_PanAmnt, PanAmnt, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_PanFreq, PanFreq, 4095.f, 1.f);
  MK_FLT_PAR_ABS(f_PanOffset, PanOffset, 4095.f, 1.f);

  // --- This is the loop where the audio-output is written ---
  for (uint32_t i = 0; i < bufSz; i++)
  {
    // --- Vector XFades LFOs ---
    if(t_LfoWaveTblsXfadeActive)
    {
      float lfo_value = lfoXfadeWTs.Process();
      if (t_LfoWaveTblsXfadeIsSquare)
        SINE_TO_UPPER_HALF(lfo_value);     // SINE_TO_SQUARE, SINE_TO_UPPER_HALF and SINE_TO_LOWER_HALF as alternatives! ###
      if(t_ModWaveTblsXfade)
        f_XfadeWaveTbls += f_LfoWaveTblsXfadeRange * (lfo_value+xFadeWTs_eg.Process());
      else
        f_XfadeWaveTbls += f_LfoWaveTblsXfadeRange * lfo_value;
      CONSTRAIN(f_XfadeWaveTbls,0.f, 1.f);
    }
    else      // Maybe no LFO but only EG to modulate the X-Fade?
    {
      if(t_ModWaveTblsXfade)
        f_XfadeWaveTbls += f_LfoWaveTblsXfadeRange/4.f * xFadeWTs_eg.Process();   // We reduce the range, because EG is only going "upwards"
    }

    if(t_ModSamplesXfade)
    {
      f_XfadeSamples += xFadeSamples_eg.Process();
      CONSTRAIN(f_XfadeSamples,0.f, 1.f);
    }
    // --- Oscillator-Mix and Mastervolume (truncate audio in case of clipping) ---
    f_val_result = out_A[i] * (1.f-f_XfadeWaveTbls) * 0.25f;    // Get precalculated data from wavetable-voice A for output
    f_val_result += out_C[i] * (f_XfadeWaveTbls) * 0.25f;       // Mix with precalculated data from wavetable-voice B for output
    f_val_result += sample_buf_D[i] * (1.f-f_XfadeSamples) * 0.25f;      // Crossfade samples and mix with wavetables
    f_val_result += sample_buf_B[i] * (f_XfadeSamples)  * 0.25f;
    f_val_result *= f_Volume;                                             // Adjust Master-Volume
    CONSTRAIN(f_val_result, -1.f, 1.f);                          // Limit result to max. audio-level

    // --- Output of DSP-results ---
    data.buf[i * 2 + processCh] = f_val_result;    // ### This is left channel only for now, correct when implementing Panner-effect!
  }
}

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

  // Alloc mem for one wavetable (Oscillator C)
  buffer_C = (int16_t*)heap_caps_malloc(260*64*2, MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT); // 260 = wavetable size after prep, 64 wavetables, 2 bytes per sample (int16)
  assert(buffer_C != NULL);
  memset(buffer_C, 0, 260*64*2);
  fbuffer_C = (float*)heap_caps_malloc(512*4, MALLOC_CAP_8BIT|MALLOC_CAP_INTERNAL); // buffer for wavetable prep computations
  assert(fbuffer_C != NULL);
  memset(fbuffer_C, 0, 512*4);
  wt_osc_C.Init();

  // Get offset of samples that are no wavetables
  wtSliceOffset = sampleRom.GetFirstNonWaveTableSlice();

  // Create sample-oscillators / Rompler-voices and initialize Rompler voices regarding parameters not actively used here ---
  for( int i=0; i < 2; i++)
  {
    romplers[i] = std::make_unique<RomplerVoice>();

    romplers[i]->params.s = 1.f;          // Set Sustain to max, "just in case"
    romplers[i]->params.egAM = 0.5f;
    romplers[i]->params.egFM = 0.5f;
    romplers[i]->params.egFMFilter = 0.5f;
    romplers[i]->params.cutoff = 0.f;
    romplers[i]->params.resonance = 0.f;
    romplers[i]->params.filterType = static_cast<RomplerVoice::FilterType>(0.f);  // ### This may change if we use it as Low-Cut for instance...
  }

  // --- Initialize Volume Envelope ---
  vol_eg.SetSampleRate(44100.f);    // Sync Env with our audio-processing
  vol_eg.SetModeExp();                   // Logarithmic scaling
  vol_eg.Reset();

  // --- Vector-EGs ---
  xFadeSamples_eg.SetSampleRate(44100.f);
  xFadeSamples_eg.SetModeLin();
  xFadeSamples_eg.SetLoop(true);
  xFadeSamples_eg.Trigger();

  xFadeWTs_eg.SetSampleRate(44100.f);
  xFadeWTs_eg.SetModeLin();
  xFadeWTs_eg.SetLoop(true);
  xFadeWTs_eg.Trigger();

  // --- Initialize LFOs ---
  lfoXfadeWTs.SetSampleRate(44100.f);
  lfoXfadeWTs.SetFrequency(1.f);
  lfoXfadeSamples.SetSampleRate(44100.f);
  lfoXfadeSamples.SetFrequency(1.f);

  Gate = false;   // Init Gate to off for instanziation of object, so that we wait for a trigger
}

ctagSoundProcessorVctrSnt::~ctagSoundProcessorVctrSnt()
{
  // Free mem for one wavetable (Oscillator A)
  heap_caps_free(buffer_A);
  heap_caps_free(fbuffer_A);
  // Free mem for one wavetable (Oscillator C)
  heap_caps_free(buffer_C);
  heap_caps_free(fbuffer_C);
}

void ctagSoundProcessorVctrSnt::knowYourself()
{
  // autogenerated code here
  // sectionCpp0
	pMapPar.emplace("Gate", [&](const int val){ Gate = val;});
	pMapTrig.emplace("Gate", [&](const int val){ trig_Gate = val;});
	pMapPar.emplace("Freq", [&](const int val){ Freq = val;});
	pMapCv.emplace("Freq", [&](const int val){ cv_Freq = val;});
	pMapPar.emplace("Volume", [&](const int val){ Volume = val;});
	pMapCv.emplace("Volume", [&](const int val){ cv_Volume = val;});
	pMapPar.emplace("VolWT_A", [&](const int val){ VolWT_A = val;});
	pMapCv.emplace("VolWT_A", [&](const int val){ cv_VolWT_A = val;});
	pMapPar.emplace("VolOsc_B", [&](const int val){ VolOsc_B = val;});
	pMapCv.emplace("VolOsc_B", [&](const int val){ cv_VolOsc_B = val;});
	pMapPar.emplace("VolWT_C", [&](const int val){ VolWT_C = val;});
	pMapCv.emplace("VolWT_C", [&](const int val){ cv_VolWT_C = val;});
	pMapPar.emplace("VolOsc_D", [&](const int val){ VolOsc_D = val;});
	pMapCv.emplace("VolOsc_D", [&](const int val){ cv_VolOsc_D = val;});
	pMapPar.emplace("XfadeWaveTbls", [&](const int val){ XfadeWaveTbls = val;});
	pMapCv.emplace("XfadeWaveTbls", [&](const int val){ cv_XfadeWaveTbls = val;});
	pMapPar.emplace("XfadeSamples", [&](const int val){ XfadeSamples = val;});
	pMapCv.emplace("XfadeSamples", [&](const int val){ cv_XfadeSamples = val;});
	pMapPar.emplace("LfoWaveTblsXfadeActive", [&](const int val){ LfoWaveTblsXfadeActive = val;});
	pMapTrig.emplace("LfoWaveTblsXfadeActive", [&](const int val){ trig_LfoWaveTblsXfadeActive = val;});
	pMapPar.emplace("LfoWaveTblsXfadeIsSquare", [&](const int val){ LfoWaveTblsXfadeIsSquare = val;});
	pMapTrig.emplace("LfoWaveTblsXfadeIsSquare", [&](const int val){ trig_LfoWaveTblsXfadeIsSquare = val;});
	pMapPar.emplace("LfoWaveTblsXfadeRange", [&](const int val){ LfoWaveTblsXfadeRange = val;});
	pMapCv.emplace("LfoWaveTblsXfadeRange", [&](const int val){ cv_LfoWaveTblsXfadeRange = val;});
	pMapPar.emplace("LfoWaveTblsXfadeSpeed", [&](const int val){ LfoWaveTblsXfadeSpeed = val;});
	pMapCv.emplace("LfoWaveTblsXfadeSpeed", [&](const int val){ cv_LfoWaveTblsXfadeSpeed = val;});
	pMapPar.emplace("ModWaveTblsXfade", [&](const int val){ ModWaveTblsXfade = val;});
	pMapTrig.emplace("ModWaveTblsXfade", [&](const int val){ trig_ModWaveTblsXfade = val;});
	pMapPar.emplace("AttackWaveTblsXfd", [&](const int val){ AttackWaveTblsXfd = val;});
	pMapCv.emplace("AttackWaveTblsXfd", [&](const int val){ cv_AttackWaveTblsXfd = val;});
	pMapPar.emplace("DecayWaveTblsXfd", [&](const int val){ DecayWaveTblsXfd = val;});
	pMapCv.emplace("DecayWaveTblsXfd", [&](const int val){ cv_DecayWaveTblsXfd = val;});
	pMapPar.emplace("ModSamplesXfade", [&](const int val){ ModSamplesXfade = val;});
	pMapTrig.emplace("ModSamplesXfade", [&](const int val){ trig_ModSamplesXfade = val;});
	pMapPar.emplace("AttackSamplesXfd", [&](const int val){ AttackSamplesXfd = val;});
	pMapCv.emplace("AttackSamplesXfd", [&](const int val){ cv_AttackSamplesXfd = val;});
	pMapPar.emplace("DecaySamplesXfd", [&](const int val){ DecaySamplesXfd = val;});
	pMapCv.emplace("DecaySamplesXfd", [&](const int val){ cv_DecaySamplesXfd = val;});
	pMapPar.emplace("ScanWavTblAauto", [&](const int val){ ScanWavTblAauto = val;});
	pMapTrig.emplace("ScanWavTblAauto", [&](const int val){ trig_ScanWavTblAauto = val;});
	pMapPar.emplace("AttackScanA", [&](const int val){ AttackScanA = val;});
	pMapCv.emplace("AttackScanA", [&](const int val){ cv_AttackScanA = val;});
	pMapPar.emplace("DecayScanA", [&](const int val){ DecayScanA = val;});
	pMapCv.emplace("DecayScanA", [&](const int val){ cv_DecayScanA = val;});
	pMapPar.emplace("ScanWavTblCauto", [&](const int val){ ScanWavTblCauto = val;});
	pMapTrig.emplace("ScanWavTblCauto", [&](const int val){ trig_ScanWavTblCauto = val;});
	pMapPar.emplace("AttackScanC", [&](const int val){ AttackScanC = val;});
	pMapCv.emplace("AttackScanC", [&](const int val){ cv_AttackScanC = val;});
	pMapPar.emplace("DecayScanC", [&](const int val){ DecayScanC = val;});
	pMapCv.emplace("DecayScanC", [&](const int val){ cv_DecayScanC = val;});
	pMapPar.emplace("WaveTblA", [&](const int val){ WaveTblA = val;});
	pMapCv.emplace("WaveTblA", [&](const int val){ cv_WaveTblA = val;});
	pMapPar.emplace("ScanWavTblA", [&](const int val){ ScanWavTblA = val;});
	pMapCv.emplace("ScanWavTblA", [&](const int val){ cv_ScanWavTblA = val;});
	pMapPar.emplace("pitch_A", [&](const int val){ pitch_A = val;});
	pMapCv.emplace("pitch_A", [&](const int val){ cv_pitch_A = val;});
	pMapPar.emplace("tune_A", [&](const int val){ tune_A = val;});
	pMapCv.emplace("tune_A", [&](const int val){ cv_tune_A = val;});
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
	pMapPar.emplace("WaveTblC", [&](const int val){ WaveTblC = val;});
	pMapCv.emplace("WaveTblC", [&](const int val){ cv_WaveTblC = val;});
	pMapPar.emplace("ScanWavTblC", [&](const int val){ ScanWavTblC = val;});
	pMapCv.emplace("ScanWavTblC", [&](const int val){ cv_ScanWavTblC = val;});
	pMapPar.emplace("pitch_C", [&](const int val){ pitch_C = val;});
	pMapCv.emplace("pitch_C", [&](const int val){ cv_pitch_C = val;});
	pMapPar.emplace("tune_C", [&](const int val){ tune_C = val;});
	pMapCv.emplace("tune_C", [&](const int val){ cv_tune_C = val;});
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
	pMapPar.emplace("FreqModActive", [&](const int val){ FreqModActive = val;});
	pMapTrig.emplace("FreqModActive", [&](const int val){ trig_FreqModActive = val;});
	pMapPar.emplace("FreqModSinus", [&](const int val){ FreqModSinus = val;});
	pMapTrig.emplace("FreqModSinus", [&](const int val){ trig_FreqModSinus = val;});
	pMapPar.emplace("FreqModAmnt", [&](const int val){ FreqModAmnt = val;});
	pMapCv.emplace("FreqModAmnt", [&](const int val){ cv_FreqModAmnt = val;});
	pMapPar.emplace("FreqModSpeed", [&](const int val){ FreqModSpeed = val;});
	pMapCv.emplace("FreqModSpeed", [&](const int val){ cv_FreqModSpeed = val;});
	pMapPar.emplace("EGvolActive", [&](const int val){ EGvolActive = val;});
	pMapTrig.emplace("EGvolActive", [&](const int val){ trig_EGvolActive = val;});
	pMapPar.emplace("EGvolGate", [&](const int val){ EGvolGate = val;});
	pMapTrig.emplace("EGvolGate", [&](const int val){ trig_EGvolGate = val;});
	pMapPar.emplace("AttackVol", [&](const int val){ AttackVol = val;});
	pMapCv.emplace("AttackVol", [&](const int val){ cv_AttackVol = val;});
	pMapPar.emplace("DecayVol", [&](const int val){ DecayVol = val;});
	pMapCv.emplace("DecayVol", [&](const int val){ cv_DecayVol = val;});
	pMapPar.emplace("SustainVol", [&](const int val){ SustainVol = val;});
	pMapCv.emplace("SustainVol", [&](const int val){ cv_SustainVol = val;});
	pMapPar.emplace("ReleaseVol", [&](const int val){ ReleaseVol = val;});
	pMapCv.emplace("ReleaseVol", [&](const int val){ cv_ReleaseVol = val;});
	pMapPar.emplace("PanAmnt", [&](const int val){ PanAmnt = val;});
	pMapCv.emplace("PanAmnt", [&](const int val){ cv_PanAmnt = val;});
	pMapPar.emplace("PanFreq", [&](const int val){ PanFreq = val;});
	pMapCv.emplace("PanFreq", [&](const int val){ cv_PanFreq = val;});
	pMapPar.emplace("PanOffset", [&](const int val){ PanOffset = val;});
	pMapCv.emplace("PanOffset", [&](const int val){ cv_PanOffset = val;});
	isStereo = true;
	id = "VctrSnt";
	// sectionCpp0
}

