/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.
(c) 2023 MIDI-Voicemodes and Eventhandling aka 'class Midi' by Mathias Brüssel. 

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "Midi.hpp"

using namespace CTAG::CTRL;

// === Private Helper-Funtions ===
// --- Map incoming CCs for Voices A-D to CVs according their representation as used with the TBD BBA, to be distributed according to names mapped via the UI ---
Midi::CV_id_abcd Midi::ccToCVid_abcd(uint8_t cc) 
{
    switch (cc)     // Voices A-D: Convert incoming CC-numbers to valid enum of processed CCs or 'none' which effectively is 0 and unused!
    {
        case 0:
            return bank_00;    // For "strong typing" we use typed enums, but still cast to normal numeric values, because we need them as indexes
        case 32:
            return sbnk_32; 
        case 1:
            return mw_01; 
        case 2:
            return bc_02; 
        case 71:
            return res_71; 
        case 72:
            return rel_72; 
        case 73:
            return atk_73; 
        case 74:
            return cut_74; 
        case 75:
            return sc6_75; 
        case 76:
            return sc7_76; 
        case 77:
            return sc8_77; 
        case 78:
            return sc9_78;
        case 123:           // All notes off, not mappable, but used to directly reset current triggers to prevent hanging notes...   
            allNotesOff();  // No break: same return as default -> We listen to MIDI all notes off event on all channels! (Here channel 1 or indirectly 15 or 16)     
            return abcd_cc_invalid;   // Unexpected value, so we know the incoming CC can't be mapped to values known for the Voices A-D
        default:
            return abcd_cc_invalid;   // Unexpected value, so we know the incoming CC can't be mapped to values known for the Voices A-D
 
    }
}

// --- Map incoming CCs for "global channel" to CVs according their representation as used with the TBD BBA, to be distributed according to names mapped via the UI ---
Midi::CV_id_glob Midi::ccToCVid_glob(uint8_t cc) 
{
    switch (cc)     // Voices A-D: Convert incoming CC-numbers to valid enum of processed CCs or 'none' which effectively is 0 and unused!
    {
        case 1:
            return g_mw_01;    // For "strong typing" we use typed enums, but still cast to normal numeric values, because we need them as indexes
        case 2:
            return g_bc_02; 
        case 4:
            return g_foot_04; 
        case 6:
            return g_data_06; 
        case 7:
            return g_vol_07; 
        case 8:
            return g_bal_08; 
        case 10:
            return g_pan_10;
        case 11:
            return g_expr_11; 
        case 12:
            return g_fx1_12; 
        case 13:
            return g_fx2_13; 
        case 64:
            return g_sustp_64; 
        case 65:
            return g_portp_65; 
        case 66:
            return g_sstnp_66; 
        case 67:
            return g_softp_67; 
        case 69:
            return g_holdp_69;
        case 123:           // All notes off, not mappable, but used to directly reset current triggers to prevent hanging notes...   
            allNotesOff();  // We listen to MIDI all notes off event on all channels! (Here channel 2-5 or indirectly 6-14) 
            return glob_cc_invalid;   // Unexpected value, so we know the incoming CC can't be mapped to values known for the globals 
        default:
            return glob_cc_invalid;   // Unexpected value, so we know the incoming CC can't be mapped to values known for the globals
 
    }
}

// --- Map incoming CCs for Voices A-D to Triggers according their representation as used with the TBD BBA, to be distributed according to names mapped via the UI ---
Midi::TRIG_id_abcd Midi::ccToTrigId_abcd(uint8_t cc) 
{
    switch (cc)     // Convert incoming CC-numbers to valid enum of processed CCs or 'none' which effectively is 0 and unused!
    {
        case 75:      
            return t_sc6_75;      // For "strong typing" we use typed enums, but still cast to normal numeric values, because we need them as indexes
        case 76:      
            return t_sc7_76;
        case 77:      
            return t_sc8_77;
        case 79:      
            return t_sc9_79;
        default:
            return t_abcd_cc_invalid;   // Unexpected value, so we know the incoming CC can't be mapped to values known for the TBD BBA...
    }
}

// --- Map incoming CCs from global channel to Triggers according their representation as used with the TBD BBA, to be distributed according to names mapped via the UI ---
Midi::TRIG_id_glob Midi::ccToTrigId_glob(uint8_t cc) 
{
    switch (cc)     // Convert incoming CC-numbers to valid enum of processed CCs or 'none' which effectively is 0 and unused!
    {
        case 12:      
            return t_g_fx1_12;  // For "strong typing" we use typed enums, but still cast to normal numeric values, because we need them as indexes
        case 13:      
            return t_g_fx2_13;
        case 64:      
            return t_g_sustp_64;
        case 65:      
            return t_g_portp_65;
        case 66:      
            return t_g_sostp_66;
        case 67:      
            return t_g_softp_67;
        case 69:      
            return t_g_holdp_69;
        default:
            return t_glob_cc_invalid;   // Unexpected value, so we know the incoming CC can't be mapped to values known for the TBD BBA...
    }
}

