#include <atomic>
#include "ctagSoundProcessor.hpp"
#include "helpers/ctagSineSource.hpp"
#include "helpers/ctagFastMath.hpp"
#include "helpers/ctagADEnv.hpp"            // Needed for AD EG (Attack/Decay Enveloppe Generator)

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorAPCpp : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;
            ctagSoundProcessorAPCpp();
            virtual ~ctagSoundProcessorAPCpp();

        private:
            virtual void knowYourself() override;
            inline int process_param_trig( const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id ); // rescale incoming data to bool
            inline float process_param_float( const ProcessData &data, int cv_myparm, int my_parm, float out_min = 0.f, float out_max = 1.f ); // rescale incoming data to 0.0-1.0

            HELPERS::ctagSineSource osc_1;  // Main Oscillators
            HELPERS::ctagSineSource osc_2;

            HELPERS::ctagSineSource pwm_1;  // LFOs for Pulse Width Modulation
            HELPERS::ctagSineSource pwm_2;

            HELPERS::ctagSineSource fm_1;   // LFOs for Frequency Modulation
            HELPERS::ctagSineSource fm_2;

            HELPERS::ctagADEnv vol_env;     // Envelope

            // Remember status of triggers / buttons
            enum trig_states {e_MOD_active_1, e_MOD_active_2, e_FreqmodSquare_active_1, e_FreqmodSquare_active_2, e_MOD_is_PWM_1, e_MOD_is_PWM_2,
                              e_SmoothOSC_1, e_SmoothOSC_2, e_Env_active, e_Trigger_env, e_Env_loop_active, e_APC_options_max };
            int prev_trig_state[e_APC_options_max] = {0,0,0,0,0,0,0,0,0,0,0};

            inline float noteToFreq(float incoming_note) { return  (HELPERS::fastpow2 ((incoming_note - 69.f) / 12.f) *440.f); } // MIDItoFrequency, inspired by: https://github.com/little-scale/mtof/blob/master/mtof.cpp

            // sectionHpp
            atomic<int32_t> MOD_freq_1, cv_MOD_freq_1;
            atomic<int32_t> Freq_1, cv_Freq_1;
            atomic<int32_t> MOD_active_1, trig_MOD_active_1;
            atomic<int32_t> MOD_is_PWM_1, trig_MOD_is_PWM_1;
            atomic<int32_t> SmoothOSC_1, trig_SmoothOSC_1;
            atomic<int32_t> SmoothOSC_2, trig_SmoothOSC_2;
            atomic<int32_t> MOD_is_PWM_2, trig_MOD_is_PWM_2;
            atomic<int32_t> MOD_active_2, trig_MOD_active_2;
            atomic<int32_t> Freq_2, cv_Freq_2;
            atomic<int32_t> MOD_freq_2, cv_MOD_freq_2;
            atomic<int32_t> Freqmod_amount_1, cv_Freqmod_amount_1;
            atomic<int32_t> Freqmod_freq_1, cv_Freqmod_freq_1;
            atomic<int32_t> FreqmodSquare_active_1, trig_FreqmodSquare_active_1;
            atomic<int32_t> FreqmodSquare_active_2, trig_FreqmodSquare_active_2;
            atomic<int32_t> Freqmod_freq_2, cv_Freqmod_freq_2;
            atomic<int32_t> Freqmod_amount_2, cv_Freqmod_amount_2;
            atomic<int32_t> Vol_amount, cv_Vol_amount;
            atomic<int32_t> Env_active, trig_Env_active;
            atomic<int32_t> Trigger_env, trig_Trigger_env;
            atomic<int32_t> Env_Attack, cv_Env_Attack;
            atomic<int32_t> Env_Decay, cv_Env_Decay;
            atomic<int32_t> Env_loop_active, trig_Env_loop_active;
	          // sectionHpp
        };
    }
}