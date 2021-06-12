#include <atomic>
#include "ctagSoundProcessor.hpp"
#include "helpers/ctagSineSource.hpp"
#include "helpers/ctagWNoiseGen.hpp"
#include "helpers/ctagPNoiseGen.hpp"
#include "helpers/ctagFastMath.hpp"
#include "helpers/ctagADEnv.hpp"            // Needed for AD EG (Attack/Decay Envelope Generator)
#include "helpers/ctagADSREnv.hpp"          // Needed for ADSR EG (Attack/Decay/Sustain/Release Envelope Generator)
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

            // --- Remember status of triggers / buttons ---
            inline int process_param_trig( const ProcessData &data, int trig_myparm, int my_parm, int prev_trig_state_id, int gate_type ); // rescale incoming data to bool
            enum trig_states
            {
              e_Gate, e_LfoWTxFadeActive_1, e_LfoWTxFadeActive_2, e_LfoSAMPxFadeActive_1, e_LfoSAMPxFadeActive_2,
              e_SubOsc2VCF_A, e_SubOsc2VCF_C, e_ExclSubOSCmasterPitch, e_ExclWTmasterPitch, e_loop_D, e_loop_B,
              e_loop_pipo_B, e_PannerOn, e_FilterLFOsquare_A, e_EGvolGate, e_QuantizePitch, e_ExclSMPmasterPitch,
              e_StereoSplit, e_ScanWavTbl_A, e_ScanWavTbl_C, e_FreqModActive, e_FreqModExclWT, e_FreqModExclSample,
              e_FilterLFOon_A, e_FilterLFOon_C, e_loop_pipo_D, e_ModulateSubOscXfade_A, e_ModulateSubOscXfade_C,
              e_FilterLFOon_D, e_EGvolActive, e_FilterLFOsquare_C, e_FreqModExclSubOSC, e_FilterLFOon_B, e_SubOscPWM_A,
              e_SubOscPWM_C, e_EGvolSlow, e_EGvolADSRon, e_VctrSnt_options_max
            };
            int prev_trig_state[e_VctrSnt_options_max] = {0};   // Initialize _all_ entries with "low value"
            bool low_reached[e_VctrSnt_options_max] = {false};  // We need this for look for toggle-events

            // --- Sample and Hold for LFOs ---
            float applySnH(float sine_lfo_val, int enum_val);
            enum snh_members
            {
              e_XfadeWT_1, e_XfadeWT_2, e_XfadeSAMP_1, e_XfadeSAMP_2, e_WT_A, e_WT_C, e_PitchMod, e_filterLFO_A, e_filterLFO_C, e_snh_max
            };
            ctagWNoiseGen oscSnH[e_snh_max];        // This is a list of objects, so we must not initilize it (again)
            float saved_sample[e_snh_max] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
            float previous_sine_val[e_snh_max] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

            // --- Helper functions: convert wavetable to MI-format, morph waves ---
            void prepareWavetable(const int16_t **wavetables, int currentBank, bool *isWaveTableGood, float **fbuffer, int16_t **ibuffer );
            float morph_sine_wave(float sine_val, int morph_mode, int enum_sine);  // Morph sinewave to square, pseudo triangle, sample and hold and similar

            // --- Wavetable A ---
            ctagSampleRom sample_rom;
            plaits::WavetableOscillator<256, 64> wt_osc_A;
            int16_t *buffer_A = NULL;
            float *fbuffer_A = NULL;
            const int16_t *wavetables_A[64];
            int currentBank_A = 0;
            int lastBank_A = -1;
            bool isWaveTableGood_A = false;

            // --- Wavetable C ---
            plaits::WavetableOscillator<256, 64> wt_osc_C;
            int16_t *buffer_C = NULL;
            float *fbuffer_C = NULL;
            const int16_t *wavetables_C[64];
            int currentBank_C = 0;
            int lastBank_C = -1;
            bool isWaveTableGood_C = false;

            // --- Sample Oscillators B and D ---
            std::unique_ptr<RomplerVoice> romplers[2];
            float sample_buf_B[32];
            float sample_buf_D[32];
            uint32_t wtSliceOffset = 0;
            ctagSampleRom sampleRom;

            // --- Filters (for wavetables) ---
            stmlib::Svf svf_A;
            stmlib::Svf svf_C;

            // --- Volume EG --
            ctagADEnv vol_eg;
            ctagADSREnv vol_eg_adsr;

            // --- LFOs ---
            ctagSineSource lfoXfadeWT_1;
            ctagSineSource lfoXfadeWT_2;
            ctagSineSource lfoXfadeSample_1;
            ctagSineSource lfoXfadeSample_2;
            ctagSineSource lfoPanner;
            ctagSineSource lfoPitch;
            ctagSineSource lfo_A;
            ctagSineSource lfo_WT_A;
            ctagSineSource lfo_C;
            ctagSineSource lfo_WT_C;
            ctagSineSource lfoPWM;

            // --- Suboscillators ---
            ctagSineSource oscSub_A;
            ctagSineSource oscSub_C;
            ctagWNoiseGen oscWnoise_A;
            ctagWNoiseGen oscWnoise_C;
            ctagPNoiseGen oscPnoise_A;
            ctagPNoiseGen oscPnoise_C;

            // --- Random Source ---
            ctagWNoiseGen lfoRandom;  // We use this for some "analogue" modification for frequency modulation if needed

            // private attributes could go here
            // autogenerated code here
            // sectionHpp
	atomic<int32_t> Gate, trig_Gate;
	atomic<int32_t> EGvolGate, trig_EGvolGate;
	atomic<int32_t> MasterPitch, cv_MasterPitch;
	atomic<int32_t> MasterTune, cv_MasterTune;
	atomic<int32_t> QuantizePitch, trig_QuantizePitch;
	atomic<int32_t> Volume, cv_Volume;
	atomic<int32_t> ExclSubOSCmasterPitch, trig_ExclSubOSCmasterPitch;
	atomic<int32_t> ExclWTmasterPitch, trig_ExclWTmasterPitch;
	atomic<int32_t> ExclSMPmasterPitch, trig_ExclSMPmasterPitch;
	atomic<int32_t> PWMintensity, cv_PWMintensity;
	atomic<int32_t> PWMspeed, cv_PWMspeed;
	atomic<int32_t> SubOscPWM_A, trig_SubOscPWM_A;
	atomic<int32_t> SubOsc2VCF_A, trig_SubOsc2VCF_A;
	atomic<int32_t> PitchSubOsc_A, cv_PitchSubOsc_A;
	atomic<int32_t> SubOscFade_A, cv_SubOscFade_A;
	atomic<int32_t> SubOscPWM_C, trig_SubOscPWM_C;
	atomic<int32_t> SubOsc2VCF_C, trig_SubOsc2VCF_C;
	atomic<int32_t> PitchSubOsc_C, cv_PitchSubOsc_C;
	atomic<int32_t> SubOscFade_C, cv_SubOscFade_C;
	atomic<int32_t> VolWT_A, cv_VolWT_A;
	atomic<int32_t> VolOsc_B, cv_VolOsc_B;
	atomic<int32_t> VolWT_C, cv_VolWT_C;
	atomic<int32_t> VolOsc_D, cv_VolOsc_D;
	atomic<int32_t> StereoSplit, trig_StereoSplit;
	atomic<int32_t> XfadeWaveTbls, cv_XfadeWaveTbls;
	atomic<int32_t> XfadeSamples, cv_XfadeSamples;
	atomic<int32_t> LfoWTxFadeActive_1, trig_LfoWTxFadeActive_1;
	atomic<int32_t> LfoTypeWTxFade_1, cv_LfoTypeWTxFade_1;
	atomic<int32_t> LfoWTxFadeRange_1, cv_LfoWTxFadeRange_1;
	atomic<int32_t> LfoWTxFadeSpeed_1, cv_LfoWTxFadeSpeed_1;
	atomic<int32_t> LfoWTxFadeActive_2, trig_LfoWTxFadeActive_2;
	atomic<int32_t> ModulateSubOscXfade_A, trig_ModulateSubOscXfade_A;
	atomic<int32_t> LfoTypeWTxFade_2, cv_LfoTypeWTxFade_2;
	atomic<int32_t> LfoWTxFadeRange_2, cv_LfoWTxFadeRange_2;
	atomic<int32_t> LfoWTxFadeSpeed_2, cv_LfoWTxFadeSpeed_2;
	atomic<int32_t> LfoSAMPxFadeActive_1, trig_LfoSAMPxFadeActive_1;
	atomic<int32_t> LfoTypeSAMPxFade_1, cv_LfoTypeSAMPxFade_1;
	atomic<int32_t> LfoSAMPxFadeRange_1, cv_LfoSAMPxFadeRange_1;
	atomic<int32_t> LfoSAMPxFadeSpeed_1, cv_LfoSAMPxFadeSpeed_1;
	atomic<int32_t> LfoSAMPxFadeActive_2, trig_LfoSAMPxFadeActive_2;
	atomic<int32_t> ModulateSubOscXfade_C, trig_ModulateSubOscXfade_C;
	atomic<int32_t> LfoTypeSAMPxFade_2, cv_LfoTypeSAMPxFade_2;
	atomic<int32_t> LfoSAMPxFadeRange_2, cv_LfoSAMPxFadeRange_2;
	atomic<int32_t> LfoSAMPxFadeSpeed_2, cv_LfoSAMPxFadeSpeed_2;
	atomic<int32_t> WaveTblA, cv_WaveTblA;
	atomic<int32_t> ScanWavTblA, cv_ScanWavTblA;
	atomic<int32_t> pitch_A, cv_pitch_A;
	atomic<int32_t> tune_A, cv_tune_A;
	atomic<int32_t> ScanWavTbl_A, trig_ScanWavTbl_A;
	atomic<int32_t> LFOzScanType_A, cv_LFOzScanType_A;
	atomic<int32_t> LFOzScanAmt_A, cv_LFOzScanAmt_A;
	atomic<int32_t> LFOzScanSpeed_A, cv_LFOzScanSpeed_A;
	atomic<int32_t> fmode_A, cv_fmode_A;
	atomic<int32_t> fcut_A, cv_fcut_A;
	atomic<int32_t> freso_A, cv_freso_A;
	atomic<int32_t> FilterLFOon_A, trig_FilterLFOon_A;
	atomic<int32_t> LFOfilterType_A, cv_LFOfilterType_A;
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
	atomic<int32_t> lfo2filtfm_B, cv_lfo2filtfm_B;
	atomic<int32_t> lfospeed_B, cv_lfospeed_B;
	atomic<int32_t> WaveTblC, cv_WaveTblC;
	atomic<int32_t> ScanWavTblC, cv_ScanWavTblC;
	atomic<int32_t> pitch_C, cv_pitch_C;
	atomic<int32_t> tune_C, cv_tune_C;
	atomic<int32_t> ScanWavTbl_C, trig_ScanWavTbl_C;
	atomic<int32_t> LFOzScanType_C, cv_LFOzScanType_C;
	atomic<int32_t> LFOzScanAmt_C, cv_LFOzScanAmt_C;
	atomic<int32_t> LFOzScanSpeed_C, cv_LFOzScanSpeed_C;
	atomic<int32_t> fmode_C, cv_fmode_C;
	atomic<int32_t> fcut_C, cv_fcut_C;
	atomic<int32_t> freso_C, cv_freso_C;
	atomic<int32_t> FilterLFOon_C, trig_FilterLFOon_C;
	atomic<int32_t> LFOfilterType_C, cv_LFOfilterType_C;
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
	atomic<int32_t> lfo2filtfm_D, cv_lfo2filtfm_D;
	atomic<int32_t> lfospeed_D, cv_lfospeed_D;
	atomic<int32_t> FreqModActive, trig_FreqModActive;
	atomic<int32_t> FreqModExclSubOSC, trig_FreqModExclSubOSC;
	atomic<int32_t> FreqModExclWT, trig_FreqModExclWT;
	atomic<int32_t> FreqModExclSample, trig_FreqModExclSample;
	atomic<int32_t> FreqModType, cv_FreqModType;
	atomic<int32_t> FreqModAmnt, cv_FreqModAmnt;
	atomic<int32_t> FreqModSpeed, cv_FreqModSpeed;
	atomic<int32_t> FreqModAnalog, cv_FreqModAnalog;
	atomic<int32_t> EGvolActive, trig_EGvolActive;
	atomic<int32_t> EGvolSlow, trig_EGvolSlow;
	atomic<int32_t> AttackVol, cv_AttackVol;
	atomic<int32_t> DecayVol, cv_DecayVol;
	atomic<int32_t> EGvolADSRon, trig_EGvolADSRon;
	atomic<int32_t> SustainVol, cv_SustainVol;
	atomic<int32_t> ReleaseVol, cv_ReleaseVol;
	atomic<int32_t> PannerOn, trig_PannerOn;
	atomic<int32_t> PanAmnt, cv_PanAmnt;
	atomic<int32_t> PanFreq, cv_PanFreq;
	// sectionHpp
        };
    }
}