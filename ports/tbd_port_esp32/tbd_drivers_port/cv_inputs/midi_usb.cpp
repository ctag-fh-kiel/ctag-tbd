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
#include <tbd/drivers/common/midi_usb.hpp>

#include "tinyusb.h"
#include "esp_log.h"
#include "esp_attr.h"

// Interface counter
enum interface_count {
#if CFG_TUD_MIDI
    ITF_NUM_MIDI = 0,
    ITF_NUM_MIDI_STREAMING,
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
};

/** TinyUSB descriptors **/

#define TUSB_DESCRIPTOR_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_MIDI * TUD_MIDI_DESC_LEN)

/**
 * @brief String descriptor
 */
static const char *s_str_desc[5] = {
        // array of pointer to string descriptors
        (char[]) {0x09, 0x04},  // 0: is supported language is English (0x0409)
        "CTAG",             // 1: Manufacturer
        "CTAG-TBD-BBA",      // 2: Product
        "123456",              // 3: Serials, should use chip ID
        "MIDI device", // 4: MIDI
};

/**
 * @brief Configuration descriptor
 *
 * This is a simple configuration descriptor that defines 1 configuration and a MIDI interface
 */
static const uint8_t s_midi_cfg_desc[] = {
        // Configuration number, interface count, string index, total length, attribute, power in mA
        TUD_CONFIG_DESCRIPTOR(1, ITF_COUNT, 0, TUSB_DESCRIPTOR_TOTAL_LEN, 0, 100),

        // Interface number, string index, EP Out & EP In address, EP size
        TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 4, EPNUM_MIDI, (0x80 | EPNUM_MIDI), 64),
};

namespace tbd::drivers {

void MidiUsb::Init() {
    ESP_LOGI("TUSB", "USB initialization");

    tinyusb_config_t const tusb_cfg = {
            .device_descriptor = NULL, // If device_descriptor is NULL, tinyusb_driver_install() will use Kconfig
            .string_descriptor = s_str_desc,
            .string_descriptor_count = sizeof(s_str_desc) / sizeof(s_str_desc[0]),
            .external_phy = false,
            .configuration_descriptor = s_midi_cfg_desc,
            .self_powered = false,
            .vbus_monitor_io = 0
    };
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
}

IRAM_ATTR uint32_t MidiUsb::Read(uint8_t *data, uint32_t len) {
    return tud_midi_n_stream_read( ITF_NUM_MIDI, ITF_NUM_MIDI_STREAMING, data, len);
}

uint32_t MidiUsb::Write(const uint8_t *data, uint32_t len) {
    return tud_midi_stream_write(0, data, len);
}

}