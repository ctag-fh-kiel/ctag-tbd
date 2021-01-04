/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020 by Robert Manzke. All rights reserved.

(c) 2020 for the "Karpuskl"-Plugin by Mathias Brüssel
String Synthesizer based on the Karplus Strong algorithm by Kevin Karplus und Alex Strong as described here: https://blog.demofox.org/2016/06/16/synthesizing-a-pluked-string-sound-with-the-karplus-strong-algorithm/
Chord option based on the Stradella bass system as used with accordions: https://en.wikipedia.org/wiki/Stradella_bass_system
"Fuzz+Sustain" uses a modified formula from Ragnar Bendiksen's "Digitale lydeffekter" https://web.archive.org/web/20070826204128/http://www.notam02.no/~rbendiks/Diplom.html#Overstyring
Ring Modulator inspired by Bob Moog's original design as used with the MF-102: https://www.moogmusic.com/products/moogerfooger-mf-102-ring-modulator

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "ctagSoundProcessorKarpuskl.hpp"
#include <cstdlib>


#define GATE_HIGH_NEW       2
#define GATE_HIGH           1
#define GATE_LOW            0
#define GATE_LOW_NEW        -1

using namespace CTAG::SP;

inline int ctagSoundProcessorKarpuskl::process_param_trig(const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id )
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

// --- Helper function: rescale CV or Pot to float 0...1.0 (CV is already in correct format, we still keep it inside this method for convenience ---
inline float ctagSoundProcessorKarpuskl::process_param_float(const ProcessData &data, int cv_myparm, int my_parm, float out_min, float out_max )
{
  if(cv_myparm != -1)
  {
    if (data.cv[cv_myparm] >= 0.0f)     // This is a bypass solution to avoid negative values in rare cases
      return data.cv[cv_myparm]*(out_max - out_min) + out_min;     // Rescale float of 0.0..1.0 to min...max output-range       //
    else
      return out_min;                   // Unexpected, return minimum valid value
  }
  else
    return (my_parm/4095.f)*(out_max - out_min) + out_min;     // convert to float of 0.0..1.0 and scale to min..max output-range
}

// --- Helper function: rescale CV or Pot to float -0.5...+0.5 (CV is already in correct format, we still keep it inside this method for convenience ---
inline float ctagSoundProcessorKarpuskl::process_center_param_float(const ProcessData &data, int cv_myparm, int my_parm )
{
  if(cv_myparm != -1)
  {
    if (data.cv[cv_myparm] >= 0.0f)     // This is a bypass solution to avoid negative values in rare cases
      return data.cv[cv_myparm] - 0.5f;     // Rescale float of 0.0..1.0 to min...max output-range       //
    else
      return -0.5f;                       // Unexpected, return minimum valid value
  }
  else
    return ((my_parm+2048)/4095.f) - 0.5f;     // convert to float of 0.0..1.0 and scale to min..max output-range
}

