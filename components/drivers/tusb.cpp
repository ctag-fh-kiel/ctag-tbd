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
#include "esp_log.h"
#include "esp_attr.h"

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
        "TBD",             // 1: Manufacturer
        "TBD-BBA",      // 2: Product
        "123456",              // 3: Serials, should use chip ID
        "TBD midi", // 4: MIDI
        "TBD net", // 5: NCM
        "", // 6: MAC
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

    tinyusb_config_t const tusb_cfg = {
            .device_descriptor = NULL, // If device_descriptor is NULL, tinyusb_driver_install() will use Kconfig
            .string_descriptor = s_str_desc,
            .string_descriptor_count = 7,
            .external_phy = false,
            .fs_configuration_descriptor = s_midi_cfg_desc,
            .hs_configuration_descriptor = s_midi_hs_cfg_desc,
            .self_powered = false,
            .vbus_monitor_io = 0
    };
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    vTaskDelay(500 / portTICK_PERIOD_MS);
}

IRAM_ATTR uint32_t CTAG::DRIVERS::tusb::Read(uint8_t *data, uint32_t len) {
    return tud_midi_n_stream_read( ITF_NUM_MIDI, ITF_NUM_MIDI_STREAMING, data, len);
}

uint32_t CTAG::DRIVERS::tusb::Write(const uint8_t *data, uint32_t len) {
    return tud_midi_stream_write(0, data, len);
}
