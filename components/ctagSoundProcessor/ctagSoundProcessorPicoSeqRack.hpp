/***************
TBD-16 — Macro/Preset System & PicoSeqRack

(c) 2025-2026 Per-Olov Jernberg (possan). https://possan.codes

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

dadamachines has a commercial license to use this code in the TBD-16 product.
Other commercial use requires a separate license agreement.

Provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include <atomic>
#include "ctagSoundProcessor.hpp"
#include "esp_heap_caps.h"
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
#include "rack/RackDBD.hpp"
#include "rack/RackABD.hpp"
#include "rack/RackDSD.hpp"
#include "rack/RackASD.hpp"
#include "rack/RackHH1.hpp"
#include "rack/RackHH2.hpp"
#include "rack/RackFMB.hpp"
#include "rack/RackRimshot.hpp"
#include "rack/RackClap.hpp"
#include "rack/RackRompler.hpp"
#include "rack/RackTBD03.hpp"
#include "rack/RackPolyPad.hpp"
#include "rack/RackMO.hpp"
#include "rack/RackWTOsc.hpp"
#include "rack/RackTBDings.hpp"
#include "rack/RackInput.hpp"
#include "rack/RackFxDelay.hpp"
#include "rack/RackFxReverb.hpp"
#include "rack/RackFxMaster.hpp"
#include "rack/RackChannelMixer.hpp"






#define BUF_SZ 32

#define MK_BOOL_PAR_NOCV(outname, inname) \
    bool outname = inname;

#define MK_FLT_PAR_ABS_NOCV(outname, inname, norm, scale) \
    float outname = inname / norm * scale;

#define MK_FLT_PAR_ABS_ADD_NOCV(outname, inname, norm, scale) \
    float outname = inname / norm * scale;

#define MK_FLT_PAR_ABS_SFT_NOCV(outname, inname, norm, scale) \
    float outname = inname / norm * scale;

#define MK_FLT_PAR_NOCV(outname, inname, norm, scale) \
    float outname = inname / norm * scale;

#define MK_INT_PAR_ABS_NOCV(outname, inname, scale) \
    int outname = inname * scale / 4096;

#define MK_INT_PAR_NOCV(outname, inname, scale) \
    int outname = inname * scale / 4096;

#define MK_FLT_PAR_ABS_MIN_MAX_NOCV(outname, inname, norm, out_min, out_max) \
    float outname = inname/norm * (out_max-out_min)+out_min;

#define MK_FLT_PAR_ABS_PAN_NOCV(outname, inname, norm, scale)  \
    float outname = (inname/norm+1.f)/2.f * scale;

#define CC_TO_MAP_KEY(ch, cc) (((ch) * 256) + (cc))

// PSRAM-backed STL allocator: routes all allocations to SPIRAM
// so PicoSeqRack's 378 CC map entries don't exhaust internal SRAM
template <typename T>
struct PsramAllocator {
    using value_type = T;
    PsramAllocator() noexcept = default;
    template <typename U> PsramAllocator(const PsramAllocator<U>&) noexcept {}
    T* allocate(std::size_t n) {
        void* p = heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM);
        if (!p) {
            // fallback to default malloc if SPIRAM fails
            p = malloc(n * sizeof(T));
        }
        return static_cast<T*>(p);
    }
    void deallocate(T* p, std::size_t) noexcept {
        heap_caps_free(p);
    }
};
template <class T, class U>
bool operator==(const PsramAllocator<T>&, const PsramAllocator<U>&) { return true; }
template <class T, class U>
bool operator!=(const PsramAllocator<T>&, const PsramAllocator<U>&) { return false; }

// Convenience types using PSRAM allocator
template <typename T>
using PsramVector = std::vector<T, PsramAllocator<T>>;

using PsramCCMap = std::map<
    const uint16_t,
    PsramVector<std::function<void(const int)>>,
    std::less<const uint16_t>,
    PsramAllocator<std::pair<const uint16_t, PsramVector<std::function<void(const int)>>>>
>;



namespace CTAG {
    namespace SP {
        typedef void (DrumRackParameterSetter)(const int value);

		class ctagSoundProcessorPicoSeqRack : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;
            // no ctor, use Init() instead, is called from factory after successful creation
            virtual void Init(std::size_t blockSize, void *blockPtr) override;
            virtual ~ctagSoundProcessorPicoSeqRack();

			void registerParamAndCC(const PickSeqRackInitData *initdata, const char *suffix, int cc, function<DrumRackParameterSetter> setter);
			void parseIncomingMidiMessages(const uint8_t *buf, const size_t len);

			void setTrackMachine(const uint8_t trackIndex, const std::string machineId, float volumeMultiplier);
			// Lightweight volmult-only update — single float write into the rack
			// mixer. Caller MUST already hold SPManager::processMutex (the
			// MacroSPManager reload paths already take it; do NOT take it here
			// or you deadlock the audio task).
			void setTrackVolumeMultiplier(const uint8_t trackIndex, float volumeMultiplier) override;
			void setTrackBank(const uint8_t trackIndex, const uint16_t bankIndex);

			// Propagates Pico-side track mute into the channel mixer's `muted` flag
			// so `enabled` stays false while the user mutes — gates the sum output
			// regardless of LEVEL. See RackChannelMixer for the enabled-check.
			// Overrides the ctagSoundProcessor base-class default no-op so the
			// virtual dispatch from MacroTranslator lands here.
			void setTrackMute(const uint8_t trackIndex, bool muted) override;

			void handleMidiNoteOn(const uint8_t channel, uint8_t note, uint8_t velocity);
			void handleMidiNoteOff(const uint8_t channel, uint8_t note, uint8_t velocity);
			void handleMidiControlChange(const uint8_t channel, uint8_t control, uint8_t value);
			void handleMidiControlChangeNRPM(const uint8_t channel, uint8_t control, uint16_t value);

        private:
            virtual void knowYourself() override;

            // map<const uint8_t, string> pMapCC;
            // map<const uint8_t, string> pMapMacroCC;
			PsramCCMap pMapParCC;
			// map<const uint8_t, function<void(const int)>> pMapMacroParCC;

			// rack components
			RackChannelMixer ch1;
			RackDBD ch1_db;
			RackABD ch1_ab;
			RackRompler ch1_smp;
			uint32_t ch1_render_time;

			RackChannelMixer ch2;
			RackFMB ch2_fmb1;
			RackRompler ch2_smp;
			uint32_t ch2_render_time;

			RackChannelMixer ch3;
			RackDSD ch3_ds;
			RackASD ch3_as;
			RackRompler ch3_smp;
			uint32_t ch3_render_time;

			RackChannelMixer ch4;
			RackHH1 ch4_hh1;
			RackHH2 ch4_hh2;
			RackRompler ch4_smp;
			uint32_t ch4_render_time;

			RackChannelMixer ch5;
			RackRimshot ch5_rs;
			RackRompler ch5_smp;
			uint32_t ch5_render_time;

			RackChannelMixer ch6;
			RackClap ch6_cl;
			RackRompler ch6_smp;
			uint32_t ch6_render_time;

			RackRompler ch7_smp;
			RackChannelMixer ch7;
			uint32_t ch7_render_time;

			RackRompler ch8_smp;
			RackChannelMixer ch8;
			uint32_t ch8_render_time;

			RackTBD03 ch9_td3;
			RackChannelMixer ch9;
			RackRompler ch9_smp;
			uint32_t ch9_render_time;

			RackTBD03 ch10_td3;
			RackRompler ch10_smp;
			RackChannelMixer ch10;
			uint32_t ch10_render_time;

			RackMO ch11_mo;
			RackRompler ch11_smp;
			RackChannelMixer ch11;
			uint32_t ch11_render_time;

			RackWTOsc ch12_wtosc;
			RackMO ch12_mo;
			RackTBDings ch12_tbd;
			RackRompler ch12_smp;
			RackChannelMixer ch12;
			uint32_t ch12_render_time;

			RackChannelMixer ch13;
			RackRompler ch13_smp;
			uint32_t ch13_render_time;

			RackChannelMixer ch14;
			RackRompler ch14_smp;
			uint32_t ch14_render_time;

			RackChannelMixer ch15;
			RackPolyPad ch15_pp;
			RackTBDings ch15_tbd;
			RackRompler ch15_smp;
			uint32_t ch15_render_time;

			RackInput ch16_in;
			RackChannelMixer ch16;
			uint32_t ch16_render_time;

			RackFxReverb fx_reverb;
			uint32_t fx_reverb_render_time;

			RackFxDelay fx_delay;
			uint32_t fx_delay_render_time;

			RackFxMaster fx_master;
			uint32_t fx_master_render_time;

            // compressor
            chunkware_simple::SimpleComp sumCompressor;
            float fCompMUPGain_pre {0.f};
            float side_l {0.f};
            float side_r {0.f};

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
            // float fDelayTime {0.0f};
            float delaySamples {32};
            float fSyncTimeStamp {0.0f};
            int32_t timer {0}, pre_timer {0};
            stmlib::OnePole lp_l, hp_l;
            stmlib::OnePole lp_r, hp_r;
            // Delay-input HP (independent of the feedback-path HP) and
            // reverb-input HP shelf (independent of the in-loop LP fx2_lp).
            stmlib::OnePole dly_input_hp_l, dly_input_hp_r;
            stmlib::OnePole rev_hp_l, rev_hp_r;
			int last_scaledbpm { 1200 };
			float last_msPerBeat { 500.0f };

            // reverb
            float *reverbBuffer;
            mifx::Reverb reverb;
			int framecounter;

            // Pre-delay ring buffer ahead of the reverb input path. Max
            // 200 ms = 8820 samples at 44.1 kHz, mono (reverb sums L+R on
            // input). ~35 KB in SPIRAM.
            float *preDelayBuf;
            static constexpr int preDelayBufSize = 8820;
            int preDelayWriteIdx {0};

        	float audio_in[BUF_SZ*2];
            // Snapshot of combined_out right after ch16 (Audio Input
            // track) mixes — used to separate the input track's
            // contribution from the synth tracks' contribution for
            // the OLED Input / Output peak meters.
            float ch16_combined_snapshot[BUF_SZ * 2];
        	float combined_out[BUF_SZ*2];
        	float send1_out[BUF_SZ*2];
        	float send2_out[BUF_SZ*2];

            // romplers
            CTAG::SP::HELPERS::ctagSampleRom sampleRom;

            // private attributes could go here
            // autogenerated code here
            // sectionHpp
			atomic<int32_t> fx1_time_ms;
			atomic<int32_t> fx1_sync;
			atomic<int32_t> fx1_freeze;
			atomic<int32_t> fx1_tape_digital;
			atomic<int32_t> fx1_st_width;
			atomic<int32_t> fx1_fx_send;
			atomic<int32_t> fx1_feedback;
			atomic<int32_t> fx1_base;
			atomic<int32_t> fx1_width;
			atomic<int32_t> fx2_time;
			atomic<int32_t> fx2_lp;
			// fx2_diffuse drives reverb.set_diffusion() (was hardcoded 0.7).
			// fx2_predelay maps to a mono ring buffer ahead of the tank
			// (preDelayBuf, 8820 samples / ~200 ms at 44.1 kHz).
			atomic<int32_t> fx2_diffuse;
			atomic<int32_t> fx2_predelay;
			// fx2_modulation scales reverb LFO1/LFO2 frequencies 0..2× of
			// their bases (0.5 / 0.3 Hz, see reverb.h:45-46). Wire 64 = 1×
			// preserves prior sound; wire 0 freezes both LFOs.
			atomic<int32_t> fx2_modulation;
			// fx2_input_gain → reverb.set_input_gain (legacy hardcoded 0.5).
			// Wire 64 ≈ 0.5 preserves prior behaviour.
			atomic<int32_t> fx2_input_gain;
			// fx2_tank_level → reverb.set_amount (legacy hardcoded 1.0).
			// Independent from the Master Reverb return (fx2_amount cc 42):
			// this is the internal tank gain, the Master knob is the bus
			// return.
			atomic<int32_t> fx2_tank_level;
			// HP filters: fx1_input_hp on the delay dry input (independent
			// from the feedback-path HP fx1_base cc 27); fx2_hp on the
			// reverb input before the tank. Both wire 0 → 20 Hz (effectively
			// bypassed), wire 127 → ~2 kHz. Defaults preserve prior sound.
			atomic<int32_t> fx1_input_hp;
			atomic<int32_t> fx2_hp;
			// sum_drive: variable-depth soft saturation on the final output
			// bus via stmlib::SoftLimit. Wire 0 = transparent (the path is
			// skipped entirely — see renderMasterOutput). Wire 127 = 4×
			// drive with /sqrt(drive) loudness makeup.
			atomic<int32_t> sum_drive;
			// TBDings global "AIR" master — FaseAcht §4.2 single-slider behaviour.
			// CC 67 / channel 13. Fans out to every enabled RackTBDings instance's
			// air_blend field so one knob opens all "pickups" simultaneously.
			atomic<int32_t> tbd_air_master;
			atomic<int32_t> c_thres;
			atomic<int32_t> c_ratio;
			atomic<int32_t> c_atk;
			atomic<int32_t> c_rel;
			atomic<int32_t> c_lpf;
			atomic<int32_t> c_gain;
			atomic<int32_t> c_mix;
			// CCs 67/68 (c_dly_level / c_rev_level) retired — DSP never
			// referenced them. Registration removed in knowYourself().
			atomic<int32_t> sum_mute;
			atomic<int32_t> sum_lev;
			atomic<int32_t> fx1_amount;
			atomic<int32_t> fx2_amount;
			// sectionHpp
        };
    }
}