// --- "Panic-function": avoid hanging notes, especially after voidemode-Change ---
void Midi::allNotesOff()                // Typically to be triggered by all notes off message (Continouus Controller)
{                                       
    monophonic_keys_down = 0;           // No more active keys for monophonic mode...
    memset( midi_note_pressed, 0, sizeof(midi_note_pressed) );  // No monophonic keys pressed anymore (please note: this is a two dimensional aray, but we disable settings for all channels!)
    
    for( int polyChannels=1; polyChannels<5; polyChannels++)           // Reset MPT/Mono- Duo- and/or Polyphonic notes
    {
        last_midi_note_pitch_abcd[polyChannels] = MIDI_INVALD_NOTE;    // Reset active note
        trig_entry = trig_distributor[polyChannels][(uint8_t)t_note];  // Retrieve MIDI-channel (internally 0-15) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
        midi_triggers[(uint8_t)trig_entry.element_trig_id] = TRIG_OFF;        // Set Triggers to off
    }
}


// --- Handle Noteoffs via MIDI (Either by noteoff-event or noteon-event with velocity 0), set Triggers accordinly and reset playing note ---
void Midi::handleNoteOff(uint8_t*  msg) 
{
    // === Retrieve MIDI-channel (internally 0-15) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index ===
    channel = msg[0]&0x0F;   // Please note: channel 0 is global, 2-5 are voices, 6-16 are mapped to 2-5, (16 may be global for MPE or unused, so that we don't get any data here anyhow)
    
    // === Get type and ID for CV and/or Trigger with given CC and Channel, if any (we use different CCs on Master and Voice-channel) ===
    // --- Monophonic voice mode ---
    if( channel == MIDI_GLOBAL_CHANNEL ) 
    {
        channel = MIDI_VOICE_A;     // We map monophonic play to Voice A
        midi_note_pressed[vmode_mono][msg[1]] = 0;    // This note will no longer be used for monophonic play mode
        monophonic_keys_down = max(0, monophonic_keys_down-1 ); // Reduce number of pressed notes, make sure to not become negative by accident
        
        if( monophonic_keys_down )  // Still notes pressed?
        {
            for( int i=0; i<sizeof(midi_note_pressed[vmode_mono]); i++)
            {
                if(midi_note_pressed[vmode_mono][i])              // Entry for Legato found?
                {
                    msg[1] = i;                       // Set note for legato pitch-change     
                    msg[2] = midi_note_pressed[vmode_mono][i];    // Set velocity for legato velocity-change
                    midi_cvs[(uint8_t)A_NOTE] = min(1.f, (msg[1]-MIDI_NOTE_0V)*SCALEVALUE_MIDI_NOTE_TO_CV);    // Set CV at given offset as just retrieved, rescale according to CV from CCs 
                    midi_cvs[(uint8_t)A_VELO] = msg[2] * SCALEVALUE_MIDI_CTRLS_TO_CV;      // Set CV at given offset as just retrieved, rescale according to CV from CCs 
                    return; // We exit here in order to avoid resetting the trigger for this channel, because we found a new note via legato!
                }    
            }
        }   // "else": do the normal "on exit processing" to reset the trigger for this note now.
    }
    else // Not monophonic Play mode
    {
        // --- Duophonic or Polyphonic mode? Check if we found a noteOff of the currently playing note? ---
        if(IS_UPPER_GLOBAL(channel))              // Convert Duophonic or Polyphonc to Voice A+B or A-D respectively?
        {
            // --- Duophonic or Polyphonic mode? Decide on upper limit for loop through channels ---
            uint8_t voice_mode = vmode_duo_ab;      // "Default" poly voicemode: Duophonic-AB
            uint8_t duo_offset = 0;                 // This is a channel-offset to distinguish between Duo-AB or Duo-CD, default is AB
            uint8_t upper_channel_for_voices = 3;   // Max two voices, beginning with MIDI-Channel 2, so upper MIDI-Channel limit is 3 (internally 2).
            
            // --- Make basic settings, depending on voicemode ---
            switch(channel)
            {
                case MIDI_GLOBAL_CHANNEL_15:        // Voicemode Duophonic_CD
                    duo_offset = 2;
                    upper_channel_for_voices = 5;
                    voice_mode = vmode_duo_cd;
                    break;

                case MIDI_GLOBAL_CHANNEL_16:        // Voicemode Polyphonic
                    upper_channel_for_voices = 5;
                    voice_mode = vmode_4voice;
                    break;        
            }
            // --- We have a MIDI-note of, regardless if it has been playing or not, unmark pressed key now ---
            midi_note_pressed[voice_mode][msg[1]] = 0;

            // --- Check if we have a noteoff for a currently playing voice ---
            channel = 0; // Set channel to a valid range for array, but unless something goes wrong we'll only use MIDI-channels 2-5 (Voices A-d) for notes!
            bool note_off_for_playing_note = false;     // We assume that the current noteoff is not associated with an already playing voice
            for(int channel_for_noteoff=MIDI_VOICE_A+duo_offset; channel_for_noteoff < upper_channel_for_voices; channel_for_noteoff++)  
            {
                // --- Low note prio => Find the highest note to "steal" -> Unused Voices are set to "high value" (including global on channel 1 which does not play notes) ---                
                if( last_midi_note_pitch_abcd[channel_for_noteoff] == msg[1] )        // Voice X associated with current noteoff?
                {
                    channel = channel_for_noteoff;      // We found a noteoff on Channel A,B OR A-D respectively
                    note_off_for_playing_note = true;   // The notoff we have actually stops a sounding note
                    last_midi_note_pitch_abcd[channel] = MIDI_INVALD_NOTE;  // Not playing anymore on current voice
                    break;   
                }
            }
            if(!note_off_for_playing_note)    // If loop as above did not register a valid MIDI-Channel => No valid noteoff found! (A key was pressed and released, not resulting in sound at all due to the limited voices)
                return;         // Ignore current noteoff, because it's not associated to a playing note and don't change triggers for audio-thread!
            latest_noteoff_channel = channel; // We may need that to prioritize reusing channels in round-robin voicemodes
            // === If we reach here, we found a valid noteoff for a Duophonic or Polyphonic voice previously playing, so we may repurpose another currently pressed yet inactive key ===
            // --- Legato? -> Check for possible inactive notes that may become active now after a playing not was released ---
            bool ignore_playing_note = false;                
            for( int legato_note_pitch=0; legato_note_pitch<sizeof(midi_note_pressed[voice_mode]); legato_note_pitch++)
            {
                if(midi_note_pressed[voice_mode][legato_note_pitch])              // Entry for Legato found?
                {
                    // --- Check if note in list to substitue previously playing note via legato is not already playing anyhow ---
                    ignore_playing_note = false;    // We assume that the note we may find is not already part of a sounding voice...
                    for(int available_channel=MIDI_VOICE_A+duo_offset; available_channel < upper_channel_for_voices; available_channel++) 
                    {
                        if( legato_note_pitch == last_midi_note_pitch_abcd[available_channel] )  // Note playing already => ignore
                        {
                            ignore_playing_note = true;     // We did not find a key pressed to be used as pitch via legato on any of the voices
                            break;                          // => Continue with outer loop to give finding a note for that another try...
                        }                                   // ...(until there are no more keys pressed to look at anymore)
                    }
                    if(!ignore_playing_note)                // We found a note in the list that is not identical with a already playing for any voice
                    {
                        // --- If we reach here, we found a note already pressed but not playing that can substitue the released, before playing note ---
                        msg[1] = legato_note_pitch;       // Set note for legato pitch-change     
                        msg[2] = midi_note_pressed[voice_mode][legato_note_pitch];    // Set velocity for legato velocity-change
               
                        // --- The channel associated to Duophonic or 4-voice Polyphonic has been set above already (MIDI-Channels 2+3 or 2-5) ---
                        last_midi_note_pitch_abcd[channel] = legato_note_pitch; // Reset Pitch in list of playing notes as well!
                        
                        cv_entry = cv_distributor[channel][(uint8_t)note];  // Retrieve MIDI-channel (internally 1-4) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
                        midi_cvs[(uint8_t)cv_entry.element_cv_id] = min(1.f, (msg[1]-MIDI_NOTE_0V)*SCALEVALUE_MIDI_NOTE_TO_CV);    // Set CV at given offset as just retrieved, rescale according to CV from CCs 
                        
                        cv_entry = cv_distributor[channel][(uint8_t)velocity];  // Retrieve MIDI-channel (internally 1-4) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
                        midi_cvs[(uint8_t)cv_entry.element_cv_id] = msg[2] * SCALEVALUE_MIDI_CTRLS_TO_CV;      // Set CV at given offset as just retrieved, rescale according to CV from CCs 
                        return; // We exit here to _not_ reset the trigger for this channel, because we found a new note via legato!
                    }
                } // ! Please note: in case of no match found: no legato needed, we will handle the note on exit as usually
            }
        }
        else    // Channel is 2-5 natively (for instance for "MPE-Mode"), check if we have a note-off trigger
        {    
            channel = (channel-1)%4 +1;     // Voices A-D: We 'map down' MIDI-channels above 2-5 to channels, 2-5 accordingly! So via "voice-stealing", Channel 6 will be stored as 2 again and so on
            if( last_midi_note_pitch_abcd[channel] != msg[1] )   
                return;   // Nothing to do here anymore, else reset trigger for valid note found "on exit" below!
        }
    }
    // --- If we reach here, we found a valid note-off we have to set to Trigger-value off and memorize/delete from the list as well ---
    trig_entry = trig_distributor[channel][(uint8_t)t_note];         // Retrieve MIDI-channel (internally 0-15) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
    midi_triggers[(uint8_t)trig_entry.element_trig_id] = TRIG_OFF;          // Set Trigger to off
    last_midi_note_pitch_abcd[channel] = MIDI_INVALD_NOTE;           // Set pitch of previously played note to invalid, we especially make use of that for polypony
 }

