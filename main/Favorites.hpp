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
            static void DisableFavoritesUI();
            static void EnableFavoritesUI();
#if defined(CONFIG_TBD_PLATFORM_MK2) || defined(CONFIG_TBD_PLATFORM_AEM) || defined(CONFIG_TBD_PLATFORM_BBA)
            static void StartUI();
#endif
#ifdef CONFIG_TBD_PLATFORM_BBA
            static void SetProgramChangeValue(uint32_t const &v);
#endif
            static void TouchPadHandler();
        private:
            static bool isUIEnabled;
            static FavoritesModel model;
            static int32_t activeFav;
            enum MenuStates {CLEAR, FAV_ACTIVE_NAME, FAV_ACTIVE_USTRING, FAV_SELECT, FAV_SELECT_CONFIRM};
            static MenuStates uiMenuState;
#if defined(CONFIG_TBD_PLATFORM_MK2) || defined(CONFIG_TBD_PLATFORM_AEM) || defined(CONFIG_TBD_PLATFORM_BBA)
            [[noreturn]] static void ui_task(void *pvParams);
            static TaskHandle_t uiTaskHandle;
#endif
#ifdef CONFIG_TBD_PLATFORM_BBA
            static std::atomic<uint32_t> programChangeValue;
#endif
        };
    }
}

