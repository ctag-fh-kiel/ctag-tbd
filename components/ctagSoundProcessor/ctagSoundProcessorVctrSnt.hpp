#include <atomic>
#include "ctagSoundProcessor.hpp"
#include "helpers/ctagSineSource.hpp"
#include "helpers/ctagWNoiseGen.hpp"
#include "helpers/ctagFastMath.hpp"
#include "helpers/ctagADEnv.hpp"            // Needed for AD EG (Attack/Decay Enveloppe Generator)
#include "helpers/ctagSampleRom.hpp"
#include "helpers/ctagSineSource.hpp"
#include "helpers/ctagADSREnv.hpp"
#include "plaits/dsp/oscillator/wavetable_oscillator.h"
#include "stmlib/dsp/filter.h"
#include "synthesis/RomplerVoice.hpp"
#include "helpers/ctagSampleRom.hpp"
#include <memory>
#include <vector>

using namespace CTAG::SP::HELPERS;
using namespace CTAG::SYNTHESIS;

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorVctrSnt : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;
            ctagSoundProcessorVctrSnt();
            virtual ~ctagSoundProcessorVctrSnt();

        private:
            virtual void knowYourself() override;

            inline int process_param_trig( const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id ); // rescale incoming data to bool
            inline float process_param_float( const ProcessData &data, int cv_myparm, int my_parm, float out_min = 0.f, float out_max = 1.f, bool exponential = false ); // rescale incoming data to 0.0-1.0

            // Remember status of triggers / buttons
            enum trig_states {e_Gate, e_ModWaveTblsXfade, e_ModSamplesXfade, e_ScanWavTblAauto, e_ScanWavTblCauto, e_FreqModActive,
                              e_FreqModSquare, e_EGvolActive, e_EGvolGate, e_loop_D, e_loop_B, e_loop_pipo_D, e_loop_pipo_B,
                              e_LfoWaveTblsXfadeActive, e_LfoWaveTblsXfadeIsSquare, e_ScanWavTbl_A, e_ScanWavTbl_C,
                              e_PannerOn, e_FilterLFOsquare_A, e_FilterLFOsquare_C, e_FilterLFOon_A, e_FilterLFOon_C,
                              e_APC_options_max};
            int prev_trig_state[e_APC_options_max] = {0};   // Initialize _all_ entries with "low value"

            inline float noteToFreq(float incoming_note) { return  (HELPERS::fastpow2 ((incoming_note - 69.f) / 12.f) *440.f); } // MIDItoFrequency, inspired by: https://github.com/little-scale/mtof/blob/master/mtof.cpp

            void prepareWavetable(const int16_t **wavetables, int currentBank, bool *isWaveTableGood, float **fbuffer, int16_t **ibuffer );

            // --- Wavetable A ---
            ctagSampleRom sample_rom;
            plaits::WavetableOscillator<256, 64> wt_osc_A;
            int16_t *buffer_A = NULL;
            float *fbuffer_A = NULL;
            const int16_t *wavetables_A[64];
            int currentBank_A = 0;
            int lastBank_A = -1;
            bool isWaveTableGood_A = false;
            stmlib::Svf svf_A;
            ctagSineSource lfo_A;

            // --- Wavetable C ---
            plaits::WavetableOscillator<256, 64> wt_osc_C;
            int16_t *buffer_C = NULL;
            float *fbuffer_C = NULL;
            const int16_t *wavetables_C[64];
            int currentBank_C = 0;
            int lastBank_C = -1;
            bool isWaveTableGood_C = false;
            stmlib::Svf svf_C;
            ctagSineSource lfo_C;

            // --- Sample Oscillators B and D ---
            std::unique_ptr<RomplerVoice> romplers[2];
            float sample_buf_B[32];
            float sample_buf_D[32];
            uint32_t wtSliceOffset = 0;
            ctagSampleRom sampleRom;

            // --- EGs --
            ctagADEnv vol_eg;
            ctagADEnv xFadeSamples_eg;
            ctagADEnv xFadeWTs_eg;

            // --- LFOs ---
            ctagSineSource lfoXfadeWTs;
            ctagSineSource lfoXfadeSamples;
            ctagSineSource lfoPanner;
            ctagSineSource lfoPitch;

            // --- Random Source ---
            ctagWNoiseGen lfoRandom;

            ctagWNoiseGen oscWnoise_A;
            ctagWNoiseGen oscWnoise_B;

            // private attributes could go here
            // autogenerated code here
            // sectionHpp
	atomic<int32_t> Gate, trig_Gate;
	atomic<int32_t> EGvolGate, trig_EGvolGate;
	atomic<int32_t> MasterTune, cv_MasterTune;
	atomic<int32_t> Volume, cv_Volume;
	atomic<int32_t> SubOscPWM_A, trig_SubOscPWM_A;
	atomic<int32_t> SubOscFade_A, cv_SubOscFade_A;
	atomic<int32_t> SubOscSquare_C, trig_SubOscSquare_C;
	atomic<int32_t> SubOscFade_C, cv_SubOscFade_C;
	atomic<int32_t> VolWT_A, cv_VolWT_A;
	atomic<int32_t> VolOsc_B, cv_VolOsc_B;
	atomic<int32_t> VolWT_C, cv_VolWT_C;
	atomic<int32_t> VolOsc_D, cv_VolOsc_D;
	atomic<int32_t> XfadeWaveTbls, cv_XfadeWaveTbls;
	atomic<int32_t> XfadeSamples, cv_XfadeSamples;
	atomic<int32_t> LfoWaveTblsXfadeActive, trig_LfoWaveTblsXfadeActive;
	atomic<int32_t> LfoWaveTblsXfadeIsSquare, trig_LfoWaveTblsXfadeIsSquare;
	atomic<int32_t> LfoWaveTblsXfadeRange, cv_LfoWaveTblsXfadeRange;
	atomic<int32_t> LfoWaveTblsXfadeSpeed, cv_LfoWaveTblsXfadeSpeed;
	atomic<int32_t> ModWaveTblsXfade, trig_ModWaveTblsXfade;
	atomic<int32_t> AttackWaveTblsXfd, cv_AttackWaveTblsXfd;
	atomic<int32_t> DecayWaveTblsXfd, cv_DecayWaveTblsXfd;
	atomic<int32_t> LfoSampleOscXfadeActive, trig_LfoSampleOscXfadeActive;
	atomic<int32_t> LfoSampleOscIsSquare, trig_LfoSampleOscIsSquare;
	atomic<int32_t> LfoSampleOscXfadeRange, cv_LfoSampleOscXfadeRange;
	atomic<int32_t> LfoSampleOscXfadeSpeed, cv_LfoSampleOscXfadeSpeed;
	atomic<int32_t> ModSamplesXfade, trig_ModSamplesXfade;
	atomic<int32_t> AttackSamplesXfd, cv_AttackSamplesXfd;
	atomic<int32_t> DecaySamplesXfd, cv_DecaySamplesXfd;
	atomic<int32_t> WaveTblA, cv_WaveTblA;
	atomic<int32_t> ScanWavTblA, cv_ScanWavTblA;
	atomic<int32_t> pitch_A, cv_pitch_A;
	atomic<int32_t> tune_A, cv_tune_A;
	atomic<int32_t> ScanWavTbl_A, trig_ScanWavTbl_A;
	atomic<int32_t> LFOzScanSquare_A, trig_LFOzScanSquare_A;
	atomic<int32_t> LFOzScanAmt_A, cv_LFOzScanAmt_A;
	atomic<int32_t> LFOzScanSpeed_A, cv_LFOzScanSpeed_A;
	atomic<int32_t> fmode_A, cv_fmode_A;
	atomic<int32_t> fcut_A, cv_fcut_A;
	atomic<int32_t> freso_A, cv_freso_A;
	atomic<int32_t> FilterLFOon_A, trig_FilterLFOon_A;
	atomic<int32_t> FilterLFOsquare_A, trig_FilterLFOsquare_A;
	atomic<int32_t> lfo2filtfm_A, cv_lfo2filtfm_A;
	atomic<int32_t> lfospeed_A, cv_lfospeed_A;
	atomic<int32_t> bank_B, cv_bank_B;
	atomic<int32_t> slice_B, cv_slice_B;
	atomic<int32_t> speed_B, cv_speed_B;
	atomic<int32_t> pitch_B, cv_pitch_B;
	atomic<int32_t> tune_B, cv_tune_B;
	atomic<int32_t> start_B, cv_start_B;
	atomic<int32_t> length_B, cv_length_B;
	atomic<int32_t> loop_B, trig_loop_B;
	atomic<int32_t> loop_pipo_B, trig_loop_pipo_B;
	atomic<int32_t> lpstart_B, cv_lpstart_B;
	atomic<int32_t> fmode_B, cv_fmode_B;
	atomic<int32_t> fcut_B, cv_fcut_B;
	atomic<int32_t> freso_B, cv_freso_B;
	atomic<int32_t> FilterLFOon_B, trig_FilterLFOon_B;
	atomic<int32_t> FilterLFOsquare_B, trig_FilterLFOsquare_B;
	atomic<int32_t> lfo2filtfm_B, cv_lfo2filtfm_B;
	atomic<int32_t> lfospeed_B, cv_lfospeed_B;
	atomic<int32_t> WaveTblC, cv_WaveTblC;
	atomic<int32_t> ScanWavTblC, cv_ScanWavTblC;
	atomic<int32_t> pitch_C, cv_pitch_C;
	atomic<int32_t> tune_C, cv_tune_C;
	atomic<int32_t> ScanWavTbl_C, trig_ScanWavTbl_C;
	atomic<int32_t> LFOzScanSquare_C, trig_LFOzScanSquare_C;
	atomic<int32_t> LFOzScanAmt_C, cv_LFOzScanAmt_C;
	atomic<int32_t> LFOzScanSpeed_C, cv_LFOzScanSpeed_C;
	atomic<int32_t> fmode_C, cv_fmode_C;
	atomic<int32_t> fcut_C, cv_fcut_C;
	atomic<int32_t> freso_C, cv_freso_C;
	atomic<int32_t> FilterLFOon_C, trig_FilterLFOon_C;
	atomic<int32_t> FilterLFOsquare_C, trig_FilterLFOsquare_C;
	atomic<int32_t> lfo2filtfm_C, cv_lfo2filtfm_C;
	atomic<int32_t> lfospeed_C, cv_lfospeed_C;
	atomic<int32_t> bank_D, cv_bank_D;
	atomic<int32_t> slice_D, cv_slice_D;
	atomic<int32_t> speed_D, cv_speed_D;
	atomic<int32_t> pitch_D, cv_pitch_D;
	atomic<int32_t> tune_D, cv_tune_D;
	atomic<int32_t> start_D, cv_start_D;
	atomic<int32_t> length_D, cv_length_D;
	atomic<int32_t> loop_D, trig_loop_D;
	atomic<int32_t> loop_pipo_D, trig_loop_pipo_D;
	atomic<int32_t> lpstart_D, cv_lpstart_D;
	atomic<int32_t> fmode_D, cv_fmode_D;
	atomic<int32_t> fcut_D, cv_fcut_D;
	atomic<int32_t> freso_D, cv_freso_D;
	atomic<int32_t> FilterLFOon_D, trig_FilterLFOon_D;
	atomic<int32_t> FilterLFOsquare_D, trig_FilterLFOsquare_D;
	atomic<int32_t> lfo2filtfm_D, cv_lfo2filtfm_D;
	atomic<int32_t> lfospeed_D, cv_lfospeed_D;
	atomic<int32_t> FreqModActive, trig_FreqModActive;
	atomic<int32_t> FreqModSquare, trig_FreqModSquare;
	atomic<int32_t> FreqModAmnt, cv_FreqModAmnt;
	atomic<int32_t> FreqModSpeed, cv_FreqModSpeed;
	atomic<int32_t> FreqModAnalog, cv_FreqModAnalog;
	atomic<int32_t> EGvolActive, trig_EGvolActive;
	atomic<int32_t> AttackVol, cv_AttackVol;
	atomic<int32_t> DecayVol, cv_DecayVol;
	atomic<int32_t> PannerOn, trig_PannerOn;
	atomic<int32_t> PanAmnt, cv_PanAmnt;
	atomic<int32_t> PanFreq, cv_PanFreq;
	// sectionHpp
        };
    }
}