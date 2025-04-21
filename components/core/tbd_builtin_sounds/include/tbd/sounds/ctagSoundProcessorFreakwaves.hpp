#include <atomic>
#include <tbd/sound_processor.hpp>
#include "tbd/helpers/ctagRollingAverage.hpp"
#include "plaits/dsp/oscillator/wavetable_oscillator.h"
#include "plaits/dsp/physical_modelling/resonator.h"
#include "plaits/dsp/engine/engine.h"
#include "tbd/helpers/ctagFastMath.hpp"
#include "tbd/helpers/ctagSineSource.hpp"
#include "tbd/helpers/ctagNumUtil.hpp"
#include "tbd/helpers/ctagADSREnv.hpp"
#include "plaits/dsp/oscillator/wavetable_oscillator.h"
#include "braids/quantizer.h"
#include "tbd/helpers/ctagSampleRom.hpp"
#include "tbd/helpers/ctagWNoiseGen.hpp"
#include "tbd/helpers/ctagFBDelayLine.hpp"


// --- Trigger/Gate values ---
#define GATE_HIGH_NEW       2
#define GATE_HIGH           1
#define GATE_LOW            0

// --- VULT "Library for TBD" ---
#include "./vult/vult_formantor.h"
#include "./vult/vult_formantor.tables.h"

#define NUM_OF_LFOS_FW                7

