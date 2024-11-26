#include <tbd/sound_manager.hpp>

#define TBD_IS_AUDIO_LOOP_COMPILATION_UNIT

#include <tbd/system/modules.hpp>
#include <tbd/favorites.hpp>

// same compilation unit
#include <tbd/sound_manager/data_model.hpp>
#include "audio_worker_impl.hpp"



namespace {
    auto model = std::make_unique<tbd::audio::SPManagerDataModel>();
}

namespace tbd::audio {

void SoundProcessorManager::StartSoundProcessor() {
    sound_level_worker.set_blink_duration(5);
    
    // check for network reset at bootup
#ifdef TBD_INDICATOR
    // uses SW1 = BOOT of esp32-s3-devkitc to reset network credentials
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    if(gpio_get_level(GPIO_NUM_0) == 0){
        model->ResetNetworkConfiguration();
        TBD_LOGE("SP", "Network credentials reset requested!");
        drivers::LedRGB::SetLedRGB(255, 255, 255);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
#endif

    // FIXME: verfiy that this can be removed
    //
    // #ifdef CONFIG_TBD_PLATFORM_STR
    //     // inverted here as some pins are used twice --> check for issues
    //     drivers::Codec::InitCodec();
    //     CTRL::Control::Init();
    // #else ...

    // init control
    InputManager::Init();
    // init codec

    // generate internal data
    updateConfiguration();

#ifndef CONFIG_TBD_PLATFORM_STR
    
#endif
    InputManager::FlushBuffers();
    // create audio thread
    audio_worker.begin();

#if defined(CONFIG_TBD_PLATFORM_MK2) || defined(CONFIG_TBD_PLATFORM_AEM) || defined(CONFIG_TBD_PLATFORM_BBA)
    Favorites::StartUI();
#endif

    SetSoundProcessorChannel(0, model->GetActiveProcessorID(0));
    SetSoundProcessorChannel(1, model->GetActiveProcessorID(1));
    TBD_LOGI("SPManager", "Init: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             tbd_heaps_get_free_size(TBD_HEAPS_8BIT | TBD_HEAPS_INTERNAL),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_INTERNAL | TBD_HEAPS_INTERNAL),
             tbd_heaps_get_free_size(TBD_HEAPS_SPIRAM),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_SPIRAM));

    audio_worker.begin();
}

string SoundProcessorManager::GetStringID(const int chan) {
   sound_level_worker.set_blink_duration(5);
    return model->GetActiveProcessorID(chan);
}


void SoundProcessorManager::SetSoundProcessorChannel(int chan, const std::string& id) {
   sound_level_worker.set_blink_duration(5);

    // does the SP exist?
    if(!model->HasPluginID(id)) return;

    // when trying to set chan 1 and chan 0 is a stereo plugin, return
    if(chan == 1 && model->IsStereo(model->GetActiveProcessorID(0))) return;
    if(chan == 1 && model->IsStereo(id)) return;

    TBD_LOGI("SPManager", "Switching ch%d to plugin %s", chan, id.c_str());

    // destroy active plugin
    {
        auto sound_processing_guard = sound_processing_lock.guard();
        if (sound_processing_guard) {
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
            CTAG::SP::ctagSPAllocator::AllocationType aType = CTAG::SP::ctagSPAllocator::AllocationType::CH0;
            if(chan == 1) aType = CTAG::SP::ctagSPAllocator::AllocationType::CH1;
            if(model->IsStereo(id)) aType = CTAG::SP::ctagSPAllocator::AllocationType::STEREO;
            sp[chan] = CTAG::SP::ctagSoundProcessorFactory::Create(id, aType);
            model->SetActivePluginID(id, chan);
            sp[chan]->LoadPreset(model->GetActivePatchNum(chan));
        }
    }


    TBD_LOGI("SPManager", "Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             tbd_heaps_get_free_size(TBD_HEAPS_8BIT | TBD_HEAPS_INTERNAL),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_8BIT | TBD_HEAPS_INTERNAL),
             tbd_heaps_get_free_size(TBD_HEAPS_SPIRAM),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_SPIRAM));

}


