#pragma once

#include <string>


namespace tbd {

struct Network {
    static void Up();

    static void SetIsAccessPoint(bool yes);

    static void SetSSID(const std::string ssid);

    static void SetPWD(const std::string pwd);

    static void SetIP(const std::string ip);

    static void SetMDNSName(const std::string name);
};

}
