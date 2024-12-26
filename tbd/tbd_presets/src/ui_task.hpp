#include <tbd/system/task_module.hpp>

#include <memory>

#include <tbd/drivers/gpio.hpp>
#include <tbd/favorites.hpp>

#define UI_TASK_PERIOD_MS 50
#define LONG_PRESS_PERIOD_MS 1000
#define SCROLL_RATE_MS 1000
#define TIMEOUT_PERIOD_MS 5000

#if TBD_DISPLAY
    #include <tbd/display.hpp>
#endif

namespace tbd::favorites {

std::unique_ptr<FavoritesModel> model;
int32_t active_fav = -1;
int32_t selected_fav = -1;

struct UIWorker {
    uint32_t do_begin();
    uint32_t do_work();
    uint32_t do_cleanup();

    void enable() { _is_enabled = true; }
    void disable() { _is_enabled = false; }
    void clear() { _state = CLEAR; }

private:
    enum MenuState {
        CLEAR, 
        FAV_ACTIVE_NAME, 
        FAV_ACTIVE_USTRING, 
        FAV_SELECT, 
        FAV_SELECT_CONFIRM
    };

    enum Event {
        NONE, 
        SHORT, 
        LONG, 
        WAIT, 
        TIMEOUT
    } _event = NONE;

    int _timer  = 0; // btn event timer
    int _timer2 = 0; // scroll timer
    int _timer3 = 0; // timeout timer
    int _favSel = 0;

    MenuState _pre_state = CLEAR;
    MenuState _return_state = CLEAR;
    MenuState _state;
    bool _is_enabled = false;
};

uint32_t UIWorker::do_begin() {
    Display::Clear();
    _is_enabled = true;
    return 0;
}

uint32_t UIWorker::do_cleanup() {
    return 0;
}

uint32_t UIWorker::do_work() {
    system::Task::sleep(UI_TASK_PERIOD_MS);
    if(!_is_enabled) {
        return 0;
    }
    Display::Clear();
    _state = CLEAR;
    _pre_state = CLEAR;

// check button state and generate events
#if TBD_PLATFORM_MK2
    if (!drivers::GPIO::GetPushButton()) {
        return 0;
    }
#elif TBD_PLATFORM_BBA
    uint32_t touch_value;
        // read raw data.
    uint32_t pchgval = programChangeValue.load();
    if(pchgval != previousProgramChangeValue){
        previousProgramChangeValue = pchgval;
        int fav = previousProgramChangeValue & 0xFF;
        fav++; // to match fav1 = 1, fav0 = 10
        activeFav = favSel;
        ActivateFavorite(fav % 10);
    }
    if(touch_value > noTouch + 1000) {
#else
    if(drivers::GPIO::GetPushButton()) {
#endif
        if (_timer != -1) _timer++;
        if (_timer >= LONG_PRESS_PERIOD_MS / UI_TASK_PERIOD_MS) {
            _event = LONG;
            _timer = -1;
        } else {
            _event = NONE;
        }
        _timer3 = 0;
    } else {
        if (_timer > 0 && _timer < LONG_PRESS_PERIOD_MS / UI_TASK_PERIOD_MS) {
            _event = SHORT;
        } else _event = NONE;
        _timer = 0;
        _timer3++;
        // generate timeout _event if in fav select
        if (_state == FAV_SELECT_CONFIRM){
            if (_timer3 >= TIMEOUT_PERIOD_MS / UI_TASK_PERIOD_MS) {
                _event = TIMEOUT;
                _timer3 = 0;
            }
        }
    }

    // any key _event? or new state? --> process state machine for menu
    // only once if an event is not NONE or state is changing
    if (_event != NONE || _pre_state != _state) {
        _pre_state = _state;
        switch (_state) {
            case CLEAR:
                Display::Clear();
                if (_event == SHORT) _state = FAV_ACTIVE_NAME;
                if (_event == LONG){
                    _state = FAV_SELECT;
                    _return_state = CLEAR;
                }
                break;
            case FAV_ACTIVE_NAME:
                if (active_fav != -1) {
                    //Display::ShowFavorite(activeFav, model.GetFavoriteName(activeFav));
                    _state = FAV_ACTIVE_USTRING;
                } else {
                    Display::UserMode();
                    if (_event == SHORT) _state = CLEAR;
                    if (_event == LONG){
                        _state = FAV_SELECT;
                        _return_state = FAV_ACTIVE_NAME;
                    }
                }
                //if(_event == LONG) uiMenuState = FAV_SELECT;
                break;
            case FAV_ACTIVE_USTRING:
                Display::PrepareDisplayFavoriteUString(active_fav, model->GetFavoriteName(active_fav), model->GetFavoriteUString(selected_fav));
                if (_event == SHORT) _state = CLEAR;
                if (_event == LONG){
                    _state = FAV_SELECT;
                    _return_state = FAV_ACTIVE_NAME;
                }
                break;
            case FAV_SELECT:
                if (active_fav != -1) selected_fav = active_fav;
                _state = FAV_SELECT_CONFIRM;
                break;
            case FAV_SELECT_CONFIRM:
                if (_event == SHORT) {
                    selected_fav++;
                    if (selected_fav > 9) selected_fav = 0;
                }
                Display::LoadFavorite(selected_fav, model->GetFavoriteName(selected_fav));
                //Display::Confirm(favSel);
                if (_event == LONG) {
                    active_fav = selected_fav;
                    Favorites::ActivateFavorite(selected_fav);
                    _state = FAV_ACTIVE_NAME;
                }
                if (_event == TIMEOUT) {
                    _state = _return_state;
                }
                break;
        }
    }

    // generate scroll events
    if (_state == FAV_ACTIVE_USTRING) {
        _timer2++;
        _timer2 %= SCROLL_RATE_MS / UI_TASK_PERIOD_MS;
        if (_timer2 == 0) {
            Display::UpdateFavoriteUStringScroll();
        }
    }

    return 0;
}

system::TaskModule<UIWorker, system::CpuCore::system, 4095, 3> ui_worker("ui_worker");

}