void ctagSoundProcessorKarpuskl::Process(const ProcessData &data)
{
// --- Trigger/Gate input String Synth, RingMod and Pitchoptions ---
int chord_notes = 0;
int stradella_chords = 0;
int trig_karpuskl = 0;
int drive_post = 0;
int modfreq_hi = 0;
int lfo_square = 0;
int quantize_on = 0;
int stradella_base = 48;
int fourth_note = 0;

// --- CV input StringSynth ---
float damping = 0.f;
float bright = 0.f;

// --- CV input RingModulator ---
float modMix = 0.f;
float modFreq = 0.f;
float driveAmount = 0.f;
float driveCharacter = 0.f;   // Frequency for tanh() waveshaper
float lfo_rate = 0.f;
float lfo_amount = 0.f;

// --- Helper variables ---
float base_freq = 440.f;
float f_midi_note = 0.f;
float f_deviation = 0.f;
float cv_adjust = 0.f;
float vol_attack = 0.f;
float vol_decay = 0.f;
int stradella_octave = 0;
int i_midi_note = 0;
int vol_env_on = 0;
int vol_env_loop = 0;
float master_gain = 1.f;

// --- DSP calculation results ---
float f_valA = 0.f;
float f_valB = 0.f;
float f_valC = 0.f;
float f_valD = 0.f;
float f_ringMod = 1.f;
float lfo_val = 0.f;

  // --- Read and buffer triggers/options for Karplus Strong / Ringmodulator  ---
  trig_karpuskl = process_param_trig(data, trig_KarpusklTrigger, KarpusklTrigger, e_KarpusklTrigger);
  chord_notes = process_param_trig(data, trig_SingleNotes, SingleNotes, e_SingleNotes );
  fourth_note = process_param_trig(data, trig_FourNoteChords, FourNoteChords, e_FourNoteChords  );
  drive_post =  process_param_trig(data, trig_DrivePost, DrivePost, e_DrivePost);
  modfreq_hi = process_param_trig(data, trig_ModFreqLoHi, ModFreqLoHi, e_ModFreqLoHi);
  // Read trigger infos relevant for Karplus Strong / chords
  stradella_chords = process_param_trig(data, trig_PowerChords, PowerChords, e_PowerChords);
  quantize_on = process_param_trig(data, trig_QuantizeOn, QuantizeOn, e_QuantizeOn);
  if(process_param_trig(data, trig_OctUp, OctUp, e_OctUp))
    stradella_base += 12;
  if(process_param_trig(data, trig_OctDown, OctDown, e_OctDown))
    stradella_base -= 12;

  lfo_square = process_param_trig(data, trig_LFOsquare, LFOsquare, e_LFOsquare);
  vol_env_on =  process_param_trig(data, trig_EnvActive, EnvActive, e_EnvActive);
  vol_env_loop = process_param_trig(data, trig_EnvLoop, EnvLoop, e_EnvLoop);

  // --- Read and buffer controllers for Karplus Strong / Ringmodulator + calculate notes and frequencies required for sound-generation lateron ---
  damping = process_param_float(data, cv_Damping, Damping, 0.94f,1.f);    // very long to very short sustain of the string
  bright = process_param_float(data, cv_Brightness, Brightness, 1.f, 0.94f );    // changes filter intensity
  modMix = process_param_float(data, cv_ModMix, ModMix );
  if( modfreq_hi )    // As with the Moogerfooger MF-102 we have two different ranges of Carrier-Frequencies to choose from - this allows smoother transition over the whole range...
    modFreq = process_param_float(data, cv_ModFreq, ModFreq, 30.f, 4000.f );
  else
    modFreq = process_param_float(data, cv_ModFreq, ModFreq, 0.6f, 80.f );

  driveAmount = process_param_float(data, cv_DriveAmnt, DriveAmnt, 1.f, 12.f );
  driveCharacter = process_param_float(data, cv_DriveChar, DriveChar );
  lfo_amount = process_param_float(data, cv_LFOamnt, LFOamnt);
  lfo_rate = process_param_float(data, cv_LFOrate, LFOrate, 0.1f*bufSz, 25.f*bufSz); // As a performance optimisation we call this outside the main loop, so we have to speed up the LFO respectively!
  master_gain = process_param_float(data, cv_MasterGain, MasterGain, 1.0f, 8.0f );
  // Read pitch relevant infos for Karplus Strong
  f_midi_note = process_param_float(data, cv_KarpusklFrequ, KarpusklFrequ)*60.f;
  cv_adjust = process_center_param_float(data, cv_MasterPitch, MasterPitch );

  i_midi_note = int(f_midi_note+cv_adjust);      // Apply pitch-offset and round/truncate, we need the rounding especially to set stradella-chords
  if( quantize_on )     // To adjust CV-pitch with AE Modular for instance the best results were +0.255
    f_midi_note = (float)i_midi_note;               // Quantize CV via updating the floating point note value with it's integer counterpart!
  else
    f_deviation = f_midi_note-(float)i_midi_note;   // Remember the deviation caused by CV-imperfection of the "MIDI-Note" we calculated as a fraction after the decimal point - note: maybe negative, too!
  stradella_octave = i_midi_note / 12;

  // --- Get settings for volume envelope ---
  if (cv_VolAttack != -1)
  {
    if( data.cv[cv_VolAttack] >= 0.f )
      vol_attack = data.cv[cv_VolAttack] * data.cv[cv_VolAttack] * 2.f; // power of two prevents negative values and adjusts pots to more expotential behaviour
  }
  else
    vol_attack = (float) VolAttack / 4095.f * 5.f;    // get attack value from GUI, set time
  vol_env.SetAttack(vol_attack);

  if (cv_VolDecay != -1)
  {
    if( data.cv[cv_VolDecay] >= 0.f )
      vol_decay = data.cv[cv_VolDecay] * data.cv[cv_VolDecay] * 8.f;     // power of two prevents negative values and adjusts pots to more expotential behaviour
  }
  else
    vol_decay = (float) VolDecay / 4095.f * 20.f;   // get decay value from GUI, set time
  vol_env.SetDecay(vol_decay);

  vol_env.SetLoop( (bool)vol_env_loop );

  // --- If new note-trigger is given, [re]intalize the relevant data for the algorithm ---
  if( trig_karpuskl == GATE_HIGH_NEW )
  {
    if( vol_env_on )
      vol_env.Trigger();

    // --- Decide if we have single or chord-notes in case of chord-notes this may be "power-chords" or (optionally transposed) "stradella chords" ---
    if(!chord_notes)        // Single Notes! (The user decided if to use chromatic quantisation or not already before! It's part of the value of f_midi_note)
    {
      base_freq = noteToFreq(f_midi_note+36.f);  // This is the frequency of our note or base-freq of our chord...
      m_bufferA.resize(max(uint32_t(1), uint32_t(float(44100) / base_freq)));    // The size of the buffer results in a varying pitch for this algorithm
      // We reset  the chord-buffers always too, to prevent weird behaviour in case chords are set to on _after_ the note[s] got triggered - we may get powerchords instead of stradella ones then, so this is a "buggy" compromise ;-)
      m_bufferB.resize(max(uint32_t(1), uint32_t(float(44100) / (base_freq / 2.f * 3.f))));  // The size of the buffer results in a varying pitch for this algorithm
      m_bufferC.resize(max(uint32_t(1), uint32_t(float(44100) / (base_freq * 2.f)))); // The size of the buffer results in a varying pitch for this algorithm
      m_bufferD.resize(max(uint32_t(1), uint32_t(float(44100) / (base_freq / 2.f * 6.f)))); // The size of the buffer results in a varying pitch for this algorithm
    }
    else      // Chords to be played, concerning the concept of Stradella bass and chords you may look here: https://en.wikipedia.org/wiki/Stradella_bass_system
    {
      if (stradella_chords) // Stradella octaves: 1) C-E-G (C major), 2) G-Eb-C (C minor), 3) Bb-E-C (C dominant 7th) 4) Eb-C-A (C diminished 7th) 5) C-E-G# (C augmented triad)
      {
        f_midi_note = float((i_midi_note%12)+stradella_base);    // Select "middle octave" for stradella! This might be transposed an octave up or down
        if( !quantize_on )                // No chromatic quantisazion of notes wanted, but we had to round first to retrieve the notes for our Stradella-chords anyways!
          f_midi_note += f_deviation;     // Add deviation of CV from perfect tuning that we extracted before again!

        base_freq = noteToFreq(f_midi_note+stradella[stradella_octave][0]);  // Note 1 of Stradella chord
        m_bufferA.resize(max(uint32_t(1), uint32_t(float(44100) / base_freq)) );
        base_freq = noteToFreq(f_midi_note+stradella[stradella_octave][1]);  // Note 2 of Stradella chord
        m_bufferB.resize(max(uint32_t(1), uint32_t(float(44100) / base_freq)) );
        base_freq = noteToFreq(f_midi_note+stradella[stradella_octave][2]);  // Note 3 of Stradella chord
        m_bufferC.resize(max(uint32_t(1), uint32_t(float(44100) / base_freq)) );
        base_freq = noteToFreq(f_midi_note+stradella[stradella_octave][3]);  // Note 3 of Stradella chord
        m_bufferD.resize(max(uint32_t(1), uint32_t(float(44100) / base_freq)) );
      }
      else // Powerchords (The user decided if to use chromatic quantisation or not already before! It's part of the value of f_midi_note)
      {
        base_freq = noteToFreq(f_midi_note+36.f);  // This is the frequency of our note or base-freq of our chord...
        m_bufferA.resize(max(uint32_t(1), uint32_t(float(44100) / base_freq)));    // The size of the buffer results in a varying pitch for this algorithm
        m_bufferB.resize(max(uint32_t(1), uint32_t(float(44100) / (base_freq / 2.f * 3.f))));  // Pure fifth instead of well tempered ones, here!
        m_bufferC.resize(max(uint32_t(1), uint32_t(float(44100) / (base_freq * 2.f)))); // The size of the buffer results in a varying pitch for this algorithm
        m_bufferD.resize(max(uint32_t(1), uint32_t(float(44100) / (base_freq / 2.f * 6.f))));
      }
    }
    m_indexA = m_indexB = m_indexC = 0;                     // To actually trigger the note[s]: Reset Karplus-Strong buffer indices...
    m_impulse_idxA = m_impulse_idxB = m_impulse_idxC = 0;   // Also reset impulse-buffer (for KS algo) indices...
    if( fourth_note )
    {
      m_indexD = 0;
      m_impulse_idxD = 0;
    }
    else
    {
      m_impulse_idxD = m_bufferD.size();  // Do not process data for fourth note
      m_indexD = m_bufferD.size()-1;        // Do not process data for fourth note, keep one index free for filter calculation!
    }
  }

  // --- Modulate Ringmodulator with LFO (we do this outside the main loop, to gain perfomance, the LFO Frequ max is 30Hz, so it should not matter... ---
  if( lfo_amount != 0.f )   // If amount is zero, LFO is shut off...
  {
    ringModLFO.SetFrequency(lfo_rate);
    lfo_val = ringModLFO.Process();
    if( lfo_square )          // Transform Sinus to Square?
    {
      if( lfo_val >= 0.f )
        lfo_val = 1.0f;
      else
        lfo_val = -1.0f;
    }
    if( lfo_val != 0.f ) // otherwiese we leave modFreq for RingMod unchanged!
    {
      modFreq *= HELPERS::fasterpow2(lfo_val * lfo_amount);     // We use a 2-power to have a somewhat logarithmic scaling of our control-knob/slider...
      if (modfreq_hi)    // As with the Moogerfooger MF-102 we have two different ranges of Carrier-Frequencies to choose from - this allows smoother transition over the whole range...
      {
        if (modFreq > 4000.f) modFreq = 4000.f;
        if (modFreq < 30.f) modFreq = 30.f;
      }
      else
      {
        if (modFreq > 80.f) modFreq = 80.f;
        if (modFreq < 0.6f) modFreq = 0.6f;
      }
    }
  }
  ringModCarrier.SetFrequency(modFreq);
  // --- This is our main loop, where the generation of the Karplus Strong tones and ringmodulation takes place ---
  for (uint32_t i = 0; i < bufSz; i++)
  {
    // --- Calculate String-Synth - Init start-setting of Karplus-Strong buffer if needed (impulse-data) ---
    if (chord_notes > 0)                // GATE_HIGH or GATE_HIGH_NEW
    {
      if (m_impulse_idxA < m_bufferA.size())    // Once per note construct the buffer/delayline for the algorithm, fill it with initial "impulse"-data...
      {
        m_bufferA[m_impulse_idxA] = ((float) rand()) / ((float) RAND_MAX) * 2.0f - 1.0f;  // noise
        m_impulse_idxA++;
      }
      if (m_impulse_idxB < m_bufferB.size())    // Once per note construct the buffer/delayline for the algorithm, fill it with initial "impulse"-data...
      {
        m_bufferB[m_impulse_idxB] = ((float) rand()) / ((float) RAND_MAX) * 2.0f - 1.0f;  // noise
        m_impulse_idxB++;
      }
      if (m_impulse_idxC < m_bufferC.size())    // Once per note construct the buffer/delayline for the algorithm, fill it with initial "impulse"-data...
      {
        m_bufferC[m_impulse_idxC] = ((float) rand()) / ((float) RAND_MAX) * 2.0f - 1.0f;  // noise
        m_impulse_idxC++;
      }
      if( fourth_note && m_impulse_idxD < m_bufferD.size() )    // Once per note construct the buffer/delayline for the algorithm, fill it with initial "impulse"-data...
      {
        m_bufferD[m_impulse_idxD] = ((float) rand()) / ((float) RAND_MAX) * 2.0f - 1.0f;  // noise
        m_impulse_idxD++;
      }
    }
    else    // Powerchords not activated, fill Karplus Strong buffer with initial impulse if needed
    {
      if (m_impulse_idxA < m_bufferA.size())    // Once per note construct the buffer/delayline for the algorithm, fill it with initial "impulse"-data...
      {
        m_bufferA[m_impulse_idxA] = ((float) rand()) / ((float) RAND_MAX) * 2.0f - 1.0f;  // noise
        m_impulse_idxA++;
        m_impulse_idxB++;   // We increment the chord-notes too, to prevent weird behaviour in case chords are set to on _after_ the note[s] got triggered
        m_impulse_idxC++;   // We increment the chord-notes too, to prevent weird behaviour in case chords are set to on _after_ the note[s] got triggered
        m_impulse_idxD++;   // We increment the chord-notes too, to prevent weird behaviour in case chords are set to on _after_ the note[s] got triggered
      }
    }
    // --- Update the Karplus Strong buffer related stuff ---
    if (chord_notes > 0)                // GATE_HIGH or GATE_HIGH_NEW
    {
      f_valA = (m_bufferA[m_indexA] + bright*m_bufferA[(m_indexA + 1) % m_bufferA.size()]) * 0.5f *  damping; // low pass filter (average) some samples
      m_bufferA[m_indexA] = f_valA;
      f_valB = (m_bufferB[m_indexB] + bright*m_bufferB[(m_indexB + 1) % m_bufferB.size()]) * 0.5f *  damping; // low pass filter (average) some samples
      m_bufferB[m_indexB] = f_valB;
      f_valC = (m_bufferC[m_indexC] + bright*m_bufferC[(m_indexC + 1) % m_bufferC.size()]) * 0.5f *  damping; // low pass filter (average) some samples
      m_bufferC[m_indexC] = f_valC;

      if( fourth_note )
      {
        f_valD = (m_bufferD[m_indexD] + bright * m_bufferD[(m_indexD + 1) % m_bufferD.size()]) * 0.5f * damping; // low pass filter (average) some samples
        m_bufferD[m_indexD] = f_valD;
        m_indexD = (m_indexD + 1) % m_bufferD.size();
      }
      f_valA = (f_valA + f_valB + f_valC + f_valD) * 0.5f;     // Mix all three (or four) notes and lower Volume to avoid clipping with chords

      m_indexA = (m_indexA + 1) % m_bufferA.size();
      m_indexB = (m_indexB + 1) % m_bufferB.size();
      m_indexC = (m_indexC + 1) % m_bufferC.size();
    }
    else      // Single note
    {
      f_valA = (m_bufferA[m_indexA] + bright*m_bufferA[(m_indexA + 1) % m_bufferA.size()]) * 0.5f *  damping; // low pass filter (average) some samples
      m_bufferA[m_indexA] = f_valA;
      m_indexA = (m_indexA + 1) % m_bufferA.size();
    }
    // --- Calculate the RingMod (we calculate this even if Ringmod-mix is zero, just in case it is mixed in all of a sudden) ---
    f_ringMod = ringModCarrier.Process();

    // --- Adjust "overdrive" and ringmod, if mixer-settings are given the audio output, else do nothing as an performance optimisation ---
    if( driveCharacter > 0.f)
    {
      f_valB = f_valA;
      if (f_valB > 0.f)
        f_valB = 1.f - HELPERS::fasterexp(-1.f*f_valB)/1.5f;     // distort "to max amplitude", see https://dsp.stackexchange.com/questions/13142/digital-distortion-effect-algorithm
      else
      {
        if (f_valB < 0.f)
          f_valB = -1.f + HELPERS::fasterexp(f_valB)/1.5f;          // distort "to min amplitude", the natural logarithm ensures smoothing out on lower levels...
      }
      f_valC = ((1.f - driveCharacter) * f_valA) + (driveCharacter/7.f * f_valB);           // Mix in distorted signal
    }
    else
      f_valC = f_valA;

    // --- Final mix of DSP-results ---
    if( drive_post )      // Decide if fuzz is before or after ringmod
      f_valA = ((1.f - modMix) * f_valC) + (modMix * f_valC * f_ringMod * driveAmount);       // Mix driven ringmod and driven string
    else
      f_valA = ((1.f - modMix) * f_valA) + (modMix * f_valC * f_ringMod * driveAmount);       // Mix ringmod with drive to dry string

    if( vol_env_on )
      f_valA *= vol_env.Process();

    f_valA = min( 1.0f, f_valA * master_gain ); // Limit upper audio range (maybe done in the framework, too?)
    f_valA = max( -1.0f, f_valA );              // Limit lower audio range (maybe done in the framework, too?)

    data.buf[i * 2 + processCh] = f_valA;
  }
}

