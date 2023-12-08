/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.
(c) 2023 MIDI-Voicemodes and Eventhandling aka 'class Midi' by Mathias Brüssel.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0)\", available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#pragma once

#include "Control.hpp"
#include "Calibration.hpp"
#include "gpio.hpp"
#include "adc.hpp"
#include "mk2.hpp"

#include "midiuart.hpp"
#define byte uint8_t
#include "midiXparser/midiXparser.h"


// --- Define-contants for MIDI-mapping-related stuff --- 
#define MIDI_NOTE_0V                48  // is C3
#define MAX_ACTIVE_CHANNELS          5  // Size of Dimension 1: MIDI-Channels 1-5 (internally 0-4)
#define MAX_ACTIVE_CTRLVALS         18  // Size of CV-Dimension 2: One Empty Element + Number of CV-mapable Notes, Velocities, Aftertouch, Banks, Subbanks, Progchanges and CCs
#define MAX_ACTIVE_TRIGVALS          8  // Size of Trig-Dimension 2: One Empty Element + Number of Trigger-mapable Notes, Velocities, Aftertouch, ProgChange, and CCs 
#define MIDI_GLOBAL_CHANNEL          0  // MIDI Channel 1 is Master and Global channel (internally 0)
#define MIDI_VOICE_A                 1  // MIDI Channel 2 is used for Voice A, especially concerning monophonic voice-mode 
#define MIDI_SPECIAL_CHANNEL_14     13  // MIDI Channel for "global" logarithmic Pitchbend 
#define MIDI_GLOBAL_CHANNEL_15      14  // MIDI Channel for Duophonic playmode, additional controls will be remapped to global channel
#define MIDI_GLOBAL_CHANNEL_16      15  // MIDI Channel for Polyphonic playmode, additional controls will be remapped to global channelMIDI Channel 16 is secondary MPE-Master / provides logarithmic pitchbend (internally 15)
#define MIDI_INVALD_NOTE           255  // Unexpected Value for remembering currently sounding values
#define TRIG_ON                      0  // Sets Trigger to On
#define TRIG_OFF                     1  // Sets Trigger to Off
#define SCALEVALUE_MIDI_CTRLS_TO_CV             0.007874015748031f      // 1.f / 127  (Range of 7 Bit for Controllers etc)
#define SCALEVALUE_MIDI_NOTE_TO_CV              0.016666666666667f      // 1.f / 60   (5 Octaves of MIDI-Notes, -5V to +5V in "Eurorack-Land")
#define SCALEVALUE_MIDI_PITCHBEND_TO_CV         0.00012208521548f       // 1.f / 8191 (Signed 14Bit values for Pitchbend)
#define SCALEVALUE_MIDI_PITCHBEND_TO_CV_10BIT   0.000977517106549f      // 1.f / 1023 (Unsigned 10Bit values for logarithmically scaled Pitchbend)

// === class Midi provides different voice-modes and mappable events, depending on the MIDI-channel notes, and control-events are send from ===
// Input on MIDI channel 1 will be handled like a monosynth with low-voice priority and legato-option
// Input on MIDI channel 15 will be handled like a duophonic synth with low-voice priority and legato-option
// Input on MIDI channel 16 will be handled like a 4-voice polyphonic synth with low-voice priority and legato-option
// Channels 2-5 (and/or also 6-9 and 10-13 respecively) will be used as individual channels, for intance for MPE-like usage
// On the GUI this is represented in the dropdown-list of mappable MIDI-events as "global voices": 'G' or "individual voices": 'A','B','C','D'
// MIDI-notes on the global channels 1, 15 and 16 will be converted to be mappable as Voices A, B+C or B-D, depending on the 3 voicemodes
namespace CTAG::CTRL
{
    class Midi final
    {
    public:
        enum midiStatusValues 
        {
            // --- Channel Voice Messages ---
            noteOffStatus         = 0X80,
            noteOnStatus          = 0X90,
            polyKeyPressureStatus = 0XA0,
            controlChangeStatus   = 0XB0,
            programChangeStatus   = 0XC0,
            channelPressureStatus = 0XD0,
            pitchBendStatus       = 0XE0
        };

