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
#include <tbd/network/config.hpp>

#include <cstdio>
#include <fstream>
#include "rapidjson/filereadstream.h"
#include "rapidjson/writer.h"
#include <dirent.h>
#include <tbd/logging.hpp>

namespace rj = rapidjson;

namespace tbd::network {

NetworkConfig::NetworkConfig() {
    TBD_LOGI("network", "Trying to read network config");
    loadJSON(m, MODELJSONFN);
}

NetworkConfig::~NetworkConfig() {
}

void NetworkConfig::print() {
    printJSON(m);
}

void NetworkConfig::set_from_json(const std::string &data) {
    rj::Document d;
    d.Parse(data);
    if(d.HasParseError()) return;
    // Value obj(kObjectType);
    // obj.CopyFrom(d, m.GetAllocator());
    // m["configuration"] = obj.Move();
    storeJSON(d, MODELJSONFN);
}

std::string NetworkConfig::get(const std::string &id) {
    rj::Value s(rj::kStringType);
    s.CopyFrom(m[id], m.GetAllocator());
    return s.GetString();
}

void NetworkConfig::reset() {
    if (!m.HasMember("configuration")) return;
    if (!m["configuration"].HasMember("wifi")) return;
    rj::Value ssid("ctag-tbd");
    rj::Value pwd("");
    rj::Value mode("ap");
    m["configuration"]["wifi"]["ssid"] = ssid.Move();
    m["configuration"]["wifi"]["pwd"] = pwd.Move();
    m["configuration"]["wifi"]["mode"] = mode.Move();
    storeJSON(m, MODELJSONFN);
}

}