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
#include "esp_log.h"
#include "esp_cpu.h"
#include "stdint.h"
#include "string.h"
#include "codec_bba.hpp"
#include "esp_heap_caps.h"
#include "led_rgb_bba.hpp"
#include "network.hpp"
#include "tusb.hpp"
#include "RestServer.hpp"
#include "SpiAPI.hpp"
#include "Control.hpp"
#include "helpers/ctagFastMath.hpp"
#include "helpers/ctagSampleRom.hpp"
#include "stmlib/dsp/dsp.h"

// ableton link
#include "link.hpp"

#define BUF_SZ 32

using namespace CTAG;
using namespace CTAG::AUDIO;
using namespace CTAG::DRIVERS;

#define CPU_MAX_ALLOWED_CYCLES 261224 // is 32/44100kHz * 360MHz

// global variable, sdcard base directory
namespace CTAG {
    namespace RESOURCES {
        std::string sdcardRoot {"/sdcard"};
    }
}


// audio real-time task
void IRAM_ATTR SoundProcessorManager::audio_task(void *pvParams) {
    float fbuf[BUF_SZ * 2];
    float peakIn = 0.f, peakOut = 0.f;
    bool isStereoCH0 = false;
    esp_cpu_cycle_count_t start, diff;

    SP::ProcessData pd;
    pd.buf = fbuf;
    pd.controlData = nullptr;
    pd.cv = nullptr;
    pd.trig = nullptr;

    while (runAudioTask) {

        // update data from ADCs and GPIOs for real-time control
        CTAG::CTRL::Control::Update(&pd.controlData, ledStatus);
        pd.cv = (float*) pd.controlData;
        pd.trig = (uint8_t*) pd.controlData + N_CVS * sizeof(float);

        // get normalized raw data from CODEC
        DRIVERS::Codec::ReadBuffer(fbuf, BUF_SZ);

        // track the cpu cycles for audio task
        start = esp_cpu_get_cycle_count();

        // In peak detection, dc cut is done in codec
        float max = 0.f;
        // just take first sample of block for level meter
        max = fabsf(fbuf[0] + fbuf[1]) / 2.f;
        peakIn = 0.95f * peakIn + 0.05f * max;

        // led indicator, green for input
        max = 255.f + 3.2f * CTAG::SP::HELPERS::fast_dBV(peakIn); // cut away at approx -80dB
        uint32_t ledData = 0;
        //ESP_LOGI("SP", "Max %.9f %f", peakIn, max);
        if (max > 0) {
            ledData = ((uint32_t) max);
            ledData <<= 8; // green
        }

        // sound processors - safer version with RAII and additional checks
        bool processingLocked = (xSemaphoreTake(processMutex, 0) == pdTRUE);

        if (processingLocked) {
            // Validate control data and sound processors before processing
            bool canProcess = (pd.controlData != nullptr) &&
                              (sp[0] != nullptr || sp[1] != nullptr);

            if (canProcess) {
                // Process channel 0
                if (sp[0] != nullptr) {
                    isStereoCH0 = sp[0]->GetIsStereo();
                    sp[0]->Process(pd);
                }

                // Process channel 1 (only if ch0 is not stereo)
                if (!isStereoCH0) {
                    // Daisy chain: copy ch0 output to ch1 input
                    if (ch01Daisy) {
                        for (uint32_t i = 0; i < BUF_SZ; i++) {
                            fbuf[i * 2 + 1] = fbuf[i * 2];
                        }
                    }

                    if (sp[1] != nullptr) {
                        sp[1]->Process(pd);
                    }
                }
            } else {
                // Mute audio if processors unavailable
                memset(fbuf, 0, BUF_SZ * 2 * sizeof(float));
            }

            // Always release mutex
            xSemaphoreGive(processMutex);
        } else {
            // Couldn't acquire mutex - mute audio for this buffer
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
        //ESP_LOGW("PEAK", "max %.12f, peak %.12f", max, peakOut);
        max = 255.f + 3.2f * HELPERS::fast_dBV(peakOut);
        if (max > 0.f) ledData |= ((uint32_t) max) << 16; // red

        // get cpu cycles for audio task and tone led
        diff = esp_cpu_get_cycle_count() - start;
        if(diff > CPU_MAX_ALLOWED_CYCLES) ledData = 0xB39134; // orange code for cpu overflow
        ledStatus = ledData;

        // write raw float data back to CODEC
        DRIVERS::Codec::WriteBuffer(fbuf, BUF_SZ);
    }
    memset(fbuf, 0, BUF_SZ * 2 * sizeof(float));
    DRIVERS::Codec::WriteBuffer(fbuf, BUF_SZ);
    runAudioTask = 2;
    vTaskDelete(NULL);
}

void SoundProcessorManager::SetSoundProcessorChannel(const int chan, const string &id) {
    ledBlink = 5;

    // does the SP exist?
    if(!model->HasPluginID(id)) return;

    // when trying to set chan 1 and chan 0 is a stereo plugin, return
    if(chan == 1 && model->IsStereo(model->GetActiveProcessorID(0))) return;
    if(chan == 1 && model->IsStereo(id)) return;

    ESP_LOGI("SPManager", "Switching ch%d to plugin %s", chan, id.c_str());

    // destroy active plugin
    xSemaphoreTake(processMutex, portMAX_DELAY);
    if(nullptr != sp[chan]){
        delete sp[chan]; // destruct processor
        sp[chan] = nullptr;
    }
    if (model->IsStereo(id) && chan == 0) {
        if(nullptr != sp[1]){
            delete sp[1]; // destruct processor
            sp[1] = nullptr;
        }
    }

    // create new plugin
    ctagSPAllocator::AllocationType aType = ctagSPAllocator::AllocationType::CH0;
    if(chan == 1) aType = ctagSPAllocator::AllocationType::CH1;
    if(model->IsStereo(id)) aType = ctagSPAllocator::AllocationType::STEREO;
    sp[chan] = ctagSoundProcessorFactory::Create(id, aType);
    model->SetActivePluginID(id, chan);
    sp[chan]->LoadPreset(model->GetActivePatchNum(chan));
    xSemaphoreGive(processMutex);


    ESP_LOGI("SPManager", "Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

}

TaskHandle_t SoundProcessorManager::audioTaskH;
TaskHandle_t SoundProcessorManager::ledTaskH;
DRAM_ATTR ctagSoundProcessor* SoundProcessorManager::sp[2] {nullptr, nullptr};
std::unique_ptr<SPManagerDataModel> SoundProcessorManager::model;
DRAM_ATTR SemaphoreHandle_t SoundProcessorManager::processMutex;
atomic<uint32_t> SoundProcessorManager::ledBlink;
atomic<uint32_t> SoundProcessorManager::ledStatus;
atomic<uint32_t> SoundProcessorManager::ch01Daisy;
atomic<uint32_t> SoundProcessorManager::toStereoCH0;
atomic<uint32_t> SoundProcessorManager::toStereoCH1;
atomic<uint32_t> SoundProcessorManager::runAudioTask;
atomic<uint32_t> SoundProcessorManager::ch0_outputSoftClip;
atomic<uint32_t> SoundProcessorManager::ch1_outputSoftClip;

void SoundProcessorManager::StartSoundProcessor() {
    ledBlink = 5;
    model = std::make_unique<SPManagerDataModel>();

    // init tinyusb
    CTAG::DRIVERS::tusb::Init();
    // init control
    CTRL::Control::Init();
    // init codec
    DRIVERS::Codec::InitCodec();
    // generate internal data
    updateConfiguration();

    // start network
    NET::Network::SetSSID(model->GetNetworkConfigurationData("ssid"));
    NET::Network::SetPWD(model->GetNetworkConfigurationData("pwd"));
    if(model->GetNetworkConfigurationData("mode").compare("ap") == 0) {
        NET::Network::SetIfType(NET::Network::IF_TYPE::IF_TYPE_AP);
    }else if(model->GetNetworkConfigurationData("mode").compare("sta") == 0){
        NET::Network::SetIfType(NET::Network::IF_TYPE::IF_TYPE_STA);
    }else if(model->GetNetworkConfigurationData("mode").compare("usbncm") == 0){
        NET::Network::SetIfType(NET::Network::IF_TYPE::IF_TYPE_USBNCM);
    }else{
        ESP_LOGE("SPM", "Fatal: unknown network mode!");
        assert(0);
    }
    NET::Network::SetIP(model->GetNetworkConfigurationData("ip"));
    NET::Network::SetMDNSName(model->GetNetworkConfigurationData("mdns_name"));
    NET::Network::Up();
#ifdef CONFIG_TASK_REST_SERVER
    REST::RestServer::StartRestServer();
#endif
    SPIAPI::SpiAPI::StartSpiAPI();

    // Ableton Link
    CTAG::LINK::link::Init();

    // prepare threads and mutex
    processMutex = xSemaphoreCreateMutex();

    if (processMutex == NULL) {
        ESP_LOGE("SPM", "Fatal couldn't create mutex!");
    }

    // create led indicator thread
#ifdef CONFIG_TASK_RGB_INDICATOR_LED
    xTaskCreatePinnedToCore(&SoundProcessorManager::led_task, "led_task", 4096, nullptr, tskIDLE_PRIORITY + 2,
                            &ledTaskH, 0);
#endif
    // create audio thread
    runAudioTask = 1;
    xTaskCreatePinnedToCore(&SoundProcessorManager::audio_task, "audio_task", 4096, nullptr, 23, &audioTaskH, 1);

    SetSoundProcessorChannel(0, model->GetActiveProcessorID(0));
    SetSoundProcessorChannel(1, model->GetActiveProcessorID(1));
    ESP_LOGI("SPManager", "Init: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
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

    // ch01 daisy
    if (model->GetConfigurationData("ch01_daisy").compare("off") == 0) {
        ch01Daisy = 0;
    }else if(model->GetConfigurationData("ch01_daisy").compare("on") == 0){
        ch01Daisy = 1;
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
    uint32_t r = 0, g = 0, b = 0;
    uint32_t data = 0;
    while (1) {
        data = ledStatus;
        r = data & 0x00FF0000;
        r >>= 16;
        g = data & 0x0000FF00;
        g >>= 8;
        b = data & 0x000000FF;
        if ((ledBlink % 2) != 1) {
            b = 255;
        }
        DRIVERS::LedRGB::SetLedRGB(r, g, b);
        if (ledBlink > 1 && ledBlink < 42) ledBlink--; // >= 42 led blink doesn't stop
        if (ledBlink == 42) ledBlink = 44;
        vTaskDelay(50 / portTICK_PERIOD_MS); // 50ms refresh rate for led
    }
}

void SoundProcessorManager::KillAudioTask() {
    Codec::SetOutputLevels(0, 0);
    // stop audio Task, delete plugins
    runAudioTask = 0;
    while (runAudioTask != 2); // wait for audio task to be dead
    if(nullptr!=sp[0]) delete sp[0];
    if(nullptr!=sp[1]) delete sp[1];
    sp[0] = nullptr;
    sp[1] = nullptr;
    vTaskDelete(ledTaskH);
    ledTaskH = NULL;
    vTaskDelay(100 / portTICK_PERIOD_MS);
    DRIVERS::LedRGB::SetLedRGB(255, 0, 255);
    ESP_LOGI("SPManager", "Audio Task Killed: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
}

void SoundProcessorManager::DisablePluginProcessing() {
    xSemaphoreTake(processMutex, portMAX_DELAY);
    ledBlink = 42;
}

void SoundProcessorManager::EnablePluginProcessing() {
    ledBlink = 5;
    xSemaphoreGive(processMutex);
}

void SoundProcessorManager::RefreshSampleRom() {
    ledBlink = 5;
    ctagSampleRom::RefreshDataStructure();
}
