#include <tbd/sound_manager.hpp>

#include <tbd/system/modules.hpp>

// same compilation unit
#include "sound_manager_impl.cpp"


using namespace tbd::audio;

namespace CTAG::AUDIO {

namespace {
    tbd::system::ModuleTask<AudioManagerImpl> audio_task("audio_task");
}

void SoundProcessorManager::StartSoundProcessor() {
    audio_task.begin();
}

void SetSoundProcessorChannel(int chan, const std::string& id) {

}

void SetChannelParamValue(int chan, const std::string& id, const std::string& key, int val) {

}

void ChannelSavePreset(int chan, const std::string& name, int number) {

}

void ChannelLoadPreset(int chan, int number) {

}

void SoundProcessorManager::KillAudioTask() {
    audio_task.end();
}

void SoundProcessorManager::DisablePluginProcessing() {
    audio_task.pause_processing();
}

void SoundProcessorManager::EnablePluginProcessing() {
    audio_task.resume_processing();
}

}