        // === Initialisation: link CV/Trigger data-pointers initially by bba_init() in Control.cpp ===
        inline void setCVandTriggerPointers( float* midi_data, uint8_t* midi_note_trig ) 
        { 
            midi_cvs = midi_data; 
            midi_triggers = midi_note_trig; 
        }
        // === Public main Funtions ===
        // --- Process all midi-events and if needed map to CVs or Triggers for audio-thread ---
        void controlChange(uint8_t* msg);       // For MIDI-events that are assignable via GUI process Conctrol Change messages and write to transfer-queue
        void channelPressure(uint8_t* msg);     // For MIDI-events that are assignable via GUI process Pressure messages and write to transfer-queue
        void programChange(uint8_t* msg);       // For MIDI-events that are assignable via GUI process ProgramChange messages and write to transfer-queue
        void pitchBend(uint8_t* msg);           // For MIDI-events that are assignable via GUI process PitchBend messages and write to transfer-queue
        void noteOn(uint8_t* msg);              // For MIDI-events that are assignable via GUI process Note messages (CV and/or Trigger on/off) and write to transfer-queue
        void noteOff(uint8_t* msg);             // For MIDI-events that are assignable via GUI process Note messages (CV and/or Trigger off) and write to transfer-queue

    private:
        // === I/O data to be exchanged with audio-thread, pointers will be linked initially by bba_init() in Control.cpp Midi::viasetCVandTriggerPointers() ===
        float* midi_cvs;              // Normally only to be set once         
        uint8_t* midi_triggers;       // Normally only to be set once

        constexpr static uint16_t lut_logarithmic_10bit[1024] = 
        {
            0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 
            3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
            4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 
            5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
            6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 
            9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11,
            11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 
            14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 
            17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 21, 21, 
            21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 26, 
            26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 31, 31, 31, 31, 31, 
            32, 32, 32, 32, 32, 33, 33, 33, 33, 34, 34, 34, 34, 35, 35, 35, 35, 36, 36, 36, 36, 36, 37, 37, 37, 38, 38, 38, 38, 39, 
            39, 39, 39, 40, 40, 40, 40, 41, 41, 41, 42, 42, 42, 42, 43, 43, 43, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46, 47, 47, 47, 
            48, 48, 48, 49, 49, 49, 50, 50, 50, 51, 51, 51, 52, 52, 52, 53, 53, 54, 54, 54, 55, 55, 55, 56, 56, 57, 57, 57, 58, 58, 
            59, 59, 59, 60, 60, 61, 61, 61, 62, 62, 63, 63, 64, 64, 64, 65, 65, 66, 66, 67, 67, 68, 68, 69, 69, 69, 70, 70, 71, 71, 
            72, 72, 73, 73, 74, 74, 75, 75, 76, 76, 77, 77, 78, 79, 79, 80, 80, 81, 81, 82, 82, 83, 83, 84, 85, 85, 86, 86, 87, 88, 
            88, 89, 89, 90, 91, 91, 92, 92, 93, 94, 94, 95, 96, 96, 97, 98, 98, 99, 100, 100, 101, 102, 102, 103, 104, 105, 105, 106, 
            107, 107, 108, 109, 110, 110, 111, 112, 113, 113, 114, 115, 116, 117, 117, 118, 119, 120, 121, 121, 122, 123, 124, 125, 
            126, 126, 127, 128, 129, 130, 131, 132, 133, 134, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 
            148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 162, 163, 164, 165, 166, 167, 168, 169, 171, 172, 173, 
            174, 175, 176, 178, 179, 180, 181, 183, 184, 185, 186, 188, 189, 190, 191, 193, 194, 195, 197, 198, 199, 201, 202, 204, 
            205, 206, 208, 209, 211, 212, 213, 215, 216, 218, 219, 221, 222, 224, 225, 227, 228, 230, 232, 233, 235, 236, 238, 240, 
            241, 243, 244, 246, 248, 249, 251, 253, 255, 256, 258, 260, 262, 263, 265, 267, 269, 271, 272, 274, 276, 278, 280, 282, 
            284, 286, 288, 290, 292, 294, 296, 298, 300, 302, 304, 306, 308, 310, 312, 314, 316, 319, 321, 323, 325, 327, 330, 332, 
            334, 336, 339, 341, 343, 346, 348, 350, 353, 355, 357, 360, 362, 365, 367, 370, 372, 375, 377, 380, 383, 385, 388, 390, 
            393, 396, 398, 401, 404, 407, 409, 412, 415, 418, 421, 424, 426, 429, 432, 435, 438, 441, 444, 447, 450, 453, 456, 459, 
            463, 466, 469, 472, 475, 479, 482, 485, 488, 492, 495, 498, 502, 505, 509, 512, 516, 519, 523, 526, 530, 533, 537, 541, 
            544, 548, 552, 556, 559, 563, 567, 571, 575, 579, 583, 586, 590, 594, 599, 603, 607, 611, 615, 619, 623, 628, 632, 636, 
            641, 645, 649, 654, 658, 663, 667, 672, 676, 681, 685, 690, 695, 700, 704, 709, 714, 719, 724, 729, 734, 739, 744, 749, 
            754, 759, 764, 769, 774, 780, 785, 790, 796, 801, 807, 812, 818, 823, 829, 834, 840, 846, 851, 857, 863, 869, 875, 881, 
            887, 893, 899, 905, 911, 917, 924, 930, 936, 943, 949, 955, 962, 969, 975, 982, 988, 995, 1002, 1009, 1016, 1023 
        };
        // === We define a data-structure that tells us how to push data to possibly via the UI assigned data-elements for CV and/or Triggers ===
        
