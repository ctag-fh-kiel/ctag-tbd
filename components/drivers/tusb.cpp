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

#include "tusb.hpp"
#include "tinyusb.h"
#include "tinyusb_default_config.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Interface counter
enum interface_count {
#if CFG_TUD_MIDI
    ITF_NUM_MIDI = 0,
    ITF_NUM_MIDI_STREAMING,
#endif
#if CFG_TUD_NCM
    ITF_NUM_NET,
    ITF_NUM_NET_DATA,
#endif
    ITF_COUNT
};

// USB Endpoint numbers
enum usb_endpoints {
    // Available USB Endpoints: 5 IN/OUT EPs and 1 IN EP
    EP_EMPTY = 0,
#if CFG_TUD_MIDI
    EPNUM_MIDI,
#endif
#if CFG_TUD_NCM
    EPNUM_NET_NOTIF,
    EPNUM_NET_DATA,
#endif
};

/** TinyUSB descriptors **/

#define TUSB_DESCRIPTOR_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_MIDI * TUD_MIDI_DESC_LEN + CFG_TUD_NCM * TUD_CDC_NCM_DESC_LEN)

/**
 * @brief String descriptor
 */
static const char *s_str_desc[7] = {
        // array of pointer to string descriptors
        (char[]) {0x09, 0x04},  // 0: is supported language is English (0x0409)
        "dadamachines",             // 1: Manufacturer
        "tbd-16",      // 2: Product
        "123456",              // 3: Serials, should use chip ID
        "tbd-16 midi", // 4: MIDI
        "tbd-16 net", // 5: NCM
        "020211223301", // 6: MAC address (must be 12 hex chars, matches net_config mac_addr)
};

/**
 * @brief Configuration descriptor
 *
 * This is a simple configuration descriptor that defines 1 configuration and a MIDI interface
 */
static const uint8_t s_midi_cfg_desc[] = {
        // Configuration number, interface count, string index, total length, attribute, power in mA
        TUD_CONFIG_DESCRIPTOR(1, ITF_COUNT, 0, TUSB_DESCRIPTOR_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

        // MIDI Interface number, string index, EP Out & EP In address, EP size
        TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 4, EPNUM_MIDI, (0x80 | EPNUM_MIDI), 64),

        // NCM Interface number, description string index, MAC address string index, EP notification address and size, EP data address (out, in), and size, max segment size
        TUD_CDC_NCM_DESCRIPTOR(ITF_NUM_NET, 5, 6, (0x80 | EPNUM_NET_NOTIF), 64, EPNUM_NET_DATA, (0x80 | EPNUM_NET_DATA), 64, CFG_TUD_NET_MTU),
};

static const uint8_t s_midi_hs_cfg_desc[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_COUNT, 0, TUSB_DESCRIPTOR_TOTAL_LEN, 0, 100),

    // Interface number, string index, EP Out & EP In address, EP size
    TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 4, EPNUM_MIDI, (0x80 | EPNUM_MIDI), 512),

    // NCM Interface number, description string index, MAC address string index, EP notification address and size, EP data address (out, in), and size, max segment size
    TUD_CDC_NCM_DESCRIPTOR(ITF_NUM_NET, 5, 6, (0x80 | EPNUM_NET_NOTIF), 512, EPNUM_NET_DATA, (0x80 | EPNUM_NET_DATA), 512, CFG_TUD_NET_MTU),

};

void CTAG::DRIVERS::tusb::Init() {
    ESP_LOGI("TUSB", "USB initialization");

    tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG();
    tusb_cfg.port = TINYUSB_PORT_HIGH_SPEED_0;
    tusb_cfg.task = TINYUSB_TASK_CUSTOM(4096, 10, 0); // stack 4096, priority 10, CPU0
    tusb_cfg.descriptor.string = s_str_desc;
    tusb_cfg.descriptor.string_count = 7;
    tusb_cfg.descriptor.full_speed_config = s_midi_cfg_desc;
    tusb_cfg.descriptor.high_speed_config = s_midi_hs_cfg_desc;
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    // Link starts UP by default (netd_init sets link_is_up = true).
    // macOS sees link UP from the initial SET_INTERFACE notification and
    // may start DHCP before our server is ready — that is fine; the DHCP
    // client retries, and we force a link bounce later to guarantee a
    // clean exchange (see if_init_usbncm in network.cpp).

    // Wait for USB device to be mounted by host (up to 3 seconds)
    // Use tud_mounted() — tud_ready() fails on macOS due to brief suspend
    int retries = 30;
    while (!tud_mounted() && retries > 0) {
        vTaskDelay(pdMS_TO_TICKS(100));
        retries--;
    }

    if (tud_mounted()) {
        ESP_LOGI("TUSB", "USB enumerated successfully");
    } else {
        ESP_LOGW("TUSB", "USB not enumerated after timeout, continuing anyway");
    }
}

IRAM_ATTR uint32_t CTAG::DRIVERS::tusb::Read(uint8_t *data, uint32_t len) {
    return tud_midi_n_stream_read( ITF_NUM_MIDI, ITF_NUM_MIDI_STREAMING, data, len);
}

uint32_t CTAG::DRIVERS::tusb::Write(const uint8_t *data, uint32_t len) {
    return tud_midi_stream_write(0, data, len);
}