// === Public main Funtions ===
// --- Process incoming Control-Change events, decide if could be mapped to a GUI element and pass on resulting CV-Data to audio-thread accordingly ---
void Midi::controlChange(uint8_t* msg)
{
    // === Retrieve CC-number and MIDI-channel (internally 0-15) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index ===
    channel = msg[0]&0x0F;   // Please note: channel 0 is global, 2-5 are voices, 6-16 are mapped to 2-5, (16 may be global for MPE or unused, so that we don't get any data here anyhow)
    cc_num = msg[1];        // Continuous Controller Number is at second byte of the MIDI-message

    // === Get type and ID for CV and/or Trigger with given CC and Channel, if any (we use different CCs on Master and Voice-channel) ===
    if( IS_UPPER_GLOBAL(channel) ) // Map additional global channels
        channel = MIDI_GLOBAL_CHANNEL;      // All CCs on MIDI Channels 15 or 16 work as global controllers, Channel 15 and 16 only use different voice-modes (Duophonic or 4-Voice Polyphonc)

    if( channel != MIDI_GLOBAL_CHANNEL ) // Voices A-D
    {
        channel = (channel-1)%4 +1;     // We 'map down' MIDI-channels above 2-5 to channels, 2-5 accordingly! So via "voice-stealing", Channel 6 will be stored as 2 again and so on  
        abcdCVid = ccToCVid_abcd(cc_num);  // Retrieve MIDI-channel (internally 1-4) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
        if(abcdCVid != abcd_cc_invalid) // Valid CC for CV found?
        {
            uint8_t ctrl_val = msg[2];
            cv_entry = cv_distributor[channel][(uint8_t)abcdCVid];  // Retrieve MIDI-channel (internally 1-4) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
            
            // --- Check if we have to reduce value 1 to 0 (for Bank and Subbank CCs) or if we rescale logarithmically ---
            if(ctrl_val==1 && cv_entry.special_option==opt_oneIsZero)
                ctrl_val = 0;
            midi_cvs[(uint8_t)cv_entry.element_cv_id] = ctrl_val * SCALEVALUE_MIDI_CTRLS_TO_CV;      // Set CV at given offset as just retrieved, rescale according to CV from CCs
        }
        abcdTRIGid = ccToTrigId_abcd(cc_num);
        if( abcdTRIGid != t_abcd_cc_invalid )   // Valid CC for Trigger found?
        {
            trig_entry = trig_distributor[channel][(uint8_t)abcdTRIGid];  // Retrieve MIDI-channel (internally 0-15) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
            midi_triggers[(uint8_t)trig_entry.element_trig_id] = (msg[2] < 64);  // Set Trigger according to value from Continuous controller (We get rid of the first "none"-entry to calculate the CV-index)Set Trig to 1 if off, 0 if on at given offset as just retrieved, rescale according to CV from CCs  
        }
    }
    else    // Global channel (internally 0)
    {
        globCVid = ccToCVid_glob(cc_num);  // Retrieve MIDI-channel (internally 1-4) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
        if( globCVid != glob_cc_invalid)    // Valid CC for CV found?
        {
            int8_t ctrl_val = msg[2];
            cv_entry = cv_distributor[channel][(uint8_t)globCVid];  // Retrieve MIDI-channel (internally 1-4) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
            
            // --- Check if we have to convert our input to bipolar values or if we rescale logarithmically ---
            if(cv_entry.special_option==opt_bipolar)
            {
                ctrl_val -= 64;     // Value 64 is our relative zero
                ctrl_val *= 2;      // We supply values from -128 to 128 now    
            }
            midi_cvs[(uint8_t)cv_entry.element_cv_id] = ctrl_val * SCALEVALUE_MIDI_CTRLS_TO_CV;      // Set CV at given offset as just retrieved, rescale according to CV from CCs
        }
        globTRIGid = ccToTrigId_glob(cc_num);
        if( globTRIGid != t_glob_cc_invalid )   // Valic CC for Trigger found?
        {
            trig_entry = trig_distributor[channel][(uint8_t)globTRIGid];  // Retrieve MIDI-channel (internally 0-15) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
            midi_triggers[(uint8_t)trig_entry.element_trig_id] = (msg[2] < 64);  // Set Trigger according to value from Continuous controller (We get rid of the first "none"-entry to calculate the CV-index)Set Trig to 1 if off, 0 if on at given offset as just retrieved, rescale according to CV from CCs  
        }
    }
}

