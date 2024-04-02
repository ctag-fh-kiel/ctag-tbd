/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de
(c) 2020/2021 by Robert Manzke. All rights reserved.

(c) 2021 for the "Bjorklund"-Plugin by Mathias Brüssel
The idea of Bjorklund is to have a simple yet fun to control synthesizer that can be optionally triggered by an internal algorithmical engine
for rhythmical patterns and melodies. Concerning Bjorklunds ideas and implementation of "Euclidean rhythms" please refer to:
http://cgm.cs.mcgill.ca/~godfried/publications/banff.pdf
Concerning the concept of mathematical palindroms, which get used to generate melodies here, please see: https://en.wikipedia.org/wiki/Palindromic_number
The plugin "Bjorklund" uses a wavefolder, as diode-ladder filter and a sawtooth and triangle oscillator by Carlos Laguna Ruiz implemented in his VULT language,
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

#include "ctagSoundProcessorBjorklund.hpp"

using namespace CTAG::SP;

#include "helpers/ctagNumUtil.hpp"
#include "plaits/dsp/engine/engine.h"
#include <algorithm>

// --- VULT "Library for TBD" ---
// #include "./vult/vult_formantor.cpp"  // Already defined in ctagSoundProcessorFormantor, so to avoid double defines we omit it here!
// #include "./vult/vultin.cpp"          // Already defined in ctagSoundProcessorFormantor, so to avoid double defines we omit it here!

#define GATE_HIGH_NEW       2
#define GATE_HIGH           1
#define GATE_LOW            0

// --- Replace function-call of frequency-conversion with macro for increasing speed just a bit ---
#define noteToFreq(incoming_note) (HELPERS::fastpow2 ((incoming_note - 69.f) / 12.f) *440.f)

// --- Rescale random values from 0...1 to needed output ---
#define RESCALE_FLT_MIN_MAX(inname, out_min, out_max) (inname * (out_max-out_min)+out_min)

// --- Additional Macro for automated parameter evaluations ---
#define MK_PITCH_PAR(outname, inname)     float outname = inname; if(cv_##inname != -1) outname += data.cv[cv_##inname]*60.f;
#define MK_BEAT_PAR(outname, inname)      int outname = 33-((int)inname); if(cv_##inname != -1) outname = 33-(((int)(data.cv[cv_##inname]*31.f))+1);

#define MK_TRIG_PAR(outname, inname) int outname = process_param_trig(data, trig_##inname, inname, e_##inname);
#define MK_GATE_PAR(outname, inname) bool outname = (bool)process_param_trig(data, trig_##inname, inname, e_##inname, 1);
#define MK_ADEG_PAR(outname, inname) int outname = process_param_trig(data, trig_##inname, inname, e_##inname, 2);

// --- Modify sine-wave for Squarewave/PWM or various modulations (including Pitch-Mod, Filter-Mod, Z-Scan and Vector-Modulation) ---
#define SINE_TO_SQUARE(sine_val)                      sine_val = (sine_val >= 0) ? 1.f : -1.f;
#define X_FADE(fader, val_left, val_right)            ((1.f-fader)*val_left + fader*val_right)

// --- Process trigger signals and keep their state internally ---
inline int ctagSoundProcessorBjorklund::process_param_trig(const ProcessData &data, int trig_myparm, int my_parm,
                                                           int enum_trigger_id, int gate_type = 0) {
    int trig_status = 0;

    if (trig_myparm != -1)       // Trigger given via CV/Gate or button?
    {
        trig_status = (data.trig[trig_myparm] == 0); // HIGH is 0, so we negate for boolean logic
        if (gate_type == 1)
            return (trig_status);

        if (trig_status)    // Statuschange from HIGH to LOW or LOW to HIGH? Startup-Status for prev_trig_state is -1, so first change is always new
        {
            if (low_reached[enum_trigger_id])    // We had a trigger low before the new trigger high
            {
                if (prev_trig_state[enum_trigger_id] == GATE_LOW || gate_type == 2)   // Toggle or AD EG Trigger...
                {
                    prev_trig_state[enum_trigger_id] = GATE_HIGH;       // Remember status for next round
                    low_reached[enum_trigger_id] = false;
                    return (GATE_HIGH_NEW);           // New trigger
                } else        // previous status was high!
                {
                    prev_trig_state[enum_trigger_id] = GATE_LOW;       // Remember status for next round
                    low_reached[enum_trigger_id] = false;
                    return (GATE_LOW);           // New trigger
                }
            }
        } else
            low_reached[enum_trigger_id] = true;
    } else                        // We may have a trigger set by activating the button via the GUI
    {
        if (my_parm != prev_trig_state[enum_trigger_id])    // Statuschange from HIGH to LOW or LOW to HIGH?
        {
            prev_trig_state[enum_trigger_id] = my_parm;       // Remember status
            if (my_parm != 0)                   // LOW if 0
                return (GATE_HIGH_NEW);          // New trigger
            else
                return (GATE_LOW);           // Trigger released
        }
    }
    return (prev_trig_state[enum_trigger_id]);            // No change (1 for active, 0 for inactive)
}

