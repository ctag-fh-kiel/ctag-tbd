/*************** 
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020 by Robert Manzke. All rights reserved.

(c) 2020 by Mathias Brüssel for this "ByteBeatsXFade"-Plugin. All rights reserved.
Includes ByteBeat algorithms by Matt Wand - make sure to have a look at his other great work at: hot-air.bandcamp.com

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "ctagSoundProcessorBBeats.hpp"

using namespace CTAG::SP;

// data.buf[i * 2 + processCh] = (float)((byte)(t & 128));    // Square wave, working ok!

// --- Helper function: rescale CV or Pot to integer of given range 0...max ---
inline int ctagSoundProcessorBBeats::process_param(const ProcessData &data, int cv_myparm, int my_parm, int parm_range, int max_idx )
{
  if (cv_myparm != -1) 
  {
    if( data.cv[cv_myparm] >= 0.0 )       // This is a bypass solution to avoid negative values in rare cases
      return (int)(data.cv[cv_myparm] * parm_range);
    else
      return 0;
  }
  else
    return my_parm * parm_range / max_idx;
}

// --- Helper function: rescale CV or Pot to float 0...1.0 (CV is already in correct format, we still keep it inside this method for convenience ---
inline float ctagSoundProcessorBBeats::process_param_float(const ProcessData &data, int cv_myparm, int my_parm, int max_idx )
{
  if(cv_myparm != -1)
  {
    if (data.cv[cv_myparm] >= 0.0f)     // This is a bypass solution to avoid negative values in rare cases
      return data.cv[cv_myparm];
    else
      return 0.0f;
  }
  else
    return my_parm / (float)max_idx;
}


inline bool ctagSoundProcessorBBeats::process_param_bool(const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id, bool direct_eg_trigger )
{
  if(trig_myparm != -1)       // Trigger given via CV/Gate or button?
  {
    if (data.trig[trig_myparm] != prev_trig_state[prev_trig_state_id])    // Statuschange from HIGH to LOW or LOW to HIGH?
    {
      prev_trig_state[prev_trig_state_id] = data.trig[trig_myparm];       // Remember status
      if( data.trig[trig_myparm] == 0 )                                   // If changed to HIGH
      {
        if (!direct_eg_trigger)                                           // Normal toggle-function of EG-Trigger with active looping of EG
          toggle_trig_state[prev_trig_state_id] = !toggle_trig_state[prev_trig_state_id];
        else                      // Direct Trigger option only is active, if looping-parameter is off, as given via 'direct_eg_trigger'
        {
          toggle_trig_state[prev_trig_state_id] = true;                   // Set to "direct triggered" here, too, also the function will return true below, then!
          direct_trigger[prev_trig_state_id] = true;
        }
      }
    }
    return toggle_trig_state[prev_trig_state_id];                         // If true it was toggled from false or directly trggered, if used with EG we still have to decide there if direct triggering happened
  }
  else                        // We may have a trigger set by activating the button via the GUI
  {
    if (my_parm != 0)         // Button on GUI set to "on"
    {
      prev_trig_state[prev_trig_state_id] = 0;  // Remember "on" status
      return true;
    }
    else
    {
      prev_trig_state[prev_trig_state_id] = 1;  // Remember "off" status
      return false;
    }
  }
}

void ctagSoundProcessorBBeats::Process(const ProcessData &data)
{
  // --- Read triggers or bools from GUI about every 3 milliseconds and buffer results as private member variables ---
  if( ++cv_counter%11 == 0)
  {
    // --- Read and buffer options for ByteBeat A ---
    stop_beatA = process_param_bool(data, trig_beatA_stop, beatA_stop, e_stop_beatA);
    reverse_beatA = process_param_bool(data, trig_beatA_backwards, beatA_backwards, e_reverse_beatA);

    // --- Read and buffer option for ByteBeat B ---
    stop_beatB = process_param_bool(data, trig_beatB_stop, beatB_stop, e_stop_beatB);
    reverse_beatB = process_param_bool(data, trig_beatB_backwards, beatB_backwards, e_reverse_beatB);

    // --- Read and buffer option for resetting ByteBeats on stop ---
    reset_bbeats_on_stop = process_param_bool(data, trig_reset_bbeats_on_stop, reset_bbeats_on_stop,
                                              e_reset_bbeats_on_stop);

    // Read and buffer triggers and options for enveloppe 1
    eg_loop_1 = process_param_bool(data, trig_loopEG_1, loopEG_1, e_loopEG_1);
    env_1.SetLoop(eg_loop_1);

    activateEG_1 = process_param_bool(data, trig_activateEG_1, activateEG_1, e_activateEG_1, !eg_loop_1);
    if( activateEG_1 )
    {
      if( eg_was_off_1 || direct_trigger[e_activateEG_1] )  // If no looping is active, trigger EG directly! (Direct trigger may get set in process_param_bool, this is a bit dirty but well ;-)
      {
        eg_was_off_1 = false;
        direct_trigger[e_activateEG_1] = false;
        env_1.Trigger();
      }
    }
    else
      eg_was_off_1 = true;

    // Read and buffer triggers and options for enveloppe 2
    eg_loop_2 = process_param_bool( data, trig_loopEG_2, loopEG_2, e_loopEG_2 );
    env_2.SetLoop( eg_loop_2 );

    activateEG_2 = process_param_bool(data, trig_activateEG_2, activateEG_2, e_activateEG_2, !eg_loop_2);
    if(activateEG_2)
    {
      if( eg_was_off_2 || direct_trigger[e_activateEG_2] )  //  If no looping is active, trigger EG directly!
      {
        eg_was_off_2 = false;
        direct_trigger[e_activateEG_2] = false;
        env_2.Trigger();
      }
    }
    else
      eg_was_off_2 = true;

    // Read and buffer triggers and options for enveloppe 3
    eg_loop_3 = process_param_bool( data, trig_loopEG_3, loopEG_3, e_loopEG_3 );
    env_3.SetLoop( eg_loop_3 );

    activateEG_3 = process_param_bool(data, trig_activateEG_3, activateEG_3, e_activateEG_3, !eg_loop_3);
    if(activateEG_3)
    {
      if( eg_was_off_3 || direct_trigger[e_activateEG_3] ) // If no looping is active, trigger EG directly!
      {
        eg_was_off_3 = false;
        direct_trigger[e_activateEG_3] = false;
        env_3.Trigger();
      }
    }
    else
      eg_was_off_3 = true;

    // Read and buffer triggers and options for enveloppe 4
    eg_loop_4 = process_param_bool( data, trig_loopEG_4, loopEG_4, e_loopEG_4 );
    env_4.SetLoop( eg_loop_4 );

    activateEG_4 = process_param_bool(data, trig_activateEG_4, activateEG_4, e_activateEG_4, !eg_loop_4);
    if(activateEG_4)
    {
      if( eg_was_off_4 || direct_trigger[e_activateEG_4] )  // If no looping is active, trigger EG directly!
      {
        eg_was_off_4 = false;
        direct_trigger[e_activateEG_4] = false;
        env_4.Trigger();
      }
    }
    else
      eg_was_off_4 = true;

  }
  // --- Read controllers from GUI or CV about every 16 milliseconds and buffer results as private member variables ---
  if( ++cv_counter%100 == 0 )
  {
    // --- Read and buffer controllers for ByteBeat A ---
    if (stop_beatA && reset_bbeats_on_stop)
      reverse_beatA ? t1 = -1 : t1 = 1;      // reset incrementor for bytebeat algorithms, avoid 0 to not devide by zero
    beat_index_A = process_param(data, cv_beatA_select, beatA_select, BEAT_A_MAX_IDX, BEAT_A_MAX_IDX);
    slow_down_A_factor = process_param(data, cv_beatA_pitch, beatA_pitch, BEAT_MAX_PITCH, BEAT_MAX_PITCH);

    // --- Read and buffer controllers for ByteBeat B ---
    if (stop_beatB && reset_bbeats_on_stop)
      reverse_beatB ? t2 = -1 : t2 = 1;    // reset incrementor for bytebeat algorithms, avoid 0 to not devide by zero
    beat_index_B = process_param(data, cv_beatB_select, beatB_select, BEAT_B_MAX_IDX, BEAT_B_MAX_IDX);
    slow_down_B_factor = process_param(data, cv_beatB_pitch, beatB_pitch, BEAT_MAX_PITCH, BEAT_MAX_PITCH);

    // --- Read and buffer controllers for mixing ByteBeat A with ByteBeat B ---
    vca_vol = process_param_float(data, cv_volume, volume, 4095);
    xfade_val = process_param_float(data, cv_xFadeA_B, xFadeA_B, 4095);

    // --- Read and buffer controllers for Enveloppe 1 ---
    eg_amount_1 = 2.f*process_param_float(data, cv_amountEG_1, amountEG_1, 4095);  // We add additional headroom to "overdrive" the EG by doubling the max amount
    eg_dest_1 = process_param(data, cv_destinationEG_1, destinationEG_1, 6, 6);

    if (cv_attackEG_1 != -1)
    {
      if( data.cv[cv_attackEG_1] >= 0.f )
        attackVal_1 = data.cv[cv_attackEG_1] * data.cv[cv_attackEG_1] * 2.f; // power of two prevents negative values and adjusts pots to more expotential behaviour
    }
    else
      attackVal_1 = (float) attackEG_1 / 4095.f * 5.f;    // get attack value from GUI, set time
    env_1.SetAttack(attackVal_1);

    if (cv_decayEG_1 != -1)
    {
      if( data.cv[cv_decayEG_1] >= 0.f )
        decayVal_1 = data.cv[decayEG_1] * data.cv[decayEG_1] * 4.f;     // power of two prevents negative values and adjusts pots to more expotential behaviour
    }
    else
      decayVal_1 = (float) decayEG_1 / 4095.f * 10.f;   // get decay value from GUI, set time
    env_1.SetDecay(decayVal_1);

    // --- Read and buffer controllers for Enveloppe 2 ---
    eg_amount_2 = 2.f*process_param_float(data, cv_amountEG_2, amountEG_2, 4095); // We add additional headroom to "overdrive" the EG by doubling the max. amount
    eg_dest_2 = process_param(data, cv_destinationEG_2, destinationEG_2, 6, 6);

    if (cv_attackEG_2 != -1)
    {
      if( data.cv[cv_attackEG_2] >= 0.f )
        attackVal_2 = data.cv[cv_attackEG_2] * data.cv[cv_attackEG_2] * 2.f;  // power of two prevents negative values and adjusts pots to more expotential behaviour
    }
    else
      attackVal_2 = (float) attackEG_2 / 4095.f * 5.f;      // get attack value from GUI, set time
    env_2.SetAttack(attackVal_2);

    if (cv_decayEG_2 != -1)
    {
      if( data.cv[cv_decayEG_2] >= 0.f )
        decayVal_2 = data.cv[decayEG_2] * data.cv[decayEG_2] * 4.f; // power of two prevents negative values and adjusts pots to more expotential behaviour
    }
    else
      decayVal_2 = (float) decayEG_2 / 4095.f * 10.f;     // get decay value from GUI, set time
    env_2.SetDecay(decayVal_2);
  }

  // --- Read and buffer controllers for Enveloppe 3 ---
  eg_amount_3 = 2.f*process_param_float(data, cv_amountEG_3, amountEG_3, 4095);  // We add additional headroom to "overdrive" the EG by doubling the max amount
  eg_dest_3 = process_param(data, cv_destinationEG_3, destinationEG_3, 6, 6);

  if (cv_attackEG_3 != -1)
  {
    if( data.cv[cv_attackEG_3] >= 0.f )
      attackVal_3 = data.cv[cv_attackEG_3] * data.cv[cv_attackEG_3] * 2.f; // power of two prevents negative values and adjusts pots to more expotential behaviour
  }
  else
    attackVal_3 = (float) attackEG_3 / 4095.f * 5.f;    // get attack value from GUI, set time
  env_3.SetAttack(attackVal_3);

  if (cv_decayEG_3 != -1)
  {
    if( data.cv[cv_decayEG_3] >= 0.f )
      decayVal_3 = data.cv[decayEG_3] * data.cv[decayEG_3] * 4.f;     // power of two prevents negative values and adjusts pots to more expotential behaviour
  }
  else
    decayVal_3 = (float) decayEG_3 / 4095.f * 10.f;   // get decay value from GUI, set time
  env_3.SetDecay(decayVal_3);

  // --- Read and buffer controllers for Enveloppe 4 ---
  eg_amount_4 = 2.f*process_param_float(data, cv_amountEG_4, amountEG_4, 4095);  // We add additional headroom to "overdrive" the EG by doubling the max amount
  eg_dest_4 = process_param(data, cv_destinationEG_4, destinationEG_4, 6, 6);

  if (cv_attackEG_4 != -1)
  {
    if( data.cv[cv_attackEG_4] >= 0.f )
      attackVal_4 = data.cv[cv_attackEG_4] * data.cv[cv_attackEG_4] * 2.f; // power of two prevents negative values and adjusts pots to more expotential behaviour
  }
  else
    attackVal_4 = (float) attackEG_4 / 4095.f * 5.f;    // get attack value from GUI, set time
  env_4.SetAttack(attackVal_4);

  if (cv_decayEG_4 != -1)
  {
    if( data.cv[cv_decayEG_4] >= 0.f )
      decayVal_4 = data.cv[decayEG_4] * data.cv[decayEG_4] * 4.f;     // power of two prevents negative values and adjusts pots to more expotential behaviour
  }
  else
    decayVal_4 = (float) decayEG_4 / 4095.f * 10.f;   // get decay value from GUI, set time
  env_4.SetDecay(decayVal_4);

  // --- Locally remember the relevant member-variables needed for continuous modulation by modulator 1 and 2 in main loop ---
  int l_beat_index_A = beat_index_A;
  int l_slow_down_A_level = BEAT_PITCH_BOUNDARY-slow_down_A_factor;
  int l_beat_index_B = beat_index_B;
  int l_slow_down_B_level = BEAT_PITCH_BOUNDARY-slow_down_B_factor;
  float l_xfade_val = xfade_val;
  float l_vca_vol = vca_vol;

  // --- This is our main loop, where the generation and mixing of ByteBeats takes place ---
  for (uint32_t i = 0; i < bufSz; i++)
  {
    // --- Process Modulators from EG-1 ---
    if(activateEG_1)
    {
      switch (eg_dest_1)
      {
        case 0:     // EG is off, nothing to do here - note: either uses the originally set value or value has been modulated by modulator 2 in last round if the two modulators have the identical destination! ---
          break;
        case 1:     // In all cases: add modulator value to current setting, use range from current value to maximum, may be reduced by amount-setting to a lower range
          l_beat_index_A = (int) (eg_amount_1 * env_1.Process() * BEAT_A_MAX_IDX) * (BEAT_A_MAX_IDX - beat_index_A) / BEAT_A_MAX_IDX + beat_index_A;  // Map to range between current and max index
          break;
        case 2:
          l_slow_down_A_level = BEAT_PITCH_BOUNDARY-((int)(eg_amount_1 * env_1.Process() * BEAT_MAX_PITCH) * (BEAT_MAX_PITCH - slow_down_A_factor) / BEAT_MAX_PITCH + slow_down_A_factor);
          break;
        case 3:
          l_beat_index_B = (int) (eg_amount_1 * env_1.Process() * BEAT_B_MAX_IDX) * (BEAT_B_MAX_IDX - beat_index_B) / BEAT_B_MAX_IDX + beat_index_B;  // Map to range between current and max index
          break;
        case 4:
          l_slow_down_B_level = BEAT_PITCH_BOUNDARY-((int)(eg_amount_1 * env_1.Process() * BEAT_MAX_PITCH) * (BEAT_MAX_PITCH - slow_down_B_factor) / BEAT_MAX_PITCH + slow_down_B_factor);
          break;
        case 5:
          l_vca_vol = eg_amount_1 * env_1.Process() * (1.f - vca_vol) + vca_vol; // We raise the volume for the EG quite a bit to make it work more similar to normal volumes without EG
          break;
        case 6:
          l_xfade_val = eg_amount_1 * env_1.Process() * (1.f - xfade_val) + xfade_val; // We shift the Xfader to the right for the EG just a bit to make it work more similar to normal xFades
          break;
        default:     // Unexpected, do nothing
          break;
      }
    }
    // --- Process Modulators from EG-2 - note: modulator 2 may overwrite modulation of modulator 1 if destinations are identical, in other words: (to save performance) targets are exclusive and modulation does not add up
    if( activateEG_2 )
    {
      switch (eg_dest_2)
      {
        case 0:     // EG is off, nothing to do here - note: either uses the originally set value or value has been modulated by modulator 1 above, if the two modulators have the identical destination! ---
          break;
        case 1:     // In all cases: add modulator value to current setting, use range from current value to maximum, may be reduced by amount-setting to a lower range
          l_beat_index_A = (int) (eg_amount_2 * env_2.Process() * BEAT_A_MAX_IDX) * (BEAT_A_MAX_IDX - beat_index_A) / BEAT_A_MAX_IDX + beat_index_A;  // Map to range between current and max index
          break;
        case 2:
          l_slow_down_A_level = BEAT_PITCH_BOUNDARY-((int)(eg_amount_2 * env_2.Process() * BEAT_MAX_PITCH) * (BEAT_MAX_PITCH - slow_down_A_factor) / BEAT_MAX_PITCH + slow_down_A_factor);
          break;
        case 3:
          l_beat_index_B = (int) (eg_amount_2 * env_2.Process() * BEAT_B_MAX_IDX) * (BEAT_B_MAX_IDX - beat_index_B) / BEAT_A_MAX_IDX + beat_index_B;  // Map to range between current and max index
          break;
        case 4:
          l_slow_down_B_level = BEAT_PITCH_BOUNDARY-((int)(eg_amount_2 * env_2.Process() * BEAT_MAX_PITCH) * (BEAT_MAX_PITCH - slow_down_B_factor) / BEAT_MAX_PITCH + slow_down_B_factor);
          break;
        case 5:
          l_vca_vol = eg_amount_2 * env_2.Process() * (1.f - vca_vol) + vca_vol; // We raise the volume for the EG quite a bit to make it work more similar to normal volumes without EG
          break;
        case 6:
          l_xfade_val = eg_amount_2 * env_2.Process() * (1.f - xfade_val) + xfade_val; // We shift the Xfader to the right for the EG just a bit to make it work more similar to normal xFades
          break;
        default:     // Unexpected, do nothing
          break;
      }
    }

    // --- Process Modulators from EG-3 - note: modulator 2 may overwrite modulation of modulator 1 if destinations are identical, in other words: (to save performance) targets are exclusive and modulation does not add up
    if( activateEG_3 )
    {
      switch (eg_dest_3)
      {
        case 0:     // EG is off, nothing to do here - note: either uses the originally set value or value has been modulated by modulator 1 above, if the two modulators have the identical destination! ---
          break;
        case 1:     // In all cases: add modulator value to current setting, use range from current value to maximum, may be reduced by amount-setting to a lower range
          l_beat_index_A = (int) (eg_amount_3 * env_3.Process() * BEAT_A_MAX_IDX) * (BEAT_A_MAX_IDX - beat_index_A) / BEAT_A_MAX_IDX + beat_index_A;  // Map to range between current and max index
          break;
        case 2:
          l_slow_down_A_level = BEAT_PITCH_BOUNDARY-((int)(eg_amount_3 * env_3.Process() * BEAT_MAX_PITCH) * (BEAT_MAX_PITCH - slow_down_A_factor) / BEAT_MAX_PITCH + slow_down_A_factor);
          break;
        case 3:
          l_beat_index_B = (int) (eg_amount_3 * env_3.Process() * BEAT_B_MAX_IDX) * (BEAT_B_MAX_IDX - beat_index_B) / BEAT_A_MAX_IDX + beat_index_B;  // Map to range between current and max index
          break;
        case 4:
          l_slow_down_B_level = BEAT_PITCH_BOUNDARY-((int)(eg_amount_3 * env_3.Process() * BEAT_MAX_PITCH) * (BEAT_MAX_PITCH - slow_down_B_factor) / BEAT_MAX_PITCH + slow_down_B_factor);
          break;
        case 5:
          l_vca_vol = eg_amount_3 * env_3.Process() * (1.f - vca_vol) + vca_vol; // We raise the volume for the EG quite a bit to make it work more similar to normal volumes without EG
          break;
        case 6:
          l_xfade_val = eg_amount_3 * env_3.Process() * (1.f - xfade_val) + xfade_val; // We shift the Xfader to the right for the EG just a bit to make it work more similar to normal xFades
          break;
        default:     // Unexpected, do nothing
          break;
      }
    }

    // --- Process Modulators from EG-4 - note: modulator 2 may overwrite modulation of modulator 1 if destinations are identical, in other words: (to save performance) targets are exclusive and modulation does not add up
    if( activateEG_4 )
    {
      switch (eg_dest_4)
      {
        case 0:     // EG is off, nothing to do here - note: either uses the originally set value or value has been modulated by modulator 1 above, if the two modulators have the identical destination! ---
          break;
        case 1:     // In all cases: add modulator value to current setting, use range from current value to maximum, may be reduced by amount-setting to a lower range
          l_beat_index_A = (int) (eg_amount_4 * env_4.Process() * BEAT_A_MAX_IDX) * (BEAT_A_MAX_IDX - beat_index_A) / BEAT_A_MAX_IDX + beat_index_A;  // Map to range between current and max index
          break;
        case 2:
          l_slow_down_A_level = BEAT_PITCH_BOUNDARY-((int)(eg_amount_4 * env_4.Process() * BEAT_MAX_PITCH) * (BEAT_MAX_PITCH - slow_down_A_factor) / BEAT_MAX_PITCH + slow_down_A_factor);
          break;
        case 3:
          l_beat_index_B = (int) (eg_amount_4 * env_4.Process() * BEAT_B_MAX_IDX) * (BEAT_B_MAX_IDX - beat_index_B) / BEAT_A_MAX_IDX + beat_index_B;  // Map to range between current and max index
          break;
        case 4:
          l_slow_down_B_level = BEAT_PITCH_BOUNDARY-((int)(eg_amount_4 * env_4.Process() * BEAT_MAX_PITCH) * (BEAT_MAX_PITCH - slow_down_B_factor) / BEAT_MAX_PITCH + slow_down_B_factor);
          break;
        case 5:
          l_vca_vol = eg_amount_4 * env_4.Process() * (1.f - vca_vol) + vca_vol; // We raise the volume for the EG quite a bit to make it work more similar to normal volumes without EG
          break;
        case 6:
          l_xfade_val = eg_amount_4 * env_4.Process() * (1.f - xfade_val) + xfade_val; // We shift the Xfader to the right for the EG just a bit to make it work more similar to normal xFades
          break;
        default:     // Unexpected, do nothing
          break;
      }
    }
    // --- In case of overflow truncate indices, volume or Xfade value, this may happen if we have a high amount and purposly "overdrove" the EG! ---
    l_beat_index_A = l_beat_index_A > BEAT_A_MAX_IDX ? BEAT_A_MAX_IDX : l_beat_index_A;
    l_beat_index_B = l_beat_index_B > BEAT_B_MAX_IDX ? BEAT_B_MAX_IDX : l_beat_index_B;
    l_vca_vol = l_vca_vol>1.f ? 1.f : l_vca_vol;
    l_xfade_val = l_xfade_val>1.f ? 1.f : l_xfade_val;

    // --- Process ByteBeat A ---
    if (slow_down_A >= l_slow_down_A_level)  // High frequencies are low rates of repetition of previous audio-calculation
    {
      slow_down_A = 0;
      if( t1==0 )
        reverse_beatA ? t1-- : t1++;    // Avoid division by zero for some algos (including modulo operations!)
      if( !stop_beatA )
      {
        beat_byte_A = beats_P1[l_beat_index_A](t1);       // We may will also need the numeric value for logic operations on the ByteBeats
        beat_val_A = (float) (beat_byte_A - 127) / 127.0f; // beat_val_A: private member, so we buffer the result
        reverse_beatA ? t1-- : t1++;   // Decrement or increment iterator for ByteBeat1 algorithm
      }
    }
    slow_down_A++;  // We increment a counter for Beat1 every loop, so we can decide with next loop if we generate a new valur

    // --- Process ByteBeat B ---
    if (slow_down_B >= l_slow_down_B_level) // High frequencies are low rates of repetition of previous audio-calculation
    {
      slow_down_B = 0;
      if( t2==0 )
        reverse_beatA ? t2-- : t2++;    // Avoid devision by zero for some algos (including modulo operations!)
      if( !stop_beatB )
      {
        beat_byte_B = beats_P2[l_beat_index_B](t2);       // We may will also need the numeric value for logic operations on the ByteBeats
        beat_val_B = (float)(beat_byte_B - 127) / 127.0f; // beat_val_B: private member, so we buffer the result
        reverse_beatB ? t2-- : t2++;   // Decrement or increment iterator for ByteBeat1 algorithm
      }
    }
    slow_down_B++;  // We increment a counter for Beat1 every loop, so we can decide with next loop if we generate a new valur

    // --- Mix ByteBeat A and ByteBeat B, apply calculations that may have happend via EG-modulation already, too! ---
    data.buf[i * 2 + processCh] = (beat_val_A * (1.0f - l_xfade_val) + beat_val_B * l_xfade_val) * l_vca_vol;        // Mix both ByteBeats, depending on XFade-factor, add envelope
  }
}

void ctagSoundProcessorBBeats::Init(std::size_t const &blockSize, void *const blockPtr)
{
  // construct internal data model
  knowYourself();
  model = std::make_unique<ctagSPDataModel>(id, isStereo);
  LoadPreset(0);

  // Add AD-envelopes
  env_1.SetSampleRate(44100.f);    // Sync Env with our audio-processing
  env_1.SetModeLin();                   // Linear scaling
  env_2.SetSampleRate(44100.f);    // Sync Env with our audio-processing
  env_2.SetModeExp();                   // Logarithmic scaling
  env_3.SetSampleRate(44100.f);    // Sync Env with our audio-processing
  env_3.SetModeLin();                   // Linear scaling
  env_4.SetSampleRate(44100.f);    // Sync Env with our audio-processing
  env_4.SetModeExp();
}

ctagSoundProcessorBBeats::~ctagSoundProcessorBBeats()
{
}

void ctagSoundProcessorBBeats::knowYourself()
{
  // autogenerated code here
  // sectionCpp0
  pMapPar.emplace("beatA_stop", [&](const int val){ beatA_stop = val;});
  pMapTrig.emplace("beatA_stop", [&](const int val){ trig_beatA_stop = val;});
  pMapPar.emplace("beatA_backwards", [&](const int val){ beatA_backwards = val;});
  pMapTrig.emplace("beatA_backwards", [&](const int val){ trig_beatA_backwards = val;});
  pMapPar.emplace("beatA_select", [&](const int val){ beatA_select = val;});
  pMapCv.emplace("beatA_select", [&](const int val){ cv_beatA_select = val;});
  pMapPar.emplace("beatA_pitch", [&](const int val){ beatA_pitch = val;});
  pMapCv.emplace("beatA_pitch", [&](const int val){ cv_beatA_pitch = val;});
  pMapPar.emplace("beatB_stop", [&](const int val){ beatB_stop = val;});
  pMapTrig.emplace("beatB_stop", [&](const int val){ trig_beatB_stop = val;});
  pMapPar.emplace("beatB_backwards", [&](const int val){ beatB_backwards = val;});
  pMapTrig.emplace("beatB_backwards", [&](const int val){ trig_beatB_backwards = val;});
  pMapPar.emplace("beatB_select", [&](const int val){ beatB_select = val;});
  pMapCv.emplace("beatB_select", [&](const int val){ cv_beatB_select = val;});
  pMapPar.emplace("beatB_pitch", [&](const int val){ beatB_pitch = val;});
  pMapCv.emplace("beatB_pitch", [&](const int val){ cv_beatB_pitch = val;});
  pMapPar.emplace("reset_bbeats_on_stop", [&](const int val){ reset_bbeats_on_stop = val;});
  pMapTrig.emplace("reset_bbeats_on_stop", [&](const int val){ trig_reset_bbeats_on_stop = val;});
  pMapPar.emplace("volume", [&](const int val){ volume = val;});
  pMapCv.emplace("volume", [&](const int val){ cv_volume = val;});
  pMapPar.emplace("xFadeA_B", [&](const int val){ xFadeA_B = val;});
  pMapCv.emplace("xFadeA_B", [&](const int val){ cv_xFadeA_B = val;});
  pMapPar.emplace("destinationEG_1", [&](const int val){ destinationEG_1 = val;});
  pMapCv.emplace("destinationEG_1", [&](const int val){ cv_destinationEG_1 = val;});
  pMapPar.emplace("activateEG_1", [&](const int val){ activateEG_1 = val;});
  pMapTrig.emplace("activateEG_1", [&](const int val){ trig_activateEG_1 = val;});
  pMapPar.emplace("loopEG_1", [&](const int val){ loopEG_1 = val;});
  pMapTrig.emplace("loopEG_1", [&](const int val){ trig_loopEG_1 = val;});
  pMapPar.emplace("amountEG_1", [&](const int val){ amountEG_1 = val;});
  pMapCv.emplace("amountEG_1", [&](const int val){ cv_amountEG_1 = val;});
  pMapPar.emplace("attackEG_1", [&](const int val){ attackEG_1 = val;});
  pMapCv.emplace("attackEG_1", [&](const int val){ cv_attackEG_1 = val;});
  pMapPar.emplace("decayEG_1", [&](const int val){ decayEG_1 = val;});
  pMapCv.emplace("decayEG_1", [&](const int val){ cv_decayEG_1 = val;});
  pMapPar.emplace("destinationEG_2", [&](const int val){ destinationEG_2 = val;});
  pMapCv.emplace("destinationEG_2", [&](const int val){ cv_destinationEG_2 = val;});
  pMapPar.emplace("activateEG_2", [&](const int val){ activateEG_2 = val;});
  pMapTrig.emplace("activateEG_2", [&](const int val){ trig_activateEG_2 = val;});
  pMapPar.emplace("loopEG_2", [&](const int val){ loopEG_2 = val;});
  pMapTrig.emplace("loopEG_2", [&](const int val){ trig_loopEG_2 = val;});
  pMapPar.emplace("amountEG_2", [&](const int val){ amountEG_2 = val;});
  pMapCv.emplace("amountEG_2", [&](const int val){ cv_amountEG_2 = val;});
  pMapPar.emplace("attackEG_2", [&](const int val){ attackEG_2 = val;});
  pMapCv.emplace("attackEG_2", [&](const int val){ cv_attackEG_2 = val;});
  pMapPar.emplace("decayEG_2", [&](const int val){ decayEG_2 = val;});
  pMapCv.emplace("decayEG_2", [&](const int val){ cv_decayEG_2 = val;});
  pMapPar.emplace("destinationEG_3", [&](const int val){ destinationEG_3 = val;});
  pMapCv.emplace("destinationEG_3", [&](const int val){ cv_destinationEG_3 = val;});
  pMapPar.emplace("activateEG_3", [&](const int val){ activateEG_3 = val;});
  pMapTrig.emplace("activateEG_3", [&](const int val){ trig_activateEG_3 = val;});
  pMapPar.emplace("loopEG_3", [&](const int val){ loopEG_3 = val;});
  pMapTrig.emplace("loopEG_3", [&](const int val){ trig_loopEG_3 = val;});
  pMapPar.emplace("amountEG_3", [&](const int val){ amountEG_3 = val;});
  pMapCv.emplace("amountEG_3", [&](const int val){ cv_amountEG_3 = val;});
  pMapPar.emplace("attackEG_3", [&](const int val){ attackEG_3 = val;});
  pMapCv.emplace("attackEG_3", [&](const int val){ cv_attackEG_3 = val;});
  pMapPar.emplace("decayEG_3", [&](const int val){ decayEG_3 = val;});
  pMapCv.emplace("decayEG_3", [&](const int val){ cv_decayEG_3 = val;});
  pMapPar.emplace("destinationEG_4", [&](const int val){ destinationEG_4 = val;});
  pMapCv.emplace("destinationEG_4", [&](const int val){ cv_destinationEG_4 = val;});
  pMapPar.emplace("activateEG_4", [&](const int val){ activateEG_4 = val;});
  pMapTrig.emplace("activateEG_4", [&](const int val){ trig_activateEG_4 = val;});
  pMapPar.emplace("loopEG_4", [&](const int val){ loopEG_4 = val;});
  pMapTrig.emplace("loopEG_4", [&](const int val){ trig_loopEG_4 = val;});
  pMapPar.emplace("amountEG_4", [&](const int val){ amountEG_4 = val;});
  pMapCv.emplace("amountEG_4", [&](const int val){ cv_amountEG_4 = val;});
  pMapPar.emplace("attackEG_4", [&](const int val){ attackEG_4 = val;});
  pMapCv.emplace("attackEG_4", [&](const int val){ cv_attackEG_4 = val;});
  pMapPar.emplace("decayEG_4", [&](const int val){ decayEG_4 = val;});
  pMapCv.emplace("decayEG_4", [&](const int val){ cv_decayEG_4 = val;});
  isStereo = false;
  id = "BBeats";
  // sectionCpp0
}