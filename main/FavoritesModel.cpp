/***************
TBD-16 — dadamachines WebUI & REST API

(c) 2024-2026 Johannes Elias Lohbihler for dadamachines.

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

Part of the dadamachines additions to the CTAG TBD platform.
See LICENSE in the repository root for full terms.

Provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/


#include "FavoritesModel.hpp"
#include "rapidjson/filereadstream.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

string CTAG::FAV::FavoritesModel::GetAllFavorites() {
    loadJSON(m, CTAG::RESOURCES::sdcardRoot + "/data/favs.json");
    json.Clear();
    Writer<StringBuffer> writer(json);
    if (!m.IsArray()) return "";
    m.Accept(writer);
    return json.GetString();
}

string CTAG::FAV::FavoritesModel::GetFavorite(int const &i) {
    loadJSON(m, CTAG::RESOURCES::sdcardRoot + "/data/favs.json");
    json.Clear();
    Writer<StringBuffer> writer(json);
    if (!m.IsArray()) return "";
    Value o = m[i].GetObject();
    o.Accept(writer);
    return json.GetString();
}

string CTAG::FAV::FavoritesModel::GetFavoriteName(int const &i) {
    loadJSON(m, CTAG::RESOURCES::sdcardRoot + "/data/favs.json");
    if (!m.IsArray()) return "";
    Value o = m[i].GetObject();
    return o["name"].GetString();
}

string CTAG::FAV::FavoritesModel::GetFavoriteUString(int const &i) {
    loadJSON(m, CTAG::RESOURCES::sdcardRoot + "/data/favs.json");
    if (!m.IsArray()) return "";
    Value o = m[i].GetObject();
    return o["ustring"].GetString();
}

string CTAG::FAV::FavoritesModel::GetFavoritePluginID(int const &i, const int &channel) {
    loadJSON(m, CTAG::RESOURCES::sdcardRoot + "/data/favs.json");
    if (!m.IsArray()) return "";
    if (!m[i].IsObject()) return "";
    string key {"plug_0"};
    if(channel == 1) key = string("plug_1");
    if(m[i].HasMember(key)) return m[i][key].GetString();
    return "";
}

int CTAG::FAV::FavoritesModel::GetFavoritePreset(int const &i, const int &channel) {
    loadJSON(m, CTAG::RESOURCES::sdcardRoot + "/data/favs.json");
    if (!m.IsArray()) return 0;
    string key {"pre_0"};
    if(channel == 1) key = string("pre_1");
    if(m[i].HasMember(key)) return m[i][key].GetInt();
    return 0;
}

void CTAG::FAV::FavoritesModel::SetFavorite(int const &id, const string &data) {
    loadJSON(m, CTAG::RESOURCES::sdcardRoot + "/data/favs.json");
    if (!m.IsArray()) return;
    Document d;
    d.Parse(data);
    m[id] = d.Move();
    storeJSON(m, CTAG::RESOURCES::sdcardRoot + "/data/favs.json");
}
