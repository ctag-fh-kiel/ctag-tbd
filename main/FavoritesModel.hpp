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


#include <string>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "ctagResources.hpp"
#include "SPManagerDataModel.hpp"

using namespace std;
using namespace rapidjson;

namespace CTAG {
    namespace FAV {
        class FavoritesModel final : public CTAG::SP::ctagDataModelBase {
        public:
            string GetAllFavorites();
            string GetFavorite(int const &i);
            string GetFavoriteName(int const &i);
            string GetFavoriteUString(int const &i);
            string GetFavoritePluginID(int const &i, int const &channel);
            int GetFavoritePreset(int const &i, int const &channel);
            void SetFavorite(int const &id, const string &data);

        private:
            Document m;
            string MODELJSONFN = CTAG::RESOURCES::spiffsRoot + "/data/favs.jsn";
        };
    }
}