        // --- Can be used as index to an individual CV Control-elements, as possible "CVs" via WebGUI ---
        enum struct Control_element_cv_id 
        {   
            A_NOTE, A_VELO, A_BANK, A_SBNK, A_PROG, A_PB, A_PB_LG, A_AT, A_MW_01, A_BC_02, 
            B_NOTE, B_VELO, B_BANK, B_SBNK, B_PROG, B_PB, B_PB_LG, B_AT, B_MW_01, B_BC_02, 
            C_NOTE, C_VELO, C_BANK, C_SBNK, C_PROG, C_PB, C_PB_LG, C_AT, C_MW_01, C_BC_02, 
            D_NOTE, D_VELO, D_BANK, D_SBNK, D_PROG, D_PB, D_PB_LG, D_AT, D_MW_01, D_BC_02, 
            A_RES_71, A_REL_72, A_ATK_73, A_CUT_74, A_SC6_75, A_SC7_76, A_SC8_77, A_SC9_78, 
            B_RES_71, B_REL_72, B_ATK_73, B_CUT_74, B_SC6_75, B_SC7_76, B_SC8_77, B_SC9_78, 
            C_RES_71, C_REL_72, C_ATK_73, C_CUT_74, C_SC6_75, C_SC7_76, C_SC8_77, C_SC9_78, 
            D_RES_71, D_REL_72, D_ATK_73, D_CUT_74, D_SC6_75, D_SC7_76, D_SC8_77, D_SC9_78, 
            G_PB, G_14_PB_LG, G_AT, G_MW_01, G_BC_02, G_FOOT_04, G_DATA_06, G_VOL_07, G_BAL_08, G_PAN_10, 
            G_EXPR_11, G_FX1_12, G_FX2_13, G_SUSTP_64, G_PORTP_65, G_SOSTP_66, G_SOFTP_67, G_HOLDP_69
        };

        // --- Can be used as index to an individual Trigger Control-elements,  as possible "Triggers" or "Gates" via WebGUI ---  
        enum struct Control_element_trig_id 
        {   
            T_A_NOTE, T_A_VELO, T_A_PROG, T_A_AT, 
            T_B_NOTE, T_B_VELO, T_B_PROG, T_B_AT, 
            T_C_NOTE, T_C_VELO, T_C_PROG, T_C_AT, 
            T_D_NOTE, T_D_VELO, T_D_PROG, T_D_AT,  
            T_A_SC6_75, T_A_SC7_76, T_A_SC8_77, T_A_SC9_78, 
            T_B_SC6_75, T_B_SC7_76, T_B_SC8_77, T_B_SC9_78, 
            T_C_SC6_75, T_C_SC7_76, T_C_SC8_77, T_C_SC9_78, 
            T_D_SC6_75, T_D_SC7_76, T_D_SC8_77, T_D_SC9_78, 
            T_AT, T_G_FX1_12, T_G_FX2_13, T_G_SUSTP_64, T_G_PORTP_65, T_G_SSTNP_66, T_G_SOFTP_67, T_G_HOLDP_69
        };  

