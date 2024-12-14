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

#include <string>

#include "Display.hpp"
#include "version.hpp"
#include "esp_log.h"

#if CONFIG_TBD_PLATFORM_MK2
    #define SCL_GPIO 32
    #define SDA_GPIO 33
#elif CONFIG_TBD_PLATFORM_BBA
    #define SCL_GPIO 4
    #define SDA_GPIO 5
#else
    #define SCL_GPIO 23
    #define SDA_GPIO 5
#endif

using namespace CTAG::DRIVERS;

SSD1306_t Display::I2CDisplay;
std::vector<std::string> Display::userString_v;
int Display::currentUserStringRow {0};

void Display::Init() {
    i2c_master_init(&I2CDisplay, SDA_GPIO, SCL_GPIO, 42);
    I2CDisplay._flip = true;
    ssd1306_init(&I2CDisplay, 128, 64);
    ssd1306_clear_screen(&I2CDisplay, false);
    ssd1306_contrast(&I2CDisplay, 0xff);
}

void Display::Clear() {
    ssd1306_clear_screen(&I2CDisplay, false);
}

void Display::ShowFavorite(const int &id, const std::string &name) {
    std::string s {"Active Favorite:"};
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 0, s.c_str(), s.length(), false);
    s = std::string("");
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 1, s.c_str(), s.length(), false);
    s = std::string(std::to_string(id) + ": " + name);
    s = s.substr(0, 16);
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 2, s.c_str(), s.length(), false);
    s = std::string("");
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 3, s.c_str(), s.length(), false);
}

void Display::ShowFWVersion() {
    ssd1306_clear_screen(&I2CDisplay, false);
    std::string s {"TBD fw:"};
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 0, s.c_str(), s.length(), false);
    s = std::string(TBD_FW_VERSION);
    if(s.length()>16)s = s.substr(0, 16);
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 1, s.c_str(), s.length(), false);
    s = std::string("TBD hw:");
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 2, s.c_str(), s.length(), false);
    s = std::string(TBD_HW_VERSION);
    if(s.length()>16)s = s.substr(0, 16);
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 3, s.c_str(), s.length(), false);
}

void Display::ShowUserString(std::string const &s) {
    ssd1306_clear_screen(&I2CDisplay, false);
    ssd1306_display_text(&I2CDisplay, 0, s.c_str(), s.length() > 16 ? 16 : s.length(), false);
}

void Display::ShowUserString(std::vector<std::string> const &sv) {
    ssd1306_clear_screen(&I2CDisplay, false);
    for(int i=0;i<sv.size();i++){
        ssd1306_display_text(&I2CDisplay, i, sv[i].c_str(), sv[i].length() > 16 ? 16 : sv[i].length(), false);
    }
}

void Display::PrepareDisplayFavoriteUString(int const &id, std::string const &name, std::string const &us) {
    // create slices
    std::string s {us}, title{"#" + std::to_string(id) + ": " + name};
    title.append(16-title.length(), ' ');
    userString_v.clear();
    userString_v.push_back(title);
    userString_v.push_back("                ");
    while(s.length() > 0){
        if(s.length() < 16) s.append(16-s.length(), ' ');
        userString_v.push_back(s.substr(0, 16));
        s = s.substr(16, s.length() - 16);
    }
    userString_v.shrink_to_fit();
    if(userString_v.size() > 4){
        userString_v.push_back("                ");
    }else{
        while(userString_v.size() < 4){
            userString_v.push_back("                ");
        }
    }
    userString_v.shrink_to_fit();
    currentUserStringRow = 0;
    ssd1306_clear_screen(&I2CDisplay, false);
    ssd1306_software_scroll(&I2CDisplay, I2CDisplay._pages - 1, 0);
    UpdateFavoriteUStringScroll();
    UpdateFavoriteUStringScroll();
    UpdateFavoriteUStringScroll();
    UpdateFavoriteUStringScroll();
}

void Display::Confirm(const int &id) {
    std::string s {"Load Fav: " + std::to_string(id)};
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 0, s.c_str(), s.length(), false);
    s = std::string("Confirm?");
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 1, s.c_str(), s.length(), false);
    s = std::string("y -> Long Press");
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 2, s.c_str(), s.length(), false);
    s = std::string("n -> Short Press");
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 3, s.c_str(), s.length(), false);
}

void Display::UserMode() {
    std::string s {"User Mode"};
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 0, s.c_str(), s.length(), false);
    s = std::string("");
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 1, s.c_str(), s.length(), false);
    s = std::string("No Fav Active!");
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 2, s.c_str(), s.length(), false);
    s = std::string("");
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 3, s.c_str(), s.length(), false);
}

void Display::LoadFavorite(int const &id, const std::string &name) {
    std::string s {"Load Favorite:"};
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 0, s.c_str(), s.length(), false);
    s = std::string(std::to_string(id) + ": " + name);
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 1, s.c_str(), s.length(), false);
    s = std::string("");
    s = s.substr(0, 16);
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 2, s.c_str(), s.length(), false);
    s = std::string("-> Long Press");
    s.append(16-s.length(), ' ');
    ssd1306_display_text(&I2CDisplay, 3, s.c_str(), s.length(), false);
}

void Display::UpdateFavoriteUStringScroll() {
    if(userString_v.size() == 0) return;
    if(userString_v.size() <= 4 && currentUserStringRow == userString_v.size()) return;
    if(currentUserStringRow >= userString_v.size()) currentUserStringRow = 0;
    ssd1306_scroll_text(&I2CDisplay, userString_v[currentUserStringRow].c_str(), 16, false);
    currentUserStringRow++;
}