ctagSoundProcessorKarpuskl::ctagSoundProcessorKarpuskl()
{
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    // --- Initialize buffer for the initial frequency for Karplus Strong ! This is a kind of dummy only to get realistic sized, but it will not play ---
    m_bufferA.resize(max(uint32_t(1),uint32_t(float(44100) / 440.f)));
    m_bufferB.resize(max(uint32_t(1),uint32_t(float(44100) / (440.f / 2.f * 3.f))));  // The size of the buffer results in a varying pitch for this algorithm
    m_bufferC.resize(max(uint32_t(1),uint32_t(float(44100) / (440.f*2.f) ))); // The size of the buffer results in a varying pitch for this algorithm
    m_bufferD.resize(max(uint32_t(1),uint32_t(float(44100) / (440.f/ 2.f * 6.f) ))); // The size of the buffer results in a varying pitch for this algorithm

    // --- Prevent initial playing on startup of the plugin, before triggering by setting the impulse-index to the end of the buffer ---
    m_impulse_idxA = m_bufferA.size();
    m_impulse_idxB = m_bufferB.size();
    m_impulse_idxC = m_bufferC.size();
    m_impulse_idxD = m_bufferD.size();

    // --- Initialize Ringmod ---
    ringModCarrier.SetSampleRate(44100.f);
    ringModCarrier.SetFrequency(120.f);
    ringModLFO.SetSampleRate(44100.f);
    ringModLFO.SetFrequency(5.f);

    // --- Initialize Envelope ---
    vol_env.SetSampleRate(44100.f);    // Sync Env with our audio-processing
    vol_env.SetModeExp();              // Logarithmic scaling
}