void ctagSoundProcessorBjorklund::Process(const ProcessData &data) {
// --- Main processing output ---
    float f_val_result = 0.f;         // Sum of results for DSP-output
    float f_val_saw = 0.f;            // Results for Saw Osc
    float f_val_pulse = 0.f;          // Results for Pulse Osc
    float f_val_noise = 0.f;          // Results for Noise Osc
    bool new_accent_trigger = false;  // We need this for possibly triggering accents according to seperate accent-patterns!

    // === Global section ===
    MK_ADEG_PAR(t_Trigger, Trigger);     // We may have a new trigger
    MK_TRIG_PAR(t_InternalClock, InternalClock);
    MK_FLT_PAR_ABS_MIN_MAX(f_ClockSpeed, ClockSpeed, 4095.f, 1.f,
                           128.f); // Equivalent to 60 to 240 BPM and quarters to 32ths...
    MK_BEAT_PAR(i_BeatDivider, BeatDivider);
    MK_PITCH_PAR(f_MasterPitch, MasterPitch);
    MK_FLT_PAR_ABS_SFT(f_MasterTune, MasterTune, 1200.f, 1.f);
    MK_TRIG_PAR(t_QuantizePitch, QuantizePitch);
    MK_FLT_PAR_ABS(f_ScaleCorrect, ScaleCorrect, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_Volume, Volume, 4095.f, 2.f);
    float f_current_note = f_MasterPitch + f_MasterTune;
    int i_current_note = (int) cvPitch.dejitter(
            f_current_note);   // Round and cast to nearest note, we need this for pitch quantize and/or formant selection by key
    if (t_QuantizePitch)
        f_current_note = (float) i_current_note;
    bool new_trigger = (t_Trigger == GATE_HIGH_NEW);

    // === Oscillator section ===
    MK_PITCH_PAR(f_SawPitch, SawPitch);
    MK_FLT_PAR_ABS_SFT(f_SawTune, SawTune, 1200.f, 1.f);
    MK_FLT_PAR_ABS_SFT(f_PulseTune, PulseTune, 1200.f, 1.f);
    MK_PITCH_PAR(f_PulsePitch, PulsePitch);
    MK_TRIG_PAR(t_PWMon, PWMon);
    MK_FLT_PAR_ABS(f_PWMamount, PWMamount, 4095.f, 1.f);
    MK_FLT_PAR_ABS_MIN_MAX(f_PWMspeed, PWMspeed, 4095.f, 0.05f, 20.f);

    // === Mixer section ===
    MK_FLT_PAR_ABS(f_NoiseVol, NoiseVol, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_SawVol, SawVol, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_PulseVol, PulseVol, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_PulseWidth, PulseWidth, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_OSCmix, OSCmix, 4095.f, 1.f);

    // === Effects/Filter section (Distortion, WaveFolder, Ringmodulator, Diode Ladder Filter) ===
    MK_FLT_PAR_ABS(f_WaveFolder, WaveFolder, 4095.f, 1.f);
    MK_TRIG_PAR(t_RingOnSaw, RingOnSaw);
    MK_TRIG_PAR(t_RingOnPulse, RingOnPulse);
    MK_TRIG_PAR(t_AMisSquare, AMisSquare);
    MK_FLT_PAR_ABS_MIN_MAX(f_RingModFreq, RingModFreq, 4095.f, 0.5f, 200.f);
    MK_FLT_PAR_ABS(f_RingModAmnt, RingModAmnt, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_Cutoff, Cutoff, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_Resonance, Resonance, 4095.f, 5.f);
    MK_FLT_PAR_ABS_SFT(f_FilterTracking, FilterTracking, 2047.f, 1.f);
    if (data.cv[cv_MasterPitch] != -1)     // CV in for Master Pitch?
        f_Cutoff += f_FilterTracking *
                    data.cv[cv_MasterPitch]; // adjust Cutoff-frequency according to Filter key tracking setting...
    MK_FLT_PAR_ABS(f_FilterLeakage, FilterLeakage, 4095.f, 1.f);

    // === Bjorklund/Euclid + palindromes based algorithmic sequencer ===
    MK_TRIG_PAR(t_PatternSequencer, PatternSequencer);
    MK_ADEG_PAR(t_ResetSequencer, ResetSequencer);
    MK_TRIG_PAR(t_BjorklundOff, BjorklundOff);
    MK_FLT_PAR_ABS(f_BjorklundPattern, BjorklundPattern, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_BjorklundShift, BjorklundShift, 4095.f, 1.f);
    MK_TRIG_PAR(t_PalindromeOff, PalindromeOff);
    MK_FLT_PAR_ABS(f_PalindromeSelect, PalindromeSelect, 4095.f, 1.f);
    MK_PITCH_PAR(f_PalindromeRootkey, PalindromeRootkey);
    MK_TRIG_PAR(t_AccentOff, AccentOff);
    MK_TRIG_PAR(t_AccentSync, AccentSync);
    MK_INT_PAR_ABS(i_AccentDestination, AccentDestination, 6);
    MK_BEAT_PAR(i_AccentBeatDivider, AccentBeatDivider);
    MK_TRIG_PAR(t_AccentIsBjorklund, AccentIsBjorklund);
    MK_FLT_PAR_ABS(f_AccentSelect, AccentSelect, 4095.f, 1.f);
    MK_FLT_PAR_ABS(f_AccentShift, AccentShift, 4095.f, 1.f);
    MK_FLT_PAR_ABS_SFT(f_AccentAmount, AccentAmount, 2047.f, 1.f);

    // === Pitchquantisation or precalculation of algorithmic sequencer? ===
    if (!t_PatternSequencer)    // No Bjorkldund-patterns, but we may need scale correction on the incoming notes instead
    {
        if (t_QuantizePitch)      // If quantisation is not enabled take the "raw" notes
            f_current_note = (float) (scale_pattern[scale_pat_idx][i_current_note % 12] + (i_current_note / 12) *
                                                                                          12);   // Pick the note from the scale-correction array and adjust the octave
    } else  // Bjorklund Pattern sequencer is on!
    {
        if (t_InternalClock)     // Generate "internal clock" if we don't get the trigger-signal via Gate-in
        {
            lfoClock.SetFrequency(f_ClockSpeed);
            new_trigger = (process_param_trig(data, -1, (int) (lfoClock.Process() >= 0), e_clockVal, 2) ==
                           GATE_HIGH_NEW);
        }
        if (t_ResetSequencer == GATE_HIGH_NEW || t_InternalClock ==
                                                 GATE_HIGH_NEW)   // Internal clock started new of we have a reset-request for the sequencer
        {
            gate_idx = 0;           // Reset the indexes of all relevant arrays of the sequencer to their start-position
            note_idx = 0;
            accent_idx = 0;
        }
        // --- Playback gate pattern as ring-loop and switch to next note from ring when required ---
        if (new_trigger)     // New trigger already detected as sync-signal via gate-in or just generated internally?
        {
            // --- Process Beatdivider for Bjorklund patterns ---
            if (i_BeatDivider != last_beat_divider_counter)
                last_beat_divider_counter = beat_divider_counter = i_BeatDivider;

            beat_divider_counter--;    // This is a countdown from max 32 to min 1 as start to 0 - we will trigger on 0!, so we devide whole notes to 32th notes max and everything in betwean!
            if (beat_divider_counter <= 0)
                beat_divider_counter = last_beat_divider_counter; // Reset for next round! But we had a actual beat-trigger, so new_trigger stays "true"!
            else
                new_trigger = false;    // We had a clock to sync, but no actual new trigger here!

            // --- Process additional Beatdivider for additional Accents on Bjorklund-patterns, targeting optional modulation destinations ---
            if (i_AccentBeatDivider != last_accent_beat_divider_counter)
                last_accent_beat_divider_counter = accent_beat_divider_counter = i_AccentBeatDivider;

            accent_beat_divider_counter--;    // This is a countdown from max 32 to min 1 as start to 0 - we will trigger on 0!, so we devide whole notes to 32th notes max and everything in betwean!
            if (accent_beat_divider_counter <= 0) {
                accent_beat_divider_counter = last_beat_divider_counter; // Reset for next round! But we had a actual accent-trigger
                new_accent_trigger = true;    // We had a clock to sync, and an actual new accent-trigger here! Otherwise new_accent_trigger stays false as initialized!
            }
        }
        // --- Process accent-data (check if we have a trigger from the accent-specific beat-divider! ---
        if (t_AccentOff)                 // Accent patterns are disabled
        {
            new_accent_trigger = false;     // Ignore the accent trigger, if any
            f_accent = 0.f;                 // Delete the last accent, if any
        }
        if (new_accent_trigger)            // We have a trigger for the accents pattern, after evaluation of it's own beat-devider and Accents are enabled
        {
            if (t_AccentIsBjorklund)       // Use Bjorklund patterns for accents
            {
                accent_pat_idx = min(gate_pattern_max_idx, (int) (f_AccentSelect *
                                                                  gate_pattern_size));  // Select scale-pattern depending on setting
                accents_len = strlen(
                        gate_pattern[accent_pat_idx]);  // We only calculate number of notes for pattern with new clock to gain performance
                int cur_accent_idx = (int) (f_AccentShift * (accents_len -
                                                             1));    // Calculate potential setting of Gate-pattern 1 depending on Pot3

                if (cur_accent_idx !=
                    last_accent_idx)            // Check if different setting for current gate-event selected via GUI or CV
                    last_accent_idx = accent_idx = cur_accent_idx;    // Set new index
                // "else": we leave the accent_idx alone, it will be incremented and wrap-around for the current pattern of gate-events...

                if (t_AccentSync && gate_idx ==
                                    0)   // Check if we have to sync/reset the accent-index according to the Bjorklund-pattern
                    accent_idx = 0;                   // Please note: it's important that we check for accents before checking Bjorklund beats, because gate_idx may have become 0 in last round of checking the Bjorklund-index already...

                f_accent = 0.f;                     // [Re]init the accent value to 0, it will be set and/or remembered as we move on...
                if (tolower(gate_pattern[accent_pat_idx][accent_idx]) == 'x')
                    f_accent = f_AccentAmount;        // Major accent, will be remembered as member-variable
            } else                            // Use metrical patterns for accents
            {
                accent_pat_idx = min(accent_pattern_max_idx, (int) (f_AccentSelect *
                                                                    accent_pattern_size));  // Select scale-pattern depending on setting
                accents_len = strlen(
                        accent_pattern[accent_pat_idx]);  // We only calculate number of notes for pattern with new clock to gain performance
                int cur_accent_idx = (int) (f_AccentShift * (accents_len -
                                                             1));    // Calculate potential setting of Gate-pattern 1 depending on Pot3

                if (cur_accent_idx !=
                    last_accent_idx)            // Check if different setting for current gate-event selected via GUI or CV
                    last_accent_idx = accent_idx = cur_accent_idx;    // Set new index
                // "else": we leave the accent_idx alone, it will be incremented and wrap-around for the current pattern of gate-events...

                if (t_AccentSync && gate_idx ==
                                    0)   // Check if we have to sync/reset the accent-index according to the Bjorklund-pattern
                    accent_idx = 0;                   // Please note: it's important that we check for accents before checking Bjorklund beats, because gate_idx may have become 0 in last round of checking the Bjorklund-index already...

                f_accent = 0.f;                     // [Re]init the accent value to 0, it will be set and/or remembered as we move on...
                if (accent_pattern[accent_pat_idx][accent_idx] == 'X')
                    f_accent = f_AccentAmount;        // Major accent, will be remembered as member-variable
                if (accent_pattern[accent_pat_idx][accent_idx] ==
                    'x')  // If no major or minor accent-charactor is encountered f_accent simply will remain 0
                    f_accent = f_AccentAmount * 0.75f;  // Lesser accent, will be remembered as member-variable
            }
            accent_idx++;                       // Increment the gate-index for next round within a ring of data
            accent_idx %= accents_len;          // Remember position for next round, use index within ring of gate-events for notes
        }
        if (new_trigger)           // New trigger as actual trigger (after beat-divider) for Bjorklund-patters now, or maybe a pause to be generated?
        {
            // --- Play new note or pause with each valid beat! ---
            note_pat_idx = min(note_pattern_max_idx, (int) (f_PalindromeSelect *
                                                            note_pattern_size)); // Select note-pattern depending on setting
            gate_pat_idx = min(gate_pattern_max_idx, (int) (f_BjorklundPattern *
                                                            gate_pattern_size));           // Select gate-pattern depending on setting
            scale_pat_idx = min(scale_pattern_max_idx, (int) (f_ScaleCorrect *
                                                              scale_pattern_size));  // Select scale-pattern depending on setting
            gates_len = strlen(
                    gate_pattern[gate_pat_idx]);  // We have to calculate the number of steps in the gates pattern to calculate ring-index!
            notes_len = strlen(
                    note_pattern[note_pat_idx]);  // We only calculate number of notes for pattern with new clock to gain performance
            int cur_gate_idx = (int) (f_BjorklundShift * (gates_len -
                                                          1));    // Calculate potential setting of Gate-pattern 1 depending on Pot3

            if (cur_gate_idx !=
                last_gate_idx)            // Check if different setting for current gate-event selected via GUI or CV
                last_gate_idx = gate_idx = cur_gate_idx;    // Set new index
            // "else": we leave the gate_idx alone, it will be incremented and wrap-around for the current pattern of gate-events...

            // --- Here we mainly find out which note to play ---
            if (tolower(gate_pattern[gate_pat_idx][gate_idx]) == 'x' ||
                t_BjorklundOff)  // Note to be played, "active" gate pattern event
            {                                                                                 // Note: if Bjorklund-mode is turned off, notes will play with every valid beat!
                if (isdigit(note_pattern[note_pat_idx][note_idx]))
                    note_offset = note_pattern[note_pat_idx][note_idx] - '0';
                else
                    note_offset = (note_pattern[note_pat_idx][note_idx] - 'a') + 10;
                CONSTRAIN(note_offset, 0, 31);                       // Just in case we have invalid characters, here...

                if (t_QuantizePitch)           // If quantisation is not enabled take the "raw" notes from the algorithmic patterns...
                    note_offset = scale_pattern[scale_pat_idx][note_offset % 12] + (note_offset / 12) *
                                                                                   12;   // Pick the note from the scale-correction array and adjust the octave

                note_idx++;                     // Increment the note-index for next round within a ring of data
                note_idx %= notes_len;          // Set new note to be played now, use index within ring of note-events for next round
            } else
                new_trigger = false;            // No new note to be played now, wait until next gate-event from Bjorklund-pattern

            gate_idx++;                       // Increment the gate-index for next round within a ring of data
            gate_idx %= gates_len;            // Remember position for next round, use index within ring of gate-events for notes
        }
        if (!t_PalindromeOff)  // "else": If palindrome-melodies are not enabled, Bjorklund patterns will apply triggers only, but notes will be recognized via CV in instead!
        {
            f_current_note = f_MasterPitch + f_PalindromeRootkey +
                             note_offset;   // We apply the note-offset to the masterpitch in the same manner as if it would be played via CV-in
            f_Cutoff += f_FilterTracking * (note_offset /
                                            31.f); // Adjust Filter key tracking setting according to melody-notes, has been set by MasterPitch CV before!...
        }
        // Please note: if Bjorklund, Palindrome, and Accent-mode are all off, we may have done some calculations, but effectively this will be handled as a gate/trigger generator!
    }
    // === Easy edit mode? ===
    MK_TRIG_PAR(t_EasyEditOn, EasyEditOn); // Disable all EGs apart from Volume Envelope!

    // === AD Envelopes section ===
    // --- Volume EG ---
    float vol_eg_process = 1.f;              // Set to max, in case if EG is unused this will work, too!
    MK_TRIG_PAR(t_EGvolActive,
                EGvolActive); // If active we also "precalculate" the Process() values for our main loop (Vol EG first)
    MK_TRIG_PAR(t_EGvolNegative, EGvolNegative);
    MK_FLT_PAR_ABS(f_VolAttack, VolAttack, 4095.f, 8.f);
    MK_FLT_PAR_ABS(f_VolDecay, VolDecay, 4095.f, 14.f);
    MK_FLT_PAR_ABS(f_VolEnvAmount, VolEnvAmount, 4095.f, 1.f);
    MK_TRIG_PAR(t_EGvolLoop, EGvolLoop);
    if (t_EGvolActive)     // Is the envelope activated anyways?
    {
        vol_eg_ad.SetAttack(f_VolAttack);
        vol_eg_ad.SetDecay(f_VolDecay);
        vol_eg_ad.SetLoop((bool) t_EGvolLoop);
        if (new_trigger)    // New trigger encountered?
            vol_eg_ad.Trigger();
        t_EGvolNegative ? vol_eg_process = vol_eg_ad.Process() * (1.f - f_VolEnvAmount) : vol_eg_process =
                                                                                                  vol_eg_ad.Process() *
                                                                                                  f_VolEnvAmount;   // Precalculate current Volume EG, it will be added in the "main" DSP-loop below
    }   // Please note: we don't constrain the volume level from the EG, because this will be evaluated in combination with the master-volume

    // --- Noise EG ---
    MK_TRIG_PAR(t_EGnoiseActive,
                EGnoiseActive); // If active we also "precalculate" the Process() values for our main loop (Vol EG first)
    MK_TRIG_PAR(t_EGnoiseNegative, EGnoiseNegative);
    MK_FLT_PAR_ABS(f_NoiseAttack, NoiseAttack, 4095.f, 8.f);
    MK_FLT_PAR_ABS(f_NoiseDecay, NoiseDecay, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_NoiseEnvAmount, NoiseEnvAmount, 4095.f, 1.f);
    MK_TRIG_PAR(t_EGnoiseLoop, EGnoiseLoop);
    if (t_EGnoiseActive && !t_EasyEditOn)     // Is the envelope activated anyways?
    {
        noise_eg_ad.SetAttack(f_NoiseAttack);
        noise_eg_ad.SetDecay(f_NoiseDecay);
        noise_eg_ad.SetLoop((bool) t_EGnoiseLoop);
        if (new_trigger)    // New trigger encountered?
            noise_eg_ad.Trigger();
        t_EGnoiseNegative ? f_NoiseVol -= noise_eg_ad.Process() * f_NoiseEnvAmount : f_NoiseVol +=
                                                                                             noise_eg_ad.Process() *
                                                                                             f_NoiseEnvAmount;
    }
    // --- OSCmixEnv EG ---
    MK_TRIG_PAR(t_EGoscMixActive,
                EGoscMixActive); // If active we also "precalculate" the Process() values for our main loop (Vol EG first)
    MK_TRIG_PAR(t_EGoscMixNegative, EGoscMixNegative);
    MK_FLT_PAR_ABS(f_OscMixAttack, OscMixAttack, 4095.f, 8.f);
    MK_FLT_PAR_ABS(f_OscMixDecay, OscMixDecay, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_OscMixEnvAmount, OscMixEnvAmount, 4095.f, 1.f);
    MK_TRIG_PAR(t_EGoscMixLoop, EGoscMixLoop);
    if (t_EGoscMixActive && !t_EasyEditOn)     // Is the envelope activated anyways?
    {
        osc_mix_eg_ad.SetAttack(f_OscMixAttack);
        osc_mix_eg_ad.SetDecay(f_OscMixDecay);
        osc_mix_eg_ad.SetLoop((bool) t_EGoscMixLoop);
        if (new_trigger)    // New trigger encountered?
            osc_mix_eg_ad.Trigger();
        t_EGoscMixNegative ? f_OSCmix -= osc_mix_eg_ad.Process() * f_OscMixEnvAmount : f_OSCmix +=
                                                                                               osc_mix_eg_ad.Process() *
                                                                                               f_OscMixEnvAmount;
    }
    // --- Ringmod EG ---
    MK_TRIG_PAR(t_EGringActive,
                EGringActive); // If active we also "precalculate" the Process() values for our main loop (Vol EG first)
    MK_TRIG_PAR(t_EGringNegative, EGringNegative);
    MK_FLT_PAR_ABS(f_RingAttack, RingAttack, 4095.f, 8.f);
    MK_FLT_PAR_ABS(f_RingDecay, RingDecay, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_RingAmount, RingAmount, 4095.f, 1.f);
    MK_TRIG_PAR(t_EGringLoop, EGringLoop);
    if (t_EGringActive && !t_EasyEditOn)     // Is the envelope activated anyways?
    {
        am_eg_ad.SetAttack(f_RingAttack);
        am_eg_ad.SetDecay(f_RingDecay);
        am_eg_ad.SetLoop((bool) t_EGringLoop);
        if (new_trigger)    // New trigger encountered?
            am_eg_ad.Trigger();
        t_EGringNegative ? f_RingModAmnt -= am_eg_ad.Process() * f_RingAmount : f_RingModAmnt += am_eg_ad.Process() *
                                                                                                 f_RingAmount;
    }
    // --- Wavesfolder EG ---
    MK_TRIG_PAR(t_EGwfActive,
                EGwfActive); // If active we also "precalculate" the Process() values for our main loop (Vol EG first)
    MK_TRIG_PAR(t_EGwfNegative, EGwfNegative);
    MK_FLT_PAR_ABS(f_WfAttack, WfAttack, 4095.f, 8.f);
    MK_FLT_PAR_ABS(f_WfDecay, WfDecay, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_WfEnvAmount, WfEnvAmount, 4095.f, 1.f);
    MK_TRIG_PAR(t_EGwfLoop, EGwfLoop);
    if (t_EGwfActive && !t_EasyEditOn)      // Is the envelope activated anyways?
    {
        wf_eg_ad.SetAttack(f_WfAttack);
        wf_eg_ad.SetDecay(f_WfDecay);
        wf_eg_ad.SetLoop((bool) t_EGwfLoop);
        if (new_trigger)    // New trigger encountered?
            wf_eg_ad.Trigger();
        t_EGwfNegative ? f_WaveFolder += wf_eg_ad.Process() * f_WfEnvAmount : f_WaveFolder += wf_eg_ad.Process() *
                                                                                              f_WfEnvAmount;
    }
    // --- Filter EG ---
    MK_TRIG_PAR(t_EGfiltActive,
                EGfiltActive); // If active we also "precalculate" the Process() values for our main loop (Vol EG first)
    MK_TRIG_PAR(t_EGfiltNegative, EGfiltNegative);
    MK_FLT_PAR_ABS(f_FiltAttack, FiltAttack, 4095.f, 8.f);
    MK_FLT_PAR_ABS(f_FiltDecay, FiltDecay, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_FiltAmount, FiltAmount, 4095.f, 1.f);
    MK_TRIG_PAR(t_EGfiltLoop, EGfiltLoop);
    if (t_EGfiltActive && !t_EasyEditOn)     // Is the envelope activated anyways?
    {
        filter_eg_ad.SetAttack(f_FiltAttack);
        filter_eg_ad.SetDecay(f_FiltDecay);
        filter_eg_ad.SetLoop((bool) t_EGfiltLoop);
        if (new_trigger)    // New trigger encountered?
            filter_eg_ad.Trigger();
        t_EGfiltNegative ? f_Cutoff -= filter_eg_ad.Process() * f_FiltAmount : f_Cutoff += filter_eg_ad.Process() *
                                                                                           f_FiltAmount;
    }
    // --- Filter Leakage EG ---
    MK_TRIG_PAR(t_EGfilterLeakActive,
                EGfilterLeakActive); // If active we also "precalculate" the Process() values for our main loop (Vol EG first)
    MK_TRIG_PAR(t_EGfilterLeakNegative, EGfilterLeakNegative);
    MK_FLT_PAR_ABS(f_FilterLeakAttack, FilterLeakAttack, 4095.f, 8.f);
    MK_FLT_PAR_ABS(f_FilterLeakDecay, FilterLeakDecay, 4095.f, 10.f);
    MK_FLT_PAR_ABS(f_FilterLeakEnvAmount, FilterLeakEnvAmount, 4095.f, 1.f);
    MK_TRIG_PAR(t_FilterLeakEnvLoop, FilterLeakEnvLoop);
    if (t_EGfilterLeakActive && !t_EasyEditOn)     // Is the envelope activated anyways?
    {
        filt_leak_eg_ad.SetAttack(f_FilterLeakAttack);
        filt_leak_eg_ad.SetDecay(f_FilterLeakDecay);
        filt_leak_eg_ad.SetLoop((bool) t_FilterLeakEnvLoop);
        if (new_trigger)    // New trigger encountered?
            filt_leak_eg_ad.Trigger();
        t_EGfilterLeakNegative ? f_FilterLeakage -= filt_leak_eg_ad.Process() * f_FilterLeakEnvAmount
                               : f_FilterLeakage += filt_leak_eg_ad.Process() * f_FilterLeakEnvAmount;
    }
    // === Precalculation for realtime DSP loop ===
    oscRing.SetFrequency(f_RingModFreq);
    float f_val_ring = oscRing.Process();                   // Set value for AM/Ringmodulation
    if (t_AMisSquare)                                      // Use square for AM?
        SINE_TO_SQUARE(f_val_ring);
    if (t_PWMon)                                           // PWM activated?
    {
        lfoPWM.SetFrequency(f_PWMspeed);                      // Set LFO for PWM
        f_PulseWidth += lfoPWM.Process() * f_PWMamount;
        CONSTRAIN(f_PulseWidth, 0.f, 1.f);
    }
    float saw_cv = (f_current_note + f_SawPitch + f_SawTune) /
                   120.f;  // These VULT-oscillators are scaled from 0.0 to 1.0 for values betwean "MIDI"-Note 0...127
    CONSTRAIN(saw_cv, 0.f, 1.f);
    float pulse_cv = (f_current_note + f_PulsePitch + f_PulseTune) /
                     120.f;  // These VULT-oscillators are scaled from 0.0 to 1.0 for values betwean "MIDI"-Note 0...127
    // Please note: We don't need to restrict CV for the BLIT algorithm to 0...1

    // --- Precalculate accents if required, and also constrain all EG-amount values as derived already before! ---
    switch (i_AccentDestination) {
        case 0:
            f_VolEnvAmount += f_accent;
            break;
        case 1:
            f_NoiseVol += f_accent;
            break;
        case 2:
            f_OSCmix += f_accent;
            break;
        case 3:
            f_RingModAmnt += f_accent;
            break;
        case 4:
            f_WaveFolder += f_accent;
            break;
        case 5:
            f_Cutoff += f_accent;
            break;
        case 6:
            f_FilterLeakage += f_accent;
            break;
    }
    // --- Constrain all modulatable targets at last, after modulation and accents have been applied ---
    // Please note: no constrain for volume (Target 0) necessary, will be done in context with final adjustment of master-volume
    CONSTRAIN(f_NoiseVol, 0.f, 1.f);          // Target 1
    CONSTRAIN(f_OSCmix, 0.f, 1.f);            // Target 2
    CONSTRAIN(f_RingModAmnt, 0.f, 1.f);       // Target 3
    CONSTRAIN(f_WaveFolder, 0.f, 1.f);        // Target 4
    CONSTRAIN(f_Cutoff, 0.f, 1.f);            // Target 5
    CONSTRAIN(f_FilterLeakage, 0.f, 1.f);     // Target 6

    // --- Xfade Saw and Pulse waves ---
    f_SawVol *= (1.f - f_OSCmix);
    f_PulseVol *= f_OSCmix;

    // --- Precalculate AM/Ringmodulator if required ---
    if (t_RingOnSaw)
        f_SawVol = f_SawVol * f_val_ring * f_RingModAmnt;
    if (t_RingOnPulse)
        f_PulseVol = f_PulseVol * f_val_ring * f_RingModAmnt;

    // Precalculate X-Fade of Filterleakage
    float f_FilterLeakage_left = 1.f - f_FilterLeakage;

    // --- Precalculate EG with VCA ---
    vol_eg_process *= f_Volume;

    // === Realtime DSP output loop ===
    uint32_t target = bufSz * 2;

    for (uint32_t i = 0; i < target; i++) {
        // --- Calculate the oscillators ---
        if ((i % 3) == 0)
            f_val_noise = Noise_process(noise_data, 0.f);
        f_val_saw = Saw_eptr_process(saw_data, saw_cv);
        f_val_pulse = Blit_osc_blit(pulse_data, pulse_cv, f_PulseWidth,
                                    0.f); // data, cv, pulse_width, wave - last parameter !=1 => pulse wave

        // --- Wave Folder and optionally Ringmodulator ---
        f_val_result = Fold_do((f_val_noise * f_NoiseVol + f_val_pulse * f_PulseVol + f_val_saw * f_SawVol),
                               f_WaveFolder);   // Apply wavefolder on mix of all oscillators, X-fading Pulse and Saw before

        // --- Diode Ladder Filter (with optional "leakage", meaning optional bypassing of the raw signal) ---
        f_val_result =
                Ladder_process_euler(ladder_vintage_data, f_val_result, f_Cutoff, f_Resonance) * f_FilterLeakage_left +
                f_val_result * f_FilterLeakage; // Euler-logic for cutoff-pitch

        // --- Adjust volume and restrict to max. range ---
        f_val_result *= vol_eg_process;

        // --- Output of DSP-results ---
        data.buf[i] = f_val_result;
        data.buf[++i] = f_val_result;
    }


}