// --- Process incoming Pressure events, decide if it could be mapped to a GUI element and pass on resulting CV-Data to audio-thread accordingly ---
void Midi::channelPressure(uint8_t* msg)
{
    // === Retrieve MIDI-channel (internally 0-15) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index ===
    channel = msg[0]&0x0F;   // Please note: channel 0 is global, 2-5 are voices, 6-16 are mapped to 2-5, (16 may be global for MPE or unused, so that we don't get any data here anyhow)
 
    // === Get type and ID for CV and/or Trigger with given CC and Channel, if any ===
    if( IS_UPPER_GLOBAL(channel) ) // Map additional global channels
        channel = MIDI_GLOBAL_CHANNEL;      // Aftertouch on MIDI Channels 15 or 16 work as global controllers, Channel 15 and 16 only use different voice-modes (Duophonic or 4-Voice Polyphonc)
    
    if(channel == MIDI_GLOBAL_CHANNEL)
    {
        cv_entry = cv_distributor[channel][(uint8_t)g_at];          // Retrieve MIDI-channel (internally 05) by masking the MIDI status-byte, We cast the associated enum in order to use it as a regular array-index
        trig_entry = trig_distributor[channel][(uint8_t)t_g_at];    // Retrieve MIDI-channel (internally 05) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
    }
    else            // Voices A-D
    {
        channel = (channel-1)%4 +1;         // We 'map down' MIDI-channels above 2-5 to channels, 2-5 accordingly! So via "voice-stealing", Channel 6 will be stored as 2 again and so on  
        cv_entry = cv_distributor[channel][(uint8_t)aftertouch];  // Retrieve MIDI-channel (internally 05) by masking the MIDI status-byte, We cast the associated enum in order to use it as a regular array-index
        trig_entry = trig_distributor[channel][(uint8_t)t_aftertouch];  // Retrieve MIDI-channel (internally 05) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
    }
    // === Get type and ID for CV and Trigger with given CC and Channel, if any (we use different CCs on Master and Voice-channel!) ===
    midi_cvs[(uint8_t)cv_entry.element_cv_id] = msg[1] * SCALEVALUE_MIDI_CTRLS_TO_CV;      // Set CV at given offset as just retrieved. (We get rid of the first "none"-entry to calculate the CV-index). Rescale according to 0.-1.f according 0-5V CV from CCs 
    midi_triggers[(uint8_t)trig_entry.element_trig_id] = (msg[1] < 64);  // Set Trigger according to value from Continuous controller (We get rid of the first "none"-entry to calculate the CV-index)Set Trig to 1 if off, 0 if on at given offset as just retrieved
}