void SoundProcessorManager::SetChannelParamValue(int chan, const std::string& id, const std::string& key, int val) {
    sound_level_worker.set_blink_duration(3);
    sp[chan]->SetParamValue(id, key, val);
}


void SoundProcessorManager::ChannelSavePreset(int chan, const std::string& name, int number) {
    sound_level_worker.set_blink_duration(3);
    if (sp[chan] == nullptr) return;
    if (sp[chan] == nullptr) return;

    {
        auto sound_processing_guard = sound_processing_lock.guard();
        if (sound_processing_guard) {
            sp[chan]->SavePreset(name, number);
            model->SetActivePatchNum(number, chan);
        }
    }
}


void SoundProcessorManager::ChannelLoadPreset(int chan, int number) {
    sound_level_worker.set_blink_duration(3);
    if (sp[chan] == nullptr) return;
    {
        auto sound_processing_guard = sound_processing_lock.guard();
        if (sound_processing_guard) {
            sp[chan]->LoadPreset(number);
            model->SetActivePatchNum(number, chan);
        }
    }
}


void SoundProcessorManager::KillAudioTask() {
    Favorites::DisableFavoritesUI();

    // stop audio Task, delete plugins
    audio_worker.end(true);
    if(nullptr!=sp[0]) delete sp[0];
    if(nullptr!=sp[1]) delete sp[1];
    sp[0] = nullptr;
    sp[1] = nullptr;
    CTAG::SP::ctagSPAllocator::ReleaseInternalBuffer();
#ifndef CONFIG_TBD_PLATFORM_STR
    audio_worker.end();
    system::Task::sleep(100);
    drivers::Indicator::SetLedRGB(255, 0, 255);
#endif
    TBD_LOGI("SPManager", "Audio Task Killed: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             tbd_heaps_get_free_size(TBD_HEAPS_8BIT | TBD_HEAPS_INTERNAL),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_8BIT | TBD_HEAPS_INTERNAL),
             tbd_heaps_get_free_size(TBD_HEAPS_SPIRAM),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_SPIRAM));
}


void SoundProcessorManager::DisablePluginProcessing() {
    audio_worker.pause_processing();
}


void SoundProcessorManager::EnablePluginProcessing() {
    audio_worker.resume_processing();
}

