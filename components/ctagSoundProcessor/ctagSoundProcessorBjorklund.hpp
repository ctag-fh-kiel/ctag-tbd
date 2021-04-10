#include <atomic>
#include "ctagSoundProcessor.hpp"
#include "helpers/ctagADEnv.hpp"            // Needed for AD EG (Attack/Decay Envelope Generator)
#include "helpers/ctagADSREnv.hpp"          // Needed for ADSR EG (Attack/Decay/Sustain/Release Envelope Generator)
#include "helpers/ctagWNoiseGen.hpp"
#include "helpers/ctagSineSource.hpp"
#include "helpers/ctagFastMath.hpp"

// --- Utility stuff ---
#include "helpers/ctagRollingAverage.hpp"

// --- VULT "Library for TBD" ---
#include "./vult/vult_formantor.h"
#include "./vult/vult_formantor.tables.h"

#define GATE_PATTERN_ELEMENTS 24
#define ACCENT_PATTERN_ELEMENTS 35
#define NOTE_PATTERN_ELEMENTS 64
#define SCALE_PATTERN_ELEMENTS 12

using namespace CTAG::SP::HELPERS;

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorBjorklund : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;
            ctagSoundProcessorBjorklund();
            virtual ~ctagSoundProcessorBjorklund();

        private:
            virtual void knowYourself() override;

            // --- Remember status of triggers / buttons ---
            inline int process_param_trig( const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id, int gate_type ); // rescale incoming data to bool
            enum trig_states
            {
               e_Trigger, e_EGnoiseLoop, e_SeperateOutput, e_InternalClock, e_clockVal, e_EGnoiseNegative, e_PatternSequencer, e_BjorklundOff,
               e_EasyEditOn, e_EGoscMixActive, e_EGoscMixNegative, e_EGoscMixLoop, e_EGfilterLeakActive, e_EGfilterLeakNegative, e_FilterLeakEnvLoop,
               e_EGwfNegative, e_EGringNegative, e_EGfiltNegative, e_EGvolLoop, e_EGvolNegative, e_EGringLoop, e_EGwfLoop, e_EGfiltLoop, e_AccentIsBjorklund,
               e_AccentOff, e_AccentSync, e_ResetSequencer, e_PalindromeOff, e_RingOn, e_RingOnPulse, e_RingOnSaw, e_EGvolActive, e_EGwfActive,
               e_EGnoiseActive, e_EGringActive, e_EGfiltActive, e_AMisSquare, e_PWMon, e_QuantizePitch, e_VintageFilter, e_Bjorklund_options_max
            };
            int prev_trig_state[e_Bjorklund_options_max] = {0};   // Initialize _all_ entries with "low value"
            bool low_reached[e_Bjorklund_options_max] = {false};  // We need this for look for toggle-events

            // --- Dejitter CV in for notes ---
            ctagRollingAverage cvPitch = ctagRollingAverage();

            // --- VULT Stuff ---
            Saw_eptr__ctx_type_0 saw_data;              // Saw oscillator data-structure
            Blit__ctx_type_1 pulse_data;                 // Pulse oscillator data-structure
            Noise__ctx_type_1 noise_data;               // Noise oscillator data-structure
            Ladder__ctx_type_8 ladder_data;             // Ladder filter data-structure
            Ladder__ctx_type_6 ladder_vintage_data;     // Euker ladder algorithm

            // --- Additional oscillator[s] ---
            ctagSineSource oscRing;

            // --- LFO ---
            ctagSineSource lfoPWM;
            ctagSineSource lfoClock;

            // --- EGs --
            ctagADEnv vol_eg_ad;
            ctagADEnv noise_eg_ad;
            ctagADEnv osc_mix_eg_ad;
            ctagADEnv wf_eg_ad;
            ctagADEnv am_eg_ad;
            ctagADEnv filter_eg_ad;
            ctagADEnv filt_leak_eg_ad;

            // === Algorithmic patterns ===
            // --- Eucledian/Björklund rhythms - you can add you own here! ---
            const char* gate_pattern[GATE_PATTERN_ELEMENTS] =         // Euclidan rhythms - for further explanations please refer to: http://cgm.cs.mcgill.ca/~godfried/publications/banff.pdf
            {
              // --- Non-Eucledian "standard patterns" ---
              (char*)"x",                                // Play each note - please be aware: this will result in a constant Gate on
              (char*)"x.",                               // Alternate notes with pauses
              // --- The following Euclidean rhythms are Euclideanstrings --- // Comments as found in the before-mentioned paper by Godfried Toussaint (School of Computer Science, Mc Gill University Montreal, Quebec, Canada)
              (char*)"x.x..",                            // E(2,5)=[x. x . .] = (23)(classical, jazz and Persian).
              (char*)"x.x.x..",                          // E(3,7)=[x. x . x . .] = (223)(Bulgarian folk).
              (char*)"x.x.x.x..",                        // E(4,9)= [x. x . x . x . .] = (2223)(Turkey).
              (char*)"x.x.x.x.x..",                      // E(5,11)=[x. x . x . x . x . .] = (22223)(classical).
              (char*)"x..x..x..x..x....",                // E(5,16)= [x. . x . . x . . x . . x . . . .] = (33334)(Brazilian necklace).
              // --- The following Euclidean rhythms are reverse Euclidean strings ---
              (char*)"x.x",                              // E(2,3)= [x. x]= (21)(West Africa,Latin America).
              (char*)"x.xx",                             // E(3,4)=[x. x x]= (211)(Trinidad,Persia).
              (char*)"x.x.x",                            // E(3,5)=[x. x . x]= (221)(Rumanian and Persian necklaces).
              (char*)"x..x..x.",                         // E(3,8)=[x. . x . . x .] = (332)(West Africa or Cuban tresillo also called (char*)"Habanero").
              (char*)"x.x.x.x",                          // E(4,7)=[x. x . x . x]= (2221)(Bulgaria).
              (char*)"x..x..x..x.",                      // E(4,11)= [x. . x . . x . . x .] = (3332)(Frank Zappa).
              (char*)"x.xxxx",                           // E(5,6)=[x. x x x x]= (21111)(Arab).
              (char*)"x.xx.xx",                          // E(5,7)=[x. x x . x x]= (21211)(Arab).
              (char*)"x.x.x.x.x",                        // E(5,9)=[x. x . x . x . x]= (22221)(Arab rhythm, South African and Rumanian necklaces).
              (char*)"x..x.x..x.x.",                     // E(5,12)= [x. . x . x . . x . x .] = (32322)(South Africa).
              (char*)"x.xxxxxx",                         // E(7,8)= [x. x x x x x x]= (2111111)(Tuareg rhythm of Libya).
              (char*)"x..x.x.x..x.x.x.",                 // E(7,16)= [x. . x . x . x . . x . x . x .] = (3223222)(Brazilian necklace).
              (char*)"x..x.x.x.x.x..x.x.x.x.x.",         // E(11,24)= [x. . x . x . x . x . x . . x . x . x . x . x .] = (32222322222)(Central Africa).
              // --- The following Euclidean rhythms are neither Euclidean nor reverse Euclidean strings ---
              (char*)"x.xx.xx.",                         // E(5,8)=[x. x x . x x .] = (21212)(West Africa).
              (char*)"x.xx.x.xx.x.",                     // E(7,12)= [x. x x . x . x x . x .] = (2122122)(West Africa).
              (char*)"x.xx.x.x.xx.x.x.",                 // E(9,16)= [x. x x . x . x . x x . x . x .] = (212221222)(West and Central African and Brazilian necklaces).
              (char*)"x.xx.x.x.x.x.xx.x.x.x.x."          // E(13,24)= [x. x x . x . x . x . x . x x . x . x . x . x .] = (2122222122222)(Central African necklace)
            };
            // --- Accent patterns, can optionally be combined with Bjorklund patterns and have positive of negative accents on 7 different modulation destinations ---
            const char* accent_pattern[ACCENT_PATTERN_ELEMENTS] =         // Common metrical accents, inspired by: https://www.guitarnoise.com/community/guitar-and-music-theory/metronome-and-time-signatures
            {
              (char*)"X",                                 // Accent on all notes
              (char*)"X.",                                // 2/4 beat
              (char*)"X..",                               // 3/4 beat
              (char*)"X.x.",                              // 4/4, second strong beat less strong
              (char*)"x.X.",                              // 4/4, first strong beat less strong
              (char*)".X.X",                              // 4/4, off beat
              (char*)".X.x",                              // 4/4, off beat - first strong beat less strong
              (char*)".x.X",                              // 4/4, off beat - second strong beat less strong
              (char*)"X....",                             // 5/4, beat accent only on first beat
              (char*)"X.x..",                             // 5/4, beat accent on 2/4 + 3/4
              (char*)"X..x.",                             // 5/4, beat accent on 3/4 + 2/4
              (char*)"X.x.x.",                            // 6/4, beat  (half note based accent, in contrast to regular 3/4)
              (char*)"X..x..",                            // 6/4, beat
              (char*)"X...x..",                           // 7/4, beat  4/4 + 3/4
              (char*)"X..x...",                           // 7/4, beat  3/4 + 4/4
              (char*)"X.x.x..",                           // 7/4, beat  variant 1
              (char*)"X.x.X.x",                           // 7/4, beat  variant 2
              (char*)"X..x..x..",                         // 9/8
              (char*)"X..x..x..x..",                      // 12/8
              (char*)"X..x..x..x...",                     // 13/8 (15 variants!)
              (char*)"X.x.x.x.x.x..",   					        // 13/8, from here on: Five duple, one triple (six choices)
              (char*)"X.x.x.x.x..x.",
              (char*)"X.x.x.x..x.x.",
              (char*)"X.x.x..x.x.x.",
              (char*)"X.x..x.x.x.x.",
              (char*)"X..x.x.x.x.x.",
              (char*)"X.x.x..x..x..",   					        // 13/8, from here on: Two duple, three triple (nine choices)
              (char*)"X.x..x.x..x..",
              (char*)"X.x..x..x.x..",
              (char*)"X.x..x..x..x.",
              (char*)"X..x.x.x..x..",
              (char*)"X...x.x..x.x..",
              (char*)"X..x..x.x.x..",
              (char*)"X..x..x.x..x.",
              (char*)"X..x..x..x.x."
            };
			      // --- Mainly Palindromic numbers with base 10 or 16 and some palindromic sentenses - you can add you own here! ---
            char* note_pattern[NOTE_PATTERN_ELEMENTS] =
            {
              (char*)"1",                      // Steady tone, yet palindromic
              (char*)"1357dhkquxxuqkhd7531",   // A palindronic sequence, made up with no real logic to it
              // All palindromic primes with 5 digits according to: https://oeis.org/A002385
              (char*)"10301", (char*)"10501", (char*)"10601", (char*)"11311", (char*)"11411", (char*)"12421", (char*)"12721", (char*)"12821", (char*)"13331", (char*)"13831", (char*)"13931",
              (char*)"14341", (char*)"14741", (char*)"15451", (char*)"15551", (char*)"16061", (char*)"16361", (char*)"16561", (char*)"16661", (char*)"17471", (char*)"17971", (char*)"18181",
              // Mutiplications giving numbers dividable by 11, up to 111111111 * 111111111
              (char*)"121", (char*)"12321", (char*)"1234321", (char*)"123454321", (char*)"12345654321", (char*)"1234567654321", (char*)"123456787654321",  (char*)"12345678987654321",
              // Hex palindromes starting with 07 and 6 digits
              (char*)"070070", (char*)"071170", (char*)"072270", (char*)"073370", (char*)"074470", (char*)"075570", (char*)"076670", (char*)"077770", (char*)"078870", (char*)"079970",
              (char*)"07aa70", (char*)"07bb70",  (char*)"07cc70", (char*)"07dd70", (char*)"07ee70",  (char*)"07ff70",
              // Famous palindromic sentences, according to: http://www2.cs.arizona.edu/icon/oddsends/palinsen.htm or https://newsfeed.time.com/2013/03/06/madam-im-adam-palindrome-masters-go-head-to-head-in-championship
              (char*)"SoreWasIereISawEros",
              (char*)"OozyRatInAsanitaryZoo",
              (char*)"MadamInEdenImAdam",
              (char*)"AmanAplanAcanalPanama",
              (char*)"NoWordNoBondRowOn",
              (char*)"StressedWasIereIsawDesserts",
              (char*)"DoGoodsDeedsLiveonNoEvilsDeedsDoOgod",
              (char*)"DocNoteIdissentAfastNeverPreventsAfatnessIdietOnCod",
              (char*)"StopNineMyriadMurmurPutUpRumRumDairymenInPots",
              (char*)"SirrahDeliverDeifiedDessertsDetartratedStressedDeifiedReviledHarris",
              (char*)"DeliverDessertsDemandedNemesisEmendedNamedStressedReviled",
              (char*)"NowSawYeNoMossesOrFoamOrAromaOfRosesSoMoneyWasWon",
              (char*)"EreHypocrisiesOrPosesAreInMyHymnIEraseSoProseISirCopyHere",
              (char*)"DegasAreWeNotDrawnOnwardWeFreerFewDrawnOnwardToNewErasAged",
              (char*)"tEliotTopBardNotesPutridTangEmanatingIsSadIdAssignItAnameGnatDirtUpsetOnDrabPotToilet",
              (char*)"DoomRewardAmaniacIreMadeTargetAhaMixAmaniaTerrorOnOhioIhonorOrRetainAmaximAhateGratedAmericaInAmadRawerMood"
            };
            // --- List of patterns for scale-correction, you can add your own here ---
            int scale_pattern[SCALE_PATTERN_ELEMENTS][12] =
            {
              { 0,1,2,3,4,5,6,7,8,9,10,11 },  // Chromatic (no change to original notes pattern)
              { 0,0,2,2,4,5,5,7,7,9,9,11 },   // Major
              { 0,0,2,2,5,5,7,7,7,9,9,9 },    // Slendro
              { 0,0,2,3,3,5,5,7,7,9,9,11 },   // Minor
              { 2,2,2,3,3,5,5,8,8,9,10,10 },  // Pelog
              { 0,0,2,2,3,3,5,5,7,7,8,9 },    // Dorian
              { 0,0,2,2,2,4,4,6,7,7,9,11 },   // Lydian
              { 0,1,1,3,3,5,5,7,8,8,10,10 },  // Phrygian
              { 0,0,0,0,5,5,5,5,5,9,9,9 },    // Added forth and sixth
              { 0,0,0,0,0,7,7,7,7,7,0,0 },    // Octaves and fifth only
              { 9,9,9,9,9,9,9,9,9,9,9,9 },    // Sixth only
              { 0,0,2,2,4,4,4,7,7,7,9,9 }     // Pentatonic
            };
            // --- Sequencer helper variables ---
            const int note_pattern_size = NOTE_PATTERN_ELEMENTS;
            const int accent_pattern_size = ACCENT_PATTERN_ELEMENTS;
            const int gate_pattern_size = GATE_PATTERN_ELEMENTS;
            const int scale_pattern_size = SCALE_PATTERN_ELEMENTS;
            const int note_pattern_max_idx = NOTE_PATTERN_ELEMENTS-1;
            const int accent_pattern_max_idx = ACCENT_PATTERN_ELEMENTS-1;
            const int gate_pattern_max_idx = GATE_PATTERN_ELEMENTS-1;
            const int scale_pattern_max_idx = SCALE_PATTERN_ELEMENTS-1;

            int note_pat_idx = 0;   // Index for one entry from note-patterns, selectable via GUI or CV
            int gate_pat_idx = 0;   // Index for one entry from gate-patterns to trigger samples, selectable via GUI or CV
            int scale_pat_idx = 0;  // Index for one entry from scale-patterns, selectable via GUI or CV
            int accent_pat_idx = 0;  // Index for one entry from accent-patterns, selectable via GUI or CV
            int last_gate_idx = 0;
            int last_accent_idx = 0;
            int beat_divider_counter = 0;
            int last_beat_divider_counter = 0;
            int accent_beat_divider_counter = 0;
            int last_accent_beat_divider_counter = 0;
            int gate_idx = 0;
            int note_idx = 0;
            int accent_idx = 0;
            float f_accent = 0.f;             // We apply this to a specific modulation-target if desired
            int note_offset = 0;
            int notes_len = strlen(note_pattern[note_pat_idx]);         // will be selected via Pot later!
            int gates_len = strlen(gate_pattern[gate_pat_idx]);         // Gate for notes, will be selected via Pot later!
            int accents_len = strlen(accent_pattern[accent_pat_idx]);   // Accents for notes, will be selected via Pot later!

            // private attributes could go here
            // autogenerated code here
            // sectionHpp
	atomic<int32_t> Trigger, trig_Trigger;
	atomic<int32_t> BeatDivider, cv_BeatDivider;
	atomic<int32_t> InternalClock, trig_InternalClock;
	atomic<int32_t> ClockSpeed, cv_ClockSpeed;
	atomic<int32_t> MasterPitch, cv_MasterPitch;
	atomic<int32_t> MasterTune, cv_MasterTune;
	atomic<int32_t> QuantizePitch, trig_QuantizePitch;
	atomic<int32_t> ScaleCorrect, cv_ScaleCorrect;
	atomic<int32_t> Volume, cv_Volume;
	atomic<int32_t> SawPitch, cv_SawPitch;
	atomic<int32_t> SawTune, cv_SawTune;
	atomic<int32_t> PulsePitch, cv_PulsePitch;
	atomic<int32_t> PulseTune, cv_PulseTune;
	atomic<int32_t> PulseWidth, cv_PulseWidth;
	atomic<int32_t> PWMon, trig_PWMon;
	atomic<int32_t> PWMspeed, cv_PWMspeed;
	atomic<int32_t> PWMamount, cv_PWMamount;
	atomic<int32_t> SeperateOutput, trig_SeperateOutput;
	atomic<int32_t> NoiseVol, cv_NoiseVol;
	atomic<int32_t> SawVol, cv_SawVol;
	atomic<int32_t> PulseVol, cv_PulseVol;
	atomic<int32_t> OSCmix, cv_OSCmix;
	atomic<int32_t> RingOnSaw, trig_RingOnSaw;
	atomic<int32_t> RingOnPulse, trig_RingOnPulse;
	atomic<int32_t> AMisSquare, trig_AMisSquare;
	atomic<int32_t> RingModFreq, cv_RingModFreq;
	atomic<int32_t> RingModAmnt, cv_RingModAmnt;
	atomic<int32_t> WaveFolder, cv_WaveFolder;
	atomic<int32_t> VintageFilter, trig_VintageFilter;
	atomic<int32_t> Cutoff, cv_Cutoff;
	atomic<int32_t> Resonance, cv_Resonance;
	atomic<int32_t> FilterTracking, cv_FilterTracking;
	atomic<int32_t> FilterLeakage, cv_FilterLeakage;
	atomic<int32_t> PatternSequencer, trig_PatternSequencer;
	atomic<int32_t> ResetSequencer, trig_ResetSequencer;
	atomic<int32_t> BjorklundOff, trig_BjorklundOff;
	atomic<int32_t> BjorklundPattern, cv_BjorklundPattern;
	atomic<int32_t> BjorklundShift, cv_BjorklundShift;
	atomic<int32_t> PalindromeOff, trig_PalindromeOff;
	atomic<int32_t> PalindromeSelect, cv_PalindromeSelect;
	atomic<int32_t> PalindromeRootkey, cv_PalindromeRootkey;
	atomic<int32_t> AccentOff, trig_AccentOff;
	atomic<int32_t> AccentSync, trig_AccentSync;
	atomic<int32_t> AccentDestination, cv_AccentDestination;
	atomic<int32_t> AccentBeatDivider, cv_AccentBeatDivider;
	atomic<int32_t> AccentIsBjorklund, trig_AccentIsBjorklund;
	atomic<int32_t> AccentSelect, cv_AccentSelect;
	atomic<int32_t> AccentShift, cv_AccentShift;
	atomic<int32_t> AccentAmount, cv_AccentAmount;
	atomic<int32_t> EGvolActive, trig_EGvolActive;
	atomic<int32_t> EGvolNegative, trig_EGvolNegative;
	atomic<int32_t> VolAttack, cv_VolAttack;
	atomic<int32_t> VolDecay, cv_VolDecay;
	atomic<int32_t> VolEnvAmount, cv_VolEnvAmount;
	atomic<int32_t> EGvolLoop, trig_EGvolLoop;
	atomic<int32_t> EasyEditOn, trig_EasyEditOn;
	atomic<int32_t> EGnoiseActive, trig_EGnoiseActive;
	atomic<int32_t> EGnoiseNegative, trig_EGnoiseNegative;
	atomic<int32_t> NoiseAttack, cv_NoiseAttack;
	atomic<int32_t> NoiseDecay, cv_NoiseDecay;
	atomic<int32_t> NoiseEnvAmount, cv_NoiseEnvAmount;
	atomic<int32_t> EGnoiseLoop, trig_EGnoiseLoop;
	atomic<int32_t> EGoscMixActive, trig_EGoscMixActive;
	atomic<int32_t> EGoscMixNegative, trig_EGoscMixNegative;
	atomic<int32_t> OscMixAttack, cv_OscMixAttack;
	atomic<int32_t> OscMixDecay, cv_OscMixDecay;
	atomic<int32_t> OscMixEnvAmount, cv_OscMixEnvAmount;
	atomic<int32_t> EGoscMixLoop, trig_EGoscMixLoop;
	atomic<int32_t> EGringActive, trig_EGringActive;
	atomic<int32_t> EGringNegative, trig_EGringNegative;
	atomic<int32_t> RingAttack, cv_RingAttack;
	atomic<int32_t> RingDecay, cv_RingDecay;
	atomic<int32_t> RingAmount, cv_RingAmount;
	atomic<int32_t> EGringLoop, trig_EGringLoop;
	atomic<int32_t> EGwfActive, trig_EGwfActive;
	atomic<int32_t> EGwfNegative, trig_EGwfNegative;
	atomic<int32_t> WfAttack, cv_WfAttack;
	atomic<int32_t> WfDecay, cv_WfDecay;
	atomic<int32_t> WfEnvAmount, cv_WfEnvAmount;
	atomic<int32_t> EGwfLoop, trig_EGwfLoop;
	atomic<int32_t> EGfiltActive, trig_EGfiltActive;
	atomic<int32_t> EGfiltNegative, trig_EGfiltNegative;
	atomic<int32_t> FiltAttack, cv_FiltAttack;
	atomic<int32_t> FiltDecay, cv_FiltDecay;
	atomic<int32_t> FiltAmount, cv_FiltAmount;
	atomic<int32_t> EGfiltLoop, trig_EGfiltLoop;
	atomic<int32_t> EGfilterLeakActive, trig_EGfilterLeakActive;
	atomic<int32_t> EGfilterLeakNegative, trig_EGfilterLeakNegative;
	atomic<int32_t> FilterLeakAttack, cv_FilterLeakAttack;
	atomic<int32_t> FilterLeakDecay, cv_FilterLeakDecay;
	atomic<int32_t> FilterLeakEnvAmount, cv_FilterLeakEnvAmount;
	atomic<int32_t> FilterLeakEnvLoop, trig_FilterLeakEnvLoop;
	// sectionHpp
        };
    }
}