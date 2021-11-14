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
#include <sstream>
#include "ctagResources.hpp"

using namespace CTAG::AUDIO;

// global variable, spiffs base directory
namespace CTAG {
    namespace RESOURCES {
        std::string spiffsRoot {""};
    }
}

std::mutex audioMutex;

void SPManager::Start(const string &spiffsPath) {
    CTAG::RESOURCES::spiffsRoot = spiffsPath;
    // configure channels
    model = std::make_unique<SPManagerDataModel>("{\"activeProcessors\":[],\"lastPatches\":[[],[]]}");
    favModel = std::make_unique<CTAG::FAV::FavoritesModel>();
    sp[0] = ctagSoundProcessorFactory::Create(model->GetActiveProcessorID(0));
    sp[0]->SetProcessChannel(0);
    sp[0]->LoadPreset(model->GetActivePatchNum(0));
    if (!sp[0]->GetIsStereo()) {
        sp[1] = ctagSoundProcessorFactory::Create(model->GetActiveProcessorID(1));
        sp[1]->SetProcessChannel(1);
        sp[1]->LoadPreset(model->GetActivePatchNum(1));
    }
}

void SPManager::SetSoundProcessorChannel(const int chan, const string &id) {
    printf("Switching plugin %d to %s", chan, id.c_str());
    // does the SP exist?
    if(!model->HasPluginID(id)) return;

    // when trying to set chan 1 and chan 0 is a stereo plugin, return
    if(chan == 1 && model->IsStereo(model->GetActiveProcessorID(0))) return;
    audioMutex.lock();
    blue = true;
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
    blue = true;
    sp[chan]->SetParamValue(id, key, val);
}

void SPManager::ChannelSavePreset(const int chan, const string &name, const int number) {
    if (sp[chan] == nullptr) return;
    audioMutex.lock();
    blue = true;
    sp[chan]->SavePreset(name, number);
    model->SetActivePatchNum(number, chan);
    audioMutex.unlock();
}

void SPManager::ChannelLoadPreset(const int chan, const int number) {
    if (sp[chan] == nullptr) return;
    audioMutex.lock();
    blue = true;
    sp[chan]->LoadPreset(number);
    model->SetActivePatchNum(number, chan);
    audioMutex.unlock();
}

string SPManager::GetStringID(const int chan) {
    return model->GetActiveProcessorID(chan);
}

void SPManager::Process(const CTAG::SP::ProcessData &data) {
    // sound processors
    bool isStereoCH0 {false};

    fv3::dccut_f in_dccutl, in_dccutr;
    in_dccutl.setCutOnFreq(3.7f, 44100.f);
    in_dccutr.setCutOnFreq(3.7f, 44100.f);
    // dc cut input
    for (uint32_t i = 0; i < 32; i++) {
        data.buf[i * 2] = in_dccutl(data.buf[i * 2]);
        data.buf[i * 2 + 1] = in_dccutr(data.buf[i * 2 + 1]);
    }

    if (audioMutex.try_lock()) {
        if (sp[0] != nullptr) {
            isStereoCH0 = sp[0]->GetIsStereo();
            sp[0]->Process(data);
        }
        if (!isStereoCH0)
            if (sp[1] != nullptr)
                sp[1]->Process(data); // 0 is not a stereo processor
        audioMutex.unlock();

    }
    // softclip out
    for(int i=0;i<32;i++){
        data.buf[i*2] = stmlib::SoftClip(data.buf[i*2]);
        data.buf[i*2 + 1] = stmlib::SoftClip(data.buf[i*2 + 1]);
    }
}

bool SPManager::GetBlueStatus() {
    bool stat = blue;
    blue = false;
    return stat;
}

string SPManager::GetSPManagerDataModel() {
    return model->GetSPManagerDataModel();
}

void SPManager::SetSPManagerDataModel(const string &json) {
    model->SetSPManagerDataModel(json);
    if(sp[0] != nullptr) sp[0] = nullptr;
    if(sp[1] != nullptr) sp[1] = nullptr;
    sp[0] = ctagSoundProcessorFactory::Create(model->GetActiveProcessorID(0));
    sp[0]->SetProcessChannel(0);
    sp[0]->LoadPreset(model->GetActivePatchNum(0));
    if (!sp[0]->GetIsStereo()) {
        sp[1] = ctagSoundProcessorFactory::Create(model->GetActiveProcessorID(1));
        sp[1]->SetProcessChannel(1);
        sp[1]->LoadPreset(model->GetActivePatchNum(1));
    }
}

string SPManager::GetAllFavorites() {
    return favModel->GetAllFavorites();
}

void SPManager::StoreFavorite(const int &id, const string &fav) {
    favModel->SetFavorite(id, fav);
}

void SPManager::ActivateFavorite(const int &id) {
    if(id < 0 || id > 9) return;
    // NOTE: all checks if plugins exists and if presets exists are done in SPManager
    string p0id = favModel->GetFavoritePluginID(id, 0);
    int p0pre = favModel->GetFavoritePreset(id, 0);
    SetSoundProcessorChannel(0, p0id);
    ChannelLoadPreset(0, p0pre);
    string p1id = favModel->GetFavoritePluginID(id, 1);
    int p1pre = favModel->GetFavoritePreset(id, 1);
    SetSoundProcessorChannel(1, p1id);
    ChannelLoadPreset(1, p1pre);
}