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

#include <tbd/favorites.hpp>
#include <tbd/sound_manager.hpp>


#if TBD_DISPLAY
    #include "ui_task.hpp"
#else
    #include "ui_worker_dummy.hpp"
#endif

#if TBD_CV_MIDI
    static uint32_t noTouch {0}, previousProgramChangeValue {0xFF000000};
    std::atomic<uint32_t> tbd::Favorites::programChangeValue {0xFF000000};
    void tbd::Favorites::SetProgramChangeValue(uint32_t const &v) {
        programChangeValue.store(v);
    }
#endif


using namespace tbd::favorites;

namespace tbd {

string Favorites::GetAllFavorites() {
    return model.GetAllFavorites();
}


void Favorites::StoreFavorite(int const &id, const string &fav) {
    model.SetFavorite(id, fav);
    active_fav = id;
    ui_worker.clear();
}


void Favorites::ActivateFavorite(const int &id) {
    if(id < 0 || id > 9) return;
    // NOTE: all checks if plugins exists and if presets exists are done in SPManager
    string p0id = model.GetFavoritePluginID(id, 0);
    int p0pre = model.GetFavoritePreset(id, 0);
    tbd::audio::SoundProcessorManager::SetSoundProcessorChannel(0, p0id);
    tbd::audio::SoundProcessorManager::ChannelLoadPreset(0, p0pre);
    string p1id = model.GetFavoritePluginID(id, 1);
    int p1pre = model.GetFavoritePreset(id, 1);
    tbd::audio::SoundProcessorManager::SetSoundProcessorChannel(1, p1id);
    tbd::audio::SoundProcessorManager::ChannelLoadPreset(1, p1pre);
    active_fav = id;
    ui_worker.clear();
}


void Favorites::StartUI() {
#if CONFIG_TBD_PLATFORM_BBA
        TouchPad::init();
        std::vector<std::string> vs;
        vs.emplace_back("Touch sensor");
        vs.emplace_back("calibration:");
        vs.emplace_back("Do not touch!");
        Display::ShowUserString(vs);
        vTaskDelay(2000/ portTICK_PERIOD_MS);
        uint32_t touch_value;
        TouchPad::calibrate();
        xTaskCreatePinnedToCore(&Favorites::ui_task, "ui_task", 4096, nullptr, tskIDLE_PRIORITY + 3, &uiTaskHandle, 0);
#endif
    ui_worker.begin();
    ui_worker.enable();
}


void Favorites::DeactivateFavorite() {
    active_fav = -1;
    ui_worker.clear();
}


void Favorites::DisableFavoritesUI() {
    ui_worker.disable();
}


void Favorites::EnableFavoritesUI() {
    ui_worker.enable();
}

}
