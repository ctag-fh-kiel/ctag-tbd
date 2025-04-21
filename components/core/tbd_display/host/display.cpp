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

#include <tbd/display/common/display.hpp>
#include <tbd/version.hpp>


namespace tbd {

void Display::Init() {
}

void Display::Clear() {

}

void Display::ShowFavorite(const int &id, const std::string &name) {
}

void Display::ShowFWVersion() {

}

void Display::ShowUserString(std::string const &s) {

}

void Display::ShowUserString(std::vector<std::string> const &sv) {

}

void Display::PrepareDisplayFavoriteUString(int const &id, std::string const &name, std::string const &us) {

}

void Display::Confirm(const int &id) {

}

void Display::UserMode() {

}

void Display::LoadFavorite(int const &id, const std::string &name) {

}

void Display::UpdateFavoriteUStringScroll() {

}

}