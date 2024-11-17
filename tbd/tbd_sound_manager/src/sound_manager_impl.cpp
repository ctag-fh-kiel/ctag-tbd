#include <tbd/sound_processor.hpp>
#include <tbd/sound_manager/data_model.hpp>
#include <tbd/sound_registry/sound_processor_factory.hpp>

#include "module_private.hpp"


namespace tbd::audio {

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

};


uint32_t AudioManagerImpl::do_begin() {
    float fbuf[BUF_SZ * 2];
    float peakIn = 0.f, peakOut = 0.f;
    float peakL = 0.f, peakR = 0.f;
    int ngState = NG_OPEN;
    float lramp[BUF_SZ];
    bool isStereoCH0 = false;


    fv3::dccut_f in_dccutl, in_dccutr;
    //fv3::dccut_f out_dccutl, out_dccutr;
    in_dccutl.setCutOnFreq(3.7f, 44100.f);
    in_dccutr.setCutOnFreq(3.7f, 44100.f);
    /*
    out_dccutl.setCutOnFreq(3.7f, 44100.f);
    out_dccutr.setCutOnFreq(3.7f, 44100.f);
    */

    CTAG::SP::ProcessData pd;
    pd.buf = fbuf;

    // generate linear ramp ]0,1[ squared
    for (uint32_t i = 0; i < BUF_SZ; i++) {
        lramp[i] = (float) (i + 1) / (float) (BUF_SZ + 1);
        lramp[i] *= lramp[i];
    }
    return 0;
}

}