        // --- Enum to be used as index for CVs to an individual Control-element, visable as possible "CVs" via WebGUI --- 
        enum struct CV_id_abcd  // Channel 2-5: Voice A-D
        {   
            note, velocity, progchange, pitchbend, pitchbend_log, aftertouch,       // MIDI status events (pitchbend_log is for rescaling pitchbend logarithmically, pitchbend will be stored twice for that usecase)
            bank_00, sbnk_32, mw_01, bc_02, res_71, rel_72, atk_73, cut_74, sc6_75, sc7_76, sc8_77, sc9_78, abcd_cc_invalid  // MIDI CCs (status: continuous controller)    
        }; 
        enum struct CV_id_glob  // Channel 1: Global 
        {   
            g_pitchbend, g_16_pb_log, g_at,                                           // MIDI status events (g_16_pb_log is for rescaling pitchbend (from channel 16) logarithmically)
            g_mw_01, g_bc_02, g_foot_04, g_data_06, g_vol_07, g_bal_08, g_pan_10, // MIDI CCs (status: continuous controller)
            g_expr_11, g_fx1_12, g_fx2_13, g_sustp_64, g_portp_65, g_sstnp_66, g_softp_67, g_holdp_69, glob_cc_invalid // MIDI CCs (status: continuous controller)
        }; 
        enum SpecialOptions  // Special Options how a specific control should be handled
        {
            opt_none, opt_oneIsZero, opt_logarithmic, opt_bipolar, opt_invalid
        };       

        // --- Enum to be used as index Triggers to an individual Control-element, visable as possible "Triggers" via WebGUI --- 
        enum struct TRIG_id_abcd // Channel 2-5: Voice A-D     
        {   
            t_note, t_velocity, t_progchange, t_aftertouch, t_sc6_75, t_sc7_76, t_sc8_77, t_sc9_79, t_abcd_cc_invalid          // MIDI status events incl. MIDI CCs (status: continious controller) 
        };
        enum struct TRIG_id_glob  // Channel 1: Global 
        {   
            t_g_at, t_g_fx1_12, t_g_fx2_13, t_g_sustp_64, t_g_portp_65, t_g_sostp_66, t_g_softp_67, t_g_holdp_69, t_glob_cc_invalid   // MIDI CCs (status: continious controller) 
        }; 

        // --- Structure to be retrieved for cvs, based on MIDI-Event-type (Status), channel and possibly CC-number ---
        typedef struct 
        {
            // --- ID as associated to the control that can be mapped via an UI list for CVs or Triggers ---
            Control_element_cv_id element_cv_id; // Will be 0 (enum:none) if not available or not set for this control!
            uint8_t special_option;          // Map 7bit value directly or via logarithmic curve, adjust 1 to 0 for bank or process bipolarilly?    
        } ControlCVtags;

        // --- Structure to be retrieved for triggers, based on MIDI-Event-type (Status), channel and possibly CC-number ---
        typedef struct 
        {
            // --- ID as associated to the control that can be mapped via an UI list for CVs or Triggers ---
            Control_element_trig_id element_trig_id;        // Will be 0 (enum:none) if not available or not set for this control!   
        } ControlTrigTags;

        // --- We keep using stronly typed enums, but still want to be able to use the results without getting too verbose (getting rid of type-specifiers) ---
        using enum Control_element_cv_id;
        using enum Control_element_trig_id;
        using enum CV_id_abcd;
        using enum CV_id_glob;
        using enum TRIG_id_abcd;
        using enum TRIG_id_glob;