void ctagSoundProcessorBjorklund::Init(std::size_t blockSize, void *blockPtr) {
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    // --- Additional oscillator for Ringmod ---
    oscRing.SetSampleRate(44100.f / bufSz);   // Performance-optimisation for AM: bufSz currently is 32 => 1378,125 Hz
    oscRing.SetFrequency(1.f);

    // --- LFO for PWM of Pulse Oscillator and internal Clock ---
    lfoPWM.SetSampleRate(44100.f / bufSz);    // bufSz currently is 32 => 1378,125 Hz
    lfoPWM.SetFrequency(1.f);

    lfoClock.SetSampleRate(44100.f / bufSz);    // bufSz currently is 32 => 1378,125 Hz
    lfoClock.SetFrequency(1.f);

    // --- Initialize Envelopes ---
    noise_eg_ad.SetSampleRate(44100.f / bufSz);    // Sync Env with our audio-processing
    noise_eg_ad.SetModeExp();  // Logarithmic scaling

    osc_mix_eg_ad.SetSampleRate(44100.f / bufSz);    // Sync Env with our audio-processing
    osc_mix_eg_ad.SetModeExp();  // Logarithmic scaling

    filt_leak_eg_ad.SetSampleRate(44100.f / bufSz);    // Sync Env with our audio-processing
    filt_leak_eg_ad.SetModeExp();  // Logarithmic scaling

    wf_eg_ad.SetSampleRate(44100.f / bufSz);    // Sync Env with our audio-processing
    wf_eg_ad.SetModeExp();  // Logarithmic scaling

    am_eg_ad.SetSampleRate(44100.f / bufSz);    // Sync Env with our audio-processing
    am_eg_ad.SetModeExp();  // Logarithmic scaling

    filter_eg_ad.SetSampleRate(44100.f / bufSz);    // Sync Env with our audio-processing
    filter_eg_ad.SetModeExp();  // Logarithmic scaling

    vol_eg_ad.SetSampleRate(44100.f / bufSz);    // Sync Env with our audio-processing
    vol_eg_ad.SetModeExp();  // Logarithmic scaling

    // --- VULT ---
    Noise_process_init(noise_data);       // Noise (White or Pink)
    Saw_eptr_process_init(saw_data);      // Saw wave
    Blit__ctx_type_1_init(pulse_data);    // Variable rectangle wave

    Ladder_process_euler_init(
            ladder_vintage_data); // Diode Ladder Filter with Euler based resonance frequency smoothing

}