void SoundProcessorManager::updateConfiguration() {
    sound_level_worker.set_blink_duration(3);
    InputManager::SetCVChannelBiPolar(model->GetConfigurationData("cv_ch0") == "bipolar",
                              model->GetConfigurationData("cv_ch1") == "bipolar",
                              model->GetConfigurationData("cv_ch2") == "bipolar",
                              model->GetConfigurationData("cv_ch3") == "bipolar");

    // noise gate configuration
    if (model->GetConfigurationData("ng_config").compare("off") == 0) {
        params.noiseGateCfg = 0;
    } else if (model->GetConfigurationData("ng_config").compare("dual") == 0) {
        audio_worker.let_signal_settle();
        params.noiseGateCfg = 1;
    } else if (model->GetConfigurationData("ng_config").compare("ch0") == 0) {
        audio_worker.let_signal_settle();
        params.noiseGateCfg = 2;
    } else if (model->GetConfigurationData("ng_config").compare("ch1") == 0) {
        audio_worker.let_signal_settle();
        params.noiseGateCfg = 3;
    }

    // ch01 daisy
    if (model->GetConfigurationData("ch01_daisy").compare("off") == 0) {
        params.ch01Daisy = 0;
    }else if(model->GetConfigurationData("ch01_daisy").compare("on") == 0){
        params.ch01Daisy = 1;
    }
    // mono to stereo channel cfg
    if (model->GetConfigurationData("ch0_toStereo").compare("off") == 0) {
        params.toStereoCH0 = 0;
    } else if (model->GetConfigurationData("ch0_toStereo").compare("on") == 0) {
        params.toStereoCH0 = 1;
    } else if (model->GetConfigurationData("ch0_toStereo").compare("mix") == 0) {
        params.toStereoCH0 = 2;
    }
    if (model->GetConfigurationData("ch1_toStereo").compare("off") == 0) {
        params.toStereoCH1 = 0;
    } else if (model->GetConfigurationData("ch1_toStereo").compare("on") == 0) {
        params.toStereoCH1 = 1;
    } else if (model->GetConfigurationData("ch1_toStereo").compare("mix") == 0) {
        params.toStereoCH1 = 2;
    }

    // soft clipping?
    if (model->GetConfigurationData("ch0_outputSoftClip").compare("off") == 0) {
        params.ch0_outputSoftClip = 0;
    } else if (model->GetConfigurationData("ch0_outputSoftClip").compare("on") == 0) {
        params.ch0_outputSoftClip = 1;
    }
    if (model->GetConfigurationData("ch1_outputSoftClip").compare("off") == 0) {
        params.ch1_outputSoftClip = 0;
    } else if (model->GetConfigurationData("ch1_outputSoftClip").compare("on") == 0) {
        params.ch1_outputSoftClip = 1;
    }

    // output levels of codec
    if(model->GetConfigurationData("ch0_codecLvlOut").compare("") != 0){
        if(model->GetConfigurationData("ch0_codecLvlOut").compare("") != 0){
            int lLevel = std::stoi(model->GetConfigurationData("ch0_codecLvlOut"));
            int rLevel = std::stoi(model->GetConfigurationData("ch1_codecLvlOut"));
#ifdef CONFIG_TBD_BBA_CODEC_ES8388
            CONSTRAIN(rLevel, 0, 36)
            CONSTRAIN(lLevel, 0, 36)
#else
            CONSTRAIN(rLevel, 0, 63)
            CONSTRAIN(lLevel, 0, 63)
#endif
            audio_worker.set_output_levels(lLevel, rLevel);
        }
    }
}

const char* SoundProcessorManager::GetCStrJSONSoundProcessors() {
    sound_level_worker.set_blink_duration(1);
    return model->GetCStrJSONSoundProcessors();
}


const char* SoundProcessorManager::GetCStrJSONActivePluginParams(int chan) {
    sound_level_worker.set_blink_duration(1);
    return sp[chan]->GetCStrJSONParamSpecs();
}


const char* SoundProcessorManager::GetCStrJSONGetPresets(int chan) {
    sound_level_worker.set_blink_duration(1);
    return sp[chan]->GetCStrJSONPresets();
}


const char* SoundProcessorManager::GetCStrJSONAllPresetData(int chan) {
    sound_level_worker.set_blink_duration(1);
    return sp[chan]->GetCStrJSONAllPresetData();        
}


const char* SoundProcessorManager::GetCStrJSONConfiguration() {
    sound_level_worker.set_blink_duration(1);
    return model->GetCStrJSONConfiguration();
}


const char* SoundProcessorManager::GetCStrJSONSoundProcessorPresets(const std::string& id) {
    sound_level_worker.set_blink_duration(1);
    return model->GetCStrJSONSoundProcessorPresets(id);
}


void SoundProcessorManager::SetCStrJSONSoundProcessorPreset(const char* id, const char* data) {
    sound_level_worker.set_blink_duration(1);
    model->SetCStrJSONSoundProcessorPreset(id, data);
}


void SoundProcessorManager::SetConfigurationFromJSON(const std::string& data) {
    sound_level_worker.set_blink_duration(3);   
    {
        auto sound_processing_guard = sound_processing_lock.guard();
        if (sound_processing_guard) {
            model->SetConfigurationFromJSON(data);
            updateConfiguration();
        }
    }
}

void SoundProcessorManager::RefreshSampleRom() {
    sound_level_worker.set_blink_duration(3);   
    ctagSampleRom::RefreshDataStructure();
}


}
