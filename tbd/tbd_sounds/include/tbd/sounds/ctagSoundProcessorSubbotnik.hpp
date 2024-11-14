#include <atomic>
#include <tbd/sound_processor.hpp>
#include "braids/analog_oscillator.h"
#include "braids/signature_waveshaper.h"
#include "braids/macro_oscillator.h"
#include "braids/settings.h"
#include "braids/quantizer.h"
#include "helpers/ctagADEnv.hpp"
#include "helpers/ctagSineSource.hpp"
#include "helpers/ctagADSREnv.hpp"
#include "helpers/ctagWNoiseGen.hpp"

// --- VULT "Library for TBD" ---
#include "./vult/vult_formantor.h"
#include "./vult/vult_formantor.tables.h"

#define NUM_OF_LFOS                 10
#define CRITICAL_SHAPES_NUM     6

using namespace CTAG::SP::HELPERS;

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorSubbotnik : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;
           virtual void Init(std::size_t blockSize, void *blockPtr) override;
            virtual ~ctagSoundProcessorSubbotnik();

        private:
            virtual void knowYourself() override;

            // --- Remember status of triggers / buttons ---
            inline int process_param_trig( const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id, int gate_type ); // rescale incoming data to bool
            enum trig_states
            {
                e_Gate, e_osc_active_A, e_osc_active_B, e_xModActive_B, e_xModActive_A, e_enableFilter, e_lfoActive_1, e_lfoActive_2,
                e_lfoActive_3, e_lfoActive_4, e_lfoActive_5, e_lfoppActive_1, e_lfoppActive_2, e_lfoppActive_3, e_lfoppActive_4, e_lfoppActive_5,
                e_loopOnlyWithGate, e_enableVolADSReg, e_slowVolADSReg, e_slowADeg, e_filterIsSVF,
                e_lfoppActive_10, e_loopADeg, e_enableADeg, e_ADegIsLowpassGate, e_Subbotnik_options_max
            };
            int prev_trig_state[e_Subbotnik_options_max] = {0};   // Initialize _all_ entries with "low value"
            bool low_reached[e_Subbotnik_options_max] = {false};  // We need this for look for toggle-events

            // --- Braids related stuff ---
            braids::MacroOscillator osc_A;
            int16_t buffer_A[32];
            const uint8_t sync_A[32] = {0};
            braids::MacroOscillator osc_B;
            int16_t buffer_B[32];
            const uint8_t sync_B[32] = {0};

            const int critical_shapes[CRITICAL_SHAPES_NUM] = {22,23,24,25,32,33};   // This is a list of shapes that cause high CPU load, if both Oscillators are active we force the filter to SVF to save load
            inline bool shape_is_critical(int my_shape) { for(int i=0; i<CRITICAL_SHAPES_NUM; i++) if(my_shape==critical_shapes[i]) return(true); return false;}; // rescale incoming data to bool

            // --- Braids results and crossmodulation member variables ---
            float m_vol_A_incl_am = 0.f;
            float m_vol_B_incl_am = 0.f;

            float f_val_result = 0.f; // Result of DSP calculations...
            float f_val_result_1_A = 0.f;
            float f_val_result_2_A = 0.f;
            float f_val_result_1_B = 0.f;
            float f_val_result_2_B = 0.f;

            // --- Volume EG --
            ctagADEnv ad_eg;
            ctagADSREnv vol_eg_adsr;

            // --- LFOs incl. random sources ---
            ctagSineSource lfo[NUM_OF_LFOS];
            int t_lfoActive[NUM_OF_LFOS] = {0,0,0,0,0,0,0,0,0,0};         // LFO may be switched off
            int i_lfoDestination[NUM_OF_LFOS] = {0,0,0,0,0,0,0,0,0,0};    // Contains IDs of LFO destination
            int i_lfoType[NUM_OF_LFOS] = {0,0,0,0,0,0,0,0,0,0};           // LFO Wavetype, the last 5 LFOs can be modulated in tempo (LFOs 1-5 on GUI)
            float f_lfoSpeed[NUM_OF_LFOS] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};    // LFO speed
            float f_lfoAmnt[NUM_OF_LFOS] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};     // LFO amount
            float f_lfo_val[NUM_OF_LFOS] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};     // Lfo process result

            float morph_sine_wave(float sine_val, int morph_mode, int enum_sine);  // Morph sinewave to square, pseudo triangle, sample and hold and similar

            // --- Sample and Hold for LFOs ---
            float applySnH(float sine_lfo_val, int enum_val);
            enum snh_members
            {
                snh_lfo_6, snh_lfo_7, snh_lfo_8, snh_lfo_9, snh_lfo_10, snh_lfo_1, snh_lfo_2, snh_lfo_3, snh_lfo_4, snh_lfo_5, e_snh_max
            };
            ctagWNoiseGen oscSnH[e_snh_max];        // This is a list of objects, so we must not initilize it (again)
            float saved_sample[e_snh_max] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
            float previous_sine_val[e_snh_max] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

            // --- VULT Stuff ---
            Ladder__ctx_type_8 ladder_data;             // Ladder filter data-structure
            Svf__ctx_type_4 svf_data;                 // State variable filter

            // private attributes could go here
            // autogenerated code here
            // sectionHpp
			std::atomic<int32_t> Gate, trig_Gate;
			std::atomic<int32_t> Volume, cv_Volume;
			std::atomic<int32_t> xFadeOscAoscB, cv_xFadeOscAoscB;
			std::atomic<int32_t> osc_active_A, trig_osc_active_A;
			std::atomic<int32_t> shape_A, cv_shape_A;
			std::atomic<int32_t> vol_A, cv_vol_A;
			std::atomic<int32_t> pitch_A, cv_pitch_A;
			std::atomic<int32_t> tune_A, cv_tune_A;
			std::atomic<int32_t> timbre_A, cv_timbre_A;
			std::atomic<int32_t> color_A, cv_color_A;
			std::atomic<int32_t> xModActive_A, trig_xModActive_A;
			std::atomic<int32_t> am_xModB_A, cv_am_xModB_A;
			std::atomic<int32_t> freqX_factor_A, cv_freqX_factor_A;
			std::atomic<int32_t> freq_xModB_A, cv_freq_xModB_A;
			std::atomic<int32_t> timbre_xModB_A, cv_timbre_xModB_A;
			std::atomic<int32_t> color_xModB_A, cv_color_xModB_A;
			std::atomic<int32_t> osc_active_B, trig_osc_active_B;
			std::atomic<int32_t> shape_B, cv_shape_B;
			std::atomic<int32_t> vol_B, cv_vol_B;
			std::atomic<int32_t> pitch_B, cv_pitch_B;
			std::atomic<int32_t> tune_B, cv_tune_B;
			std::atomic<int32_t> timbre_B, cv_timbre_B;
			std::atomic<int32_t> color_B, cv_color_B;
			std::atomic<int32_t> xModActive_B, trig_xModActive_B;
			std::atomic<int32_t> am_xModA_B, cv_am_xModA_B;
			std::atomic<int32_t> freqX_factor_B, cv_freqX_factor_B;
			std::atomic<int32_t> freq_xModA_B, cv_freq_xModA_B;
			std::atomic<int32_t> timbre_xModA_B, cv_timbre_xModA_B;
			std::atomic<int32_t> color_xModA_B, cv_color_xModA_B;
			std::atomic<int32_t> enableFilter, trig_enableFilter;
			std::atomic<int32_t> filterIsSVF, trig_filterIsSVF;
			std::atomic<int32_t> WavefolderAmnt, cv_WavefolderAmnt;
			std::atomic<int32_t> Cutoff, cv_Cutoff;
			std::atomic<int32_t> Resonance, cv_Resonance;
			std::atomic<int32_t> ADfilterAmnt, cv_ADfilterAmnt;
			std::atomic<int32_t> enableADeg, trig_enableADeg;
			std::atomic<int32_t> ADegIsLowpassGate, trig_ADegIsLowpassGate;
			std::atomic<int32_t> attackADeg, cv_attackADeg;
			std::atomic<int32_t> decayADeg, cv_decayADeg;
			std::atomic<int32_t> loopADeg, trig_loopADeg;
			std::atomic<int32_t> enableVolADSReg, trig_enableVolADSReg;
			std::atomic<int32_t> attackVolADSReg, cv_attackVolADSReg;
			std::atomic<int32_t> decayVolADSReg, cv_decayVolADSReg;
			std::atomic<int32_t> sustainVolADSReg, cv_sustainVolADSReg;
			std::atomic<int32_t> releaseVolADSReg, cv_releaseVolADSReg;
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
			std::atomic<int32_t> lfoppActive_1, trig_lfoppActive_1;
			std::atomic<int32_t> lfoppDestination_1, cv_lfoppDestination_1;
			std::atomic<int32_t> lfoppType_1, cv_lfoppType_1;
			std::atomic<int32_t> lfoppSpeed_1, cv_lfoppSpeed_1;
			std::atomic<int32_t> lfoppAmnt_1, cv_lfoppAmnt_1;
			std::atomic<int32_t> lfoppActive_2, trig_lfoppActive_2;
			std::atomic<int32_t> lfoppDestination_2, cv_lfoppDestination_2;
			std::atomic<int32_t> lfoppType_2, cv_lfoppType_2;
			std::atomic<int32_t> lfoppSpeed_2, cv_lfoppSpeed_2;
			std::atomic<int32_t> lfoppAmnt_2, cv_lfoppAmnt_2;
			std::atomic<int32_t> lfoppActive_3, trig_lfoppActive_3;
			std::atomic<int32_t> lfoppDestination_3, cv_lfoppDestination_3;
			std::atomic<int32_t> lfoppType_3, cv_lfoppType_3;
			std::atomic<int32_t> lfoppSpeed_3, cv_lfoppSpeed_3;
			std::atomic<int32_t> lfoppAmnt_3, cv_lfoppAmnt_3;
			std::atomic<int32_t> lfoppActive_4, trig_lfoppActive_4;
			std::atomic<int32_t> lfoppDestination_4, cv_lfoppDestination_4;
			std::atomic<int32_t> lfoppType_4, cv_lfoppType_4;
			std::atomic<int32_t> lfoppSpeed_4, cv_lfoppSpeed_4;
			std::atomic<int32_t> lfoppAmnt_4, cv_lfoppAmnt_4;
			std::atomic<int32_t> lfoppActive_5, trig_lfoppActive_5;
			std::atomic<int32_t> lfoppDestination_5, cv_lfoppDestination_5;
			std::atomic<int32_t> lfoppType_5, cv_lfoppType_5;
			std::atomic<int32_t> lfoppSpeed_5, cv_lfoppSpeed_5;
			std::atomic<int32_t> lfoppAmnt_5, cv_lfoppAmnt_5;
			// sectionHpp
        };
    }
}