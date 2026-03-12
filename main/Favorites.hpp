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

#pragma once

#include "FavoritesModel.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <atomic>

namespace CTAG {
    namespace FAV {
        class Favorites final {
        public:
            Favorites() = delete;
            static string GetAllFavorites();
            static void StoreFavorite(int const &id, const string &fav);
            static void ActivateFavorite(const int &id);
            static void DeactivateFavorite();
        private:
            static FavoritesModel model;
            static int32_t activeFav;
        };
    }
}

