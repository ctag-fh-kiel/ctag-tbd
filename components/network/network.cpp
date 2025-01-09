#include "network.hpp"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_log.h"
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
#include "tinyusb.h"
#include "tinyusb_net.h"
#include "dhcpserver/dhcpserver_options.h"
#include "lwip/esp_netif_net_stack.h"
#include "lwip/apps/netbiosns.h"

using namespace CTAG::NET;
using namespace std;

#define EXAMPLE_ESP_MAXIMUM_RETRY  10

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;

string Network::_ssid = "";
string Network::_ip = "";
string Network::_pwd = "";
string Network::_mdns = "";
string Network::_mdns_instance = "";
Network::IF_TYPE Network::_if_type = Network::IF_TYPE::IF_TYPE_AP;
uint32_t Network::_ip_addr = 0;
esp_netif_t *Network::netif = nullptr;


esp_err_t Network::netif_recv_callback(void *buffer, uint16_t len, void *ctx)
{
    // FIXME: where is buf_copy de-allocated?
    if (netif) {
        void *buf_copy = malloc(len);
        if (!buf_copy) {
            return ESP_ERR_NO_MEM;
        }
        memcpy(buf_copy, buffer, len);
        return esp_netif_receive(netif, buf_copy, len, nullptr);
    }
    return ESP_OK;
}

esp_err_t wired_send(void *buffer, uint16_t len, void *buff_free_arg)
{
    return tinyusb_net_send_sync(buffer, len, buff_free_arg, pdMS_TO_TICKS(100));
}

static void l2_free(void *h, void *buffer)
{
    free(buffer);
}

static esp_err_t netif_transmit (void *h, void *buffer, size_t len)
{
    esp_err_t err = wired_send(buffer, len, nullptr);
    if(err != ESP_OK && err != ESP_ERR_INVALID_STATE){
        ESP_LOGE(TAG, "Failed to send buffer to USB %d!", err);
    }
    return err;
}



void Network::event_handler_sta(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void Network::wifi_init_sta(void) {
    s_wifi_event_group = xEventGroupCreate();

    netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler_sta, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler_sta, nullptr));

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
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler_sta));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler_sta));
    vEventGroupDelete(s_wifi_event_group);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected");
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Could not connect, starting as AP");
        Network::SetIfType(IF_TYPE::IF_TYPE_AP);
        Network::SetSSID(Network::_mdns);
        Network::SetPWD("");
        Network::wifi_init_softap();
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}


void Network::wifi_event_handler_ap(void *arg, esp_event_base_t event_base,
                                    int32_t event_id, void *event_data) {
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


void Network::wifi_init_softap(void) {
    netif = esp_netif_create_default_wifi_ap();


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    // issues with wifi:bcn_timout,ap_probe_send_start
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler_ap, nullptr));

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

    // Assign a static IP to the AP
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(netif, &ip_info);
    ip_info.ip.addr = _ip_addr;
    ip_info.netmask.addr = ESP_IP4TOADDR(255, 255, 255, 0);
    ip_info.gw.addr = _ip_addr;
    esp_netif_set_ip_info(netif, &ip_info);


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

void Network::initialise_mdns(const string hostname, const string instance_name_set) {
    mdns_init();

    mdns_register_netif(netif);
    mdns_netif_action(netif, mdns_event_actions_t(MDNS_EVENT_ENABLE_IP4 | MDNS_EVENT_ENABLE_IP6));
    mdns_netif_action(netif, mdns_event_actions_t(MDNS_EVENT_ANNOUNCE_IP4 | MDNS_EVENT_ANNOUNCE_IP6));
    mdns_netif_action(netif, mdns_event_actions_t(MDNS_EVENT_IP4_REVERSE_LOOKUP | MDNS_EVENT_IP6_REVERSE_LOOKUP));

    mdns_hostname_set(hostname.c_str());
    mdns_instance_name_set(instance_name_set.c_str());

    mdns_txt_item_t serviceTxtData[] = {
            {"board", hostname.c_str()},
            {"path",  "/"}
    };

    ESP_ERROR_CHECK(mdns_service_add(hostname.c_str(), "_http", "_tcp", 80, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));
}

