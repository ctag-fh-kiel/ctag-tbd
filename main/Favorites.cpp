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
#include "driver/gpio.h"
#include "Display.hpp"

#define UI_TASK_PERIOD_MS 50
#define LONG_PRESS_PERIOD_MS 1000
#define SCROLL_RATE_MS 1000
#define TIMEOUT_PERIOD_MS 5000

#if defined(CONFIG_TBD_PLATFORM_AEM)
    #define PIN_PUSH_BTN GPIO_NUM_2
#elif defined(CONFIG_TBD_PLATFORM_MK2)
    #define PIN_PUSH_BTN GPIO_NUM_34
#endif

CTAG::FAV::FavoritesModel CTAG::FAV::Favorites::model;
int32_t CTAG::FAV::Favorites::activeFav {-1};
#if defined(CONFIG_TBD_PLATFORM_MK2) || defined(CONFIG_TBD_PLATFORM_AEM)
    TaskHandle_t CTAG::FAV::Favorites::uiTaskHandle {nullptr};
#endif
CTAG::FAV::Favorites::MenuStates CTAG::FAV::Favorites::uiMenuState {CLEAR};

string CTAG::FAV::Favorites::GetAllFavorites() {
    return model.GetAllFavorites();
}

void CTAG::FAV::Favorites::StoreFavorite(int const &id, const string &fav) {
    model.SetFavorite(id, fav);
    activeFav = id;
    uiMenuState = CLEAR;
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
    uiMenuState = CLEAR;
}
#if defined(CONFIG_TBD_PLATFORM_MK2) || defined(CONFIG_TBD_PLATFORM_AEM)
    void CTAG::FAV::Favorites::StartUI() {
        gpio_set_direction(PIN_PUSH_BTN, (gpio_mode_t)GPIO_MODE_DEF_INPUT);
        xTaskCreatePinnedToCore(&CTAG::FAV::Favorites::ui_task, "ui_task", 4096, nullptr, tskIDLE_PRIORITY + 3, &uiTaskHandle, 0);
    }
#endif

void CTAG::FAV::Favorites::DeactivateFavorite() {
    activeFav = -1;
    uiMenuState = CLEAR;
}

#if defined(CONFIG_TBD_PLATFORM_MK2) || defined(CONFIG_TBD_PLATFORM_AEM)
// UI task menu state machine
[[noreturn]] void CTAG::FAV::Favorites::ui_task(void *pvParams) {
    int timer {0}; // btn event timer
    int timer2 {0}; // scroll timer
    int timer3 {0}; // timeout timer
    int favSel {0};
    enum Event {NONE, SHORT, LONG, WAIT, TIMEOUT} event {NONE};
    MenuStates pre_state {CLEAR};
    MenuStates return_state {CLEAR};
    DRIVERS::Display::Clear();
    while (1) {
        // check button state and generate events
#if defined(CONFIG_TBD_PLATFORM_MK2)
        if (!gpio_get_level(PIN_PUSH_BTN)) {
#else
        if(gpio_get_level(PIN_PUSH_BTN)){
#endif
            if (timer != -1) timer++;
            if (timer >= LONG_PRESS_PERIOD_MS / UI_TASK_PERIOD_MS) {
                event = LONG;
                timer = -1;
            } else {
                event = NONE;
            }
            timer3 = 0;
        } else {
            if (timer > 0 && timer < LONG_PRESS_PERIOD_MS / UI_TASK_PERIOD_MS) {
                event = SHORT;
            } else event = NONE;
            timer = 0;
            timer3++;
            // generate timeout event if in fav select
            if (uiMenuState == FAV_SELECT_CONFIRM){
                if (timer3 >= TIMEOUT_PERIOD_MS / UI_TASK_PERIOD_MS) {
                    event = TIMEOUT;
                    timer3 = 0;
                }
            }
        }

        // any key event? or new state? --> process state machine for menu
        // only once if an event is not NONE or state is changing
        if (event != NONE || pre_state != uiMenuState) {
            pre_state = uiMenuState;
            switch (uiMenuState) {
                case CLEAR:
                    DRIVERS::Display::Clear();
                    if (event == SHORT) uiMenuState = FAV_ACTIVE_NAME;
                    if (event == LONG){
                        uiMenuState = FAV_SELECT;
                        return_state = CLEAR;
                    }
                    break;
                case FAV_ACTIVE_NAME:
                    if (activeFav != -1) {
                        //DRIVERS::Display::ShowFavorite(activeFav, model.GetFavoriteName(activeFav));
                        uiMenuState = FAV_ACTIVE_USTRING;
                    } else {
                        DRIVERS::Display::UserMode();
                        if (event == SHORT) uiMenuState = CLEAR;
                        if (event == LONG){
                            uiMenuState = FAV_SELECT;
                            return_state = FAV_ACTIVE_NAME;
                        }
                    }
                    //if(event == LONG) uiMenuState = FAV_SELECT;
                    break;
                case FAV_ACTIVE_USTRING:
                    DRIVERS::Display::PrepareDisplayFavoriteUString(model.GetFavoriteUString(activeFav));
                    if (event == SHORT) uiMenuState = CLEAR;
                    if (event == LONG){
                        uiMenuState = FAV_SELECT;
                        return_state = FAV_ACTIVE_NAME;
                    }
                    break;
                case FAV_SELECT:
                    if (activeFav != -1) favSel = activeFav;
                    uiMenuState = FAV_SELECT_CONFIRM;
                    break;
                case FAV_SELECT_CONFIRM:
                    if (event == SHORT) {
                        favSel++;
                        if (favSel > 9) favSel = 0;
                    }
                    DRIVERS::Display::LoadFavorite(favSel, model.GetFavoriteName(favSel));
                    //DRIVERS::Display::Confirm(favSel);
                    if (event == LONG) {
                        activeFav = favSel;
                        ActivateFavorite(favSel);
                        uiMenuState = FAV_ACTIVE_NAME;
                    }
                    if (event == TIMEOUT) {
                        uiMenuState = return_state;
                    }
                    break;
            }
        }
        // generate scroll events
        if (uiMenuState == FAV_ACTIVE_USTRING) {
            timer2++;
            timer2 %= SCROLL_RATE_MS / UI_TASK_PERIOD_MS;
            if (timer2 == 0) {
                DRIVERS::Display::UpdateFavoriteUStringScroll();
            }
        }
        vTaskDelay(UI_TASK_PERIOD_MS / portTICK_PERIOD_MS);
    }
}
#endif