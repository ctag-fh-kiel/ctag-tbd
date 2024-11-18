#include <tbd/sound_processor.hpp>

#include <tbd/system/locks.hpp>
#include <helpers/ctagFastMath.hpp>
#include <tbd/sound_manager/data_model.hpp>
#include <tbd/sound_registry/sound_processor_factory.hpp>

#include "input_manager.hpp"
#include "module_private.hpp"

#define NOISE_GATE_LEVEL_CLOSE 0.0001f
#define NOISE_GATE_LEVEL_OPEN 0.0003f

namespace tbd::audio {

template<typename CodecT>
struct AudioManagerImpl {
    void set_processor_for_channel(int channel, std::string id);

    uint32_t do_begin();
    uint32_t do_work();
    uint32_t do_cleanup();

private:
    // static Lock audioTaskH, ledTaskH;
    static CTAG::SP::ctagSoundProcessor *sp[2];
    static std::unique_ptr<CTAG::AUDIO::SPManagerDataModel> model;
    
    static std::atomic<uint32_t> ledBlink;
    static std::atomic<uint32_t> ledStatus;
    static std::atomic<uint32_t> noiseGateCfg;
    static std::atomic<uint32_t> ch01Daisy;
    static std::atomic<uint32_t> toStereoCH0;
    static std::atomic<uint32_t> toStereoCH1;

    static std::atomic<uint32_t> ch0_outputSoftClip;
    static std::atomic<uint32_t> ch1_outputSoftClip;

    fv3::dccut_f in_dccutl, in_dccutr;
    float fbuf[BUF_SZ * 2];
    float peakIn = 0.f, peakOut = 0.f;
    float peakL = 0.f, peakR = 0.f;
    int ngState = NG_OPEN;
    float lramp[BUF_SZ];
    bool isStereoCH0 = false;
    CTAG::SP::ProcessData pd;

