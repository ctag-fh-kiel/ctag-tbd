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


#include "SPManager.hpp"
#include "Calibration.hpp"
#include "esp_log.h"
#include "driver/i2s.h"
#include "esp_system.h"
#include "stdint.h"
#include "string.h"
#include "gpio.hpp"
#include "adc.hpp"
#include "codec.hpp"
#include "esp_heap_caps.h"
#include "led_rgb.hpp"
#include "network.hpp"
#include "RestServer.hpp"
#include <math.h>
#include "helpers/ctagFastMath.hpp"
#include "helpers/ctagSampleRom.hpp"
#include "freeverb3/efilter.hpp"
#include "stmlib/dsp/dsp.h"

#define MAX(x, y) ((x)>(y)) ? (x) : (y)
#define MIN(x, y) ((x)<(y)) ? (x) : (y)
#define BUF_SZ 32
//#define NOISE_GATE_LEVEL_CLOSE 0.000065f
#define NOISE_GATE_LEVEL_CLOSE 0.0001f
#define NOISE_GATE_LEVEL_OPEN 0.0003f

using namespace CTAG;
using namespace CTAG::AUDIO;
using namespace CTAG::DRIVERS;

#define NG_OPEN 0
#define NG_BOTH 1
#define NG_LEFT 2
#define NG_RIGHT 3
#define N_CVS 4
#define N_TRIGS 2

