#pragma once

#include <string>
#include "esp_event.h"

using namespace std;

namespace CTAG {
    namespace NET {
        class Network {
        public:
            static void Up();

            static void SetIsAccessPoint(bool yes);

            static void SetSSID(const string ssid);

            static void SetPWD(const string pwd);

            static void SetIP(const string ip);

            static void SetMDNSName(const string name);

        private:
            static void initialise_mdns(const string hostname, const string instance_name_set);

            static void wifi_init_softap();

            static void wifi_event_handler_ap(void *arg, esp_event_base_t event_base,
                                              int32_t event_id, void *event_data);

            static void wifi_init_sta(void);

            static void event_handler_sta(void *arg, esp_event_base_t event_base,
                                          int32_t event_id, void *event_data);

            static string _ssid, _pwd, _mdns, _mdns_instance;
            static bool isAP;
            static string _ip;
            static esp_netif_t *netif;
        };
    }
}