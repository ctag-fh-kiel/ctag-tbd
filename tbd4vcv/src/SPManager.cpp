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
#include <mutex>
#include <cmath>
#include "esp_spi_flash.h"

using namespace CTAG::AUDIO;

std::mutex audioMutex;

void SPManager::Start(){
    /*
    // start fake sample rom
    cout << "Trying to open sample rom file (define own with -s command line option): " << sromFile << endl;
    spi_flash_emu_init(sromFile.c_str());
    // Initialize simulator parameters
    simModel = std::make_unique<SimDataModel>();
    int mode[6], value[6];
    for (int i = 0; i < 6; i++) {
        mode[i] = simModel->GetArrayElement("mode", i);
        value[i] = simModel->GetArrayElement("value", i);
    }
     */
}

void SPManager::Stop() {
    /*
    // free sample rom emulation
    spi_flash_emu_release();
    if (isWaveInput) tinywav_close_read(&tw);
    if (audio.isStreamRunning() && audio.isStreamOpen()) {
        audio.stopStream();
        audio.closeStream();
    }
     */
}

void SPManager::SetSoundProcessorChannel(const int chan, const string &id) {
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

void SPManager::SetChannelParamValue(const int chan, const string &id, const string &key, const int val) {
    sp[chan]->SetParamValue(id, key, val);
}

void SPManager::ChannelSavePreset(const int chan, const string &name, const int number) {
    if (sp[chan] == nullptr) return;
    audioMutex.lock();
    sp[chan]->SavePreset(name, number);
    model->SetActivePatchNum(number, chan);
    audioMutex.unlock();
}

void SPManager::ChannelLoadPreset(const int chan, const int number) {
    if (sp[chan] == nullptr) return;
    audioMutex.lock();
    sp[chan]->LoadPreset(number);
    model->SetActivePatchNum(number, chan);
    audioMutex.unlock();
}

string SPManager::GetStringID(const int chan) {
    return model->GetActiveProcessorID(chan);
}

void SPManager::SetProcessParams(const string &params) {
    /*
    simModel->SetModelJSONString(params);
    int mode[6], value[6];
    for (int i = 0; i < 6; i++) {
        mode[i] = simModel->GetArrayElement("mode", i);
        value[i] = simModel->GetArrayElement("value", i);
    }
    stimulus.UpdateStimulus(mode, value);
     */
}