ctagSoundProcessorKarpuskl::~ctagSoundProcessorKarpuskl()
{

}

void ctagSoundProcessorKarpuskl::knowYourself(){
    // autogenerated code here
    // sectionCpp0
  pMapPar.emplace("SingleNotes", [&](const int val){ SingleNotes = val;});
  pMapTrig.emplace("SingleNotes", [&](const int val){ trig_SingleNotes = val;});
  pMapPar.emplace("PowerChords", [&](const int val){ PowerChords = val;});
  pMapTrig.emplace("PowerChords", [&](const int val){ trig_PowerChords = val;});
  pMapPar.emplace("FourNoteChords", [&](const int val){ FourNoteChords = val;});
  pMapTrig.emplace("FourNoteChords", [&](const int val){ trig_FourNoteChords = val;});
  pMapPar.emplace("OctUp", [&](const int val){ OctUp = val;});
  pMapTrig.emplace("OctUp", [&](const int val){ trig_OctUp = val;});
  pMapPar.emplace("OctDown", [&](const int val){ OctDown = val;});
  pMapTrig.emplace("OctDown", [&](const int val){ trig_OctDown = val;});
  pMapPar.emplace("KarpusklTrigger", [&](const int val){ KarpusklTrigger = val;});
  pMapTrig.emplace("KarpusklTrigger", [&](const int val){ trig_KarpusklTrigger = val;});
  pMapPar.emplace("KarpusklFrequ", [&](const int val){ KarpusklFrequ = val;});
  pMapCv.emplace("KarpusklFrequ", [&](const int val){ cv_KarpusklFrequ = val;});
  pMapPar.emplace("Damping", [&](const int val){ Damping = val;});
  pMapCv.emplace("Damping", [&](const int val){ cv_Damping = val;});
  pMapPar.emplace("Brightness", [&](const int val){ Brightness = val;});
  pMapCv.emplace("Brightness", [&](const int val){ cv_Brightness = val;});
  pMapPar.emplace("DrivePost", [&](const int val){ DrivePost = val;});
  pMapTrig.emplace("DrivePost", [&](const int val){ trig_DrivePost = val;});
  pMapPar.emplace("DriveChar", [&](const int val){ DriveChar = val;});
  pMapCv.emplace("DriveChar", [&](const int val){ cv_DriveChar = val;});
  pMapPar.emplace("DriveAmnt", [&](const int val){ DriveAmnt = val;});
  pMapCv.emplace("DriveAmnt", [&](const int val){ cv_DriveAmnt = val;});
  pMapPar.emplace("ModMix", [&](const int val){ ModMix = val;});
  pMapCv.emplace("ModMix", [&](const int val){ cv_ModMix = val;});
  pMapPar.emplace("ModFreq", [&](const int val){ ModFreq = val;});
  pMapCv.emplace("ModFreq", [&](const int val){ cv_ModFreq = val;});
  pMapPar.emplace("ModFreqLoHi", [&](const int val){ ModFreqLoHi = val;});
  pMapTrig.emplace("ModFreqLoHi", [&](const int val){ trig_ModFreqLoHi = val;});
  pMapPar.emplace("LFOamnt", [&](const int val){ LFOamnt = val;});
  pMapCv.emplace("LFOamnt", [&](const int val){ cv_LFOamnt = val;});
  pMapPar.emplace("LFOrate", [&](const int val){ LFOrate = val;});
  pMapCv.emplace("LFOrate", [&](const int val){ cv_LFOrate = val;});
  pMapPar.emplace("LFOsquare", [&](const int val){ LFOsquare = val;});
  pMapTrig.emplace("LFOsquare", [&](const int val){ trig_LFOsquare = val;});
  pMapPar.emplace("MasterGain", [&](const int val){ MasterGain = val;});
  pMapCv.emplace("MasterGain", [&](const int val){ cv_MasterGain = val;});
  pMapPar.emplace("EnvActive", [&](const int val){ EnvActive = val;});
  pMapTrig.emplace("EnvActive", [&](const int val){ trig_EnvActive = val;});
  pMapPar.emplace("VolAttack", [&](const int val){ VolAttack = val;});
  pMapCv.emplace("VolAttack", [&](const int val){ cv_VolAttack = val;});
  pMapPar.emplace("VolDecay", [&](const int val){ VolDecay = val;});
  pMapCv.emplace("VolDecay", [&](const int val){ cv_VolDecay = val;});
  pMapPar.emplace("EnvLoop", [&](const int val){ EnvLoop = val;});
  pMapTrig.emplace("EnvLoop", [&](const int val){ trig_EnvLoop = val;});
  pMapPar.emplace("MasterPitch", [&](const int val){ MasterPitch = val;});
  pMapCv.emplace("MasterPitch", [&](const int val){ cv_MasterPitch = val;});
  pMapPar.emplace("QuantizeOn", [&](const int val){ QuantizeOn = val;});
  pMapTrig.emplace("QuantizeOn", [&](const int val){ trig_QuantizeOn = val;});
  isStereo = false;
  id = "Karpuskl";
	// sectionCpp0
}