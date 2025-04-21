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
#pragma  once

#include <string>
#include "rapidjson/document.h"
#include <tbd/config_base.hpp>


namespace tbd::network {

struct NetworkConfig final : config::ConfigBase {
    NetworkConfig();
    ~NetworkConfig();

    std::string ssid() { return get("ssid"); }
    std::string pwd() { return get("pwd"); }
    bool is_access_point() { return get("mode").compare("ap") == 0; }
    std::string ip() { return get("ip"); }
    std::string mdns_name() { return get("mdns_name"); }

    /** @brief restore defaults
     * 
     */
    void reset();

    void set_from_json(const std::string &data);

    void print();

    std::string data(const std::string &id);
private:

    std::string get(const std::string &which);

    rapidjson::Document m;
#ifndef TBD_SIM
            const std::string MODELJSONFN = "/spiffs/data/network.jsn";
#else
            const std::string MODELJSONFN = "../../spiffs_image/data/network.jsn";
#endif
};

}

