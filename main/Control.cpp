/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.
(c) 2023 MIDI-Message-Parser aka 'bba_update()' by Mathias Brüssel. 

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "Control.hpp"
#include "Calibration.hpp"
#include "gpio.hpp"
#include "adc.hpp"
#include "mk2.hpp"


// --- Calculate size of buffer for "CV" and "Gate/Trigger" values to be exchanged with audio-thread / plugins ---
#define DATA_SZ  (N_CVS * 4 + N_TRIGS + 2)

#if CONFIG_TBD_PLATFORM_MK2
// nothing to do here
#elif CONFIG_TBD_PLATFORM_BBA
    #define DEBUG_MIDI

    #include "midiuart.hpp"
    #include "tinyusb.h"
    #include "midi.hpp"                         // Provide methods and data for MIDI-message processing and communiction of detected events to audio-thread

    using namespace CTAG::CTRL;
    #define MIDI_BUF_SZ (RX_BUF_SIZE+32)

    // --- Instanciate objects for lowlevel and highlevel MIDI processing ---
    static CTAG::DRIVERS::midiuart midi;              // UART reader (and writer) for MIDI-messages
    DRAM_ATTR static CTAG::CTRL::Midi distribute;     // Instanciate Midi-Class as object for MIDI-message distribution, according to events mapped via WebUI

    // === Buffer to pass on MIDI-Event as virtual CV and Gate 'voltages', normalized to -1.f...+1.f (CV) and 0 or 1 integers (Triggers/Gates) ===
    DRAM_ATTR static uint8_t buf0[DATA_SZ];     // Common Array of Data for CVs and Triggers, will be passed on at audio-rate, so that Plugins can process this data
    DRAM_ATTR static float *midi_data = (float *) buf0; // CVs: Array of floats, positioned directly before Triggers in a common array for CVs+Triggers
    DRAM_ATTR static uint8_t *midi_note_trig = &buf0[N_CVS *
                                                     4];  // Triggers: Array of Bytes, positioned directly behind CVs in a common array for CVs+Triggers

    // --- MIDI incomind messages buffer to be read via UART ---
    DRAM_ATTR static uint8_t msgBuffer[MIDI_BUF_SZ]; // ## ??? Message-buffer for MIDI-parsing with added alligned space, in principle we only need 130 (128+2) Byte, though...

    #ifdef DEBUG_MIDI
    // debug queue
    #include <freertos/queue.h>
    QueueHandle_t debug_queue;
    struct debug_msg {
        uint32_t value;
        uint32_t max;
    };
    static void debug_task(void *) {
        debug_msg msg;
        while (true) {
            xQueueReceive(debug_queue, &msg, portMAX_DELAY);
            ESP_LOGE("Midi", "Buffer status: %li, maximum filling %li", msg.value, msg.max);
        }
    }
    #endif

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


    // --- General BBA Initialisation Method ---
    static void bba_init() {
    #ifdef DEBUG_MIDI
        debug_queue = xQueueCreate(10, sizeof(debug_msg));
        xTaskCreatePinnedToCore(debug_task, "debug", 4096, NULL, 5, NULL, 0);
    #endif
        memset(buf0, 0, DATA_SZ);                       // Reset "virtual CV"-data at startup
        memset(midi_note_trig, 1,
               N_TRIGS);             // Reset "virtual Gate/Trigger"-data at startup (1==off aka TRIG_OFF)
        distribute.setCVandTriggerPointers(midi_data, midi_note_trig);    // Pass on pointer to CV and Trigger shared data

        ESP_LOGI("Control", "USB initialization");

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

    // === Persistant variables for MIDI-parsing ===
    DRAM_ATTR static uint8_t missing_bytes_offset = 0;   // We may have to add that before we fetch our next buffer?
    DRAM_ATTR static int len = 0;
    DRAM_ATTR static uint8_t *ptr = NULL;

    // --- The macro below is used to exit the parser in case there is an invalid 2byte message: statusbyte when only data is expected ---
    #define SUPRESS_INVALID_3BYTE_MESSAGE() \
                if( (*(ptr+1))&0x80 )           \
                /* ESP_LOGI("MIDI", "*** Unexpected status in message => Ignored: %02x %02x %02x, Len: %03d", *ptr, *(ptr+1), *(ptr+2), len ); */ \
                { len--;  ptr++; break;  }      \
                if( (*(ptr+2))&0x80 )           \
                /* ESP_LOGI("MIDI", "+++ Ignored Message %02x %02x %02x, Len: %03d", *ptr, *(ptr+1), *(ptr+2), len ); */ \
                { len -= 2; ptr += 2; break; }

    // --- The macro below is used to exit the parser in case there is an invalid 2byte message: statusbyte when only data is expected ---
    #define SUPRESS_INVALID_2BYTE_MESSAGE() \
                if( (*(ptr+1))&0x80 )           \
                /* ESP_LOGI("MIDI", "--- Unexpected status in message => Ignored: %02x %02x %02x, Len: %03d", *ptr, *(ptr+1), *(ptr+2), len ); */ \
                { len--;  ptr++; break;  }

    // ===  MIDI-parsing method (Please note: Running status is not processed correctly with this implementation!) ===
    IRAM_ATTR static uint8_t *bba_update() {
        // --- Check if we have "leftover" data to be processed or if we need to ask for new MIDI-messages via polling ---
        if (len == 0)                        // Only read new buffer if we ran out of data
        {
            // get all available MIDI messages from USB
            uint8_t packet[4];
            uint32_t len2{0};
            bool read = false;
            while (tud_midi_available() && missing_bytes_offset + len2 < (MIDI_BUF_SZ - 32)) { // safety margin 32 bytes
                read = tud_midi_packet_read(packet);
                if (read) {
                    memcpy(&msgBuffer[missing_bytes_offset + len2], &packet[1],
                           3); // first USB Midi byte is the cable number, we don't need it
                    len2 += 3;
                }
            }
            // get all available MIDI messages from UART
            if (missing_bytes_offset + len2 < (MIDI_BUF_SZ - 32)) // safety margin
                midi.read(&msgBuffer[missing_bytes_offset + len2], &len);  // Read UART data into MIDI-buffer
            len += len2;

            if (len == 0)                   // Nothing to process now, better luck next time?
                return buf0;                // We return the identical CV / Trigger data as last round

            ptr = msgBuffer;                // We found a new message (or several new messages), reassign message-pointer!
            len += missing_bytes_offset;    // We may have incomplete messages from last round to process here, so we add their (partial) length, zero otherwise!
            missing_bytes_offset = 0;       // Reset missing-bytes counter, may be rearranged with next incomplete message due to incomplete buffering
        }
    #ifdef DEBUG_MIDI
        // debug buffer consumption
        static debug_msg msg {0, 0};
        msg.value = len;
        msg.max = msg.max < len ? len : msg.max;
        xQueueSendToFront(debug_queue, &msg, 0);
    #endif
        // --- Parse incoming MIDI-Channel-Voice-Messages and distribute to equivalent handler-methods ---
        while (len > 0)                    // Read complete buffer at once
        {
            switch ((*ptr) & 0xF0)                     // Mask out (possible) status-byte and check it for usage
            {
                case Midi::noteOnStatus:            // May also contain noteOff events that are send as noteOns with Velocity 0
                    if (len >= 3)                    // Message-Lenght as expected
                    {
                        SUPRESS_INVALID_3BYTE_MESSAGE();
                        distribute.noteOn(
                                ptr);     // Process message and fill buffer for virtual CV/Gates which can be looked up from plugin if tagged via WebUI
                        len -= 3;                   // Noteon Messages have 3 byte, we already read the status byte
                        ptr += 3;                   // Advance buffer-pointer to next message for next round of parsing
                    } else                            // Message is shorter than anticipaded
                    {
                        msgBuffer[0] = *ptr;        // Add first databyte to beginning of new buffer
                        missing_bytes_offset = len; // Len is 1 or 2
                        if (len > 1)
                            msgBuffer[1] = *(ptr + 1);// Add second databyte to beginning of new buffer
                        len = 0;                    // Not enough data left (buffer underrun?), skip message and read complete buffer next time!
                    }
                    break;

                case Midi::noteOffStatus:            // Switchs of Triggers of notes if noteOff for same Pitch as the active note is detected
                    if (len >= 3) {
                        SUPRESS_INVALID_3BYTE_MESSAGE();
                        distribute.noteOff(ptr);
                        ptr += 3;                   // Advance buffer-pointer to next message for next round of parsing
                        len -= 3;                   // Noteoff Messages have 3 byte, we already read the status byte
                    } else                            // Message is shorter than anticipaded
                    {
                        msgBuffer[0] = *ptr;        // Add first databyte to beginning of new buffer
                        missing_bytes_offset = len; // Len is 1 or 2
                        if (len > 1)
                            msgBuffer[1] = *(ptr + 1);// Add second databyte to beginning of new buffer
                        len = 0;                    // Not enough data left (buffer underrun?), skip message and read complete buffer next time!
                    }
                    break;

                case Midi::channelPressureStatus:    // aka AfterTouch
                    if (len >= 2) {
                        while ((len >= 4) && (*ptr == *(ptr +
                                                        2)))   // Overread redundant aftertouch messages! Only last in buffer will lead to new CV/Gate values
                        {
                            // ### ESP_LOGI("MIDI", "Aftertouch Ignored: %02x %02x, Len: %03d", *ptr, *(ptr+1), len );
                            len -= 2;   // Pitchshift Messages have 3 byte, we already read the status byte
                            ptr += 2;   // Overread current redundant pitchbend, it can't be validated by the audiothread anyhow, because we read the current MIDI-buffer in one go
                        }
                        SUPRESS_INVALID_2BYTE_MESSAGE();
                        distribute.channelPressure(
                                ptr);    // Process message and fill buffer for virtual CV/Gates which can be looked up from plugin if tagged via WebUI
                        len -= 2;                   // ChannelPressure Messages have 2 byte, we already read the status byte
                        ptr += 2;                   // Advance buffer-pointer to next message for next round of parsing
                    } else                            // Message is shorter than anticipaded
                    {
                        msgBuffer[0] = *ptr;        // Set beginning of next buffer to current data-byte
                        missing_bytes_offset = 1;   // We add this offset to the biginning of the next buffer, to "stitch together" our new message in next round
                        len = 0;                    // Not enough data left (buffer underrun?), skip message
                    }
                    break;

                case Midi::pitchBendStatus:        // 14bit Pitchbend (will be handled bipolarily)
                    if (len >= 3)                   // Message has suitable length
                    {
                        while ((len >= 6) && (*ptr == *(ptr +
                                                        3)))   // Overread redundant pitchbend messages! Only last in buffer will lead to new CV/Gate values
                        {
                            // ### ESP_LOGI("MIDI", "PitchBend Ignored: %02x %02x %02x, Len: %03d", *ptr, *(ptr+1), *(ptr+2), len );
                            len -= 3;   // Pitchshift Messages have 3 byte, we already read the status byte
                            ptr += 3;   // Overread current redundant pitchbend, it can't be validated by the audiothread anyhow, because we read the current MIDI-buffer in one go
                        }
                        SUPRESS_INVALID_3BYTE_MESSAGE();
                        distribute.pitchBend(
                                ptr);  // Process message and fill buffer for virtual CV/Gates which can be looked up from plugin if tagged via WebUI
                        len -= 3;                   // Pitchshift Messages have 3 byte, we already read the status byte
                        ptr += 3;                   // Advance buffer-pointer to next messager for next round of parsing
                    } else                            // Message is shorter than anticipaded
                    {
                        msgBuffer[0] = *ptr;        // Add first databyte to beginning of new buffer
                        missing_bytes_offset = len; // Len is 1 or 2
                        if (len > 1)
                            msgBuffer[1] = *(ptr + 1);// Add second databyte to beginning of new buffer
                        len = 0;                    // Not enough data left (buffer underrun?), skip message and read complete buffer next time!
                    }
                    break;

                case Midi::controlChangeStatus:     // Including Bank and Subbank information
                    if (len >= 3)                    // Message has correct lenght
                    {
                        while ((len >= 6) && (*ptr == *(ptr + 3)) && (*(ptr + 1) == *(ptr +
                                                                                      4)))   // Overread redundant (with identical CC-number) continous control messages! Only last in buffer will lead to new CV/Gate values
                        {
                            // ### ESP_LOGI("MIDI", "PitchBend Ignored: %02x %02x %02x, Len: %03d", *ptr, *(ptr+1), *(ptr+2), len );
                            len -= 3;   // Pitchshift Messages have 3 byte, we already read the status byte
                            ptr += 3;   // Overread current redundant pitchbend, it can't be validated by the audiothread anyhow, because we read the current MIDI-buffer in one go
                        }
                        SUPRESS_INVALID_3BYTE_MESSAGE();
                        distribute.controlChange(
                                ptr);  // Process message and fill buffer for virtual CV/Gates which can be looked up from plugin if tagged via WebUI
                        len -= 3;                   // Continuous Controller Messages have 3 byte, we already read the status byte
                        ptr += 3;                   // Advance buffer-pointer to next message for next round of parsing
                    } else                            // Message is shorter than anticipaded
                    {
                        msgBuffer[0] = *ptr;        // Add first databyte to beginning of new buffer
                        missing_bytes_offset = len; // Len is 1 or 2
                        if (len > 1)
                            msgBuffer[1] = *(ptr + 1);// Add second databyte to beginning of new buffer
                        len = 0;                    // Not enough data left (buffer underrun?), skip message and read complete buffer next time!
                    }
                    break;

                case Midi::programChangeStatus:     // We accept ProgramChange events as a kind of continuous controls, too
                    if (len >= 2) {
                        while ((len >= 4) && (*ptr == *(ptr +
                                                        2)))   // Overread redundant program-change messages! Only last in buffer will lead to new CV/Gate values
                        {
                            // ### ESP_LOGI("MIDI", "Aftertouch Ignored: %02x %02x, Len: %03d", *ptr, *(ptr+1), len );
                            len -= 2;   // Pitchshift Messages have 3 byte, we already read the status byte
                            ptr += 2;   // Overread current redundant pitchbend, it can't be validated by the audiothread anyhow, because we read the current MIDI-buffer in one go
                        }
                        SUPRESS_INVALID_2BYTE_MESSAGE();
                        distribute.programChange(
                                ptr);  // Process message and fill buffer for virtual CV/Gates which can be looked up from plugin if tagged via WebUI
                        len -= 2;                   // Program Change Messages have 3 byte, we already read the status byte
                        ptr += 2;                   // Advance buffer-pointer to next messager for next round of parsing
                    } else                            // Message is shorter than anticipaded
                    {
                        msgBuffer[0] = *ptr;        // Set beginning of next buffer to current data-byte
                        missing_bytes_offset = 1;   // We add this offset to the biginning of the next buffer, to "stitch together" our new message in next round
                        len = 0;                    // Not enough data left (buffer underrun?), skip message
                    }
                    break;

                default:        // No relevant MIDI-message (status-byte with data) found, increase on for "better luck next time"
                    len--;                          // Shorten read-lenght for next round of parsing
                    ptr++;                          // Skip invalid byte, may be simply a MIDI-clock message for instance
            }
        }
        return buf0;
    }

#else
    uint8_t CTAG::CTRL::Control::trig_data[N_TRIGS];
    float CTAG::CTRL::Control::cv_data[N_CVS];
#endif

IRAM_ATTR void CTAG::CTRL::Control::Update(uint8_t **trigs, float **cvs) {
#if defined(CONFIG_TBD_PLATFORM_MK2)
    uint8_t *data = (uint8_t *) DRIVERS::mk2::Update();
    *cvs = (float*) data;
    *trigs = &data[N_CVS*4];
    /* for debug purposes
    uint16_t *magic_number = (uint16_t*) &data[98];
    if(*magic_number != 0xcafe){
        // Debug transmission
        printf("%5d ", *magic_number);
        for(int i=0;i<8;i++){
            printf("%d ", (*trigs)[i]);
        }
        for(int i=0;i<18;i++){
            printf("%.3f ", (*cvs)[i]);
        }
        printf("\n");
    }
     */
#elif defined(CONFIG_TBD_PLATFORM_BBA)
    uint8_t *data = bba_update();
    *cvs = (float *) data;
    *trigs = &data[N_CVS * 4];
#else
    // update CVs
        CTAG::DRIVERS::ADC::Update();
        CTAG::CAL::Calibration::MapCVData(CTAG::DRIVERS::ADC::data, cv_data);
        *cvs = cv_data;

    // update trig data
        trig_data[0] = CTAG::DRIVERS::GPIO::GetTrig0();
        trig_data[1] = CTAG::DRIVERS::GPIO::GetTrig1();
        *trigs = trig_data;
#endif
}

void CTAG::CTRL::Control::SetCVChannelBiPolar(const bool &v0, const bool &v1, const bool &v2, const bool &v3) {
    // ifdefs to exclude this from BBA and MK2 are in Calibration.hpp
    CTAG::CAL::Calibration::ConfigCVChannels(v0 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v1 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v2 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v3 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar);
}

void CTAG::CTRL::Control::Init() {
    ESP_LOGI("Control", "Initializing control!");
#if CONFIG_TBD_PLATFORM_MK2
    DRIVERS::mk2::Init();
#elif CONFIG_TBD_PLATFORM_BBA
    bba_init();
#else
    DRIVERS::ADC::InitADCSystem();
    DRIVERS::GPIO::InitGPIO();
    CAL::Calibration::Init();
#endif
}

void CTAG::CTRL::Control::FlushBuffers() {
#ifdef CONFIG_TBD_PLATFORM_BBA
    midi.flush();
#endif
}