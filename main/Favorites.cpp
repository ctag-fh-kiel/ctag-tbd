/***************
TBD-16 — dadamachines WebUI & REST API

(c) 2024-2026 Johannes Elias Lohbihler for dadamachines.

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

Part of the dadamachines additions to the CTAG TBD platform.
See LICENSE in the repository root for full terms.

Provided "as is" without any express or implied warranties.

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