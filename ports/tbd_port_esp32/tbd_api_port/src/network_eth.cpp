#include <tbd/api/common/network.hpp>

// #include <stdio.h>
// #include <string.h>
#include "freertos/FreeRTOS.h"
#include <freertos/semphr.h>
// #include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
// #include "esp_log.h"
// #include "ethernet_init.h"
// #include "sdkconfig.h"

#include <esp_netif.h>

#include <tbd/logging.hpp>

#include "esp_mac.h"

#define CONFIG_EXAMPLE_USE_OPENETH 1
#define CONFIG_EXAMPLE_CONNECT_IPV4 1

namespace {
const char *TAG = "eth_example";

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
    void down();

private:
    void start();
    void stop();

    bool should_retry() {
        if (_num_retries < 666) {
            _num_retries++;
            return true;
        }
        return false;
    }

    void set_has_connected() { _num_retries = 0; }


    void init_mdns(const std::string hostname, const std::string instance_name_set);
    void init_softap();
    void init_sta();

    static void event_handler_ip(void* arg, esp_event_base_t event_base,
                                  int32_t event_id, void *event_data);

    bool _is_ap = true;
    std::string _ssid;
    std::string _pwd;
    std::string _mdns;
    std::string _mdns_instance;
    std::string _ip;

    unsigned int _num_retries = 0;

    esp_eth_handle_t _handle = nullptr;
    esp_eth_mac_t* _mac = nullptr;
    esp_eth_phy_t* _phy = nullptr;
    esp_eth_netif_glue_handle_t _eth_glue = nullptr;

    SemaphoreHandle_t _ip_address_lock = nullptr;
    esp_netif_t*_netif = nullptr;
    const char* _netif_descr = "eth0";
    // EventGroupHandle_t _event_group;
} instance;


void Impl::up() {
    _ip_address_lock = xSemaphoreCreateBinary();
    if (_ip_address_lock == nullptr) {
        TBD_LOGE(TAG, "out of memory, no IP lock acquired");
    }
    start();
    ESP_LOGI(TAG, "waiting for IP address");
    xSemaphoreTake(_ip_address_lock, portMAX_DELAY);
}

void Impl::down() {
    if (_ip_address_lock == nullptr) {
        return;
    }
    vSemaphoreDelete(_ip_address_lock);
    _ip_address_lock = nullptr;
    stop();
}

void Impl::event_handler_ip(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    xSemaphoreGive(instance._ip_address_lock);
}

void Impl::start() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_ETH();
    // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
    esp_netif_config.if_desc = _netif_descr;
    esp_netif_config.route_prio = 64;
    esp_netif_config_t netif_config = {
        .base = &esp_netif_config,
        .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH
    };
    _netif = esp_netif_new(&netif_config);
    assert(_netif);

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

    phy_config.autonego_timeout_ms = 100;
    _mac = esp_eth_mac_new_openeth(&mac_config);
    _phy = esp_eth_phy_new_dp83848(&phy_config);


    // Install Ethernet driver
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(_mac, _phy);
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &_handle));

    /* The SPI Ethernet module might doesn't have a burned factory MAC address, we cat to set it manually.
       We set the ESP_MAC_ETH mac address as the default, if you want to use ESP_MAC_EFUSE_CUSTOM mac address, please enable the
       configuration: `ESP_MAC_USE_CUSTOM_MAC_AS_BASE_MAC`
    */
    uint8_t eth_mac[6] = {0};
    ESP_ERROR_CHECK(esp_read_mac(eth_mac, ESP_MAC_ETH));
    ESP_ERROR_CHECK(esp_eth_ioctl(_handle, ETH_CMD_S_MAC_ADDR, eth_mac));

    // combine driver with netif
    _eth_glue = esp_eth_new_netif_glue(_handle);
    esp_netif_attach(_netif, _eth_glue);

    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &event_handler_ip, nullptr));

    esp_eth_start(_handle);
}

void Impl::stop() {
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_ETH_GOT_IP, &event_handler_ip));

    ESP_ERROR_CHECK(esp_eth_stop(_handle));
    ESP_ERROR_CHECK(esp_eth_del_netif_glue(_eth_glue));
    ESP_ERROR_CHECK(esp_eth_driver_uninstall(_handle));
    _handle = nullptr;
    ESP_ERROR_CHECK(_phy->del(_phy));
    ESP_ERROR_CHECK(_mac->del(_mac));

    esp_netif_destroy(_netif);
}

}

namespace tbd {

void Network::Up() {
    instance.up();
}

void Network::SetIsAccessPoint(bool yes) {
    TBD_LOGI(TAG, "SetIsAccessPoint called");
}


void Network::SetSSID(const std::string ssid) {
    TBD_LOGI(TAG, "SetSSID called");
}


void Network::SetPWD(const std::string pwd) {
    TBD_LOGI(TAG, "SetPWD called");
}


void Network::SetIP(const std::string ip) {
    TBD_LOGI(TAG, "SetIP called");
}


void Network::SetMDNSName(const std::string mdns_name) {
    TBD_LOGI(TAG, "SetMDNSName called");
}

}


