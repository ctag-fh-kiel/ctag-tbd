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

#include "Favorites.hpp"
#include "SPManager.hpp"


CTAG::FAV::FavoritesModel CTAG::FAV::Favorites::model;
int32_t CTAG::FAV::Favorites::activeFav {-1};

string CTAG::FAV::Favorites::GetAllFavorites() {
    return model.GetAllFavorites();
}

void CTAG::FAV::Favorites::StoreFavorite(int const &id, const string &fav) {
    model.SetFavorite(id, fav);
    activeFav = id;
}

void CTAG::FAV::Favorites::ActivateFavorite(const int &id) {
    if(id < 0 || id > 9) return;
    // NOTE: all checks if plugins exists and if presets exists are done in SPManager
    string p0id = model.GetFavoritePluginID(id, 0);
    int p0pre = model.GetFavoritePreset(id, 0);
    CTAG::AUDIO::SoundProcessorManager::SetSoundProcessorChannel(0, p0id);
    CTAG::AUDIO::SoundProcessorManager::ChannelLoadPreset(0, p0pre);
    string p1id = model.GetFavoritePluginID(id, 1);
    int p1pre = model.GetFavoritePreset(id, 1);
    CTAG::AUDIO::SoundProcessorManager::SetSoundProcessorChannel(1, p1id);
    CTAG::AUDIO::SoundProcessorManager::ChannelLoadPreset(1, p1pre);
    activeFav = id;
}

void CTAG::FAV::Favorites::DeactivateFavorite() {
    activeFav = -1;
}