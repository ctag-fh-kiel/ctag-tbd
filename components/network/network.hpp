#pragma once

#include <string>
#include "esp_event.h"
#include "esp_netif.h"

using namespace std;

namespace CTAG {
    namespace NET {
        class Network {
        public:
            enum class IF_TYPE : uint8_t {
                IF_TYPE_AP = 0x00,
                IF_TYPE_STA = 0x01,
                IF_TYPE_USBNCM = 0x02
            };

            static void Up();

            static void SetIfType(IF_TYPE if_type);

            static void SetSSID(const string ssid);

            static void SetPWD(const string pwd);

            static void SetIP(string ip);

            static void SetMDNSName(const string name);

        private:
            static void initialise_mdns(const string hostname, const string instance_name_set);

            static void wifi_init_softap();

            static void wifi_event_handler_ap(void *arg, esp_event_base_t event_base,
                                              int32_t event_id, void *event_data);

            static void wifi_init_sta(void);

            static void event_handler_sta(void *arg, esp_event_base_t event_base,
                                          int32_t event_id, void *event_data);

            static void if_init_usbncm(void);

            static esp_err_t netif_recv_callback(void *buffer, uint16_t len, void *ctx);

            static string _ssid, _pwd, _mdns, _mdns_instance;
            static IF_TYPE _if_type;
            static string _ip;
            static uint32_t _ip_addr;
            static esp_netif_t *netif;
        };
    }
}