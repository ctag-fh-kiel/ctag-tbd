#include <atomic>
#include "ctagSoundProcessor.hpp"
#include "plaits/dsp/drums/analog_bass_drum.h"
#include "plaits/dsp/drums/analog_snare_drum.h"
#include "plaits/dsp/drums/synthetic_bass_drum.h"
#include "plaits/dsp/drums/synthetic_snare_drum.h"
#include "plaits/dsp/drums/hi_hat.h"
#include "braids/analog_oscillator.h"
#include "braids/signature_waveshaper.h"
#include "braids/macro_oscillator.h"
#include "braids/settings.h"
#include "braids/quantizer.h"
#include "filters/ctagDiodeLadderFilter.hpp"
#include "filters/ctagDiodeLadderFilter2.hpp"
#include "filters/ctagDiodeLadderFilter3.hpp"
#include "filters/ctagDiodeLadderFilter4.hpp"
#include "filters/ctagDiodeLadderFilter5.hpp"
#include "filters/ctagFilterBase.hpp"
#include "synthesis/RomplerVoiceMinimal.hpp"
#include "synthesis/Clap.hpp"
#include "synthesis/Rimshot.hpp"
#include "synthesis/FmKick.hpp"
#include "helpers/ctagSampleRom.hpp"
#include "helpers/ctagADEnv.hpp"
#include "SimpleComp/SimpleComp.h"
#include "mifx/reverb.h"
#include "polypad/ChordSynth.hpp"

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorDrumRack : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;
            // no ctor, use Init() instead, is called from factory after successful creation
            virtual void Init(std::size_t blockSize, void *blockPtr) override;
            virtual ~ctagSoundProcessorDrumRack();

        private:
            virtual void knowYourself() override;

            // compressor
            chunkware_simple::SimpleComp sumCompressor;
            float fCompMUPGain_pre {0.f};
            float side_l {0.f};
            float side_r {0.f};

            // plaits drum models
            plaits::AnalogBassDrum abd;
            plaits::AnalogSnareDrum asd;
            plaits::SyntheticBassDrum dbd;
            plaits::SyntheticSnareDrum dsd;
            plaits::HiHat<plaits::SquareNoise, plaits::SwingVCA, true, false> hh1;
            plaits::HiHat<plaits::RingModNoise, plaits::LinearVCA, false, true> hh2;

        	// own drum models
        	CTAG::SYNTHESIS::Clap cl;
        	CTAG::SYNTHESIS::Rimshot rs;
            CTAG::SYNTHESIS::FmKick fmb;
  
			float abd_out[32];
			float asd_out[32];
			float dbd_out[32];
			float dsd_out[32];
			float fmb_out[32];
			float hh1_out[32];
			float hh2_out[32];
			float cl_out[32];
			float rs_out[32];
			float s2_out[32];
			float s1_out[32];
			float s3_out[32];
			float s4_out[32];

  
			// td003
			// ctagDiodeLadderFilter5 td3_pirkle_zdf_boost; // Pirkle ZDF with boost
            // ctagDiodeLadderFilter3 td3_karlson; // Karlson
            // ctagDiodeLadderFilter4 td3_blaukraut; // Blaukraut
            // ctagDiodeLadderFilter td3_pirkle_zdf; // Pirkle ZDF
            // ctagDiodeLadderFilter2 td3_zavalishin; // Zavalishin ZDF
            // ctagADEnv td3_adVCA, td3_adVCF;
            // braids::MacroOscillator td3_osc;
            // braids::SignatureWaveshaper td3_ws;
            // uint8_t td3_sync[32] = {0};
            // bool td3_pre_trig = false;
            // bool td3_isAccent = false;
            // float td3_pre_eg_val = 0.f;
            // float td3_pre_pitch_val = 0.f;

			// polypad
			// array<ChordSynth, 8> pp_v_voices;
            // bool pp_latchVoice = false;
            // bool pp_latched = false;
            // bool pp_toggle = false;
            // int32_t pp_preNCVoices = 0;
            // braids::Quantizer pp_quantizer;

			// macro oscillator
            // braids::MacroOscillator mo_osc;
            // braids::SignatureWaveshaper mo_ws;
            // braids::Quantizer mo_quantizer;
            // CTAG::SP::HELPERS::ctagADEnv mo_envelope;
            // const uint8_t mo_sync[32] = {0};
            // bool mo_prevTrigger = false;
            // const uint16_t mo_bit_reduction_masks[7] = {
            //         0xc000,
            //         0xe000,
            //         0xf000,
            //         0xf800,
            //         0xff00,
            //         0xfff0,
            //         0xffff};

			void renderABD(const ProcessData& data);
			void renderASD(const ProcessData& data);
			void renderDBD(const ProcessData& data);
			void renderDSD(const ProcessData& data);
			void renderFMB(const ProcessData& data);
			void renderHH1(const ProcessData& data);
			void renderHH2(const ProcessData& data);
			void renderCL(const ProcessData& data);
			void renderRS(const ProcessData& data);

			void renderS1(const ProcessData& data);
			void renderS2(const ProcessData& data);
			void renderS3(const ProcessData& data);
			void renderS4(const ProcessData& data);

			void renderIN(const ProcessData& data);

			void renderTD3(const ProcessData& data);
			void renderPP(const ProcessData& data);
			void renderMO(const ProcessData& data);

			void mixRenderOutputMono(float *source, float level, float pan, float fx1, float fx2);
			void mixRenderOutputStereo(float *source, float level, float pan, float fx1, float fx2);

			void preprocessFX1(const ProcessData& data);
			void preprocessFX2(const ProcessData& data);
			void preprocessMaster(const ProcessData& data);

			void renderMasterOutput(const ProcessData& data);

            // delay
            float *delayBuffer_l, *delayBuffer_r;
            const uint32_t delayBufferSizeMax {88200};
            uint32_t writeIndex {0};
            float readPos {0.0f}, readPosFiltered {0.0f};
            float delayOffset {0.0f};
            float duck {0.f};
            float delayTime_ms {0.0f};
            bool pre_sync {false};
            float fDelayTime {0.0f};
            float fSyncTimeStamp {0.0f};
            int32_t timer {0}, pre_timer {0};
            stmlib::OnePole lp_l, hp_l;
            stmlib::OnePole lp_r, hp_r;

            // reverb
            mifx::Reverb reverb;

            bool abd_trig_prev {false};
            bool asd_trig_prev {false};
            bool dbd_trig_prev {false};
            bool dsd_trig_prev {false};
            bool hh1_trig_prev {false};
            bool hh2_trig_prev {false};
        	bool rs_trig_prev {false};
        	bool cl_trig_prev {false};
            bool fmb_trig_prev {false};

        	float combined_out[32*2];
        	float send1_out[32*2];
        	float send2_out[32*2];
            float temp1_[32];
            float temp2_[32];

            // rompler
            CTAG::SYNTHESIS::RomplerVoiceMinimal rompler[4];
            CTAG::SP::HELPERS::ctagSampleRom sampleRom;

            // private attributes could go here
            // autogenerated code here
            // sectionHpp
	atomic<int32_t> ab_trigger, trig_ab_trigger;
	atomic<int32_t> ab_mute, trig_ab_mute;
	atomic<int32_t> ab_lev, cv_ab_lev;
	atomic<int32_t> ab_pan, cv_ab_pan;
	atomic<int32_t> ab_fx1, cv_ab_fx1;
	atomic<int32_t> ab_fx2, cv_ab_fx2;
	atomic<int32_t> ab_accent, cv_ab_accent;
	atomic<int32_t> ab_f0, cv_ab_f0;
	atomic<int32_t> ab_tone, cv_ab_tone;
	atomic<int32_t> ab_decay, cv_ab_decay;
	atomic<int32_t> ab_a_fm, cv_ab_a_fm;
	atomic<int32_t> ab_s_fm, cv_ab_s_fm;
	atomic<int32_t> db_trigger, trig_db_trigger;
	atomic<int32_t> db_mute, trig_db_mute;
	atomic<int32_t> db_lev, cv_db_lev;
	atomic<int32_t> db_pan, cv_db_pan;
	atomic<int32_t> db_fx1, cv_db_fx1;
	atomic<int32_t> db_fx2, cv_db_fx2;
	atomic<int32_t> db_accent, cv_db_accent;
	atomic<int32_t> db_f0, cv_db_f0;
	atomic<int32_t> db_tone, cv_db_tone;
	atomic<int32_t> db_decay, cv_db_decay;
	atomic<int32_t> db_dirty, cv_db_dirty;
	atomic<int32_t> db_fm_env, cv_db_fm_env;
	atomic<int32_t> db_fm_dcy, cv_db_fm_dcy;
	atomic<int32_t> fmb_trigger, trig_fmb_trigger;
	atomic<int32_t> fmb_mute, trig_fmb_mute;
	atomic<int32_t> fmb_lev, cv_fmb_lev;
	atomic<int32_t> fmb_pan, cv_fmb_pan;
	atomic<int32_t> fmb_fx1, cv_fmb_fx1;
	atomic<int32_t> fmb_fx2, cv_fmb_fx2;
	atomic<int32_t> fmb_use_ratio_mode, trig_fmb_use_ratio_mode;
	atomic<int32_t> fmb_mod_env_sync, trig_fmb_mod_env_sync;
	atomic<int32_t> fmb_f_b, cv_fmb_f_b;
	atomic<int32_t> fmb_d_b, cv_fmb_d_b;
	atomic<int32_t> fmb_f_m, cv_fmb_f_m;
	atomic<int32_t> fmb_I, cv_fmb_I;
	atomic<int32_t> fmb_d_m, cv_fmb_d_m;
	atomic<int32_t> fmb_b_m, cv_fmb_b_m;
	atomic<int32_t> fmb_A_f, cv_fmb_A_f;
	atomic<int32_t> fmb_d_f, cv_fmb_d_f;
	atomic<int32_t> as_trigger, trig_as_trigger;
	atomic<int32_t> as_mute, trig_as_mute;
	atomic<int32_t> as_lev, cv_as_lev;
	atomic<int32_t> as_pan, cv_as_pan;
	atomic<int32_t> as_fx1, cv_as_fx1;
	atomic<int32_t> as_fx2, cv_as_fx2;
	atomic<int32_t> as_accent, cv_as_accent;
	atomic<int32_t> as_f0, cv_as_f0;
	atomic<int32_t> as_tone, cv_as_tone;
	atomic<int32_t> as_decay, cv_as_decay;
	atomic<int32_t> as_a_spy, cv_as_a_spy;
	atomic<int32_t> ds_trigger, trig_ds_trigger;
	atomic<int32_t> ds_mute, trig_ds_mute;
	atomic<int32_t> ds_lev, cv_ds_lev;
	atomic<int32_t> ds_pan, cv_ds_pan;
	atomic<int32_t> ds_fx1, cv_ds_fx1;
	atomic<int32_t> ds_fx2, cv_ds_fx2;
	atomic<int32_t> ds_accent, cv_ds_accent;
	atomic<int32_t> ds_f0, cv_ds_f0;
	atomic<int32_t> ds_fm_amt, cv_ds_fm_amt;
	atomic<int32_t> ds_decay, cv_ds_decay;
	atomic<int32_t> ds_spy, cv_ds_spy;
	atomic<int32_t> hh1_trigger, trig_hh1_trigger;
	atomic<int32_t> hh1_mute, trig_hh1_mute;
	atomic<int32_t> hh1_lev, cv_hh1_lev;
	atomic<int32_t> hh1_pan, cv_hh1_pan;
	atomic<int32_t> hh1_fx1, cv_hh1_fx1;
	atomic<int32_t> hh1_fx2, cv_hh1_fx2;
	atomic<int32_t> hh1_accent, cv_hh1_accent;
	atomic<int32_t> hh1_f0, cv_hh1_f0;
	atomic<int32_t> hh1_tone, cv_hh1_tone;
	atomic<int32_t> hh1_decay, cv_hh1_decay;
	atomic<int32_t> hh1_noise, cv_hh1_noise;
	atomic<int32_t> hh2_trigger, trig_hh2_trigger;
	atomic<int32_t> hh2_mute, trig_hh2_mute;
	atomic<int32_t> hh2_lev, cv_hh2_lev;
	atomic<int32_t> hh2_pan, cv_hh2_pan;
	atomic<int32_t> hh2_fx1, cv_hh2_fx1;
	atomic<int32_t> hh2_fx2, cv_hh2_fx2;
	atomic<int32_t> hh2_accent, cv_hh2_accent;
	atomic<int32_t> hh2_f0, cv_hh2_f0;
	atomic<int32_t> hh2_tone, cv_hh2_tone;
	atomic<int32_t> hh2_decay, cv_hh2_decay;
	atomic<int32_t> hh2_noise, cv_hh2_noise;
	atomic<int32_t> rs_trigger, trig_rs_trigger;
	atomic<int32_t> rs_mute, trig_rs_mute;
	atomic<int32_t> rs_lev, cv_rs_lev;
	atomic<int32_t> rs_pan, cv_rs_pan;
	atomic<int32_t> rs_fx1, cv_rs_fx1;
	atomic<int32_t> rs_fx2, cv_rs_fx2;
	atomic<int32_t> rs_accent, cv_rs_accent;
	atomic<int32_t> rs_f0, cv_rs_f0;
	atomic<int32_t> rs_tone, cv_rs_tone;
	atomic<int32_t> rs_decay, cv_rs_decay;
	atomic<int32_t> rs_noise, cv_rs_noise;
	atomic<int32_t> cl_trigger, trig_cl_trigger;
	atomic<int32_t> cl_mute, trig_cl_mute;
	atomic<int32_t> cl_lev, cv_cl_lev;
	atomic<int32_t> cl_pan, cv_cl_pan;
	atomic<int32_t> cl_fx1, cv_cl_fx1;
	atomic<int32_t> cl_fx2, cv_cl_fx2;
	atomic<int32_t> cl_f0, cv_cl_f0;
	atomic<int32_t> cl_tone, cv_cl_tone;
	atomic<int32_t> cl_decay, cv_cl_decay;
	atomic<int32_t> cl_scale, cv_cl_scale;
	atomic<int32_t> cl_transient, cv_cl_transient;
	atomic<int32_t> s1_gate, trig_s1_gate;
	atomic<int32_t> s1_mute, trig_s1_mute;
	atomic<int32_t> s1_lev, cv_s1_lev;
	atomic<int32_t> s1_pan, cv_s1_pan;
	atomic<int32_t> s1_fx1, cv_s1_fx1;
	atomic<int32_t> s1_fx2, cv_s1_fx2;
	atomic<int32_t> s1_speed, cv_s1_speed;
	atomic<int32_t> s1_pitch, cv_s1_pitch;
	atomic<int32_t> s1_bank, cv_s1_bank;
	atomic<int32_t> s1_slice, cv_s1_slice;
	atomic<int32_t> s1_start, cv_s1_start;
	atomic<int32_t> s1_end, cv_s1_end;
	atomic<int32_t> s1_lp, trig_s1_lp;
	atomic<int32_t> s1_lp_pp, trig_s1_lp_pp;
	atomic<int32_t> s1_lp_pos, cv_s1_lp_pos;
	atomic<int32_t> s1_atk, cv_s1_atk;
	atomic<int32_t> s1_dcy, cv_s1_dcy;
	atomic<int32_t> s1_eg2fm, cv_s1_eg2fm;
	atomic<int32_t> s1_brr, cv_s1_brr;
	atomic<int32_t> s1_ft, cv_s1_ft;
	atomic<int32_t> s1_fc, cv_s1_fc;
	atomic<int32_t> s1_fq, cv_s1_fq;
	atomic<int32_t> s2_gate, trig_s2_gate;
	atomic<int32_t> s2_mute, trig_s2_mute;
	atomic<int32_t> s2_lev, cv_s2_lev;
	atomic<int32_t> s2_pan, cv_s2_pan;
	atomic<int32_t> s2_fx1, cv_s2_fx1;
	atomic<int32_t> s2_fx2, cv_s2_fx2;
	atomic<int32_t> s2_speed, cv_s2_speed;
	atomic<int32_t> s2_pitch, cv_s2_pitch;
	atomic<int32_t> s2_bank, cv_s2_bank;
	atomic<int32_t> s2_slice, cv_s2_slice;
	atomic<int32_t> s2_start, cv_s2_start;
	atomic<int32_t> s2_end, cv_s2_end;
	atomic<int32_t> s2_lp, trig_s2_lp;
	atomic<int32_t> s2_lp_pp, trig_s2_lp_pp;
	atomic<int32_t> s2_lp_pos, cv_s2_lp_pos;
	atomic<int32_t> s2_atk, cv_s2_atk;
	atomic<int32_t> s2_dcy, cv_s2_dcy;
	atomic<int32_t> s2_eg2fm, cv_s2_eg2fm;
	atomic<int32_t> s2_brr, cv_s2_brr;
	atomic<int32_t> s2_ft, cv_s2_ft;
	atomic<int32_t> s2_fc, cv_s2_fc;
	atomic<int32_t> s2_fq, cv_s2_fq;
	atomic<int32_t> s3_gate, trig_s3_gate;
	atomic<int32_t> s3_mute, trig_s3_mute;
	atomic<int32_t> s3_lev, cv_s3_lev;
	atomic<int32_t> s3_pan, cv_s3_pan;
	atomic<int32_t> s3_fx1, cv_s3_fx1;
	atomic<int32_t> s3_fx2, cv_s3_fx2;
	atomic<int32_t> s3_speed, cv_s3_speed;
	atomic<int32_t> s3_pitch, cv_s3_pitch;
	atomic<int32_t> s3_bank, cv_s3_bank;
	atomic<int32_t> s3_slice, cv_s3_slice;
	atomic<int32_t> s3_start, cv_s3_start;
	atomic<int32_t> s3_end, cv_s3_end;
	atomic<int32_t> s3_lp, trig_s3_lp;
	atomic<int32_t> s3_lp_pp, trig_s3_lp_pp;
	atomic<int32_t> s3_lp_pos, cv_s3_lp_pos;
	atomic<int32_t> s3_atk, cv_s3_atk;
	atomic<int32_t> s3_dcy, cv_s3_dcy;
	atomic<int32_t> s3_eg2fm, cv_s3_eg2fm;
	atomic<int32_t> s3_brr, cv_s3_brr;
	atomic<int32_t> s3_ft, cv_s3_ft;
	atomic<int32_t> s3_fc, cv_s3_fc;
	atomic<int32_t> s3_fq, cv_s3_fq;
	atomic<int32_t> s4_gate, trig_s4_gate;
	atomic<int32_t> s4_mute, trig_s4_mute;
	atomic<int32_t> s4_lev, cv_s4_lev;
	atomic<int32_t> s4_pan, cv_s4_pan;
	atomic<int32_t> s4_fx1, cv_s4_fx1;
	atomic<int32_t> s4_fx2, cv_s4_fx2;
	atomic<int32_t> s4_speed, cv_s4_speed;
	atomic<int32_t> s4_pitch, cv_s4_pitch;
	atomic<int32_t> s4_bank, cv_s4_bank;
	atomic<int32_t> s4_slice, cv_s4_slice;
	atomic<int32_t> s4_start, cv_s4_start;
	atomic<int32_t> s4_end, cv_s4_end;
	atomic<int32_t> s4_lp, trig_s4_lp;
	atomic<int32_t> s4_lp_pp, trig_s4_lp_pp;
	atomic<int32_t> s4_lp_pos, cv_s4_lp_pos;
	atomic<int32_t> s4_atk, cv_s4_atk;
	atomic<int32_t> s4_dcy, cv_s4_dcy;
	atomic<int32_t> s4_eg2fm, cv_s4_eg2fm;
	atomic<int32_t> s4_brr, cv_s4_brr;
	atomic<int32_t> s4_ft, cv_s4_ft;
	atomic<int32_t> s4_fc, cv_s4_fc;
	atomic<int32_t> s4_fq, cv_s4_fq;
	atomic<int32_t> in_mute, trig_in_mute;
	atomic<int32_t> in_lev, cv_in_lev;
	atomic<int32_t> in_pan, cv_in_pan;
	atomic<int32_t> in_fx1, cv_in_fx1;
	atomic<int32_t> in_fx2, cv_in_fx2;
	// atomic<int32_t> td3_trigger, trig_td3_trigger;
	// atomic<int32_t> td3_sync_trig, trig_td3_sync_trig;
	// atomic<int32_t> td3_pitch, cv_td3_pitch;
	// atomic<int32_t> td3_shape, cv_td3_shape;
	// atomic<int32_t> td3_param_0, cv_td3_param_0;
	// atomic<int32_t> td3_param_1, cv_td3_param_1;
	// atomic<int32_t> td3_filter_type, cv_td3_filter_type;
	// atomic<int32_t> td3_cutoff, cv_td3_cutoff;
	// atomic<int32_t> td3_resonance, cv_td3_resonance;
	// atomic<int32_t> td3_envelope, cv_td3_envelope;
	// atomic<int32_t> td3_saturation, cv_td3_saturation;
	// atomic<int32_t> td3_drive, cv_td3_drive;
	// atomic<int32_t> td3_accent, trig_td3_accent;
	// atomic<int32_t> td3_accent_level, cv_td3_accent_level;
	// atomic<int32_t> td3_slide, trig_td3_slide;
	// atomic<int32_t> td3_slide_level, cv_td3_slide_level;
	// atomic<int32_t> td3_decay_vca, cv_td3_decay_vca;
	// atomic<int32_t> td3_decay_vcf, cv_td3_decay_vcf;
	// atomic<int32_t> td3_p0_amt, cv_td3_p0_amt;
	// atomic<int32_t> td3_p1_amt, cv_td3_p1_amt;
	// atomic<int32_t> td3_lev, cv_td3_lev;
	// atomic<int32_t> td3_pan, cv_td3_pan;
	// atomic<int32_t> td3_fx1, cv_td3_fx1;
	// atomic<int32_t> td3_fx2, cv_td3_fx2;
	// atomic<int32_t> pp_pitch, cv_pp_pitch;
	// atomic<int32_t> pp_q_scale, cv_pp_q_scale;
	// atomic<int32_t> pp_chord, cv_pp_chord;
	// atomic<int32_t> pp_inversion, cv_pp_inversion;
	// atomic<int32_t> pp_detune, cv_pp_detune;
	// atomic<int32_t> pp_nnotes, cv_pp_nnotes;
	// atomic<int32_t> pp_ncvoices, cv_pp_ncvoices;
	// atomic<int32_t> pp_voicehold, trig_pp_voicehold;
	// atomic<int32_t> pp_lfo1_freq, cv_pp_lfo1_freq;
	// atomic<int32_t> pp_lfo1_amt, cv_pp_lfo1_amt;
	// atomic<int32_t> pp_filter_type, cv_pp_filter_type;
	// atomic<int32_t> pp_cutoff, cv_pp_cutoff;
	// atomic<int32_t> pp_resonance, cv_pp_resonance;
	// atomic<int32_t> pp_lfo2_freq, cv_pp_lfo2_freq;
	// atomic<int32_t> pp_lfo2_amt, cv_pp_lfo2_amt;
	// atomic<int32_t> pp_lfo2_rphase, trig_pp_lfo2_rphase;
	// atomic<int32_t> pp_eg_filt_amt, cv_pp_eg_filt_amt;
	// atomic<int32_t> pp_enableEG, trig_pp_enableEG;
	// atomic<int32_t> pp_latchEG, trig_pp_latchEG;
	// atomic<int32_t> pp_eg_slow_fast, trig_pp_eg_slow_fast;
	// atomic<int32_t> pp_attack, cv_pp_attack;
	// atomic<int32_t> pp_decay, cv_pp_decay;
	// atomic<int32_t> pp_sustain, cv_pp_sustain;
	// atomic<int32_t> pp_release, cv_pp_release;
	// atomic<int32_t> pp_lev, cv_pp_lev;
	// atomic<int32_t> pp_pan, cv_pp_pan;
	// atomic<int32_t> pp_fx1, cv_pp_fx1;
	// atomic<int32_t> pp_fx2, cv_pp_fx2;
	// atomic<int32_t> mo_shape, cv_mo_shape;
	// atomic<int32_t> mo_pitch, cv_mo_pitch;
	// atomic<int32_t> mo_decimation, cv_mo_decimation;
	// atomic<int32_t> mo_bit_reduction, cv_mo_bit_reduction;
	// atomic<int32_t> mo_q_scale, cv_mo_q_scale;
	// atomic<int32_t> mo_param_0, cv_mo_param_0;
	// atomic<int32_t> mo_param_1, cv_mo_param_1;
	// atomic<int32_t> mo_waveshaping, cv_mo_waveshaping;
	// atomic<int32_t> mo_fm_amt, cv_mo_fm_amt;
	// atomic<int32_t> mo_p0_amt, cv_mo_p0_amt;
	// atomic<int32_t> mo_p1_amt, cv_mo_p1_amt;
	// atomic<int32_t> mo_enableEG, trig_mo_enableEG;
	// atomic<int32_t> mo_loopEG, trig_mo_loopEG;
	// atomic<int32_t> mo_attack, cv_mo_attack;
	// atomic<int32_t> mo_decay, cv_mo_decay;
	// atomic<int32_t> mo_lev, cv_mo_lev;
	// atomic<int32_t> mo_pan, cv_mo_pan;
	// atomic<int32_t> mo_fx1, cv_mo_fx1;
	// atomic<int32_t> mo_fx2, cv_mo_fx2;
	atomic<int32_t> fx1_time_ms, cv_fx1_time_ms;
	atomic<int32_t> fx1_sync, trig_fx1_sync;
	atomic<int32_t> fx1_freeze, trig_fx1_freeze;
	atomic<int32_t> fx1_tape_digital, trig_fx1_tape_digital;
	atomic<int32_t> fx1_st_width, cv_fx1_st_width;
	atomic<int32_t> fx1_fx_send, cv_fx1_fx_send;
	atomic<int32_t> fx1_feedback, cv_fx1_feedback;
	atomic<int32_t> fx1_base, cv_fx1_base;
	atomic<int32_t> fx1_width, cv_fx1_width;
	atomic<int32_t> fx2_time, cv_fx2_time;
	atomic<int32_t> fx2_lp, cv_fx2_lp;
	atomic<int32_t> c_thres, cv_c_thres;
	atomic<int32_t> c_ratio, cv_c_ratio;
	atomic<int32_t> c_atk, cv_c_atk;
	atomic<int32_t> c_rel, cv_c_rel;
	atomic<int32_t> c_lpf, trig_c_lpf;
	atomic<int32_t> c_gain, cv_c_gain;
	atomic<int32_t> c_mix, cv_c_mix;
	atomic<int32_t> c_dly_level, cv_c_dly_level;
	atomic<int32_t> c_rev_level, cv_c_rev_level;
	atomic<int32_t> sum_mute, trig_sum_mute;
	atomic<int32_t> sum_lev, cv_sum_lev;
	atomic<int32_t> fx1_amount, cv_fx1_amount;
	atomic<int32_t> fx2_amount, cv_fx2_amount;
	// sectionHpp
        };
    }
}