// --- Process incoming ProgramChange events, decide if it could be mapped to a GUI element and pass on resulting CV-Data to audio-thread accordingly ---
void Midi::programChange(uint8_t* msg)
{
    // === Retrieve MIDI-channel (internally 0-15) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index ===
    channel = msg[0]&0x0F;   // Please note: channel 0 is global, 2-5 are voices, 6-16 are mapped to 2-5, (16 may be global for MPE or unused, so that we don't get any data here anyhow)
 
    // === Get type and ID for CV and/or Trigger with given CC and Channel, if any ===
    if( channel != MIDI_GLOBAL_CHANNEL )    // Voices A-D
        channel = (channel-1)%4 +1;         // We 'map down' MIDI-channels above 2-5 to channels, 2-5 accordingly! So via "voice-stealing", Channel 6 will be stored as 2 again and so on  
    else
        return;                             // We don't support ProgramChange on the global channel!

    // === Get type and ID for CV and Trigger with using the Channel, if any (we use different CCs on Master and Voice-channel!) ===
    cv_entry = cv_distributor[channel][(uint8_t)progchange];  // Retrieve MIDI-channel (internally 0-15) by masking the MIDI status-byte, We cast the associated enum in order to use it as a regular array-index
    midi_cvs[(uint8_t)cv_entry.element_cv_id] = msg[1] * SCALEVALUE_MIDI_CTRLS_TO_CV;      // Set CV at given offset as just retrieved. (We get rid of the first "none"-entry to calculate the CV-index). Rescale according to 0.-1.f according 0-5V CV from CCs 
    
    trig_entry = trig_distributor[channel][(uint8_t)t_progchange];  // Retrieve MIDI-channel (internally 0-15) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
    midi_triggers[((uint8_t)trig_entry.element_trig_id)] = (msg[1] < 64);  // Set Trigger according to value from Continuous controller (We get rid of the first "none"-entry to calculate the CV-index)Set Trig to 1 if off, 0 if on at given offset as just retrieved
}

// --- Process incoming PitchBend events, decide if it could be mapped to a GUI element and pass on resulting CV-Data to audio-thread accordingly ---
void Midi::pitchBend(uint8_t* msg)
{
int16_t pitchBend_val = 0;
int16_t pitchBend_val_raw = 0;

    // === Retrieve MIDI-channel (internally 0-15) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index ===
    channel = msg[0]&0x0F;   // Please note: channel 0 is global, 2-5 are voices, 6-16 are mapped to 2-5, (16 may be global for MPE or unused, so that we don't get any data here anyhow)
    pitchBend_val_raw = ((int16_t)msg[2]<<7 | (int16_t)msg[1]);    // Convert MSB and LSB to signed 14bit and adjust zero-position
    
    // === Get type and ID for CV using the Channel, if any ===
    if( IS_UPPER_GLOBAL(channel) ) // Map additional global channels
        channel = MIDI_GLOBAL_CHANNEL;

    if( channel == MIDI_GLOBAL_CHANNEL ) // Channel 1 (internally 0)
    {
        pitchBend_val = max(-8191, pitchBend_val_raw-8192);  
        midi_cvs[(uint8_t)G_PB] = pitchBend_val * SCALEVALUE_MIDI_PITCHBEND_TO_CV;      // Set CV at given offset as just retrieved. (We get rid of the first "none"-entry to calculate the CV-index). Rescale according to 0.-1.f according 0-5V CV from CCs 
        pitchBend_val = lut_logarithmic_10bit[pitchBend_val_raw>>4];    // Use 10bit of 14 bit value for logarithmic scaling
        midi_cvs[(uint8_t)G_PB_LG] = pitchBend_val * SCALEVALUE_MIDI_PITCHBEND_TO_CV_10BIT;      // Set CV at given offset as just retrieved. (We get rid of the first "none"-entry to calculate the CV-index). Rescale according to 0.-1.f according 0-5V CV from CCs
    } 
    else    // Voices A-D - Smallest pitch bend is 0, largest is 16383, 8192 means no Pitchbend, msg[1] is LSB (zero if unused), msg[2] is MSB ---
    {
        channel = (channel-1)%4 +1;     // We 'map down' MIDI-channels above 2-5 to channels, 2-5 accordingly! So via "voice-stealing", Channel 6 will be stored as 2 again and so on  
        
        cv_entry = cv_distributor[channel][(uint8_t)pitchbend];         // Retrieve MIDI-channel (internally 1-4) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
        pitchBend_val = max(-8191,pitchBend_val_raw-8192);  
        midi_cvs[(uint8_t)cv_entry.element_cv_id] = pitchBend_val * SCALEVALUE_MIDI_PITCHBEND_TO_CV;      // Set CV at given offset as just retrieved. (We get rid of the first "none"-entry to calculate the CV-index). Rescale according to 0.-1.f according 0-5V CV from CCs 

        cv_entry = cv_distributor[channel][(uint8_t)pitchbend_log];     // Retrieve MIDI-channel (internally 1-4) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
        pitchBend_val = lut_logarithmic_10bit[pitchBend_val_raw>>4];    // Use 10bit of 14 bit value for logarithmic scaling
        midi_cvs[(uint8_t)cv_entry.element_cv_id] = pitchBend_val * SCALEVALUE_MIDI_PITCHBEND_TO_CV_10BIT;      // Set CV at given offset as just retrieved. (We get rid of the first "none"-entry to calculate the CV-index). Rescale according to 0.-1.f according 0-5V CV from CCs
    }
}
    