using namespace CTAG::SP::HELPERS;

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorFreakwaves : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;
            virtual void Init(std::size_t blockSize, void *blockPtr) override;
            virtual ~ctagSoundProcessorFreakwaves();

        private:
            virtual void knowYourself() override;
            plaits::Resonator resonator;

            // --- Remember status of triggers / buttons ---
            inline int process_param_trig( const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id, int gate_type ); // rescale incoming data to bool
            enum trig_states
            {
                e_GateA, e_GateB, e_CoutOnRightCh, e_GeneratePitchC, e_ResonatorOn, e_CnotAorB, e_ChooseBforNot,
                e_lfoActive_1, e_lfoActive_2, e_lfoActive_3, e_lfoActive_4, e_lfoActive_5, e_lfoActive_6, e_lfoActive_7,
                e_QuantInputA, e_QuantInputB, e_ExtModCisOn, e_PortaA, e_PortaB, e_PortaC, e_AccentModCisOn, e_KeytrackModCisOn,
                e_UseABGate, e_DelayEnable, e_DelayTimeShortened, e_ExternalDetectToRelease, e_KeytrackingToEcho,
                e_EGvolActive_A, e_EGvolSlow_A, e_EGvolActive_B, e_EGvolSlow_B, e_EGvolActive_C, e_EGvolSlow_C,
                e_AddDelayAfterResonator, e_AccentToScanC, e_GateForCisB, e_KeytrackSlew, e_Freakwaves_options_max
            };
            int prev_trig_state[e_Freakwaves_options_max] = {0};   // Initialize _all_ entries with "low value"
            bool low_reached[e_Freakwaves_options_max] = {false};  // We need this for look for toggle-events

            // --- Average to calculate Accent Bend (Our interpretation: Pitchbend depending on overall-volume of Talkbox signal) ---
            ctagRollingAverage averageAccent;
            ctagRollingAverage averageExternal;
            ctagRollingAverage averageA;
            ctagRollingAverage averageB;
            ctagRollingAverage averageC;
            float lastResonatorPos_ = 0.f;
            float f_accentVal_ = 0.f;           // Remember average accent-level
            float accentHyteresis_ = 0.f;       // Hysteresis memory variable for Amplitude MG
            float keytrackHyteresis_ = 0.f;     // Hysteresis memory variable for Keytrack

            // --- Portamento --
            float portamentoHyteresisA_ = 0.f;   // Hysteresis memory variable for Portamento of OSC A
            float portamentoHyteresisB_ = 0.f;   // Hysteresis memory variable for Portamento of OSC B
            float portamentoHyteresisC_ = 0.f;   // Hysteresis memory variable for Portamento of OSC C
            float portaPrevNoteA_ = 0.f;         // Previous note for Portamento of OSC A
            float portaPrevNoteB_ = 0.f;         // Previous note for Portamento of OSC B
            float portaPrevNoteC_ = 0.f;         // Previous note for Portamento of OSC C

            float f_externalVal_ = 0.f;         // Remember average external
            float externalHyteresis_ = 0.f;     // Hysteresis memory variable for external-input MG

            // --- Pitch quantisation ---
            braids::Quantizer quantiz_A;
            braids::Quantizer quantiz_B;
            braids::Quantizer quantiz_C;

            // --- EGs --
            ctagADSREnv env_A;
            ctagADSREnv env_B;
            ctagADSREnv env_C;
            int g_GateC_ = GATE_LOW;   // Gate for generated third voice

            // --- Sample Oscillator (datastructures as also needed for wavetable) ---
            ctagSampleRom sample_rom;      // Contains samples and Wavetables
            uint32_t wtSliceOffset = 0;

            // --- Helper functions: convert wavetable to MI-format, morph waves ---
            void prepareWavetable(const int16_t **wavetables, int currentBank, bool *isWaveTableGood, float **fbuffer, int16_t **ibuffer );

            // --- Wavetable A ---
            plaits::WavetableOscillator<256, 64> wt_osc_A;
            int16_t *buffer_A = NULL;
            float *fbuffer_A = NULL;
            const int16_t *wavetables_A[64];
            int currentBank_A = 0;
            int lastBank_A = -1;
            bool isWaveTableGood_A = false;

            // --- Wavetable B ---
            plaits::WavetableOscillator<256, 64> wt_osc_B;
            int16_t *buffer_B = NULL;
            float *fbuffer_B = NULL;
            const int16_t *wavetables_B[64];
            int currentBank_B = 0;
            int lastBank_B = -1;
            bool isWaveTableGood_B = false;

            // --- Wavetable C ---
            plaits::WavetableOscillator<256, 64> wt_osc_C;
            int16_t *buffer_C = NULL;
            float *fbuffer_C = NULL;
            const int16_t *wavetables_C[64];
            int currentBank_C = 0;
            int lastBank_C = -1;
            bool isWaveTableGood_C = false;

            // --- Logic operations (results for OSC C) ---
            float f_pitch_stored_C_ = 0.f; // Most recent valid pitch of OSC C...

            // --- Delay ---
            const uint32_t maxDelayLength {88200};
            const uint32_t shorterMaxDelayLength = maxDelayLength / 4;    // We have an option to shorten the delay-range for slapback delays
            HELPERS::ctagFBDelayLine dlyLine {maxDelayLength};
            float m_DelayTime = 0.f;          // Hysteresis values... (avoiding too abrupt changes...)
            float m_DelayFeedback = 0.f;

            // --- LFOs incl. random sources ---
            ctagSineSource lfoPitchC;
            ctagSineSource lfo[NUM_OF_LFOS_FW];
            int t_lfoActive[NUM_OF_LFOS_FW] = {0,0,0,0,0,0,0};         // LFO may be switched off
            int i_lfoDestination[NUM_OF_LFOS_FW] = {0,0,0,0,0,0,0};    // Contains IDs of LFO destination
            int i_lfoType[NUM_OF_LFOS_FW] = {0,0,0,0,0,0,0};           // LFO Wavetype, the last 5 LFOs can be modulated in tempo (LFOs 1-5 on GUI)
            float f_lfoSpeed[NUM_OF_LFOS_FW] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};    // LFO speed
            float f_lfoAmnt[NUM_OF_LFOS_FW] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};     // LFO amount
            float f_lfo_val[NUM_OF_LFOS_FW] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};     // Lfo process result

            float morph_sine_wave(float sine_val, int morph_mode, int enum_sine);  // Morph sinewave to square, pseudo triangle, sample and hold and similar

            // --- Sample and Hold for LFOs ---
            float applySnH(float sine_lfo_val, int enum_val);
            enum snh_members
            {
                snh_lfo_1, snh_lfo_2, snh_lfo_3, snh_lfo_4, snh_lfo_5, snh_lfo_6, snh_lfo_7, e_snh_max
            };
            ctagWNoiseGen oscSnH[e_snh_max];        // This is a list of objects, so we must not initilize it (again)
            float saved_sample[e_snh_max] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
            float previous_sine_val[e_snh_max] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

            // private attributes could go here
            // autogenerated code here
            // sectionHpp
			std::atomic<int32_t> GateA, trig_GateA;
			std::atomic<int32_t> GateB, trig_GateB;
			std::atomic<int32_t> MasterPitch, cv_MasterPitch;
			std::atomic<int32_t> MasterTune, cv_MasterTune;
			std::atomic<int32_t> CoutOnRightCh, trig_CoutOnRightCh;
			std::atomic<int32_t> QuantInputA, trig_QuantInputA;
			std::atomic<int32_t> Qscale_A, cv_Qscale_A;
			std::atomic<int32_t> QuantInputB, trig_QuantInputB;
			std::atomic<int32_t> Qscale_B, cv_Qscale_B;
			std::atomic<int32_t> Qscale_C, cv_Qscale_C;
			std::atomic<int32_t> PortaA, trig_PortaA;
			std::atomic<int32_t> PortamentoTimeA, cv_PortamentoTimeA;
			std::atomic<int32_t> PortaB, trig_PortaB;
			std::atomic<int32_t> PortamentoTimeB, cv_PortamentoTimeB;
			std::atomic<int32_t> PortaC, trig_PortaC;
			std::atomic<int32_t> PortamentoTimeC, cv_PortamentoTimeC;
			std::atomic<int32_t> Vol_A, cv_Vol_A;
			std::atomic<int32_t> Vol_B, cv_Vol_B;
			std::atomic<int32_t> Vol_C, cv_Vol_C;
			std::atomic<int32_t> Vol_Ext, cv_Vol_Ext;
			std::atomic<int32_t> ExternalWet, cv_ExternalWet;
			std::atomic<int32_t> BalanceAB_C, cv_BalanceAB_C;
			std::atomic<int32_t> Volume, cv_Volume;
			std::atomic<int32_t> WaveTblA, cv_WaveTblA;
			std::atomic<int32_t> ScanWavTblA, cv_ScanWavTblA;
			std::atomic<int32_t> pitch_A, cv_pitch_A;
			std::atomic<int32_t> tune_A, cv_tune_A;
			std::atomic<int32_t> WaveTblB, cv_WaveTblB;
			std::atomic<int32_t> ScanWavTblB, cv_ScanWavTblB;
			std::atomic<int32_t> pitch_B, cv_pitch_B;
			std::atomic<int32_t> tune_B, cv_tune_B;
			std::atomic<int32_t> WaveTblC, cv_WaveTblC;
			std::atomic<int32_t> ScanWavTblC, cv_ScanWavTblC;
			std::atomic<int32_t> relative_tune_C, cv_relative_tune_C;
			std::atomic<int32_t> GeneratePitchC, trig_GeneratePitchC;
			std::atomic<int32_t> CnotAorB, trig_CnotAorB;
			std::atomic<int32_t> ChooseBforNot, trig_ChooseBforNot;
			std::atomic<int32_t> UseABGate, trig_UseABGate;
			std::atomic<int32_t> GateForCisB, trig_GateForCisB;
			std::atomic<int32_t> LFOspeedC, cv_LFOspeedC;
			std::atomic<int32_t> LFOamountC, cv_LFOamountC;
			std::atomic<int32_t> ExtModCisOn, trig_ExtModCisOn;
			std::atomic<int32_t> ExtModGain, cv_ExtModGain;
			std::atomic<int32_t> AmountForExternalMod, cv_AmountForExternalMod;
			std::atomic<int32_t> ExternalDetectToRelease, trig_ExternalDetectToRelease;
			std::atomic<int32_t> AccentModCisOn, trig_AccentModCisOn;
			std::atomic<int32_t> DetectAccentLevel, cv_DetectAccentLevel;
			std::atomic<int32_t> AmountForAccentMod, cv_AmountForAccentMod;
			std::atomic<int32_t> AccentToScanC, trig_AccentToScanC;
			std::atomic<int32_t> KeytrackModCisOn, trig_KeytrackModCisOn;
			std::atomic<int32_t> KeytrackingLevel, cv_KeytrackingLevel;
			std::atomic<int32_t> KeytrackSlew, trig_KeytrackSlew;
			std::atomic<int32_t> KeytrackingToEcho, trig_KeytrackingToEcho;
			std::atomic<int32_t> EGvolActive_A, trig_EGvolActive_A;
			std::atomic<int32_t> EGvolSlow_A, trig_EGvolSlow_A;
			std::atomic<int32_t> AttackVol_A, cv_AttackVol_A;
			std::atomic<int32_t> DecayVol_A, cv_DecayVol_A;
			std::atomic<int32_t> SustainVol_A, cv_SustainVol_A;
			std::atomic<int32_t> ReleaseVol_A, cv_ReleaseVol_A;
			std::atomic<int32_t> EGvolActive_B, trig_EGvolActive_B;
			std::atomic<int32_t> EGvolSlow_B, trig_EGvolSlow_B;
			std::atomic<int32_t> AttackVol_B, cv_AttackVol_B;
			std::atomic<int32_t> DecayVol_B, cv_DecayVol_B;
			std::atomic<int32_t> SustainVol_B, cv_SustainVol_B;
			std::atomic<int32_t> ReleaseVol_B, cv_ReleaseVol_B;
			std::atomic<int32_t> EGvolActive_C, trig_EGvolActive_C;
			std::atomic<int32_t> EGvolSlow_C, trig_EGvolSlow_C;
			std::atomic<int32_t> AttackVol_C, cv_AttackVol_C;
			std::atomic<int32_t> DecayVol_C, cv_DecayVol_C;
			std::atomic<int32_t> SustainVol_C, cv_SustainVol_C;
			std::atomic<int32_t> ReleaseVol_C, cv_ReleaseVol_C;
			std::atomic<int32_t> DelayEnable, trig_DelayEnable;
			std::atomic<int32_t> AddDelayAfterResonator, trig_AddDelayAfterResonator;
			std::atomic<int32_t> DelayDryWet, cv_DelayDryWet;
			std::atomic<int32_t> DelayTimeShortened, trig_DelayTimeShortened;
			std::atomic<int32_t> DelayTime, cv_DelayTime;
			std::atomic<int32_t> DelayFeedback, cv_DelayFeedback;
			std::atomic<int32_t> ResonatorOn, trig_ResonatorOn;
			std::atomic<int32_t> ResonatorDryWet, cv_ResonatorDryWet;
			std::atomic<int32_t> WaveShaperDryWet, cv_WaveShaperDryWet;
			std::atomic<int32_t> ResonatorPosition, cv_ResonatorPosition;
			std::atomic<int32_t> ResonatorFreq, cv_ResonatorFreq;
			std::atomic<int32_t> ResonatorStructure, cv_ResonatorStructure;
			std::atomic<int32_t> ResonatorBrightness, cv_ResonatorBrightness;
			std::atomic<int32_t> ResonatorDamping, cv_ResonatorDamping;
			std::atomic<int32_t> lfoActive_1, trig_lfoActive_1;
			std::atomic<int32_t> lfoDestination_1, cv_lfoDestination_1;
			std::atomic<int32_t> lfoType_1, cv_lfoType_1;
			std::atomic<int32_t> lfoSpeed_1, cv_lfoSpeed_1;
			std::atomic<int32_t> lfoAmnt_1, cv_lfoAmnt_1;
			std::atomic<int32_t> lfoActive_2, trig_lfoActive_2;
			std::atomic<int32_t> lfoDestination_2, cv_lfoDestination_2;
			std::atomic<int32_t> lfoType_2, cv_lfoType_2;
			std::atomic<int32_t> lfoSpeed_2, cv_lfoSpeed_2;
			std::atomic<int32_t> lfoAmnt_2, cv_lfoAmnt_2;
			std::atomic<int32_t> lfoActive_3, trig_lfoActive_3;
			std::atomic<int32_t> lfoDestination_3, cv_lfoDestination_3;
			std::atomic<int32_t> lfoType_3, cv_lfoType_3;
			std::atomic<int32_t> lfoSpeed_3, cv_lfoSpeed_3;
			std::atomic<int32_t> lfoAmnt_3, cv_lfoAmnt_3;
			std::atomic<int32_t> lfoActive_4, trig_lfoActive_4;
			std::atomic<int32_t> lfoDestination_4, cv_lfoDestination_4;
			std::atomic<int32_t> lfoType_4, cv_lfoType_4;
			std::atomic<int32_t> lfoSpeed_4, cv_lfoSpeed_4;
			std::atomic<int32_t> lfoAmnt_4, cv_lfoAmnt_4;
			std::atomic<int32_t> lfoActive_5, trig_lfoActive_5;
			std::atomic<int32_t> lfoDestination_5, cv_lfoDestination_5;
			std::atomic<int32_t> lfoType_5, cv_lfoType_5;
			std::atomic<int32_t> lfoSpeed_5, cv_lfoSpeed_5;
			std::atomic<int32_t> lfoAmnt_5, cv_lfoAmnt_5;
			std::atomic<int32_t> lfoActive_6, trig_lfoActive_6;
			std::atomic<int32_t> lfoDestination_6, cv_lfoDestination_6;
			std::atomic<int32_t> lfoType_6, cv_lfoType_6;
			std::atomic<int32_t> lfoSpeed_6, cv_lfoSpeed_6;
			std::atomic<int32_t> lfoAmnt_6, cv_lfoAmnt_6;
			std::atomic<int32_t> lfoActive_7, trig_lfoActive_7;
			std::atomic<int32_t> lfoDestination_7, cv_lfoDestination_7;
			std::atomic<int32_t> lfoType_7, cv_lfoType_7;
			std::atomic<int32_t> lfoSpeed_7, cv_lfoSpeed_7;
			std::atomic<int32_t> lfoAmnt_7, cv_lfoAmnt_7;
			// sectionHpp
        };
    }
}