        // --- Size and initialize the CV-datastructure that does the mapping of data from midi-events to CVs according to MIDI-channels and eventtypes and CC-numbers... 
        constexpr static ControlCVtags cv_distributor[MAX_ACTIVE_CHANNELS][MAX_ACTIVE_CTRLVALS] = //  'constexpr' indicates that the value, is constant and, where possible, is computed at compile time
        {    
            // MIDI Channel 1
            {
                { G_PB, opt_bipolar },              // g_pb,  
                { G_14_PB_LG, opt_logarithmic },    // g_16_pb_log,
                { G_AT, opt_none },             // g_bc_at, 
                { G_MW_01, opt_none },         // g_mw_01, 
                { G_BC_02, opt_none },         // g_bc_02,
                { G_FOOT_04, opt_none },       // g_foot_04, 
                { G_DATA_06, opt_none },       // g_data_06, 
                { G_VOL_07, opt_none },        // g_vol_07, 
                { G_BAL_08, opt_bipolar },          // g_bal_08, 
                { G_PAN_10, opt_bipolar },          // g_pan_10, 
                { G_EXPR_11, opt_none },       // g_expr_11, 
                { G_FX1_12, opt_none },        // g_fx1_12, 
                { G_FX2_13, opt_none },        // g_fx2_13,      
                { G_SUSTP_64, opt_none },      // g_sustp_64,         
                { G_PORTP_65, opt_none },      // g_portp_65, 
                { G_SOSTP_66, opt_none },      // g_sstnp_66, 
                { G_SOFTP_67, opt_none },      // g_softp_67,
                { G_HOLDP_69, opt_none }       // g_holdp_69,
            }, 
            // MIDI Channel 2
            {
                { A_NOTE, opt_none },      // note, 
                { A_VELO, opt_none },      // velo, 
                { A_PROG, opt_none },      // prog, 
                { A_PB, opt_bipolar },          // pitchbend, 
                { A_PB_LG, opt_logarithmic },   // logarithmically scaled pitchbend, will be stored in addition to normal pitchbend 
                { A_AT, opt_none },        // aftertouch, 
                { A_BANK, opt_oneIsZero },      // bank_00, 
                { A_SBNK, opt_oneIsZero },      // sbnk_32,
                { A_MW_01, opt_none },     // mw_01, 
                { A_BC_02, opt_none },     // bc_02, 
                { A_RES_71, opt_none },    // res_71, 
                { A_REL_72, opt_none },    // rel_72, 
                { A_ATK_73, opt_none },    // atk_73, 
                { A_CUT_74, opt_none },    // cut_74, 
                { A_SC6_75, opt_none },    // sc6_75, 
                { A_SC7_76, opt_none },    // sc7_76, 
                { A_SC8_77, opt_none },    // sc8_77,
                { A_SC9_78, opt_none }     // sc9_78
            },
            // MIDI Channel 3
            {
                { B_NOTE, opt_none },      // note_pitch, 
                { B_VELO, opt_none },      // note_velocity, 
                { B_PROG, opt_none },      // progchng, 
                { B_PB, opt_bipolar },        // pitchbend, 
                { B_PB_LG, opt_logarithmic }, // logarithmically scaled pitchbend, will be stored in addition to normal pitchbend 
                { B_AT, opt_none },        // aftertouch, 
                { B_BANK, opt_oneIsZero },     // bank_00, 
                { B_SBNK, opt_oneIsZero },     // sbnk_32,
                { B_MW_01, opt_none },     // mw_01, 
                { B_BC_02, opt_none },     // bc_02, 
                { B_RES_71, opt_none },    // res_71, 
                { B_REL_72, opt_none },    // rel_72, 
                { B_ATK_73, opt_none },    // atk_73, 
                { B_CUT_74, opt_none },    // cut_74, 
                { B_SC6_75, opt_none },    // sc6_75, 
                { B_SC7_76, opt_none },    // sc7_76, 
                { B_SC8_77, opt_none },    // sc8_77,
                { B_SC9_78, opt_none }     // sc8_78
            },
            // MIDI Channel 4
            {
                { C_NOTE, opt_none },      // note_pitch, 
                { C_VELO, opt_none },      // note_velocity, 
                { C_PROG, opt_none },      // progchng, 
                { C_PB, opt_bipolar },          // pitchbend,
                { C_PB_LG, opt_logarithmic },   // logarithmically scaled pitchbend, will be stored in addition to normal pitchbend  
                { C_AT, opt_none },        // aftertouch, 
                { C_BANK, opt_oneIsZero },      // bank_00, 
                { C_SBNK, opt_oneIsZero },      // sbnk_32,
                { C_MW_01, opt_none },     // mw_01, 
                { C_BC_02, opt_none },     // bc_02, 
                { C_RES_71, opt_none },    // res_71, 
                { C_REL_72, opt_none },    // rel_72, 
                { C_ATK_73, opt_none },    // atk_73, 
                { C_CUT_74, opt_none },    // cut_74, 
                { C_SC6_75, opt_none },    // sc6_75, 
                { C_SC7_76, opt_none },    // sc7_76, 
                { C_SC8_77, opt_none },    // sc8_77,
                { C_SC9_78, opt_none }     // sc8_78
            },
            // MIDI Channel 5
            {
                { D_NOTE, opt_none },      // note_pitch, 
                { D_VELO, opt_none },      // note_velocity, 
                { D_PROG, opt_none },      // progchng, 
                { D_PB, opt_bipolar },          // pitchbend,
                { D_PB_LG, opt_logarithmic },   // logarithmically scaled pitchbend, will be stored in addition to normal pitchbend  
                { D_AT, opt_none },        // aftertouch, 
                { D_BANK, opt_oneIsZero },      // bank_00, 
                { D_SBNK, opt_oneIsZero },      // sbnk_32,
                { D_MW_01, opt_none },     // mw_01, 
                { D_BC_02, opt_none },     // bc_02, 
                { D_RES_71, opt_none },    // res_71, 
                { D_REL_72, opt_none },    // rel_72, 
                { D_ATK_73, opt_none },    // atk_73, 
                { D_CUT_74, opt_none },    // cut_74, 
                { D_SC6_75, opt_none },    // sc6_75, 
                { D_SC7_76, opt_none },    // sc7_76, 
                { D_SC8_77, opt_none },    // sc8_77,
                { D_SC9_78, opt_none }     // sc8_78
            }   
        };

