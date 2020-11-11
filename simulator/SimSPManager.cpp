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

#include "SimSPManager.hpp"
#include "tinywav.h"
#include <mutex>
#include <cmath>

using namespace CTAG::AUDIO;

std::mutex audioMutex;
TinyWav tw;
bool isWaveInput = false;


// Audio callback
int SimSPManager::inout(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                        double streamTime, RtAudioStreamStatus status, void *userData) {
    bool isStereoCH0;
    SP::ProcessData pd;
    float fbuf[32 * 2];
    float cv[4] = {0.f, 0.f, 0.f, 0.f};
    uint8_t trig[2] = {0, 0};

    if (inputBuffer != NULL)
        memcpy(fbuf, inputBuffer, 32 * 2 * 4);
    else
        memset(fbuf, 0, 32 * 2 * 4);

    if (isWaveInput) {
        int nread = 0;
        do{
            nread = tinywav_read_f(&tw, fbuf, 32);
            if (nread != 32) {
                tinywav_read_reset(&tw);
            }
        }while(nread != 32);
        // check value range
        for(int i=0;i<32;i++){
            if(fbuf[i*2] > 1.f)fbuf[i*2] = 0.f;
            if(fbuf[i*2] < -1.f)fbuf[i*2] = -0.f;
            if(fbuf[i*2 + 1] > 1.f)fbuf[i*2 + 1] = 0.f;
            if(fbuf[i*2 + 1] < -1.f)fbuf[i*2 + 1] = -0.f;
        }
    }

    // process stimulus
    stimulus.Process(cv, trig);

    // create data structure
    pd.buf = fbuf;
    pd.cv = cv;
    pd.trig = trig;

    //if ( status ) std::cout << "Stream over/underflow detected." << std::endl;

    // sound processors
    if (audioMutex.try_lock()) {
        if (SimSPManager::sp[0] != nullptr) {
            isStereoCH0 = SimSPManager::sp[0]->GetIsStereo();
            SimSPManager::sp[0]->Process(pd);
        }
        if (!isStereoCH0)
            if (SimSPManager::sp[1] != nullptr)
                SimSPManager::sp[1]->Process(pd); // 0 is not a stereo processor
        audioMutex.unlock();
    }

    memcpy(outputBuffer, fbuf, 32 * 2 * 4);
    return 0;
}

void SimSPManager::StartSoundProcessor(int iSoundCardID, string wavFile, bool bOutOnly) {
    // Initialize simulator parameters
    simModel = std::make_unique<SimDataModel>();
    int mode[6], value[6];
    for (int i = 0; i < 6; i++) {
        mode[i] = simModel->GetArrayElement("mode", i);
        value[i] = simModel->GetArrayElement("value", i);
    }
    stimulus.UpdateStimulus(mode, value);
    // Scan through devices for various capabilities
    RtAudio::DeviceInfo info;
    info = audio.getDeviceInfo(iSoundCardID);
    if (info.probed == true) {
        bool bFormatSupported = false;
        // Print, for example, the maximum number of output channels for each device
        std::cout << "device = " << iSoundCardID << " " << info.name << std::endl;
        if (info.duplexChannels < 2) {
            std::cout << "No duplex device found, enabling output only!" << std::endl;
            bOutOnly = true;
        }
        for (const auto &v: info.sampleRates) {
            if (v == 44100 && (info.nativeFormats & RTAUDIO_FLOAT32) != 0) {
                bFormatSupported = true;
                break;
            }
        }
        if (!bFormatSupported) {
            std::cout << "Sample rate of 44100Hz@float32 not supported!" << std::endl;
            exit(0);
        }
    } else {
        std::cout << "Could not probe sound card!" << std::endl;
        exit(0);
    }

    // wav file
    if (!wavFile.empty()) {
        tinywav_open_read(&tw, wavFile.c_str(), TW_INTERLEAVED, TW_FLOAT32);
        if (!tw.f) {
            cout << "Could not open wav file!" << endl;
            exit(-1);
        }
        if (tw.numChannels != 2) {
            cout << "Wave file is not stereo!" << endl;
            exit(-1);
        }
        if (tw.sampFmt != TW_FLOAT32) {
            cout << "Wave file is not float32!" << endl;
            exit(-1);
        }
        isWaveInput = true;
    }

    // main stuff
    RtAudio::StreamParameters iParams, oParams;
    iParams.deviceId = iSoundCardID;
    iParams.nChannels = 2;
    oParams.deviceId = iSoundCardID;
    oParams.nChannels = 2;
    unsigned int bufferFrames = 32;
    std::cout << "Trying to open device id: " << iSoundCardID << endl;
    try {
        if (bOutOnly) {
            audio.openStream(&oParams, NULL, RTAUDIO_FLOAT32, 44100, &bufferFrames, &SimSPManager::inout);
        } else {
            audio.openStream(&oParams, &iParams, RTAUDIO_FLOAT32, 44100, &bufferFrames, &SimSPManager::inout);
        }
        // configure channels
        model = std::make_unique<SPManagerDataModel>();
        sp[0] = ctagSoundProcessorFactory::Create(model->GetActiveProcessorID(0));
        sp[0]->SetProcessChannel(0);
        sp[0]->LoadPreset(model->GetActivePatchNum(0));
        if (!sp[0]->GetIsStereo()) {
            sp[1] = ctagSoundProcessorFactory::Create(model->GetActiveProcessorID(1));
            sp[1]->SetProcessChannel(1);
            sp[1]->LoadPreset(model->GetActivePatchNum(1));
        }
    }
    catch (RtAudioError &e) {
        e.printMessage();
        exit(0);
    }
    try {
        audio.startStream();
    }
    catch (RtAudioError &e) {
        e.printMessage();
    }
}

