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

extern "C" {
    #include "ssd1306.h"
}

#include <string>
#include <vector>

namespace CTAG {
    namespace DRIVERS {
        class Display final {
            static const int I2CDisplayAddress;
            static const int I2CDisplayWidth;
            static const int I2CDisplayHeight;
            static const int I2CResetPin;
            static SSD1306_t I2CDisplay;
            static std::vector<std::string> userString_v;
            static int currentUserStringRow;
        public:
            Display() = delete;
            static void Init();
            static void Clear();
            static void ShowFavorite(int const &id, std::string const &name);
            static void ShowFWVersion();
            static void PrepareDisplayFavoriteUString(std::string const &us);
            static void UpdateFavoriteUStringScroll();
            static void LoadFavorite(int const &id, std::string const &name);
            static void Confirm(const int &id);
            static void UserMode();
        };
    }
}
