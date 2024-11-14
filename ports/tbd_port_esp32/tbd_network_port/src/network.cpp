#include <tbd/network.hpp>

#include "nvs_flash.h"
#include "esp_netif.h"
#include <cstring>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "mdns.h"
#include "lwip/apps/netbiosns.h"
#include <tbd/logging.hpp>


#define EXAMPLE_ESP_MAXIMUM_RETRY  10

/* FreeRTOS event group to signal when we are connected*/


/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1



// static int s_retry_num = 0;

// esp_netif_t *Network::netif = nullptr;

// string Network::_ssid = "";
// string Network::_ip = "";
// string Network::_pwd = "";
// string Network::_mdns = "";
// string Network::_mdns_instance = "";
// bool Network::isAP = true;

namespace {

static const char *TAG = "tbd_network";

struct Impl {
    void set_is_access_point(bool is_access_point) {
        _is_ap = is_access_point;
    }

    void set_ssid(const std::string& ssid) {
        _ssid = ssid;
    }

    void set_pwd(const std::string& pwd) {
        _pwd = pwd;
    }

    void set_ip(const std::string& ip) {
        _ip = ip;
    }

    void set_mdns_name(const std::string& mdns_name) {
        _mdns = mdns_name;
        _mdns_instance = _mdns + "TBD, you define what it is";
    }

    void up();

private:
    bool should_retry() { 
        if (_num_retries < EXAMPLE_ESP_MAXIMUM_RETRY) {
            _num_retries++;
            return true;
        }
        return false;
    }

    void set_has_connected() { _num_retries = 0; }
    

    void init_mdns(const std::string hostname, const std::string instance_name_set);
    void init_softap();
    void init_sta();

    static void wifi_event_handler_ap(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data);

    static void event_handler_sta(void* arg, esp_event_base_t event_base,
                           int32_t event_id, void* event_data);

    bool _is_ap = true;
    std::string _ssid;
    std::string _pwd;
    std::string _mdns;
    std::string _mdns_instance;
    std::string _ip;

    unsigned int _num_retries = 0;

    esp_netif_t*_netif = nullptr;
    EventGroupHandle_t _event_group;
} instance;


void Impl::up() {
    ESP_LOGI("Network", "Starting with ssid %s, pwd %s, mdns %s, ip %s, is %s",
            _ssid.c_str(), _pwd.c_str(), _mdns.c_str(), _ip.c_str(), _is_ap ? "ap" : "sta");
    
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    init_mdns(_mdns, _mdns_instance);
    
    netbiosns_init();
    netbiosns_set_name(_mdns.c_str());

    if (_is_ap) {
        init_softap();
    } else {
        init_sta();
    }
    ESP_LOGI("Network", "Disabling wifi power save mode");
    wifi_ps_type_t ps_mode = WIFI_PS_NONE;
    esp_wifi_set_ps(ps_mode);
}


void Impl::init_mdns(const std::string hostname, const std::string instance_name_set) {
    mdns_init();
    mdns_hostname_set(hostname.c_str());
    mdns_instance_name_set(instance_name_set.c_str());

    mdns_txt_item_t serviceTxtData[] = {
            {"board", hostname.c_str()},
            {"path",  "/"}
    };

    ESP_ERROR_CHECK(mdns_service_add(hostname.c_str(), "_http", "_tcp", 80, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));
}


void Impl::init_softap(void) {
    _netif = esp_netif_create_default_wifi_ap();


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    // issues with wifi:bcn_timout,ap_probe_send_start
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler_ap, NULL));

    wifi_config_t wifi_config;
    memset((void *) &wifi_config, 0, sizeof(wifi_config_t));
    wifi_config.ap.max_connection = 5;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    strcpy((char *) wifi_config.ap.ssid, _ssid.c_str());
    wifi_config.ap.ssid_len = _ssid.length();
    if (_pwd.length() == 0)
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    else {
        strcpy((char *) wifi_config.ap.password, _pwd.c_str());
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));

    /* DHCP stuff not working yet
    esp_netif_dhcpc_stop(netif);

    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(netif, &ip_info);
    uint32_t ip = esp_ip4addr_aton(_ip.c_str());
    esp_netif_set_ip4_addr(&ip_info.ip, (ip & 0x000000ff), (ip & 0x0000ff00) >> 8, (ip & 0x00ff0000) >> 16, (ip & 0xff000000) >> 24);
    esp_netif_set_ip4_addr(&ip_info.gw, (ip & 0x000000ff), (ip & 0x0000ff00) >> 8, (ip & 0x00ff0000) >> 16, (ip & 0xff000000) >> 24);
    esp_netif_set_ip_info(netif, &ip_info);

    char buf[256];
    esp_ip4addr_ntoa(&ip_info.gw, buf, 256);

    ESP_LOGE("NET", "Gateway %s", buf);
    esp_ip4addr_ntoa(&ip_info.netmask, buf, 256);
    ESP_LOGE("NET", "Netmask %s", buf);
    esp_ip4addr_ntoa(&ip_info.ip, buf, 256);
    ESP_LOGE("NET", "IP %s", buf);
    */
    ESP_ERROR_CHECK(esp_wifi_start());
}



void Impl::init_sta(void) {
    _event_group = xEventGroupCreate();

    _netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler_sta, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler_sta, NULL));

    wifi_config_t wifi_config;
    memset((void *) &wifi_config, 0, sizeof(wifi_config_t));
    strcpy((char *) wifi_config.sta.ssid, _ssid.c_str());
    strcpy((char *) wifi_config.sta.password, _pwd.c_str());

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    // Add random connection delay up to 5 seconds for multimodule setups
    vTaskDelay(esp_random() / (UINT32_MAX / 5000 * portTICK_PERIOD_MS));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler_sta));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler_sta));
    vEventGroupDelete(_event_group);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected");
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Could not connect, starting as AP");
        set_is_access_point(true);
        set_ssid(_mdns);
        set_pwd("");
        init_softap();
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}


void Impl::event_handler_sta(
    void *arg, esp_event_base_t event_base,
    int32_t event_id, void *event_data) 
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (instance.should_retry()) {
            esp_wifi_connect();
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(instance._event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        instance.set_has_connected();
        xEventGroupSetBits(instance._event_group, WIFI_CONNECTED_BIT);
    }
}


void Impl::wifi_event_handler_ap(
    void *arg, esp_event_base_t event_base,                                
    int32_t event_id, void *event_data) 
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        ESP_LOGI("Network", "station " MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        ESP_LOGI("Network", "station " MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}


} // anonymous namespace 


namespace CTAG {
namespace NET {

void Network::Up() {
    instance.up();
}


void Network::SetIsAccessPoint(bool yes) {
    instance.set_is_access_point(yes);
}


void Network::SetSSID(const std::string ssid) {
    instance.set_ssid(ssid);
}


void Network::SetPWD(const std::string pwd) {
    instance.set_pwd(pwd);
}


void Network::SetIP(const std::string ip) {
    instance.set_ip(ip);
}


void Network::SetMDNSName(const std::string mdns_name) {
    instance.set_mdns_name(mdns_name);
}

}
}