void SimSPManager::StopSoundProcessor() {
    if (isWaveInput) tinywav_close_read(&tw);
    if (audio.isStreamRunning() && audio.isStreamOpen()) {
        audio.stopStream();
        audio.closeStream();
    }
}

void SimSPManager::SetSoundProcessorChannel(const int chan, const string &id) {
    printf("Switching plugin %d to %s", chan, id.c_str());
    audioMutex.lock();
    sp[chan] = nullptr; // destruct smart ptr
    if (model->IsStereo(id) && chan == 0) {
        ESP_LOGI("SP", "Removing ch 1 plugin as ch 0 is stereo!");
        sp[1] = nullptr; // destruct smart ptr
    }
    sp[chan] = ctagSoundProcessorFactory::Create(id);
    sp[chan]->SetProcessChannel(chan);
    model->SetActivePluginID(id, chan);
    sp[chan]->LoadPreset(model->GetActivePatchNum(chan));
    audioMutex.unlock();
}

void SimSPManager::SetChannelParamValue(const int chan, const string &id, const string &key, const int val) {
    sp[chan]->SetParamValue(id, key, val);
}

void SimSPManager::ChannelSavePreset(const int chan, const string &name, const int number) {
    if (sp[chan] == nullptr) return;
    audioMutex.lock();
    sp[chan]->SavePreset(name, number);
    model->SetActivePatchNum(number, chan);
    audioMutex.unlock();
}

void SimSPManager::ChannelLoadPreset(const int chan, const int number) {
    if (sp[chan] == nullptr) return;
    audioMutex.lock();
    sp[chan]->LoadPreset(number);
    model->SetActivePatchNum(number, chan);
    audioMutex.unlock();
}

string SimSPManager::GetStringID(const int chan) {
    return model->GetActiveProcessorID(chan);
}

void SimSPManager::ListSoundCards() {
    // Determine the number of devices available
    unsigned int devices = audio.getDeviceCount();
    // Scan through devices for various capabilities
    RtAudio::DeviceInfo info;
    int audioIODevice = -1;
    for (unsigned int i = 0; i < devices; i++) {
        info = audio.getDeviceInfo(i);
        if (info.probed == true) {
            // Print, for example, the maximum number of output channels for each device
            std::cout << "device = " << i << " " << info.name;
            std::cout << ": maximum duplex channels = " << info.duplexChannels << "\n";
            std::cout << ": formats = " << (info.nativeFormats & RTAUDIO_FLOAT32);
            for (const auto &v: info.sampleRates) {
                std::cout << ": sample rates = " << v << endl;
                if (info.duplexChannels >= 2 && v == 44100 && (info.nativeFormats & RTAUDIO_FLOAT32) != 0) {
                    audioIODevice = i;
                    break;
                }
            }
        }
    }
}

void SimSPManager::SetProcessParams(const string &params) {
    simModel->SetModelJSONString(params);
    int mode[6], value[6];
    for (int i = 0; i < 6; i++) {
        mode[i] = simModel->GetArrayElement("mode", i);
        value[i] = simModel->GetArrayElement("value", i);
    }
    stimulus.UpdateStimulus(mode, value);
}


RtAudio  SimSPManager::audio;
std::unique_ptr<ctagSoundProcessor> SimSPManager::sp[2];
std::unique_ptr<SPManagerDataModel> SimSPManager::model;
std::unique_ptr<SimDataModel> SimSPManager::simModel;
SimStimulus SimSPManager::stimulus;

