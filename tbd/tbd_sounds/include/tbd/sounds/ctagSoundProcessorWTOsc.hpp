/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include <atomic>
#include <tbd/sound_processor.hpp>
#include "helpers/ctagSampleRom.hpp"
#include "helpers/ctagSineSource.hpp"
#include "helpers/ctagADSREnv.hpp"
#include "plaits/dsp/oscillator/wavetable_oscillator.h"
#include "braids/quantizer.h"
#include "stmlib/dsp/filter.h"
#include "helpers/ctagRollingAverage.hpp"

using namespace CTAG::SP::HELPERS;

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorWTOsc : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;
           virtual void Init(std::size_t blockSize, void *blockPtr) override;
            virtual ~ctagSoundProcessorWTOsc();

        private:
            virtual void knowYourself() override;
            void prepareWavetables();
            ctagSampleRom sample_rom;
            plaits::WavetableOscillator<256, 64> oscillator;
            ctagSineSource lfo;
            ctagADSREnv adsr;
            stmlib::Svf svf;
            int16_t *buffer = NULL;
            float *fbuffer = NULL;
        	float fWave = 0.f;
            const int16_t *wavetables[64];
            int currentBank = 0;
            int lastBank = -1;
            bool isWaveTableGood = false;
            float valADSR = 0.f, valLFO = 0.f;
            bool preGate = false;
            braids::Quantizer pitchQuantizer;
            float pre_fWt = 0.f;
            // private attributes could go here
            // autogenerated code here
            // sectionHpp
            std::atomic<int32_t> gain, cv_gain;
            std::atomic<int32_t> gate, trig_gate;
            std::atomic<int32_t> pitch, cv_pitch;
            std::atomic<int32_t> q_scale, cv_q_scale;
            std::atomic<int32_t> tune, cv_tune;
            std::atomic<int32_t> wavebank, cv_wavebank;
            std::atomic<int32_t> wave, cv_wave;
            std::atomic<int32_t> fmode, cv_fmode;
            std::atomic<int32_t> fcut, cv_fcut;
            std::atomic<int32_t> freso, cv_freso;
            std::atomic<int32_t> lfo2wave, cv_lfo2wave;
            std::atomic<int32_t> lfo2am, cv_lfo2am;
            std::atomic<int32_t> lfo2fm, cv_lfo2fm;
            std::atomic<int32_t> lfo2filtfm, cv_lfo2filtfm;
            std::atomic<int32_t> eg2wave, cv_eg2wave;
            std::atomic<int32_t> eg2am, cv_eg2am;
            std::atomic<int32_t> eg2fm, cv_eg2fm;
            std::atomic<int32_t> eg2filtfm, cv_eg2filtfm;
            std::atomic<int32_t> lfospeed, cv_lfospeed;
            std::atomic<int32_t> lfosync, trig_lfosync;
            std::atomic<int32_t> egfasl, trig_egfasl;
            std::atomic<int32_t> attack, cv_attack;
            std::atomic<int32_t> decay, cv_decay;
            std::atomic<int32_t> sustain, cv_sustain;
            std::atomic<int32_t> release, cv_release;
            // sectionHpp
        };
    }
}