    system::Lock sound_processing_lock;
};

template<typename CodecT>
uint32_t AudioManagerImpl<CodecT>::do_begin() {

    //fv3::dccut_f out_dccutl, out_dccutr;
    in_dccutl.setCutOnFreq(3.7f, 44100.f);
    in_dccutr.setCutOnFreq(3.7f, 44100.f);
    /*
    out_dccutl.setCutOnFreq(3.7f, 44100.f);
    out_dccutr.setCutOnFreq(3.7f, 44100.f);
    */

    pd.buf = fbuf;

    // generate linear ramp ]0,1[ squared
    for (uint32_t i = 0; i < BUF_SZ; i++) {
        lramp[i] = (float) (i + 1) / (float) (BUF_SZ + 1);
        lramp[i] *= lramp[i];
    }
    return 0;
}

template<typename CodecT>
uint32_t AudioManagerImpl<CodecT>::do_work() {
    // update data from ADCs and GPIOs for real-time control
    InputManager::Update(&pd.trig, &pd.cv);

    // get normalized raw data from CODEC
    CodecT::ReadBuffer(fbuf, BUF_SZ);

    // In peak detection
    // dc cut input
    float maxl = 0.f, maxr = 0.f;
    float max = 0.f;
    for (uint32_t i = 0; i < BUF_SZ; i++) {
        fbuf[i * 2] = in_dccutl(fbuf[i * 2]);
        float val = fabsf(fbuf[i * 2]);
        if (val > maxl) maxl = val;
        fbuf[i * 2 + 1] = in_dccutr(fbuf[i * 2 + 1]);
        val = fabsf(fbuf[i * 2 + 1]);
        if (val > maxr) maxr = val;
    }
    max = maxl >= maxr ? maxl : maxr;
    peakIn = 0.95f * peakIn + 0.05f * max;

    // noise gate
    if (noiseGateCfg == 1) { // both channels noise gate
        if (ngState == NG_OPEN && peakIn < NOISE_GATE_LEVEL_CLOSE) {
            ngState = NG_BOTH;
            for (uint32_t i = 0; i < BUF_SZ; i++) { // linearly ramp down buffer
                fbuf[i * 2] *= lramp[BUF_SZ - 1 - i];
                fbuf[i * 2 + 1] *= lramp[BUF_SZ - 1 - i];
            }
        } else if (ngState != NG_OPEN && peakIn > NOISE_GATE_LEVEL_OPEN) {
            ngState = NG_OPEN;
            for (uint32_t i = 0; i < BUF_SZ; i++) { // linearly ramp up buffer
                fbuf[i * 2] *= lramp[i];
                fbuf[i * 2 + 1] *= lramp[i];
            }
        } else if (ngState != NG_OPEN) {
            memset(fbuf, 0, BUF_SZ * 2 * sizeof(float));
        }
    } else if (noiseGateCfg == 2) { // left channel
        peakL = 0.95f * peakL + 0.05f * maxl;
        if (ngState == NG_OPEN && peakL < NOISE_GATE_LEVEL_CLOSE) {
            ngState = NG_LEFT;
            for (uint32_t i = 0; i < BUF_SZ; i++) {// linearly ramp down buffer
                fbuf[i * 2] *= lramp[BUF_SZ - 1 - i];
            }
        } else if (ngState != NG_OPEN && peakL > NOISE_GATE_LEVEL_OPEN) {
            ngState = NG_OPEN;
            for (uint32_t i = 0; i < BUF_SZ; i++) { // linear ramp up
                fbuf[i * 2] *= lramp[i];
            }
        } else if (ngState != NG_OPEN) {
            for (uint32_t i = 0; i < BUF_SZ; i++) {
                fbuf[i * 2] = 0;
            }
        }
    } else if (noiseGateCfg == 3) { // right channel
        peakR = 0.95f * peakR + 0.05f * maxr;
        if (ngState == NG_OPEN && peakR < NOISE_GATE_LEVEL_CLOSE) {
            ngState = NG_RIGHT;
            for (uint32_t i = 0; i < BUF_SZ; i++) {// linearly ramp down buffer
                fbuf[i * 2 + 1] *= lramp[BUF_SZ - 1 - i];
            }
        } else if (ngState != NG_OPEN && peakR > NOISE_GATE_LEVEL_OPEN) {
            ngState = NG_OPEN;
            for (uint32_t i = 0; i < BUF_SZ; i++) { // linear ramp up
                fbuf[i * 2 + 1] *= lramp[i];
            }
        } else if (ngState != NG_OPEN) {
            for (uint32_t i = 0; i < BUF_SZ; i++) {
                fbuf[i * 2 + 1] = 0;
            }
        }
    }

    // led indicator, green for input
    max = 255.f + 3.2f * CTAG::SP::HELPERS::fast_dBV(peakIn); // cut away at approx -80dB
    uint32_t ledData = 0;
    //TBD_LOGI("SP", "Max %.9f %f", peakIn, max);
    if (max > 0 && ngState == NG_OPEN) {
        ledData = ((uint32_t) max);
        ledData <<= 8; // green
    }

    // FIXME: who owns the sound processors? Management of sound processors should 
    //        probably be done in the audio loop
    // sound processors
    {
        auto sound_processing_guard = sound_processing_lock.guard();
        if (sound_processing_guard) {
            // apply sound processors
            if (sp[0] != nullptr) {
                isStereoCH0 = sp[0]->GetIsStereo();
                sp[0]->Process(pd);
            }
            if (!isStereoCH0){
                // check if ch0 -> ch1 daisy chain, i.e. use output of ch0 as input for ch1
                if(ch01Daisy){
                    for (uint32_t i = 0; i < BUF_SZ; i++) {
                        fbuf[i * 2 + 1] = fbuf[i * 2];
                    }
                }
                if (sp[1] != nullptr) sp[1]->Process(pd); // 0 is not a stereo processor
            }
        } else {
            // mute audio
            memset(fbuf, 0, BUF_SZ * 2 * sizeof(float));
        }
    }

    // to stereo conversion
    if (!isStereoCH0) {
        if (toStereoCH0 || toStereoCH1) {
            float sb[BUF_SZ * 2];
            memcpy(sb, fbuf, BUF_SZ * 2 * sizeof(float));
            if (toStereoCH0 == 1 && toStereoCH1 == 0) { // spread CH0 to both channels
                for (uint32_t i = 0; i < BUF_SZ; i++) {
                    fbuf[i * 2] = 0.5f * sb[i * 2];
                    fbuf[i * 2 + 1] = 0.5f * sb[i * 2] + sb[i * 2 + 1];
                }
            } else if (toStereoCH1 == 1 && toStereoCH0 == 0) { // spread CH1 to both channels
                for (uint32_t i = 0; i < BUF_SZ; i++) {
                    fbuf[i * 2] = 0.5f * sb[i * 2 + 1] + sb[i * 2];
                    fbuf[i * 2 + 1] = 0.5f * sb[i * 2 + 1];
                }
            } else if (toStereoCH0 == 1 && toStereoCH1 == 1) { // spread CH0 + CH1 to both channels
                for (uint32_t i = 0; i < BUF_SZ; i++) {
                    fbuf[i * 2] = fbuf[i * 2 + 1] = 0.5f * (sb[i * 2] + sb[i * 2 + 1]);
                }
            } else if (toStereoCH0 == 2 && toStereoCH1 == 2) { // swap channels
                for (uint32_t i = 0; i < BUF_SZ; i++) {
                    fbuf[i * 2] = sb[i * 2 + 1];
                    fbuf[i * 2 + 1] = sb[i * 2];
                }
            } else if (toStereoCH0 == 2 && toStereoCH1 == 0) { // mix CH0 with CH1 on CH1
                for (uint32_t i = 0; i < BUF_SZ; i++) {
                    fbuf[i * 2] = 0.f;
                    fbuf[i * 2 + 1] += sb[i * 2];
                }
            } else if (toStereoCH0 == 0 && toStereoCH1 == 2) { // mix CH1 with CH0 on CH0
                for (uint32_t i = 0; i < BUF_SZ; i++) {
                    fbuf[i * 2] += sb[i * 2 + 1];
                    fbuf[i * 2 + 1] = 0.f;
                }
            } else if (toStereoCH0 == 2 && toStereoCH1 == 1) { // move CH0 to CH1, spread CH1 to both
                for (uint32_t i = 0; i < BUF_SZ; i++) {
                    fbuf[i * 2] = 0.5f * sb[i * 2 + 1];
                    fbuf[i * 2 + 1] = 0.5f * sb[i * 2 + 1] + sb[i * 2];
                }
            } else if (toStereoCH0 == 1 && toStereoCH1 == 2) { // move CH1 to CH0, spread CH0 to both
                for (uint32_t i = 0; i < BUF_SZ; i++) {
                    fbuf[i * 2] = 0.5f * sb[i * 2] + sb[i * 2 + 1];
                    fbuf[i * 2 + 1] = 0.5f * sb[i * 2];
                }
            }
        }
    }

    // Out peak detection, red for output
    // limiting output
    max = 0.f;
    for (uint32_t i = 0; i < BUF_SZ; i++) {
        // soft limiting
        if (ch0_outputSoftClip) {
            fbuf[i * 2] = stmlib::SoftClip(fbuf[i * 2]);
        }
        if (ch1_outputSoftClip) {
            fbuf[i * 2 + 1] = stmlib::SoftClip(fbuf[i * 2 + 1]);
        }
        //if (fbuf[i * 2] > max) max = fbuf[i * 2];
        //if (fbuf[i * 2 + 1] > max) max = fbuf[i * 2 + 1];
    }
    // just take first sample of block for level meter
    max = fabsf(fbuf[0] + fbuf[1]) / 2.f;
    peakOut = 0.9f * peakOut + 0.1f * max;
    //TBD_LOGW("PEAK", "max %.12f, peak %.12f", max, peakOut);
    max = 255.f + 3.2f * CTAG::SP::HELPERS::fast_dBV(peakOut);
    if (max > 0.f) ledData |= ((uint32_t) max) << 16; // red
    ledStatus = ledData;

    // write raw float data back to CODEC
    CodecT::WriteBuffer(fbuf, BUF_SZ);


    return 0;
}

template<typename CodecT>
uint32_t AudioManagerImpl<CodecT>::do_cleanup() {

    return 0;
}

}