// --- Process incoming NoteOn (including Velocity) events, decide if it could be mapped to a GUI element and pass on resulting CV-Data to audio-thread accordingly ---
void Midi::noteOn(uint8_t* msg)
{
    // --- NoteOff via Velocity 0 for NoteOn? => Check if we have to silence the currently playing note ---
    if( msg[2]==0 )
    {
        handleNoteOff(msg); // Processing is dependant on channel, because we have different voicemodes for channels, 1, 15 and 16
        return; // We dont't process velocity now anymore, because this would be note-off-velocity which in this case is 0 anyhow.
    }
    // --- Retrieve MIDI-channel (internally 0-15) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index ---
    channel = msg[0]&0x0F;   // Please note: channel 0 is global, 2-5 are voices, 6-16 are mapped to 2-5, (16 may be global for MPE or unused, so that we don't get any data here anyhow)
    legato = false;                     // Non-Legato play is default "unless proven otherwise"
    new_pitch_and_velo_for_note = true; // The default is to have a new pitch and velocity, but this still could not be the case for low-priority notes in Monophonic play mode

    // --- On MIDI Channel 1: Handle Voice-Mode monophonic! ---
    if( channel == MIDI_GLOBAL_CHANNEL )
    {
        channel = MIDI_VOICE_A;     // We map monophonic play to Voice A
        // --- Mono-mode with Legato and LowKey priority ---
        if( monophonic_keys_down > 0 ) // Other key[s] pressed, so new trigger (meaning != Legato), check noteOff() for dependency!
        {
            // --- Check for key with low-note priority (legato or not) ---
            monophonic_lowest_key = 127;     // Initialize lowest key to highest possible, so we can look to find a lower one
            for( int i=0; i<sizeof(midi_note_pressed[vmode_mono]); i++)
            {
                if(midi_note_pressed[vmode_mono][i])
                {
                    monophonic_lowest_key = i;
                    break;
                }
            }        
            if( msg[1] >= monophonic_lowest_key )       // No new "alone key" and no new "lowkey" (low-key priority) => change pitch...
                new_pitch_and_velo_for_note = false;    // We found a monophonic note to keep track of, but this will not sound now
            else
                legato = true;                          // New "lowkey", but other key[s] are still pressed, meaning legato play
        }
        midi_note_pressed[vmode_mono][msg[1]] = msg[2];   // Remember velocity of currently pressed note (in theory this also can be zero, meaning note-off)
        monophonic_keys_down++;                          // Until noteoff we have [yet another] key pressed => keep track of that
    }
    else // Not Channel 1 for Monophonic playmode...
    {
        // --- On MIDI channels 13 to 16 wie have Duophonic or Polyphonic voicemode, to be assigned to Voice A+B or A-D respectively ---
        if(IS_UPPER_GLOBAL(channel)) // Scale MIDI-notes pressed to a pitch from -0.8 to +1.0 equivalent to -4V to +5V in "Eurorack-Land"
        {
            // --- Duophonic or 4-Voice with round-robin logic required, set additional local variables ---
            bool free_voice_found = false;      // This is a marker to be set to true if we find a appropriate slot for our note to be played duophonically or polyphonically
            uint8_t lowest_note = 127;          // Set lowest note to highest possible one, because we will lower it whenever we go deeper ;-)
            uint8_t lowest_note_channel = 0;    // We may need this lateron to determine the slot with the currently lowest note (which we plan to preserve!)

            
            // --- Duophonic A/B or C/D voicemode? ---
            if( IS_DUOPHONIC(channel) )         
            {
                // --- At first decide if we have to treat Duophonic-AB or Duophonic-CD mode ---
                uint8_t duo_offset = 0;             // This is a channel-offset to distinguish between Duo-AB or Duo-CD, default is AB
                if( IS_DUO_CD(channel) )            // Duophonic for Voices C and D, two channels, because we use slots C and D instead of A and B
                {
                    midi_note_pressed[vmode_duo_cd][msg[1]] = msg[2]; // We have a note pressed, remember it, along with its velocity! If this note will be not be sounding anymore later, in case a new note is pressed we still remember it for note-off and legato handling lateron! 
                    duo_offset = 2;
                }
                else                
                    midi_note_pressed[vmode_duo_ab][msg[1]] = msg[2]; // We have a note pressed, remember it, along with its velocity! If this note will be not be sounding anymore later, in case a new note is pressed we still remember it for note-off and legato handling lateron! 
          
                // --- Look if (via previous round-robin) we have are already positioned on an unused slot that has not been used directly before ---
                channel = round_robin_chan_duo+1+duo_offset;        // Our counter goes 0 to 1, we need MIDI-channels 2-3 (internally 1 and 2) to be set 
                if( last_midi_note_pitch_abcd[channel] == MIDI_INVALD_NOTE )   // Is our next entry for roundrobin free / not playing?
                {
                    if( channel != latest_noteoff_channel )   
                    {
                        free_voice_found = true;
                        round_robin_chan_duo = !round_robin_chan_duo;  // Toggle from 1 to 0 or 0 to 1 for next round
                    }   
                }
                // --- Only carry on, if we didn't by chance already find a free voice that was not released directly before ---
                if(!free_voice_found)   // Ok, we have to look deeper into finding an appropriate slot now!   
                {
                    round_robin_chan_duo = !round_robin_chan_duo;  // Toggle from 1 to 0 or 0 to 1 to advance to other slot
                    channel = round_robin_chan_duo+1+duo_offset;   // Set channel to next slot, if slot is free, we reached our new channel already here now!

                    if( last_midi_note_pitch_abcd[channel] == MIDI_INVALD_NOTE )   // Is our next entry for roundrobin free / not playing?
                        free_voice_found = true;
                    else    // We go back to the original channel and decide how/what to overwrite now!
                    {
                        round_robin_chan_duo = !round_robin_chan_duo;  // Toggle from 1 to 0 or 0 to 1 to advance to other slot
                        channel = round_robin_chan_duo+1+duo_offset; 
                        if(last_midi_note_pitch_abcd[channel] == MIDI_INVALD_NOTE)
                            free_voice_found = true;
                    }    
                    // --- Check if we now reached an empty slot (in case we decided to advance one directly before) ---
                    if( !free_voice_found )                 // Slot was not free, decide which note to overwrite now
                    {
                        // --- No empty slot available, overwrite the voice that is not the lowest note unless new note is lower anyways ---
                        if(last_midi_note_pitch_abcd[MIDI_VOICE_A+duo_offset] < last_midi_note_pitch_abcd[MIDI_VOICE_B+duo_offset] ) // Look which of our two slots contains the lowest note currently
                        {
                            if( msg[1] < last_midi_note_pitch_abcd[MIDI_VOICE_A+duo_offset] ) // Check if current note is lower than note on first sloe
                                channel = MIDI_VOICE_A+duo_offset;    // We keep the first slot, because our current note would overwrite the lowest note otherwise
                            else
                                channel = MIDI_VOICE_B+duo_offset;
                        }
                        else    // Note on second slot currently is the lowest one
                        {
                            if( msg[1] < last_midi_note_pitch_abcd[MIDI_VOICE_B+duo_offset] ) // Check if current note is lower than note on second slot
                                channel = MIDI_VOICE_B+duo_offset;    // We keep the second slot, because our current note would overwrite the lowest note otherwise
                            else
                                channel = MIDI_VOICE_A+duo_offset;
                        }    
                    }
                    round_robin_chan_duo = !round_robin_chan_duo;  // Toggle from 1 to 0 or 0 to 1 for next round
                }
            }
            else    // 4-voice round-robin
            {
                // --- We have a note pressed, remember it, along with its velocity! ---
                midi_note_pressed[vmode_4voice][msg[1]] = msg[2];  // If this note will be not be sounding anymore later, in case a new note is pressed we still remember it for note-off and legato handling lateron!
                
                // --- Look if (via previous round-robin) we have are already positioned on an unused slot that has not been used directly before ---
                if( last_midi_note_pitch_abcd[round_robin_chan_4voice+1] == MIDI_INVALD_NOTE && ((round_robin_chan_4voice+1) != latest_noteoff_channel))   // Is our next entry for roundrobin free / not playing?
                {
                    channel = round_robin_chan_4voice+1;    // Our counter goes 0 to 3, we need MIDI-channels 2-5 (internally 1-4) to be set
                    round_robin_chan_4voice = (round_robin_chan_4voice+1)%4; // Increment and allow values 0 to 3 for next round
                }
                else // Current slot not perfect => Find next free entry
                {
                    for(int i=MIDI_VOICE_A; i<MIDI_VOICE_A+4; i++)  // Look for all 4 voices which one might be free 
                    {
                        if( last_midi_note_pitch_abcd[i] == MIDI_INVALD_NOTE )
                        {
                            channel = i;
                            if(i == latest_noteoff_channel)    // We want to let last released note sound if possible!
                                continue;
                            free_voice_found = true;
                            break;
                        }
                    } 
                    if(!free_voice_found) // Look again for free slot without taking care of previously released note!
                    {
                        for(int i=MIDI_VOICE_A; i<MIDI_VOICE_A+4; i++)  // Look for all 4 voices which one might be free 
                        {
                            if( last_midi_note_pitch_abcd[i] == MIDI_INVALD_NOTE )
                            {
                                channel = i;
                                free_voice_found = true;
                                break;
                            }
                        } 
                        // --- If we reach here, we have to repurpose a slot with a currently active note ---
                        if( !free_voice_found )     
                        {
                            // --- Per default look for next slot, but make sure to not substitute the lowest note, unless our note to be played is even lower ---
                            channel = round_robin_chan_4voice+1;    
                            
                            // --- Find currently lowest note ---
                            for(int i=MIDI_VOICE_A; i<MIDI_VOICE_A+4; i++)  // Look for all 4 voices which one might be free 
                            {
                                if( last_midi_note_pitch_abcd[i] < lowest_note)
                                {
                                    lowest_note = last_midi_note_pitch_abcd[i];     // Remember lowest note for next round
                                    lowest_note_channel = i;    // Remember voice-channel the lowest note is on for comparison of currently to be set pitch
                                }    
                            }    
                            // --- If our current note will not steal the lowest note or is lower itself anyhow we are lucky, otherwhise we take next slot via roundrobin! ---
                            if((channel == lowest_note_channel) && (msg[1] > lowest_note))  // Check channel with current note would be repurposed but not we new lowest note
                            {
                                round_robin_chan_4voice = (round_robin_chan_4voice+1)%4;    // Additional roundrobin to reach next slot!
                                channel = round_robin_chan_4voice+1;                        // Set next channel to be used for current note
                            }
                            round_robin_chan_4voice = (round_robin_chan_4voice+1)%4; // "Normal roundrobin": Increment and allow values 0 to 3 for next round
                        } 
                    }
                }       
            }
        }
        else  // Not Channel 1, 14, 15 or 16 (Monophonic, Duophonic A/B, Duophonic C/D or 4 Voice Polyphonic Play modes)
            channel = (channel-1)%4 +1;     // Voices A-D: We 'map down' MIDI-channels above 2-5 to channels, 2-5 accordingly! So via "voice-stealing", Channel 6 will be stored as 2 again and so on
    }
    // --- If we reach here we either have a Voice A-D natively, reduced for 4 voices via MPE or mapped for mono, duophonic/polyphony --- 
    if(!legato)    // With legato-play we change the pitch but do not set a new trigger for the EG etc. This only is relevant for Mono voicemode, here!
    {
        trig_entry = trig_distributor[channel][(uint8_t)t_note];  // Retrieve MIDI-channel (internally 0-15) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
        midi_triggers[((uint8_t)trig_entry.element_trig_id)] = 0;  // Set Trigger according to value from Continuous controller (We get rid of the first "none"-entry to calculate the CV-index)Set Trig to 1 if off, 0 if on at given offset as just retrieved, rescale according to CV from CCs  
    }
    if(new_pitch_and_velo_for_note)     // This may have been inactivated via Mono-Mode
    {
        cv_entry = cv_distributor[channel][(uint8_t)note];  // Retrieve MIDI-channel (internally 1-4) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
        midi_cvs[(uint8_t)cv_entry.element_cv_id] = min(1.f, (msg[1]-MIDI_NOTE_0V)*SCALEVALUE_MIDI_NOTE_TO_CV);    // Set CV at given offset as just retrieved, rescale according to CV from CCs 
        last_midi_note_pitch_abcd[channel] = msg[1];    // Remember Note-value we found for trigger, so that we can decide on potential noteOff for this pitch lateron

        // --- Check for CV from Velocity and on or off trigger via NoteOn-Velocity (if value less or more than 64 == 'half velocity'), seperately via Velocity ---
        cv_entry = cv_distributor[channel][(uint8_t)velocity];  // Retrieve MIDI-channel (internally 1-4) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
        midi_cvs[(uint8_t)cv_entry.element_cv_id] = msg[2] * SCALEVALUE_MIDI_CTRLS_TO_CV;      // Set CV at given offset as just retrieved, rescale according to CV from CCs 

        // --- Check for Trigger via Velocity ---
        trig_entry = trig_distributor[channel][(uint8_t)t_velocity];  // Retrieve MIDI-channel (internally 0-15) by masking the MIDI status-byte, We cast the enum retrieved in order to use it as a regular array-index
        midi_triggers[(uint8_t)trig_entry.element_trig_id] = (msg[2] < 64);  // Set Trigger according to value from Continuous controller (We get rid of the first "none"-entry to calculate the CV-index)Set Trig to 1 if off, 0 if on at given offset as just retrieved, rescale according to CV from CCs  
    }
}

// --- Process incoming NoteOff (including Velocity) events, decide if it could be mapped to a GUI element and pass on resulting CV-Data to audio-thread accordingly ---
void Midi::noteOff(uint8_t* msg)
{
    // --- Check if we found a noteOff of the currently playing note? (We use a common sub-function, because noteoffs also can happen via noteoffs with velocity 9!) ---
    handleNoteOff(msg); // Processing is dependant on channel, because we have different voicemodes for channels, 1, 15 and 16
    return; // We don't process note-off velocity!
}