ctagSoundProcessorBjorklund::~ctagSoundProcessorBjorklund() {
}

void ctagSoundProcessorBjorklund::knowYourself() {
    // autogenerated code here
    // sectionCpp0
    pMapPar.emplace("Trigger", [&](const int val) { Trigger = val; });
    pMapTrig.emplace("Trigger", [&](const int val) { trig_Trigger = val; });
    pMapPar.emplace("BeatDivider", [&](const int val) { BeatDivider = val; });
    pMapCv.emplace("BeatDivider", [&](const int val) { cv_BeatDivider = val; });
    pMapPar.emplace("InternalClock", [&](const int val) { InternalClock = val; });
    pMapTrig.emplace("InternalClock", [&](const int val) { trig_InternalClock = val; });
    pMapPar.emplace("ClockSpeed", [&](const int val) { ClockSpeed = val; });
    pMapCv.emplace("ClockSpeed", [&](const int val) { cv_ClockSpeed = val; });
    pMapPar.emplace("MasterPitch", [&](const int val) { MasterPitch = val; });
    pMapCv.emplace("MasterPitch", [&](const int val) { cv_MasterPitch = val; });
    pMapPar.emplace("MasterTune", [&](const int val) { MasterTune = val; });
    pMapCv.emplace("MasterTune", [&](const int val) { cv_MasterTune = val; });
    pMapPar.emplace("QuantizePitch", [&](const int val) { QuantizePitch = val; });
    pMapTrig.emplace("QuantizePitch", [&](const int val) { trig_QuantizePitch = val; });
    pMapPar.emplace("ScaleCorrect", [&](const int val) { ScaleCorrect = val; });
    pMapCv.emplace("ScaleCorrect", [&](const int val) { cv_ScaleCorrect = val; });
    pMapPar.emplace("Volume", [&](const int val) { Volume = val; });
    pMapCv.emplace("Volume", [&](const int val) { cv_Volume = val; });
    pMapPar.emplace("SawPitch", [&](const int val) { SawPitch = val; });
    pMapCv.emplace("SawPitch", [&](const int val) { cv_SawPitch = val; });
    pMapPar.emplace("SawTune", [&](const int val) { SawTune = val; });
    pMapCv.emplace("SawTune", [&](const int val) { cv_SawTune = val; });
    pMapPar.emplace("PulsePitch", [&](const int val) { PulsePitch = val; });
    pMapCv.emplace("PulsePitch", [&](const int val) { cv_PulsePitch = val; });
    pMapPar.emplace("PulseTune", [&](const int val) { PulseTune = val; });
    pMapCv.emplace("PulseTune", [&](const int val) { cv_PulseTune = val; });
    pMapPar.emplace("PulseWidth", [&](const int val) { PulseWidth = val; });
    pMapCv.emplace("PulseWidth", [&](const int val) { cv_PulseWidth = val; });
    pMapPar.emplace("PWMon", [&](const int val) { PWMon = val; });
    pMapTrig.emplace("PWMon", [&](const int val) { trig_PWMon = val; });
    pMapPar.emplace("PWMspeed", [&](const int val) { PWMspeed = val; });
    pMapCv.emplace("PWMspeed", [&](const int val) { cv_PWMspeed = val; });
    pMapPar.emplace("PWMamount", [&](const int val) { PWMamount = val; });
    pMapCv.emplace("PWMamount", [&](const int val) { cv_PWMamount = val; });
    pMapPar.emplace("NoiseVol", [&](const int val) { NoiseVol = val; });
    pMapCv.emplace("NoiseVol", [&](const int val) { cv_NoiseVol = val; });
    pMapPar.emplace("SawVol", [&](const int val) { SawVol = val; });
    pMapCv.emplace("SawVol", [&](const int val) { cv_SawVol = val; });
    pMapPar.emplace("PulseVol", [&](const int val) { PulseVol = val; });
    pMapCv.emplace("PulseVol", [&](const int val) { cv_PulseVol = val; });
    pMapPar.emplace("OSCmix", [&](const int val) { OSCmix = val; });
    pMapCv.emplace("OSCmix", [&](const int val) { cv_OSCmix = val; });
    pMapPar.emplace("RingOnSaw", [&](const int val) { RingOnSaw = val; });
    pMapTrig.emplace("RingOnSaw", [&](const int val) { trig_RingOnSaw = val; });
    pMapPar.emplace("RingOnPulse", [&](const int val) { RingOnPulse = val; });
    pMapTrig.emplace("RingOnPulse", [&](const int val) { trig_RingOnPulse = val; });
    pMapPar.emplace("AMisSquare", [&](const int val) { AMisSquare = val; });
    pMapTrig.emplace("AMisSquare", [&](const int val) { trig_AMisSquare = val; });
    pMapPar.emplace("RingModFreq", [&](const int val) { RingModFreq = val; });
    pMapCv.emplace("RingModFreq", [&](const int val) { cv_RingModFreq = val; });
    pMapPar.emplace("RingModAmnt", [&](const int val) { RingModAmnt = val; });
    pMapCv.emplace("RingModAmnt", [&](const int val) { cv_RingModAmnt = val; });
    pMapPar.emplace("WaveFolder", [&](const int val) { WaveFolder = val; });
    pMapCv.emplace("WaveFolder", [&](const int val) { cv_WaveFolder = val; });
    pMapPar.emplace("Cutoff", [&](const int val) { Cutoff = val; });
    pMapCv.emplace("Cutoff", [&](const int val) { cv_Cutoff = val; });
    pMapPar.emplace("Resonance", [&](const int val) { Resonance = val; });
    pMapCv.emplace("Resonance", [&](const int val) { cv_Resonance = val; });
    pMapPar.emplace("FilterTracking", [&](const int val) { FilterTracking = val; });
    pMapCv.emplace("FilterTracking", [&](const int val) { cv_FilterTracking = val; });
    pMapPar.emplace("FilterLeakage", [&](const int val) { FilterLeakage = val; });
    pMapCv.emplace("FilterLeakage", [&](const int val) { cv_FilterLeakage = val; });
    pMapPar.emplace("PatternSequencer", [&](const int val) { PatternSequencer = val; });
    pMapTrig.emplace("PatternSequencer", [&](const int val) { trig_PatternSequencer = val; });
    pMapPar.emplace("ResetSequencer", [&](const int val) { ResetSequencer = val; });
    pMapTrig.emplace("ResetSequencer", [&](const int val) { trig_ResetSequencer = val; });
    pMapPar.emplace("BjorklundOff", [&](const int val) { BjorklundOff = val; });
    pMapTrig.emplace("BjorklundOff", [&](const int val) { trig_BjorklundOff = val; });
    pMapPar.emplace("BjorklundPattern", [&](const int val) { BjorklundPattern = val; });
    pMapCv.emplace("BjorklundPattern", [&](const int val) { cv_BjorklundPattern = val; });
    pMapPar.emplace("BjorklundShift", [&](const int val) { BjorklundShift = val; });
    pMapCv.emplace("BjorklundShift", [&](const int val) { cv_BjorklundShift = val; });
    pMapPar.emplace("PalindromeOff", [&](const int val) { PalindromeOff = val; });
    pMapTrig.emplace("PalindromeOff", [&](const int val) { trig_PalindromeOff = val; });
    pMapPar.emplace("PalindromeSelect", [&](const int val) { PalindromeSelect = val; });
    pMapCv.emplace("PalindromeSelect", [&](const int val) { cv_PalindromeSelect = val; });
    pMapPar.emplace("PalindromeRootkey", [&](const int val) { PalindromeRootkey = val; });
    pMapCv.emplace("PalindromeRootkey", [&](const int val) { cv_PalindromeRootkey = val; });
    pMapPar.emplace("AccentOff", [&](const int val) { AccentOff = val; });
    pMapTrig.emplace("AccentOff", [&](const int val) { trig_AccentOff = val; });
    pMapPar.emplace("AccentSync", [&](const int val) { AccentSync = val; });
    pMapTrig.emplace("AccentSync", [&](const int val) { trig_AccentSync = val; });
    pMapPar.emplace("AccentDestination", [&](const int val) { AccentDestination = val; });
    pMapCv.emplace("AccentDestination", [&](const int val) { cv_AccentDestination = val; });
    pMapPar.emplace("AccentBeatDivider", [&](const int val) { AccentBeatDivider = val; });
    pMapCv.emplace("AccentBeatDivider", [&](const int val) { cv_AccentBeatDivider = val; });
    pMapPar.emplace("AccentIsBjorklund", [&](const int val) { AccentIsBjorklund = val; });
    pMapTrig.emplace("AccentIsBjorklund", [&](const int val) { trig_AccentIsBjorklund = val; });
    pMapPar.emplace("AccentSelect", [&](const int val) { AccentSelect = val; });
    pMapCv.emplace("AccentSelect", [&](const int val) { cv_AccentSelect = val; });
    pMapPar.emplace("AccentShift", [&](const int val) { AccentShift = val; });
    pMapCv.emplace("AccentShift", [&](const int val) { cv_AccentShift = val; });
    pMapPar.emplace("AccentAmount", [&](const int val) { AccentAmount = val; });
    pMapCv.emplace("AccentAmount", [&](const int val) { cv_AccentAmount = val; });
    pMapPar.emplace("EGvolActive", [&](const int val) { EGvolActive = val; });
    pMapTrig.emplace("EGvolActive", [&](const int val) { trig_EGvolActive = val; });
    pMapPar.emplace("EGvolNegative", [&](const int val) { EGvolNegative = val; });
    pMapTrig.emplace("EGvolNegative", [&](const int val) { trig_EGvolNegative = val; });
    pMapPar.emplace("VolAttack", [&](const int val) { VolAttack = val; });
    pMapCv.emplace("VolAttack", [&](const int val) { cv_VolAttack = val; });
    pMapPar.emplace("VolDecay", [&](const int val) { VolDecay = val; });
    pMapCv.emplace("VolDecay", [&](const int val) { cv_VolDecay = val; });
    pMapPar.emplace("VolEnvAmount", [&](const int val) { VolEnvAmount = val; });
    pMapCv.emplace("VolEnvAmount", [&](const int val) { cv_VolEnvAmount = val; });
    pMapPar.emplace("EGvolLoop", [&](const int val) { EGvolLoop = val; });
    pMapTrig.emplace("EGvolLoop", [&](const int val) { trig_EGvolLoop = val; });
    pMapPar.emplace("EasyEditOn", [&](const int val) { EasyEditOn = val; });
    pMapTrig.emplace("EasyEditOn", [&](const int val) { trig_EasyEditOn = val; });
    pMapPar.emplace("EGnoiseActive", [&](const int val) { EGnoiseActive = val; });
    pMapTrig.emplace("EGnoiseActive", [&](const int val) { trig_EGnoiseActive = val; });
    pMapPar.emplace("EGnoiseNegative", [&](const int val) { EGnoiseNegative = val; });
    pMapTrig.emplace("EGnoiseNegative", [&](const int val) { trig_EGnoiseNegative = val; });
    pMapPar.emplace("NoiseAttack", [&](const int val) { NoiseAttack = val; });
    pMapCv.emplace("NoiseAttack", [&](const int val) { cv_NoiseAttack = val; });
    pMapPar.emplace("NoiseDecay", [&](const int val) { NoiseDecay = val; });
    pMapCv.emplace("NoiseDecay", [&](const int val) { cv_NoiseDecay = val; });
    pMapPar.emplace("NoiseEnvAmount", [&](const int val) { NoiseEnvAmount = val; });
    pMapCv.emplace("NoiseEnvAmount", [&](const int val) { cv_NoiseEnvAmount = val; });
    pMapPar.emplace("EGnoiseLoop", [&](const int val) { EGnoiseLoop = val; });
    pMapTrig.emplace("EGnoiseLoop", [&](const int val) { trig_EGnoiseLoop = val; });
    pMapPar.emplace("EGoscMixActive", [&](const int val) { EGoscMixActive = val; });
    pMapTrig.emplace("EGoscMixActive", [&](const int val) { trig_EGoscMixActive = val; });
    pMapPar.emplace("EGoscMixNegative", [&](const int val) { EGoscMixNegative = val; });
    pMapTrig.emplace("EGoscMixNegative", [&](const int val) { trig_EGoscMixNegative = val; });
    pMapPar.emplace("OscMixAttack", [&](const int val) { OscMixAttack = val; });
    pMapCv.emplace("OscMixAttack", [&](const int val) { cv_OscMixAttack = val; });
    pMapPar.emplace("OscMixDecay", [&](const int val) { OscMixDecay = val; });
    pMapCv.emplace("OscMixDecay", [&](const int val) { cv_OscMixDecay = val; });
    pMapPar.emplace("OscMixEnvAmount", [&](const int val) { OscMixEnvAmount = val; });
    pMapCv.emplace("OscMixEnvAmount", [&](const int val) { cv_OscMixEnvAmount = val; });
    pMapPar.emplace("EGoscMixLoop", [&](const int val) { EGoscMixLoop = val; });
    pMapTrig.emplace("EGoscMixLoop", [&](const int val) { trig_EGoscMixLoop = val; });
    pMapPar.emplace("EGringActive", [&](const int val) { EGringActive = val; });
    pMapTrig.emplace("EGringActive", [&](const int val) { trig_EGringActive = val; });
    pMapPar.emplace("EGringNegative", [&](const int val) { EGringNegative = val; });
    pMapTrig.emplace("EGringNegative", [&](const int val) { trig_EGringNegative = val; });
    pMapPar.emplace("RingAttack", [&](const int val) { RingAttack = val; });
    pMapCv.emplace("RingAttack", [&](const int val) { cv_RingAttack = val; });
    pMapPar.emplace("RingDecay", [&](const int val) { RingDecay = val; });
    pMapCv.emplace("RingDecay", [&](const int val) { cv_RingDecay = val; });
    pMapPar.emplace("RingAmount", [&](const int val) { RingAmount = val; });
    pMapCv.emplace("RingAmount", [&](const int val) { cv_RingAmount = val; });
    pMapPar.emplace("EGringLoop", [&](const int val) { EGringLoop = val; });
    pMapTrig.emplace("EGringLoop", [&](const int val) { trig_EGringLoop = val; });
    pMapPar.emplace("EGwfActive", [&](const int val) { EGwfActive = val; });
    pMapTrig.emplace("EGwfActive", [&](const int val) { trig_EGwfActive = val; });
    pMapPar.emplace("EGwfNegative", [&](const int val) { EGwfNegative = val; });
    pMapTrig.emplace("EGwfNegative", [&](const int val) { trig_EGwfNegative = val; });
    pMapPar.emplace("WfAttack", [&](const int val) { WfAttack = val; });
    pMapCv.emplace("WfAttack", [&](const int val) { cv_WfAttack = val; });
    pMapPar.emplace("WfDecay", [&](const int val) { WfDecay = val; });
    pMapCv.emplace("WfDecay", [&](const int val) { cv_WfDecay = val; });
    pMapPar.emplace("WfEnvAmount", [&](const int val) { WfEnvAmount = val; });
    pMapCv.emplace("WfEnvAmount", [&](const int val) { cv_WfEnvAmount = val; });
    pMapPar.emplace("EGwfLoop", [&](const int val) { EGwfLoop = val; });
    pMapTrig.emplace("EGwfLoop", [&](const int val) { trig_EGwfLoop = val; });
    pMapPar.emplace("EGfiltActive", [&](const int val) { EGfiltActive = val; });
    pMapTrig.emplace("EGfiltActive", [&](const int val) { trig_EGfiltActive = val; });
    pMapPar.emplace("EGfiltNegative", [&](const int val) { EGfiltNegative = val; });
    pMapTrig.emplace("EGfiltNegative", [&](const int val) { trig_EGfiltNegative = val; });
    pMapPar.emplace("FiltAttack", [&](const int val) { FiltAttack = val; });
    pMapCv.emplace("FiltAttack", [&](const int val) { cv_FiltAttack = val; });
    pMapPar.emplace("FiltDecay", [&](const int val) { FiltDecay = val; });
    pMapCv.emplace("FiltDecay", [&](const int val) { cv_FiltDecay = val; });
    pMapPar.emplace("FiltAmount", [&](const int val) { FiltAmount = val; });
    pMapCv.emplace("FiltAmount", [&](const int val) { cv_FiltAmount = val; });
    pMapPar.emplace("EGfiltLoop", [&](const int val) { EGfiltLoop = val; });
    pMapTrig.emplace("EGfiltLoop", [&](const int val) { trig_EGfiltLoop = val; });
    pMapPar.emplace("EGfilterLeakActive", [&](const int val) { EGfilterLeakActive = val; });
    pMapTrig.emplace("EGfilterLeakActive", [&](const int val) { trig_EGfilterLeakActive = val; });
    pMapPar.emplace("EGfilterLeakNegative", [&](const int val) { EGfilterLeakNegative = val; });
    pMapTrig.emplace("EGfilterLeakNegative", [&](const int val) { trig_EGfilterLeakNegative = val; });
    pMapPar.emplace("FilterLeakAttack", [&](const int val) { FilterLeakAttack = val; });
    pMapCv.emplace("FilterLeakAttack", [&](const int val) { cv_FilterLeakAttack = val; });
    pMapPar.emplace("FilterLeakDecay", [&](const int val) { FilterLeakDecay = val; });
    pMapCv.emplace("FilterLeakDecay", [&](const int val) { cv_FilterLeakDecay = val; });
    pMapPar.emplace("FilterLeakEnvAmount", [&](const int val) { FilterLeakEnvAmount = val; });
    pMapCv.emplace("FilterLeakEnvAmount", [&](const int val) { cv_FilterLeakEnvAmount = val; });
    pMapPar.emplace("FilterLeakEnvLoop", [&](const int val) { FilterLeakEnvLoop = val; });
    pMapTrig.emplace("FilterLeakEnvLoop", [&](const int val) { trig_FilterLeakEnvLoop = val; });
    isStereo = true;
    id = "Bjorklund";
    // sectionCpp0
}