void Network::if_init_usbncm(void){
    const tinyusb_net_config_t net_config = {
            // locally administrated address for the ncm device as it's going to be used internally
            // for configuration only
            .mac_addr = {0x02, 0x02, 0x11, 0x22, 0x33, 0x01},
            .on_recv_callback = netif_recv_callback,
            .free_tx_buffer = nullptr,
            .on_init_callback = nullptr,
            .user_context = nullptr
    };

    esp_err_t ret = tinyusb_net_init(TINYUSB_USBDEV_0, &net_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Cannot initialize USB Net device");
        assert(0);
    }

    // with OUI range MAC to create a virtual netif running http server
    // this needs to be different to usb_interface_mac (==client)
    uint8_t lwip_addr[6] =  {0x02, 0x02, 0x11, 0x22, 0x33, 0x02};

    const esp_netif_ip_info_t ip_cfg = {
            .ip = { .addr = _ip_addr },
            //.ip = { .addr = ESP_IP4TOADDR( 192, 168, 4, 1) },
            .netmask = { .addr = ESP_IP4TOADDR( 255, 255, 255, 0) },
            //.gw = { .addr = ESP_IP4TOADDR( 192, 168, 4, 1) },
            .gw = { .addr = ESP_IP4TOADDR( 0, 0, 0, 0) },
    };

    // Definition of
    // 1) Derive the base config (very similar to IDF's default WiFi AP with DHCP server)
    esp_netif_inherent_config_t base_cfg =  {
            .flags = esp_netif_flags_t(ESP_NETIF_DHCP_SERVER | ESP_NETIF_FLAG_AUTOUP), // Run DHCP server; set the netif "ip" immediately
            .mac = {0, 0, 0, 0, 0, 0},
            .ip_info = &ip_cfg,
            .get_ip_event = IP_EVENT_STA_GOT_IP,
            .lost_ip_event = IP_EVENT_STA_LOST_IP,
            .if_key = "wired",                                      // Set mame, key, priority
            .if_desc = "usb ncm config device",
            .route_prio = 10,
            .bridge_info = nullptr
    };
    // 2) Use static config for driver's config pointing only to static transmit and free functions
    esp_netif_driver_ifconfig_t driver_cfg = {
            .handle = (void *)1,                // not using an instance, USB-NCM is a static singleton (must be != nullptr)
            .transmit = netif_transmit,
            .transmit_wrap = nullptr,
            .driver_free_rx_buffer = l2_free    // point to Free Rx buffer function
    };

    // 3) USB-NCM is an Ethernet netif from lwip perspective, we already have IO definitions for that:
    struct esp_netif_netstack_config lwip_netif_config = {
            .lwip = {
                    .init_fn = ethernetif_init,
                    .input_fn = ethernetif_input
            }
    };

    // Config the esp-netif with:
    //   1) inherent config (behavioural settings of an interface)
    //   2) driver's config (connection to IO functions -- usb)
    //   3) stack config (using lwip IO functions -- derive from eth)
    esp_netif_config_t cfg = {
            .base = &base_cfg,
            .driver = &driver_cfg,
            .stack = &lwip_netif_config
    };

    netif = esp_netif_new(&cfg);
    if (netif == nullptr) {
        assert(0);
    }
    esp_netif_set_mac(netif, lwip_addr);

    /*
    esp_netif_dns_info_t dns_info = {0};
    IP_ADDR4(&dns_info.ip, 8, 8, 8, 8);
    esp_netif_set_dns_info(s_netif, ESP_NETIF_DNS_MAIN, &dns_info);
     */

    // set the minimum lease time
    uint32_t  lease_opt = 60;
    esp_netif_dhcps_option(netif, esp_netif_dhcp_option_mode_t(esp_netif_dhcp_option_mode_t::ESP_NETIF_OP_SET), esp_netif_dhcp_option_id_t(dhcp_msg_option::IP_ADDRESS_LEASE_TIME), (void*)&lease_opt, sizeof(lease_opt));

    // start the interface manually (as the driver has been started already)
    esp_netif_action_start(netif, 0, 0, 0);
}

void Network::Up() {
    ESP_LOGI("Network", "Starting with ssid %s, pwd %s, mdns %s, ip %s, is %d",
             _ssid.c_str(), _pwd.c_str(), _mdns.c_str(), _ip.c_str(), int(_if_type));
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());


    switch(_if_type) {
        case IF_TYPE::IF_TYPE_AP:
            wifi_init_softap();
            break;
        case IF_TYPE::IF_TYPE_STA:
            wifi_init_sta();
            break;
        case IF_TYPE::IF_TYPE_USBNCM:
            if_init_usbncm();
            break;
    }

    if(_if_type != IF_TYPE::IF_TYPE_USBNCM){
        ESP_LOGI("Network", "Disabling wifi power save mode");
        wifi_ps_type_t ps_mode = WIFI_PS_NONE;
        esp_wifi_set_ps(ps_mode);
    }

    // network interface to mdns services
    initialise_mdns(_mdns, _mdns_instance);
    netbiosns_init();
    netbiosns_set_name(_mdns.c_str());

}

void Network::SetIfType(IF_TYPE if_type){
    _if_type = if_type;
}

void Network::SetSSID(const string ssid) {
    _ssid = ssid;
}

void Network::SetPWD(const string pwd) {
    _pwd = pwd;
}

void Network::SetIP(string ip) {
    _ip = ip;
    // parse ip to 4 bytes and store in _ip_addr
    size_t start = 0;
    size_t end = 0;
    int index = 0;
    uint8_t octets[4];
    while ((end = ip.find('.', start)) != std::string::npos) {
        std::string token = ip.substr(start, end - start);
        int octet = std::stoi(token);
        octets[index] = static_cast<uint8_t>(octet);
        start = end + 1;
        index++;
    }
    std::string token = ip.substr(start);
    octets[index] = static_cast<uint8_t>(std::stoi(token));
    _ip_addr = ESP_IP4TOADDR(octets[0], octets[1], octets[2], octets[3]);
}

void Network::SetMDNSName(const string name) {
    _mdns = name;
    _mdns_instance = _mdns + "TBD, you define what it is";
}