// audio real-time task
void IRAM_ATTR SoundProcessorManager::audio_task(void *pvParams) {
    float fbuf[BUF_SZ * 2];
    float cv[N_CVS];
    uint8_t trig[N_TRIGS];
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

    SP::ProcessData pd;
    pd.buf = fbuf;
    pd.cv = cv;
    pd.trig = trig;

    // generate linear ramp ]0,1[ squared
    for (uint32_t i = 0; i < BUF_SZ; i++) {
        lramp[i] = (float) (i + 1) / (float) (BUF_SZ + 1);
        lramp[i] *= lramp[i];
    }

    while (runAudioTask) {
        // update data from ADCs for real-time control 
        ADC::Update();
        CAL::Calibration::MapCVData(ADC::data, cv);

        // update trig data
        trig[0] = GPIO::GetTrig0();
        trig[1] = GPIO::GetTrig1();

        // get normalized raw data from CODEC
        DRIVERS::Codec::ReadBuffer(fbuf, BUF_SZ);

        // In peak detection
        // dc cut input
        float maxl = 0.f, maxr = 0.f;
        float max = 0.f;
        for (uint32_t i = 0; i < BUF_SZ; i++) {
            fbuf[i * 2] = in_dccutl(fbuf[i * 2]);
            if (fbuf[i * 2] > maxl) maxl = fbuf[i * 2];
            fbuf[i * 2 + 1] = in_dccutr(fbuf[i * 2 + 1]);
            if (fbuf[i * 2 + 1] > maxr) maxr = fbuf[i * 2 + 1];
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
        //ESP_LOGI("SP", "Max %.9f %f", peakIn, max);
        if (max > 0 && ngState == NG_OPEN) {
            ledData = ((uint32_t) max);
            ledData <<= 8; // green
        }

        // sound processors
        if (xSemaphoreTake(processMutex, 0) == pdTRUE) {
            // apply sound processors
            if (sp[0] != nullptr) {
                isStereoCH0 = sp[0]->GetIsStereo();
                sp[0]->Process(pd);
            }
            if (!isStereoCH0) if (sp[1] != nullptr) sp[1]->Process(pd); // 0 is not a stereo processor
            xSemaphoreGive(processMutex);
        } else {
            // mute audio
            memset(fbuf, 0, BUF_SZ * 2 * sizeof(float));
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
        // dc cut output, limiting output
        max = 0.f;
        for (uint32_t i = 0; i < BUF_SZ; i++) {
            // soft limiting
            if (ch0_outputSoftClip) {
                fbuf[i * 2] = stmlib::SoftClip(fbuf[i * 2]);
            }
            if (ch1_outputSoftClip) {
                fbuf[i * 2 + 1] = stmlib::SoftClip(fbuf[i * 2 + 1]);
            }
            if (fbuf[i * 2] > max) max = fbuf[i * 2];
            if (fbuf[i * 2 + 1] > max) max = fbuf[i * 2 + 1];
        }
        peakOut = 0.9f * peakOut + 0.1f * max;
        max = 255.f + 3.2f * HELPERS::fast_dBV(peakOut);
        // ESP_LOGW("PEAK", "max %.7f, peak %.7f", max, peakOut);
        if (max > 0.f) ledData |= ((uint32_t) max) << 16; // red
        ledStatus = ledData;

        // write raw float data back to CODEC
        DRIVERS::Codec::WriteBuffer(fbuf, BUF_SZ);
    }
    runAudioTask = 2;
    vTaskDelete(NULL);
}

void SoundProcessorManager::SetSoundProcessorChannel(const int chan, const string &id) {
    ledBlink = 5;

    ESP_LOGI("SP", "Switching plugin %d to %s", chan, id.c_str());
    ESP_LOGE("SP", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    // destroy active plugin
    xSemaphoreTake(processMutex, portMAX_DELAY);
    sp[chan] = nullptr; // destruct smart ptr
    if (model->IsStereo(id) && chan == 0) {
        ESP_LOGI("SP", "Removing ch 1 plugin as ch 0 is stereo!");
        sp[1] = nullptr; // destruct smart ptr
    }

    ESP_LOGE("SP", "2: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    // create new plugin
    sp[chan] = ctagSoundProcessorFactory::Create(id);
    sp[chan]->SetProcessChannel(chan);
    model->SetActivePluginID(id, chan);
    sp[chan]->LoadPreset(model->GetActivePatchNum(chan));
    xSemaphoreGive(processMutex);


    ESP_LOGE("SP", "3: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

}

TaskHandle_t SoundProcessorManager::audioTaskH;
TaskHandle_t SoundProcessorManager::ledTaskH;
std::unique_ptr<ctagSoundProcessor> SoundProcessorManager::sp[2];
std::unique_ptr<SPManagerDataModel> SoundProcessorManager::model;
SemaphoreHandle_t SoundProcessorManager::processMutex;
atomic<uint32_t> SoundProcessorManager::ledBlink;
atomic<uint32_t> SoundProcessorManager::ledStatus;
atomic<uint32_t> SoundProcessorManager::noiseGateCfg;
atomic<uint32_t> SoundProcessorManager::toStereoCH0;
atomic<uint32_t> SoundProcessorManager::toStereoCH1;
atomic<uint32_t> SoundProcessorManager::runAudioTask;
atomic<uint32_t> SoundProcessorManager::ch0_outputSoftClip;
atomic<uint32_t> SoundProcessorManager::ch1_outputSoftClip;

void SoundProcessorManager::StartSoundProcessor() {
    ledBlink = 5;
    model = std::make_unique<SPManagerDataModel>();

    /* there should be an extra pin for this!
    // check if network reset requested trig 1 pressed at startup
    if(GPIO::GetTrig1() == 0){
        DRIVERS::LedRGB::SetLedRGB(255, 255, 255);
        model->ResetNetworkConfiguration();
        ESP_LOGE("SP", "Network credentials reset requested!");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    */

    // init codec
    DRIVERS::Codec::InitCodec();

    // generate internal data
    updateConfiguration();

    // boot network
    NET::Network::SetSSID(model->GetNetworkConfigurationData("ssid"));
    NET::Network::SetPWD(model->GetNetworkConfigurationData("pwd"));
    NET::Network::SetIsAccessPoint(model->GetNetworkConfigurationData("mode").compare("ap") == 0);
    NET::Network::SetIP(model->GetNetworkConfigurationData("ip"));
    NET::Network::SetMDNSName(model->GetNetworkConfigurationData("mdns_name"));
    NET::Network::Up();

    REST::RestServer::StartRestServer();

    // configure channels
    sp[0] = ctagSoundProcessorFactory::Create(model->GetActiveProcessorID(0));
    sp[0]->SetProcessChannel(0);
    sp[0]->LoadPreset(model->GetActivePatchNum(0));
    ESP_LOGD("SPM", "id sp 0 %s, preset %d", model->GetActiveProcessorID(0).c_str(), model->GetActivePatchNum(0));
    if (!sp[0]->GetIsStereo()) {
        sp[1] = ctagSoundProcessorFactory::Create(model->GetActiveProcessorID(1));
        sp[1]->SetProcessChannel(1);
        sp[1]->LoadPreset(model->GetActivePatchNum(1));
        ESP_LOGD("SPM", "id sp 0 %s, preset %d", model->GetActiveProcessorID(1).c_str(), model->GetActivePatchNum(1));
    }
    // prepare threads and mutex
    processMutex = xSemaphoreCreateMutex();
    if (processMutex == NULL) {
        ESP_LOGE("SPM", "Fatal couldn't create mutex!");
    }
    // create led indicator thread
    xTaskCreatePinnedToCore(&SoundProcessorManager::led_task, "led_task", 4096, nullptr, tskIDLE_PRIORITY + 2,
                            &ledTaskH, 0);
    // create audio thread
    runAudioTask = 1;
    xTaskCreatePinnedToCore(&SoundProcessorManager::audio_task, "audio_task", 4096, nullptr, 23, &audioTaskH, 1);
}

void SoundProcessorManager::SetChannelParamValue(const int chan, const string &id, const string &key, const int val) {
    ledBlink = 3;
    sp[chan]->SetParamValue(id, key, val);
}

void SoundProcessorManager::ChannelSavePreset(const int chan, const string &name, const int number) {
    ledBlink = 3;
    if (sp[chan] == nullptr) return;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    sp[chan]->SavePreset(name, number);
    model->SetActivePatchNum(number, chan);
    xSemaphoreGive(processMutex);
}

void SoundProcessorManager::ChannelLoadPreset(const int chan, const int number) {
    ledBlink = 3;
    if (sp[chan] == nullptr) return;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    sp[chan]->LoadPreset(number);
    model->SetActivePatchNum(number, chan);
    xSemaphoreGive(processMutex);
}

string SoundProcessorManager::GetStringID(const int chan) {
    ledBlink = 3;
    return model->GetActiveProcessorID(chan);
}

void SoundProcessorManager::SetConfigurationFromJSON(const string &data) {
    ledBlink = 3;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    model->SetConfigurationFromJSON(data);
    updateConfiguration();
    xSemaphoreGive(processMutex);
}

void SoundProcessorManager::updateConfiguration() {
    ledBlink = 3;
    CAL::CVConfig cvCfg[4];
    // CV config
    if (model->GetConfigurationData("cv_ch0").compare("bipolar") == 0) {
        cvCfg[0] = CAL::CVConfig::CVBipolar;
    } else {
        cvCfg[0] = CAL::CVConfig::CVUnipolar;
    }
    if (model->GetConfigurationData("cv_ch1").compare("bipolar") == 0) {
        cvCfg[1] = CAL::CVConfig::CVBipolar;
    } else {
        cvCfg[1] = CAL::CVConfig::CVUnipolar;
    }
    if (model->GetConfigurationData("cv_ch2").compare("bipolar") == 0) {
        cvCfg[2] = CAL::CVConfig::CVBipolar;
    } else {
        cvCfg[2] = CAL::CVConfig::CVUnipolar;
    }
    if (model->GetConfigurationData("cv_ch3").compare("bipolar") == 0) {
        cvCfg[3] = CAL::CVConfig::CVBipolar;
    } else {
        cvCfg[3] = CAL::CVConfig::CVUnipolar;
    }
    CAL::Calibration::ConfigCVChannels(cvCfg[0], cvCfg[1], cvCfg[2], cvCfg[3]);

    // noise gate configuration
    if (model->GetConfigurationData("ng_config").compare("off") == 0) {
        noiseGateCfg = 0;
    } else if (model->GetConfigurationData("ng_config").compare("dual") == 0) {
        DRIVERS::Codec::RecalibDCOffset();
        noiseGateCfg = 1;
    } else if (model->GetConfigurationData("ng_config").compare("ch0") == 0) {
        DRIVERS::Codec::RecalibDCOffset();
        noiseGateCfg = 2;
    } else if (model->GetConfigurationData("ng_config").compare("ch1") == 0) {
        DRIVERS::Codec::RecalibDCOffset();
        noiseGateCfg = 3;
    }

    // mono to stereo channel cfg
    if (model->GetConfigurationData("ch0_toStereo").compare("off") == 0) {
        toStereoCH0 = 0;
    } else if (model->GetConfigurationData("ch0_toStereo").compare("on") == 0) {
        toStereoCH0 = 1;
    } else if (model->GetConfigurationData("ch0_toStereo").compare("mix") == 0) {
        toStereoCH0 = 2;
    }
    if (model->GetConfigurationData("ch1_toStereo").compare("off") == 0) {
        toStereoCH1 = 0;
    } else if (model->GetConfigurationData("ch1_toStereo").compare("on") == 0) {
        toStereoCH1 = 1;
    } else if (model->GetConfigurationData("ch1_toStereo").compare("mix") == 0) {
        toStereoCH1 = 2;
    }

    // soft clipping?
    if (model->GetConfigurationData("ch0_outputSoftClip").compare("off") == 0) {
        ch0_outputSoftClip = 0;
    } else if (model->GetConfigurationData("ch0_outputSoftClip").compare("on") == 0) {
        ch0_outputSoftClip = 1;
    }
    if (model->GetConfigurationData("ch1_outputSoftClip").compare("off") == 0) {
        ch1_outputSoftClip = 0;
    } else if (model->GetConfigurationData("ch1_outputSoftClip").compare("on") == 0) {
        ch1_outputSoftClip = 1;
    }

    // output levels of codec
    if(model->GetConfigurationData("ch0_codecLvlOut").compare("") != 0){
        if(model->GetConfigurationData("ch0_codecLvlOut").compare("") != 0){
            int lLevel = std::stoi(model->GetConfigurationData("ch0_codecLvlOut"));
            int rLevel = std::stoi(model->GetConfigurationData("ch1_codecLvlOut"));
            CONSTRAIN(rLevel, 0, 63)
            CONSTRAIN(lLevel, 0, 63)
            DRIVERS::Codec::SetOutputLevels(lLevel, rLevel);
        }
    }
}

void SoundProcessorManager::led_task(void *pvParams) {
    uint32_t r = 0, g = 0;
    uint32_t data = 0;
    while (1) {
        data = ledStatus;
        r = data & 0x00FF0000;
        r >>= 16;
        g = data & 0x0000FF00;
        g >>= 8;
        if ((ledBlink % 2) == 1) {
            DRIVERS::LedRGB::SetLedRGB(r, g, 0);
        } else {
            DRIVERS::LedRGB::SetLedRGB(r, g, 255);
        }
        if (ledBlink > 1) ledBlink--;
        vTaskDelay(50 / portTICK_PERIOD_MS); // 50ms refresh rate for led
    }
}

void SoundProcessorManager::KillAudioTask() {
    // stop audio Task, delete plugins
    runAudioTask = 0;
    while (runAudioTask != 2); // wait for audio task to be dead
    sp[0] = nullptr;
    sp[1] = nullptr;
    vTaskDelete(ledTaskH);
    ledTaskH = NULL;
    vTaskDelay(100 / portTICK_PERIOD_MS);
    DRIVERS::LedRGB::SetLedRGB(255, 255, 255);
}

void SoundProcessorManager::DisablePluginProcessing() {
    xSemaphoreTake(processMutex, portMAX_DELAY);
}

void SoundProcessorManager::EnablePluginProcessing() {
    xSemaphoreGive(processMutex);
}

void SoundProcessorManager::InvalidateSampleRom() {
    ctagSampleRom::InvalidateSampleRom();
}