bool CTAG::DRIVERS::tusb::IsNCMReady() {
    // Mirrors the per-iteration check inside WaitForNCMReady. Cheap to
    // poll on every SPI response cycle.
    return tud_mounted() && tud_network_can_xmit(64);
}

bool CTAG::DRIVERS::tusb::WaitForNCMReady(uint32_t timeout_ms) {
    ESP_LOGI("TUSB", "Waiting for NCM interface to be ready...");

    uint32_t elapsed = 0;
    const uint32_t interval = 100;

    while (elapsed < timeout_ms) {
        // Check if NCM can transmit - this indicates the interface is fully ready
        // Use tud_mounted() — tud_ready() fails on macOS due to brief suspend
        if (tud_mounted() && tud_network_can_xmit(64)) {
            ESP_LOGI("TUSB", "NCM interface ready after %lu ms", elapsed);
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(interval));
        elapsed += interval;
    }

    ESP_LOGW("TUSB", "NCM interface not ready after %lu ms timeout", timeout_ms);
    return false;
}

// Declared in network.cpp — shared NCM failure state
extern volatile uint32_t s_ncm_consecutive_fails;
extern volatile bool s_ncm_ever_connected;

// Returns true if NCM link is healthy: packets have flowed AND no recent failures.
// s_ncm_ever_connected is a permanent latch — not sufficient alone because macOS
// can briefly connect (DHCP works), then abort the link.  We must also verify
// that s_ncm_consecutive_fails is 0 (no accumulating send failures).
static bool ncm_link_healthy() {
    return s_ncm_ever_connected && s_ncm_consecutive_fails == 0;
}

// NCM watchdog: macOS aborts setupDataTransfers ~50% of the time.
// Detection: check ncm_link_healthy() = ever_connected AND no consecutive fails.
// Recovery: esp_restart() — tinyusb_driver_uninstall() while the lwIP network
// stack is active causes a Guru Meditation crash (Load access fault) because
// netif_transmit() accesses freed USB structures.  A clean full reboot is the
// only safe recovery.
//
// Reboot limit: uses esp_reset_reason() — if the last boot was already a
// software restart (from us or a crash), don't restart again. This gives
// exactly 1 retry per power cycle and prevents infinite reboot loops.
// ESP32-P4 has no RTC memory, so we can't persist a counter.
static void ncm_watchdog_task(void *) {
    static const char *TAG = "TUSB";

    // Check if we're allowed to restart. Only restart once per power cycle.
    // ESP_RST_POWERON = fresh power-on → OK to restart if needed.
    // ESP_RST_SW / ESP_RST_PANIC = we already restarted → don't restart again.
    esp_reset_reason_t reason = esp_reset_reason();
    bool can_restart = (reason == ESP_RST_POWERON);
    ESP_LOGI(TAG, "NCM watchdog: reset reason=%d, restart %s",
             (int)reason, can_restart ? "allowed" : "blocked (already retried)");

    // Wait for boot + USB enum + macOS first setupDataTransfers + DHCP.
    // Network init happens around t=5s. If link works, DHCP completes by ~9s.
    vTaskDelay(pdMS_TO_TICKS(12000));

    if (ncm_link_healthy()) {
        ESP_LOGI(TAG, "NCM watchdog: link healthy, monitoring stability for 30s");

        // Phase 2: stability monitoring — keep checking for 30s.
        // macOS can connect then abort within seconds, so one check isn't enough.
        for (int i = 0; i < 15; i++) {
            vTaskDelay(pdMS_TO_TICKS(2000));
            if (!ncm_link_healthy()) {
                if (can_restart) {
                    ESP_LOGW(TAG, "NCM watchdog: link broke during stability (fails=%lu), restarting device",
                             (unsigned long)s_ncm_consecutive_fails);
                    esp_restart();
                }
                ESP_LOGW(TAG, "NCM watchdog: link broke but restart blocked — giving up");
                vTaskDelete(NULL);
                return;
            }
        }

        ESP_LOGI(TAG, "NCM watchdog: link stable for 30s, done");
        vTaskDelete(NULL);
        return;
    }

    // Link unhealthy after 12s
    if (!tud_mounted()) {
        // No USB host — charger or powerbank. Don't reboot.
        ESP_LOGI(TAG, "NCM watchdog: no USB host detected, NCM not available");
        vTaskDelete(NULL);
        return;
    }

    if (!can_restart) {
        // Already retried once — device works fine, just no WebUI this session.
        ESP_LOGW(TAG, "NCM watchdog: link unhealthy (ever_ok=%d fails=%lu) but restart blocked — giving up",
                 (int)s_ncm_ever_connected, (unsigned long)s_ncm_consecutive_fails);
        vTaskDelete(NULL);
        return;
    }

    // USB host present but NCM failed — macOS abort case, first attempt
    ESP_LOGW(TAG, "NCM watchdog: link unhealthy (ever_ok=%d fails=%lu) with USB host, restarting device",
             (int)s_ncm_ever_connected, (unsigned long)s_ncm_consecutive_fails);
    esp_restart();
}

void CTAG::DRIVERS::tusb::StartNCMWatchdog() {
    xTaskCreatePinnedToCore(ncm_watchdog_task, "ncm_wd", 2048, nullptr,
                            tskIDLE_PRIORITY + 2, nullptr, 0);
}