        // --- Size and initialize the Trigger-datastructure that does the mapping of data from midi-events to CVs or Triggers according to MIDI-channels and eventtypes and CC-numbers... 
        constexpr static ControlTrigTags trig_distributor[MAX_ACTIVE_CHANNELS][MAX_ACTIVE_TRIGVALS] = //  'constexpr' indicates that the value, is constant and, where possible, is computed at compile time
        {    
            // MIDI Channel 1
            {
                T_AT,           // t_g_at
                T_G_FX1_12,     // t_g_fx1_12
                T_G_FX2_13,     // t_g_fx2_13
                T_G_SUSTP_64,   // t_g_sustp_64
                T_G_PORTP_65,   // t_g_portp_65
                T_G_SSTNP_66,   // t_g_sstnp_66
                T_G_SOFTP_67,   // t_g_softp_67
                T_G_HOLDP_69    // t_g_holdp_69
            },
            // MIDI Channel 2
            {
                T_A_NOTE,        // t_note, 
                T_A_VELO,        // t_velo,
                T_A_PROG,        // t_prog
                T_A_AT,          // t_at
                T_A_SC6_75,      // t_sc6_75,  
                T_A_SC7_76,      // t_sc7_76, 
                T_A_SC8_77,      // t_sc8_77,
                T_A_SC9_78       // t_sc8_78 
            },  
            // MIDI Channel 3
            {
                T_B_NOTE,        // t_note, 
                T_B_VELO,        // t_velo,
                T_B_PROG,        // t_prog
                T_B_AT,          // t_at
                T_B_SC6_75,      // t_sc6_75,  
                T_B_SC7_76,      // t_sc7_76, 
                T_B_SC8_77,      // t_sc8_77,
                T_B_SC9_78       // t_sc8_78 
            },
            // MIDI Channel 4   
            {
                T_C_NOTE,        // t_note,
                T_C_VELO,        // t_velo,
                T_C_PROG,        // t_prog
                T_C_AT,          // t_at 
                T_C_SC6_75,      // t_sc6_75,  
                T_C_SC7_76,      // t_sc7_76, 
                T_C_SC8_77,      // t_sc8_77,
                T_C_SC9_78       // t_sc8_78 
            },
            // MIDI Channel 5
            {
                T_D_NOTE,        // t_note, 
                T_D_VELO,        // t_velo,
                T_D_PROG,        // t_prog
                T_D_AT,          // t_at
                T_D_SC6_75,      // t_sc6_75,  
                T_D_SC7_76,      // t_sc7_76, 
                T_D_SC8_77,      // t_sc8_77,
                T_D_SC9_78       // t_sc8_78 
            }        
        };
        // --- Remember last note for potential Trigger-Reset of voices ---
        uint8_t last_midi_note_pitch_abcd[MAX_ACTIVE_CHANNELS] = { MIDI_INVALD_NOTE, MIDI_INVALD_NOTE, MIDI_INVALD_NOTE, MIDI_INVALD_NOTE, MIDI_INVALD_NOTE }; // Notes have values 0-127, invalid note is higher, though ;)

