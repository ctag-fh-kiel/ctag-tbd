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

#include "SimDataModel.hpp"

SimDataModel::SimDataModel() {
    loadJSON(m, MODELJSONFN);
}

void SimDataModel::SetModelJSONString(const string &s) {
   m.Parse(s);
   storeJSON(m, MODELJSONFN);
}

const char * SimDataModel::GetModelJSONCString() {
    json.Clear();
    Writer<StringBuffer> writer(json);
    m.Accept(writer);
    return json.GetString();
}

SimDataModel::~SimDataModel() {

}

const string SimDataModel::GetKeyValue(const string &key) {
    if(!m.HasMember(key)) return key;
    if(!m[key].IsString()) return key;
    return m[key].GetString();
}

const int SimDataModel::GetArrayElement(const string &key, const int index) {
    if(!m.HasMember(key)) return 0;
    if(!m[key].IsArray()) return 0;
    if(m[key][index].IsString()) return std::stoi(m[key][index].GetString());
    if(m[key][index].IsInt()) return m[key][index].GetInt();
    return 0;
}
