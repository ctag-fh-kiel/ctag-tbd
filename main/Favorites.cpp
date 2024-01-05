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

#if CONFIG_TBD_PLATFORM_AEM
    #define PIN_PUSH_BTN GPIO_NUM_2
#elif CONFIG_TBD_PLATFORM_MK2
    #define PIN_PUSH_BTN GPIO_NUM_34
#elif CONFIG_TBD_PLATFORM_BBA
    #include "driver/touch_pad.h"
    #define TOUCH_PAD TOUCH_PAD_NUM6 // is GPIO_NUM_6
    static uint32_t noTouch {0}, previousProgramChangeValue {0xFF000000};
    std::atomic<uint32_t> CTAG::FAV::Favorites::programChangeValue {0xFF000000};
    void CTAG::FAV::Favorites::SetProgramChangeValue(uint32_t const &v) {
        programChangeValue.store(v);
    }
#endif

bool CTAG::FAV::Favorites::isUIEnabled {false};
CTAG::FAV::FavoritesModel CTAG::FAV::Favorites::model;
int32_t CTAG::FAV::Favorites::activeFav {-1};
#if defined(CONFIG_TBD_PLATFORM_MK2) || defined(CONFIG_TBD_PLATFORM_AEM) || defined(CONFIG_TBD_PLATFORM_BBA)
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
void CTAG::FAV::Favorites::StartUI() {
#if defined(CONFIG_TBD_PLATFORM_MK2) || defined(CONFIG_TBD_PLATFORM_AEM)
        gpio_set_direction(PIN_PUSH_BTN, (gpio_mode_t)GPIO_MODE_DEF_INPUT);
        xTaskCreatePinnedToCore(&CTAG::FAV::Favorites::ui_task, "ui_task", 4096, nullptr, tskIDLE_PRIORITY + 3, &uiTaskHandle, 0);
#elif CONFIG_TBD_PLATFORM_BBA
        touch_pad_init();
        touch_pad_config(TOUCH_PAD);
        touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
        touch_pad_fsm_start();
        std::vector<std::string> vs;
        vs.emplace_back("Touch sensor");
        vs.emplace_back("calibration:");
        vs.emplace_back("Do not touch!");
        DRIVERS::Display::ShowUserString(vs);
        vTaskDelay(2000/ portTICK_PERIOD_MS);
        uint32_t touch_value;
        for(int i=0;i<16;i++){
            touch_pad_read_raw_data(TOUCH_PAD, &touch_value);
            noTouch += touch_value;
        }
        noTouch /= 16;
        isUIEnabled = true;
        xTaskCreatePinnedToCore(&CTAG::FAV::Favorites::ui_task, "ui_task", 4096, nullptr, tskIDLE_PRIORITY + 3, &uiTaskHandle, 0);
#endif
}

void CTAG::FAV::Favorites::DeactivateFavorite() {
    activeFav = -1;
    uiMenuState = CLEAR;
}

#if defined(CONFIG_TBD_PLATFORM_MK2) || defined(CONFIG_TBD_PLATFORM_AEM) || defined(CONFIG_TBD_PLATFORM_BBA)
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
    while (true) {
        if(!isUIEnabled){
            DRIVERS::Display::Clear();
            uiMenuState = CLEAR;
            pre_state = CLEAR;
        }
        // check button state and generate events
#if CONFIG_TBD_PLATFORM_MK2
        if (!gpio_get_level(PIN_PUSH_BTN)) {
#elif CONFIG_TBD_PLATFORM_BBA
        uint32_t touch_value;
        touch_pad_read_raw_data(TOUCH_PAD, &touch_value);    // read raw data.
        uint32_t pchgval = programChangeValue.load();
        if(pchgval != previousProgramChangeValue){
            previousProgramChangeValue = pchgval;
            int fav = previousProgramChangeValue & 0xFF;
            fav++; // to match fav1 = 1, fav0 = 10
            activeFav = favSel;
            ActivateFavorite(fav % 10);
            uiMenuState = FAV_ACTIVE_NAME;
        }
        if(touch_value > noTouch + 1000) {
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
                    DRIVERS::Display::PrepareDisplayFavoriteUString(activeFav, model.GetFavoriteName(activeFav), model.GetFavoriteUString(activeFav));
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

void CTAG::FAV::Favorites::DisableFavoritesUI() {
    isUIEnabled = false;
}

void CTAG::FAV::Favorites::EnableFavoritesUI() {
    isUIEnabled = true;
}

#endif