        // --- Helpervariables for monophonic voicemode (low key priorita and legato-option) ---
        bool legato = false;                // If true we may have a new pitch, but no new trigger for the EG and similar
        bool new_pitch_and_velo_for_note = true;    // NoteOn: normally we have a new pitch and velocity (and also trigger, no legato)
        uint8_t midi_note_pressed[128] = {};    // For any active Mono-, Duo- or Polyphonic note we will remember its velocity (0 means inactive)
        int monophonic_keys_down = 0;       // Used to determine if any monophonic notes are playing at all
        uint8_t monophonic_lowest_key = 0;  // We use low key prio, so we have to check if newly active note is lower

        // === Data used for mapping MIDI-data to CVs or Triggers for audiothread ===
        ControlCVtags cv_entry;     // Will contain the result of CCs to be mapped to CVs or be empty if not found...
        ControlTrigTags trig_entry; // Will contain the result of CCs to be mapped to Triggers or be empty if not found...
        uint8_t channel = 0;   // Please note: channel 0 is global, 2-5 are voices, 6-16 are mapped to 2-5, (16 may be global for MPE or unused, so that we don't get any data here anyhow)
        uint8_t cc_num = 0;    // Continuous Controller Number is at second byte of the MIDI-message

        // === Private helper Funtions ===
        // --- "Panic-function": avoid hanging notes, especially after voidemode-Change ---
        void allNotesOff();           // Typically to be triggered by all notes off message (Continouus Controller)

        // --- Voice-Mode Management (Channel 1: monophonic, lower note priority, Legato - Channel 15: Duophonic - Channel 16: Polyphonic with 4 Voices) ---
        void handleNoteOff(uint8_t*  msg); // Handle Noteoffs via MIDI (Either by noteoff-event or noteon-event with velocity 0), set Triggers accordinly and reset playing note 

        // --- Try to map a Conctrol Change message to a for UI-list known category ---
        CV_id_abcd ccToCVid_abcd(uint8_t cc);      // Voices A-D: Map CCs to enums for CV-indexes
        CV_id_abcd abcdCVid;    // Return-value of function for examination
        CV_id_glob ccToCVid_glob(uint8_t cc);      // Global Channel: Map CCs to enums for CV-indexes
        CV_id_glob globCVid;    // Return-value of function for examination
        TRIG_id_abcd ccToTrigId_abcd(uint8_t cc);    // Voices A-D: Map CCs to enums for Trigger-indexes
        TRIG_id_abcd abcdTRIGid;   // Return-value of function for examination 
        TRIG_id_glob ccToTrigId_glob(uint8_t cc);    // Global Channel: Map CCs to enums for Trigger-indexes
        TRIG_id_glob globTRIGid;   // Return-value of function for examination
    };
}