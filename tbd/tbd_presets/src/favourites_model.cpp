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
#include <tbd/favorites/model.hpp>

#include "rapidjson/filereadstream.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <tbd/sound_processor/resources.hpp>


namespace tbd::favorites {

string FavoritesModel::GetAllFavorites() {
    loadJSON(m, CTAG::RESOURCES::spiffsRoot + "/data/favs.jsn");
    json.Clear();
    Writer<StringBuffer> writer(json);
    if (!m.IsArray()) return "";
    m.Accept(writer);
    return json.GetString();
}

string FavoritesModel::GetFavorite(int const &i) {
    loadJSON(m, CTAG::RESOURCES::spiffsRoot + "/data/favs.jsn");
    json.Clear();
    Writer<StringBuffer> writer(json);
    if (!m.IsArray()) return "";
    Value o = m[i].GetObject();
    o.Accept(writer);
    return json.GetString();
}

string FavoritesModel::GetFavoriteName(int const &i) {
    loadJSON(m, CTAG::RESOURCES::spiffsRoot + "/data/favs.jsn");
    if (!m.IsArray()) return "";
    Value o = m[i].GetObject();
    return o["name"].GetString();
}

string FavoritesModel::GetFavoriteUString(int const &i) {
    loadJSON(m, CTAG::RESOURCES::spiffsRoot + "/data/favs.jsn");
    if (!m.IsArray()) return "";
    Value o = m[i].GetObject();
    return o["ustring"].GetString();
}

string FavoritesModel::GetFavoritePluginID(int const &i, const int &channel) {
    loadJSON(m, CTAG::RESOURCES::spiffsRoot + "/data/favs.jsn");
    if (!m.IsArray()) return "";
    if (!m[i].IsObject()) return "";
    string key {"plug_0"};
    if(channel == 1) key = string("plug_1");
    if(m[i].HasMember(key)) return m[i][key].GetString();
    return "";
}

int FavoritesModel::GetFavoritePreset(int const &i, const int &channel) {
    loadJSON(m, CTAG::RESOURCES::spiffsRoot + "/data/favs.jsn");
    if (!m.IsArray()) return 0;
    string key {"pre_0"};
    if(channel == 1) key = string("pre_1");
    if(m[i].HasMember(key)) return m[i][key].GetInt();
    return 0;
}

void FavoritesModel::SetFavorite(int const &id, const string &data) {
    loadJSON(m, CTAG::RESOURCES::spiffsRoot + "/data/favs.jsn");
    if (!m.IsArray()) return;
    Document d;
    d.Parse(data);
    m[id] = d.Move();
    storeJSON(m, CTAG::RESOURCES::spiffsRoot + "/data/favs.jsn");
}

}