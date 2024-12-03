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

#include <tbd/storage/resources.hpp>


namespace tbd::favorites {

FavoritesModel::FavoritesModel() {
    auto config_path = storage::get_fs_path("/data/favs.jsn");
    if (!config_path) {
        TBD_LOGE("favorites", "failed to load favorites config");
        return;
    }
    _config_file = config_path->string();
}


string FavoritesModel::GetAllFavorites() {
    if (_config_file) {
        TBD_LOGE("favorites", "access to broken favourites");
        return {};
    }

    loadJSON(m, *_config_file);
    json.Clear();
    Writer<StringBuffer> writer(json);
    if (!m.IsArray()) return "";
    m.Accept(writer);
    return json.GetString();
}

string FavoritesModel::GetFavorite(int const &i) {
    if (_config_file) {
        TBD_LOGE("favorites", "access to broken favourites");
        return {};
    }

    loadJSON(m, *_config_file);
    json.Clear();
    Writer<StringBuffer> writer(json);
    if (!m.IsArray()) return "";
    Value o = m[i].GetObject();
    o.Accept(writer);
    return json.GetString();
}

string FavoritesModel::GetFavoriteName(int const &i) {
    if (_config_file) {
        TBD_LOGE("favorites", "access to broken favourites");
        return {};
    }

    loadJSON(m, *_config_file);
    if (!m.IsArray()) return "";
    Value o = m[i].GetObject();
    return o["name"].GetString();
}

string FavoritesModel::GetFavoriteUString(int const &i) {
    if (_config_file) {
        TBD_LOGE("favorites", "access to broken favourites");
        return {};
    }

    loadJSON(m, *_config_file);
    if (!m.IsArray()) return "";
    Value o = m[i].GetObject();
    return o["ustring"].GetString();
}

string FavoritesModel::GetFavoritePluginID(int const &i, const int &channel) {
    if (_config_file) {
        TBD_LOGE("favorites", "access to broken favourites");
        return {};
    }

    loadJSON(m, *_config_file);
    if (!m.IsArray()) return "";
    if (!m[i].IsObject()) return "";
    string key {"plug_0"};
    if(channel == 1) key = string("plug_1");
    if(m[i].HasMember(key)) return m[i][key].GetString();
    return "";
}

int FavoritesModel::GetFavoritePreset(int const &i, const int &channel) {
    if (_config_file) {
        TBD_LOGE("favorites", "access to broken favourites");
        return -1;
    }

    loadJSON(m, *_config_file);
    if (!m.IsArray()) return 0;
    string key {"pre_0"};
    if(channel == 1) key = string("pre_1");
    if(m[i].HasMember(key)) return m[i][key].GetInt();
    return 0;
}

void FavoritesModel::SetFavorite(int const &id, const string &data) {
    if (_config_file) {
        TBD_LOGE("favorites", "access to broken favourites");
        return;
    }

    loadJSON(m, *_config_file);
    if (!m.IsArray()) return;
    Document d;
    d.Parse(data);
    m[id] = d.Move();
    storeJSON(m, *_config_file);
}

}
