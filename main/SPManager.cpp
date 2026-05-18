/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020-2026 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/


#include "SPManager.hpp"
#include "esp_log.h"
#include "esp_cpu.h"
#include "esp_timer.h"
#include "stdint.h"
#include "string.h"
#include "codec_bba.hpp"
#include "esp_heap_caps.h"
#include "led_rgb_bba.hpp"
#include "network.hpp"
#include "tusb.hpp"
// File-scope peak meters fed by PicoSeqRack — used to drive the OLED
// Input / Output indicators. Declared `weak` so SPManager links even
// when PicoSeqRack.cpp isn't part of the active processor build (the
// rack header has no include guard, so direct extern was avoided).
// PicoSeqRack.cpp provides the strong definitions; if it's compiled
// out, these zero fallbacks resolve and the meters simply read 0.
__attribute__((weak)) volatile float g_peakInputTrack = 0.f;
__attribute__((weak)) volatile float g_peakSynthOnly  = 0.f;
#include "RestServer.hpp"
#if CONFIG_TBD_USE_RP2350
#include "SpiAPI.hpp"
#endif
#include "Control.hpp"
#include "helpers/ctagFastMath.hpp"
#include "helpers/ctagSampleRom.hpp"
#include "stmlib/dsp/dsp.h"
#if CONFIG_TBD_USE_RP2350
#include "rp2350_spi_stream.hpp"
#endif
// ableton link
#include "link.hpp"
#if CONFIG_TBD_USE_RP2350
#include "SpiProtocol.h"
#include "SpiProtocolHelper.hpp"
#endif
#if CONFIG_TBD_USE_SD_CARD
#include "EngineDefinitionDataModel.hpp"
#include "MacroSoundPresetDataModel.hpp"
#include "MacroDeviceDefinitionDataModel.hpp"
#include "MacroTranslator.hpp"
#include "StorageOverlay.hpp"
#endif

#define MAX(x, y) ((x)>(y)) ? (x) : (y)
#define MIN(x, y) ((x)<(y)) ? (x) : (y)

#define BUF_SZ 32

using namespace CTAG;
using namespace CTAG::AUDIO;
using namespace CTAG::DRIVERS;
#if CONFIG_TBD_USE_SD_CARD
using namespace CTAG::MACROPRESETS;
#endif

#define CPU_MAX_ALLOWED_CYCLES 300000 // 261224 // is 32/44100kHz * 360MHz
#define SPI_TRANSACTION_TIMEOUT_US 200000

// global variable, sdcard base directory
namespace CTAG {
    namespace RESOURCES {
#if CONFIG_TBD_USE_SD_CARD
        std::string sdcardRoot = "/sdcard";
#else
        std::string sdcardRoot = "/littlefs";
#endif
    }
}

volatile uint32_t SoundProcessorManager::slowProcessCounter = 0;
volatile uint32_t SoundProcessorManager::sentSynthMidiBytes = 0;
volatile uint32_t SoundProcessorManager::receivedUsbDeviceMidiBytes = 0;
volatile uint32_t SoundProcessorManager::requestCounterErrors = 0;
volatile uint32_t SoundProcessorManager::audioLockErrors = 0;

// audio real-time task
void IRAM_ATTR SoundProcessorManager::audio_task(void *pvParams) {
    float finput[BUF_SZ * 2];
    float fbuf[BUF_SZ * 2];
    float finput2[BUF_SZ * 2];
    float fbuf2[BUF_SZ * 2];
    float peakIn = 0.f, peakOut = 0.f;
    int64_t before;
    bool isStereoCH0 = false;
    esp_cpu_cycle_count_t start, diff;

#if CONFIG_TBD_USE_RP2350
    SpiProtocolHelper protocol;
#endif

    // Provide dummy cv/trig buffers so plugins that access pd.cv[x] or
    // pd.trig[x] don't crash with a null-pointer dereference.
    // The macropresets SPI protocol carries MIDI only — no CV/trig data.
    static float  dummy_cv[N_CVS]    = {};   // zero-filled
    static uint8_t dummy_trig[N_TRIGS] = {};  // zero-filled

    SP::ProcessData pd;
    pd.controlData = nullptr;
    pd.cv = dummy_cv;
    pd.trig = dummy_trig;
    pd.buf = fbuf;
    pd.sequencer_tempo = 12000;
    pd.midi_bytes_length = 0;
    memset(&pd.midi_bytes, 0, sizeof(pd.midi_bytes));

    // wait a bit to let everything initialize and stabilize
    vTaskDelay(pdMS_TO_TICKS(4000));

#if CONFIG_TBD_USE_RP2350
    int64_t nextspitime = 0;
    int64_t nextspireceivedeadline = 0;

    int responsecounter = 0;
#endif
    int framecounter = 0;

    ESP_LOGI("SPManager", "Audio task started, entering main loop.");

    while (runAudioTask) {
#if CONFIG_TBD_USE_RP2350
        //
        // Prepare a response.
        //
        int64_t now = esp_timer_get_time();

        if (protocol.shouldPrepareNextResponse()) {
            // printf("protocol: preparing next response (seq %d)\n", protocol.nextResponseSequenceCounter);

            uint8_t *sendbuffer = nullptr;
            CTAG::DRIVERS::rp2350_spi_stream::GetSendBuffer((void **)&sendbuffer);

            if (sendbuffer != nullptr) {
                // printf("protocol: got write buffer\n");

                p4_spi_response_header *send_header = (p4_spi_response_header *)sendbuffer;
                p4_spi_response2 *send_response =
                    (p4_spi_response2 *)(sendbuffer + P4_SPI_RESPONSE_HEADER_SIZE);

                //
                // prepare next response
                //

                // pack midi data from USB device midi
                uint8_t *midi_ptr = (uint8_t*) &send_response->usb_device_midi;
                uint32_t *midi_len = (uint32_t*) &send_response->usb_device_midi_length;
                *midi_len = tusb::Read(midi_ptr, P4_SPI_RESPONSE_USB_MIDI_DATA_SIZE);
                // if (*midi_len > 0) {
                //     printf("Received %d bytes of USB device midi data: %02X %02X %02X %02X...\n",
                //         (int)(*midi_len), midi_ptr[0], midi_ptr[1], midi_ptr[2], midi_ptr[3]);
                // }
                receivedUsbDeviceMidiBytes += *midi_len;

                // Pack stereo input + output samples into the response
                // (used by the Live Waveform display). Clamped before
                // the cast so values outside ±1 saturate cleanly.
                for (int i = 0; i < BUF_SZ * 2; i++) {
                    float fi = finput2[i];
                    float fo = fbuf2[i];
                    if (fi >  1.0f) fi =  1.0f; else if (fi < -1.0f) fi = -1.0f;
                    if (fo >  1.0f) fo =  1.0f; else if (fo < -1.0f) fo = -1.0f;
                    send_response->input_waveform[i]  = (uint8_t)((int)(fi * 127.0f) + 128);
                    send_response->output_waveform[i] = (uint8_t)((int)(fo * 127.0f) + 128);
                }

                // Dedicated peak bytes for the OLED header meters,
                // sourced from the rack's per-track contribution split:
                //   input_peak_byte  = g_peakInputTrack — peak of ch16's
                //                      (Audio Input track) contribution
                //                      to combined_out, captured right
                //                      after ch16 mixes.
                //   output_peak_byte = g_peakSynthOnly  — peak of synth
                //                      tracks' contribution only
                //                      (combined_out − ch16 snapshot).
                // ch16 audio shows ONLY on the input meter; synth tracks
                // ONLY on the output meter — full bus separation.
                float pin  = g_peakInputTrack;
                float pout = g_peakSynthOnly;
                if (pin  < 0.f) pin  = 0.f;
                if (pout < 0.f) pout = 0.f;
                if (pin  > 1.f) pin  = 1.f;
                if (pout > 1.f) pout = 1.f;
                send_response->input_peak_byte  = (uint8_t)(pin  * 255.0f);
                send_response->output_peak_byte = (uint8_t)(pout * 255.0f);

                // pack ableton link data (after waveforms to avoid any overflow risk)
                LINK::link_session_data_t *link_data = (LINK::link_session_data_t*)&send_response->link_data;
                LINK::link::GetLinkRtSessionData(link_data);

                // and the led color
                send_response->led_color = ledStatus;
                send_response->webui_update_counter = webuiChangeCounter.load();
                send_response->screenshot_request_counter = screenshotRequestCounter.load();
                send_response->injected_button = injectedButton.exchange(0);
                send_response->injected_button_event = injectedButtonEvent.exchange(0);
                {
                    auto if_type = CTAG::NET::Network::GetIfType();
                    uint8_t net = 0;
                    if (if_type == CTAG::NET::Network::IF_TYPE::IF_TYPE_USBNCM) {
                        net = CTAG::DRIVERS::tusb::IsNCMReady() ? 1 : 0;
                    } else if (if_type == CTAG::NET::Network::IF_TYPE::IF_TYPE_STA) {
                        net = 2;
                    } else if (if_type == CTAG::NET::Network::IF_TYPE::IF_TYPE_AP) {
                        net = 3;
                    }
                    send_response->network_status = net;
                }
                send_response->_reserved_input = 0;
                responsecounter ++;
                send_response->magic = 0xDEADBEEF;
                send_response->magic2 = 0xFEED;
                protocol.markNextResponsePrepared(
                    (p4_spi_response_header*) send_header,
                    (p4_spi_response2*) send_response);
                // printf("protocol: next response prepared\n");
            } else {
                // AUDIO-THREAD: this loop is the audio task. printf on the
                // audio thread corrupts the output buffer. Recovery path
                // (skipping the prepare) is preserved.
                // printf("protocol: no write buffer available\n");
            }
        } else {
            // printf("protocol: no need to prepare next response\n");
        }

        if (protocol.shouldSendPreparedResponse()) {
            // printf("protocol: queue response\n");
            uint8_t *sendbuffer = nullptr;
            CTAG::DRIVERS::rp2350_spi_stream::GetSendBuffer((void **)&sendbuffer);
            if (sendbuffer != nullptr) {
                p4_spi_response_header *send_header = (p4_spi_response_header *)sendbuffer;
                p4_spi_response2 *send_response =
                    (p4_spi_response2 *)(sendbuffer + P4_SPI_RESPONSE_HEADER_SIZE);
                protocol.updateResponseBeforeSending(send_header, send_response);
                CTAG::DRIVERS::rp2350_spi_stream::QueueBuffer((void *)sendbuffer);
                protocol.queuedPreparedResponse();
                nextspireceivedeadline = now + SPI_TRANSACTION_TIMEOUT_US;
            }
        } else {
            // printf("protocol: should not send response.\n");
        }

        //
        // check if current spi transaction is done
        //

        uint8_t *spi_request_ptr = nullptr;
        if (CTAG::DRIVERS::rp2350_spi_stream::GetReceivedBuffer((void **)&spi_request_ptr)) {
            p4_spi_request_header *spi_req_header = (p4_spi_request_header *)spi_request_ptr;
            p4_spi_request2 *spi_req =
                (p4_spi_request2 *)(spi_request_ptr + P4_SPI_REQUEST_HEADER_SIZE);

            if (protocol.validateRequestPacket(spi_req_header, spi_req)) {
                // printf("protocol: received transaction. (seq %d)\n", spi_req_header->request_sequence_counter);

                // for(int k=0; k<8; k++) {
                //     printf("  %02X %02X %02X %02X %02X %02X %02X %02X\n",
                //         spi_request_ptr[8 * k + 0],
                //         spi_request_ptr[8 * k + 1],
                //         spi_request_ptr[8 * k + 2],
                //         spi_request_ptr[8 * k + 3],
                //         spi_request_ptr[8 * k + 4],
                //         spi_request_ptr[8 * k + 5],
                //         spi_request_ptr[8 * k + 6],
                //         spi_request_ptr[8 * k + 7]);
                // }

                // request is valid
                // if (spi_req->synth_midi_length > 1) {
                //     printf("got %d midi bytes, seq %d\n", (int)spi_req->synth_midi_length, spi_req_header->request_sequence_counter);
                // }

                pd.controlData = (void *)1; // run processing...
                memset(&pd.midi_bytes, 0, sizeof(pd.midi_bytes));
                memcpy(&pd.midi_bytes, (uint8_t*) &spi_req->synth_midi, spi_req->synth_midi_length);
                pd.midi_bytes_length = spi_req->synth_midi_length;
                // if (spi_req->synth_midi_length > 1) {
                //     printf("Received %d bytes of synth midi data: %02X %02X %02X %02X %02X %02X %02X %02X...\n",
                //         (int)(spi_req->synth_midi_length),
                //         spi_req->synth_midi[0],
                //         spi_req->synth_midi[1],
                //         spi_req->synth_midi[2],
                //         spi_req->synth_midi[3],
                //         spi_req->synth_midi[4],
                //         spi_req->synth_midi[5],
                //         spi_req->synth_midi[6],
                //         spi_req->synth_midi[7]);
                // }
                // printf("SPI Request tempo %ld\n", spi_req.sequencer_tempo);
                pd.sequencer_tempo = spi_req->sequencer_tempo;
                sentSynthMidiBytes += spi_req->synth_midi_length;

                uint8_t expNext = protocol.getNextSequence(protocol.lastSeenRequestCounter);
                if (spi_req_header->request_sequence_counter != expNext) {
                    // printf("expected sequence %d but got %d, did we miss a packet?\n",
                    //     expNext, spi_req_header->request_sequence_counter);
                };

                protocol.markRequestSeen(spi_req_header->request_sequence_counter);
            } else {
                // AUDIO-THREAD: see prepare-buffer note above.
                // printf("protocol: packet invalid\n");
            }
        } else {
            // printf("protocol: did not receive packet\n");

            // check timeout...
            if (now > nextspireceivedeadline) {
                // AUDIO-THREAD: see prepare-buffer note above.
                // printf("SPI receive timeout.\n");
                // protocol.markRequestSeen(0);
                nextspireceivedeadline = now + SPI_TRANSACTION_TIMEOUT_US;
            }
        }
#else
        // Without RP2350: always process, read MIDI from TinyUSB directly
        pd.controlData = (void *)1;
        memset(&pd.midi_bytes, 0, sizeof(pd.midi_bytes));
        pd.midi_bytes_length = tusb::Read((uint8_t*)pd.midi_bytes, sizeof(pd.midi_bytes));
#endif // CONFIG_TBD_USE_RP2350

        taskYIELD();

        // get normalized raw data from CODEC
        DRIVERS::Codec::ReadBuffer(finput, BUF_SZ);

        memcpy(finput2, finput, BUF_SZ * 2 * sizeof(float));
        memcpy(fbuf, finput, BUF_SZ * 2 * sizeof(float));

        // track the cpu cycles for audio task
        start = esp_cpu_get_cycle_count();
        before = esp_timer_get_time();

        // In peak detection, dc cut is done in codec
        float max = 0.f;
        // just take first sample of block for level meter
        max = fabsf(fbuf[0] + fbuf[1]) / 2.f;
        peakIn = 0.95f * peakIn + 0.05f * max;

        // led indicator, green for input
        max = 255.f + 3.2f * CTAG::SP::HELPERS::fast_dBV(peakIn); // cut away at approx -80dB
        uint32_t ledData = 0;
        //ESP_LOGI("SP", "Max %.9f %f", peakIn, max);
        if (max > 0) {
            ledData = ((uint32_t) max);
            ledData <<= 8; // green
        }

        // sound processors - safer version with RAII and additional checks
        bool processingLocked = (xSemaphoreTake(processMutex, 0) == pdTRUE);
        if (processingLocked) {
            // Validate control data and sound processors before processing
            bool canProcess = (pd.controlData != nullptr) &&
                              (sp[0] != nullptr || sp[1] != nullptr);

            if (canProcess) {
#if CONFIG_TBD_USE_SD_CARD
                MacroTranslator::instance().TranslateInput(&pd);
#endif
                memset(&pd.midi_bytes, 0, pd.midi_bytes_length); // clear buffer

                // Process channel 0
                if (sp[0] != nullptr) {
                    isStereoCH0 = sp[0]->GetIsStereo();
                    sp[0]->Process(pd);
                }

                // Process channel 1 (only if ch0 is not stereo)
                if (!isStereoCH0) {
                    // Daisy chain: copy ch0 output to ch1 input
                    if (ch01Daisy) {
                        for (uint32_t i = 0; i < BUF_SZ; i++) {
                            fbuf[i * 2 + 1] = fbuf[i * 2];
                        }
                    }

                    if (sp[1] != nullptr) {
                        sp[1]->Process(pd);
                    }
                }
            } else {
                // Mute audio if processors unavailable
                memset(fbuf, 0, BUF_SZ * 2 * sizeof(float));
            }

            // memset(&pd.midi_bytes, 0, sizeof(pd.midi_bytes));
            pd.midi_bytes_length = 0;

            // Always release mutex
            xSemaphoreGive(processMutex);
        } else {
            // Couldn't acquire mutex - mute audio for this buffer
            memset(fbuf, 0, BUF_SZ * 2 * sizeof(float));
            audioLockErrors ++;
        }


        // to stereo conversion
        if (!isStereoCH0) {
            if (toStereoCH0 || toStereoCH1) {
                float sb[BUF_SZ * 2];
                memcpy(sb, fbuf, BUF_SZ * 2 * sizeof(float));
                if (toStereoCH0 == 1 && toStereoCH1 == 0) { // spread CH0 to both channels
                    for (uint32_t i = 0; i < BUF_SZ; i++) {
                        fbuf[i * 2] = 0.5f * sb[i * 2];
                        fbuf[i * 2 + 1] = 0.5f * sb[i * 2] + sb[i * 2 + 1];
                    }
                } else if (toStereoCH1 == 1 && toStereoCH0 == 0) { // spread CH1 to both channels
                    for (uint32_t i = 0; i < BUF_SZ; i++) {
                        fbuf[i * 2] = 0.5f * sb[i * 2 + 1] + sb[i * 2];
                        fbuf[i * 2 + 1] = 0.5f * sb[i * 2 + 1];
                    }
                } else if (toStereoCH0 == 1 && toStereoCH1 == 1) { // spread CH0 + CH1 to both channels
                    for (uint32_t i = 0; i < BUF_SZ; i++) {
                        fbuf[i * 2] = fbuf[i * 2 + 1] = 0.5f * (sb[i * 2] + sb[i * 2 + 1]);
                    }
                } else if (toStereoCH0 == 2 && toStereoCH1 == 2) { // swap channels
                    for (uint32_t i = 0; i < BUF_SZ; i++) {
                        fbuf[i * 2] = sb[i * 2 + 1];
                        fbuf[i * 2 + 1] = sb[i * 2];
                    }
                } else if (toStereoCH0 == 2 && toStereoCH1 == 0) { // mix CH0 with CH1 on CH1
                    for (uint32_t i = 0; i < BUF_SZ; i++) {
                        fbuf[i * 2] = 0.f;
                        fbuf[i * 2 + 1] += sb[i * 2];
                    }
                } else if (toStereoCH0 == 0 && toStereoCH1 == 2) { // mix CH1 with CH0 on CH0
                    for (uint32_t i = 0; i < BUF_SZ; i++) {
                        fbuf[i * 2] += sb[i * 2 + 1];
                        fbuf[i * 2 + 1] = 0.f;
                    }
                } else if (toStereoCH0 == 2 && toStereoCH1 == 1) { // move CH0 to CH1, spread CH1 to both
                    for (uint32_t i = 0; i < BUF_SZ; i++) {
                        fbuf[i * 2] = 0.5f * sb[i * 2 + 1];
                        fbuf[i * 2 + 1] = 0.5f * sb[i * 2 + 1] + sb[i * 2];
                    }
                } else if (toStereoCH0 == 1 && toStereoCH1 == 2) { // move CH1 to CH0, spread CH0 to both
                    for (uint32_t i = 0; i < BUF_SZ; i++) {
                        fbuf[i * 2] = 0.5f * sb[i * 2] + sb[i * 2 + 1];
                        fbuf[i * 2 + 1] = 0.5f * sb[i * 2];
                    }
                }
            }
        }

        // Output peak detection — proper per-block peak detector reading
        // BEFORE the SoftClip stage, so we can distinguish "loud but clean"
        // from "softclip is actively catching" (the actually useful signal).
        // Old code measured only fbuf[0]+fbuf[1]/2 of the first sample after
        // softclip, which couldn't reflect either intent.
        float peakRaw = 0.f;
        for (uint32_t i = 0; i < BUF_SZ; i++) {
            float lin_l = fbuf[i * 2];
            float lin_r = fbuf[i * 2 + 1];
            float a = fabsf(lin_l) > fabsf(lin_r) ? fabsf(lin_l) : fabsf(lin_r);
            if (a > peakRaw) peakRaw = a;
            // Soft limiting (legacy ctag-tbd safety clipper, default ON).
            if (ch0_outputSoftClip) fbuf[i * 2]     = stmlib::SoftClip(lin_l);
            if (ch1_outputSoftClip) fbuf[i * 2 + 1] = stmlib::SoftClip(lin_r);
        }
        peakOut = 0.9f * peakOut + 0.1f * peakRaw;

        // Multi-zone status LED. Input meter already painted green above.
        // Output zones layer on top:
        //   peakOut < 0.5    : no extra colour — input green dominates (clean).
        //   0.5..0.85         : red intensity rising; blends with input green
        //                       → yellow (signal approaching ceiling).
        //   0.85..1.0         : solid red (at ceiling, softclip about to engage).
        //   > 1.0             : red + blue → magenta (softclip ACTIVELY catching
        //                       peaks that would otherwise clip — audio is still
        //                       safe, the cubic saturator is doing its job, but
        //                       you're hearing soft saturation).
        if (peakOut > 1.0f) {
            ledData |= 255u << 16;          // full red
            ledData |= 180u << 0;           // + blue → magenta tint = "softclip catching"
        } else if (peakOut > 0.85f) {
            ledData |= 255u << 16;          // solid red — at output ceiling
        } else if (peakOut > 0.5f) {
            int r = (int)((peakOut - 0.5f) / 0.35f * 200.f);
            if (r > 200) r = 200;
            ledData |= ((uint32_t)r) << 16; // rising red; mixes with input green → yellow
        }
        // peakOut < 0.5 → output adds no colour; input meter shows alone.

        // get cpu cycles for audio task and tone led
        diff = esp_cpu_get_cycle_count() - start;
        int64_t diff2 = esp_timer_get_time() - before;
        if(diff > CPU_MAX_ALLOWED_CYCLES) {
            slowProcessCounter ++;
            // ledData = 0xB39134; // orange code for cpu overflow
        }
        ledStatus = ledData;

        memcpy(fbuf2, fbuf, BUF_SZ * 2 * sizeof(float));

        // write raw float data back to CODEC
        DRIVERS::Codec::WriteBuffer(fbuf, BUF_SZ);

        if (framecounter % 3200 == 0) {
            //     printf("Audio task cycles %d, micros %d, slow process() counter %d/%d, fbuf = [%1.3f, %1.3f...], tempo %ld, sentSynthMidi %d b, receivedUsbDeviceMidi %d b, %d new request counter errors, %d lock errors\n", (int)diff, (int)diff2, (int)slowProcessCounter, (int)framecounter, fbuf[0], fbuf[BUF_SZ], pd.sequencer_tempo, (int)sentSynthMidiBytes, (int)receivedUsbDeviceMidiBytes, (int)requestCounterErrors, (int)audioLockErrors);
            //     requestCounterErrors = 0;
            // AUDIO-THREAD: gated to every ~2.3 s, but still audio-thread.
            // printf corrupts the output buffer.
            // printf("Audio task CPU time %d uS\n", (int)diff2);
        }

        taskYIELD();

        framecounter ++;
    }
    memset(fbuf, 0, BUF_SZ * 2 * sizeof(float));
    DRIVERS::Codec::WriteBuffer(fbuf, BUF_SZ);
    runAudioTask = 2;
    vTaskDelete(NULL);
}

void SoundProcessorManager::SetSoundProcessorChannel(const int chan, const string &id) {
    ledBlink = 5;

    // does the SP exist?
    if(!model->HasPluginID(id)) return;

    // when trying to set chan 1 and chan 0 is a stereo plugin, return
    if(chan == 1 && model->IsStereo(model->GetActiveProcessorID(0))) return;
    if(chan == 1 && model->IsStereo(id)) return;

    ESP_LOGI("SPManager", "Switching ch%d to plugin %s", chan, id.c_str());

    // destroy active plugin
    xSemaphoreTake(processMutex, portMAX_DELAY);
    if(nullptr != sp[chan]){
        delete sp[chan]; // destruct processor
        sp[chan] = nullptr;
    }
    if (model->IsStereo(id) && chan == 0) {
        if(nullptr != sp[1]){
            delete sp[1]; // destruct processor
            sp[1] = nullptr;
        }
    }
#if CONFIG_TBD_USE_SD_CARD
    if (chan == 0) {
        MacroTranslator::instance().soundProcessor = nullptr;
    }
#endif

    ESP_LOGD("SPManager", "Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    // create new plugin
    ctagSPAllocator::AllocationType aType = ctagSPAllocator::AllocationType::CH0;
    if(chan == 1) aType = ctagSPAllocator::AllocationType::CH1;
    if(model->IsStereo(id)) aType = ctagSPAllocator::AllocationType::STEREO;
    sp[chan] = ctagSoundProcessorFactory::Create(id, aType);
    if (sp[chan] == nullptr) {
        ESP_LOGE("SPManager", "Failed to create plugin %s — factory returned null!", id.c_str());
        xSemaphoreGive(processMutex);
        return;
    }
#if CONFIG_TBD_USE_SD_CARD
    if (chan == 0) {
        MacroTranslator::instance().soundProcessor = sp[chan];
    }
#endif
    model->SetActivePluginID(id, chan);
    sp[chan]->LoadPreset(model->GetActivePatchNum(chan));
    xSemaphoreGive(processMutex);

    ESP_LOGD("SPManager", "Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

}

TaskHandle_t SoundProcessorManager::audioTaskH;
TaskHandle_t SoundProcessorManager::ledTaskH;
DRAM_ATTR ctagSoundProcessor* SoundProcessorManager::sp[2] {nullptr, nullptr};
std::unique_ptr<SPManagerDataModel> SoundProcessorManager::model;
DRAM_ATTR SemaphoreHandle_t SoundProcessorManager::processMutex;
atomic<uint32_t> SoundProcessorManager::ledBlink;
atomic<uint32_t> SoundProcessorManager::ledStatus;
atomic<uint32_t> SoundProcessorManager::ch01Daisy;
atomic<uint32_t> SoundProcessorManager::toStereoCH0;
atomic<uint32_t> SoundProcessorManager::toStereoCH1;
atomic<uint32_t> SoundProcessorManager::runAudioTask;
atomic<uint32_t> SoundProcessorManager::ch0_outputSoftClip;
atomic<uint32_t> SoundProcessorManager::ch1_outputSoftClip;
atomic<uint32_t> SoundProcessorManager::parameterChangeCounter = 0;
atomic<uint32_t> SoundProcessorManager::macroChangeCounter = 0;
atomic<uint32_t> SoundProcessorManager::trackMachineChangeCounter = 0;
atomic<uint32_t> SoundProcessorManager::definitionChangeCounter = 0;
atomic<uint32_t> SoundProcessorManager::webuiChangeCounter = 0;
atomic<uint32_t> SoundProcessorManager::screenshotRequestCounter = 0;
atomic<uint8_t> SoundProcessorManager::injectedButton = 0;
atomic<uint8_t> SoundProcessorManager::injectedButtonEvent = 0;


static char freertosstats[2000] = { 0, };

static void debug_task(void *pvParameters) {
  while (true) {
    vTaskDelay(4000 / portTICK_PERIOD_MS);
    // vTaskGetRunTimeStats((char *)&freertosstats);
    // vTaskDelay(200 / portTICK_PERIOD_MS);
    // ESP_LOGI("SPManager", "FreeRTOS Stats:\n%s", freertosstats);

#if CONFIG_TBD_USE_RP2350
    ESP_LOGD("SPManager", "Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!, counters: tx-err=%ld queue-err=%ld parse-err=%ld success=%ld",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM),
             DRIVERS::rp2350_spi_stream::transferErrorCount,
             DRIVERS::rp2350_spi_stream::queueErrorCount,
             DRIVERS::rp2350_spi_stream::parseErrorCount,
             DRIVERS::rp2350_spi_stream::transferSuccessCount);
#else
    ESP_LOGD("SPManager", "Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
#endif

    // printf("Audio task cycles %d, micros %d, slow process() counter %d/%d, fbuf = [%1.3f, %1.3f...], tempo %ld, sentSynthMidi %d b, receivedUsbDeviceMidi %d b, %d new request counter errors\n", (int)diff, (int)diff2, (int)slowProcessCounter, (int)framecounter, fbuf[0], fbuf[BUF_SZ], pd.sequencer_tempo, (int)sentSynthMidiBytes, (int)receivedUsbDeviceMidiBytes, (int)requestCounterErrors);
    // requestCounterErrors = 0;
  }
}

void SoundProcessorManager::StartSoundProcessor() {
    ledBlink = 5;

    // Initialize storage overlay (factory/user layout, migrate from legacy if needed)
    CTAG::STORAGE::initOverlay();

    model = std::make_unique<SPManagerDataModel>();

    // prepare threads and mutex
    processMutex = xSemaphoreCreateMutex();
    if (processMutex == NULL) {
        ESP_LOGE("SPM", "Fatal couldn't create mutex!");
    }

    CTAG::DRIVERS::tusb::Init();
    // init control
    CTRL::Control::Init();
    // init codec
    DRIVERS::Codec::InitCodec();
    // generate internal data
    updateConfiguration();

#if CONFIG_TBD_USE_SD_CARD
    InitMacroSystem();
#endif

    // start network
    NET::Network::SetSSID(model->GetNetworkConfigurationData("ssid"));
    NET::Network::SetPWD(model->GetNetworkConfigurationData("pwd"));
    if(model->GetNetworkConfigurationData("mode").compare("ap") == 0) {
        NET::Network::SetIfType(NET::Network::IF_TYPE::IF_TYPE_AP);
    }else if(model->GetNetworkConfigurationData("mode").compare("sta") == 0){
        NET::Network::SetIfType(NET::Network::IF_TYPE::IF_TYPE_STA);
    }else if(model->GetNetworkConfigurationData("mode").compare("usbncm") == 0){
        // Wait for NCM interface to be ready before starting network
        CTAG::DRIVERS::tusb::WaitForNCMReady(5000);
        NET::Network::SetIfType(NET::Network::IF_TYPE::IF_TYPE_USBNCM);
    }else{
        ESP_LOGW("SPM", "Unknown network mode '%s', defaulting to usbncm", model->GetNetworkConfigurationData("mode").c_str());
        CTAG::DRIVERS::tusb::WaitForNCMReady(5000);
        NET::Network::SetIfType(NET::Network::IF_TYPE::IF_TYPE_USBNCM);
    }
    NET::Network::SetIP(model->GetNetworkConfigurationData("ip"));
    NET::Network::SetMDNSName(model->GetNetworkConfigurationData("mdns_name"));
    NET::Network::Up();
    REST::RestServer::StartRestServer();

    // Start NCM watchdog — auto-reconnects USB if macOS aborts the NCM link
    if (model->GetNetworkConfigurationData("mode").compare("usbncm") == 0 ||
        (model->GetNetworkConfigurationData("mode").compare("ap") != 0 &&
         model->GetNetworkConfigurationData("mode").compare("sta") != 0)) {
        CTAG::DRIVERS::tusb::StartNCMWatchdog();
    }
#if CONFIG_TBD_USE_RP2350
    SPIAPI::SpiAPI::StartSpiAPI();
#endif

    // Ableton Link
    CTAG::LINK::link::Init();

    // create led indicator thread
    xTaskCreatePinnedToCore(&SoundProcessorManager::led_task, "led_task", 4096, nullptr, tskIDLE_PRIORITY + 2,
                            &ledTaskH, 0);
    // create audio thread
    runAudioTask = 1;
    ESP_LOGI("SPManager", "Init: Max stack %d", uxTaskGetStackHighWaterMark(NULL));
    xTaskCreatePinnedToCore(&SoundProcessorManager::audio_task, "audio_task", 20000, nullptr, configMAX_PRIORITIES - 1, &audioTaskH, 1);
    xTaskCreatePinnedToCore(&debug_task, "debug_task", 2048, nullptr, tskIDLE_PRIORITY + 1, NULL, 0);
    ESP_LOGI("SPManager", "Init: task id %ld", audioTaskH);

    // Load last active processors from config, with fallback to Void/PicoSeqRack
    {
        string id0 = model->GetActiveProcessorID(0);
        string id1 = model->GetActiveProcessorID(1);
        ESP_LOGI("SPManager", "Loading ch0=%s, ch1=%s from config", id0.c_str(), id1.c_str());

        SetSoundProcessorChannel(0, id0);
        if (sp[0] == nullptr) {
            ESP_LOGW("SPManager", "ch0 plugin '%s' failed, falling back to PicoSeqRack", id0.c_str());
            SetSoundProcessorChannel(0, "PicoSeqRack");
        }
        if (sp[0] == nullptr) {
            ESP_LOGW("SPManager", "ch0 PicoSeqRack also failed, falling back to Void");
            SetSoundProcessorChannel(0, "Void");
        }

        SetSoundProcessorChannel(1, id1);
        if (sp[1] == nullptr && id1 != "Void") {
            ESP_LOGW("SPManager", "ch1 plugin '%s' failed, falling back to Void", id1.c_str());
            SetSoundProcessorChannel(1, "Void");
        }
    }

    ESP_LOGI("SPManager", "Init: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
}

void SoundProcessorManager::SetChannelParamValue(const int chan, const string &id, const string &key, const int val) {
    ledBlink = 3;
    sp[chan]->SetParamValue(id, key, val);
}

void SoundProcessorManager::ChannelSavePreset(const int chan, const string &name, const int number) {
    ledBlink = 3;
    if (sp[chan] == nullptr) return;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    sp[chan]->SavePreset(name, number);
    model->SetActivePatchNum(number, chan);
    xSemaphoreGive(processMutex);
}

void SoundProcessorManager::ChannelLoadPreset(const int chan, const int number) {
    ledBlink = 3;
    if (sp[chan] == nullptr) return;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    sp[chan]->LoadPreset(number);
    model->SetActivePatchNum(number, chan);
    xSemaphoreGive(processMutex);
}

string SoundProcessorManager::GetStringID(const int chan) {
    ledBlink = 3;
    return model->GetActiveProcessorID(chan);
}

void SoundProcessorManager::SetConfigurationFromJSON(const string &data) {
    ledBlink = 3;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    model->SetConfigurationFromJSON(data);
    updateConfiguration();
    xSemaphoreGive(processMutex);
}

void SoundProcessorManager::updateConfiguration() {
    ledBlink = 3;

    // ch01 daisy
    if (model->GetConfigurationData("ch01_daisy").compare("off") == 0) {
        ch01Daisy = 0;
    }else if(model->GetConfigurationData("ch01_daisy").compare("on") == 0){
        ch01Daisy = 1;
    }
    // mono to stereo channel cfg
    if (model->GetConfigurationData("ch0_toStereo").compare("off") == 0) {
        toStereoCH0 = 0;
    } else if (model->GetConfigurationData("ch0_toStereo").compare("on") == 0) {
        toStereoCH0 = 1;
    } else if (model->GetConfigurationData("ch0_toStereo").compare("mix") == 0) {
        toStereoCH0 = 2;
    }
    if (model->GetConfigurationData("ch1_toStereo").compare("off") == 0) {
        toStereoCH1 = 0;
    } else if (model->GetConfigurationData("ch1_toStereo").compare("on") == 0) {
        toStereoCH1 = 1;
    } else if (model->GetConfigurationData("ch1_toStereo").compare("mix") == 0) {
        toStereoCH1 = 2;
    }

    // soft clipping?
    if (model->GetConfigurationData("ch0_outputSoftClip").compare("off") == 0) {
        ch0_outputSoftClip = 0;
    } else if (model->GetConfigurationData("ch0_outputSoftClip").compare("on") == 0) {
        ch0_outputSoftClip = 1;
    }
    if (model->GetConfigurationData("ch1_outputSoftClip").compare("off") == 0) {
        ch1_outputSoftClip = 0;
    } else if (model->GetConfigurationData("ch1_outputSoftClip").compare("on") == 0) {
        ch1_outputSoftClip = 1;
    }

    // output levels of codec
    if(model->GetConfigurationData("ch0_codecLvlOut").compare("") != 0){
        if(model->GetConfigurationData("ch0_codecLvlOut").compare("") != 0){
            int lLevel = std::stoi(model->GetConfigurationData("ch0_codecLvlOut"));
            int rLevel = std::stoi(model->GetConfigurationData("ch1_codecLvlOut"));
            CONSTRAIN(rLevel, 0, 63)
            CONSTRAIN(lLevel, 0, 63)
            DRIVERS::Codec::SetOutputLevels(lLevel, rLevel);
        }
    }
}

void SoundProcessorManager::led_task(void *pvParams) {
    uint32_t r = 0, g = 0, b = 0;
    uint32_t data = 0;
    while (1) {
        data = ledStatus;
        r = data & 0x00FF0000;
        r >>= 16;
        g = data & 0x0000FF00;
        g >>= 8;
        b = data & 0x000000FF;
        if ((ledBlink % 2) != 1) {
            b = 255;
        }
        DRIVERS::LedRGB::SetLedRGB(r, g, b);
        if (ledBlink > 1 && ledBlink < 42) ledBlink--; // >= 42 led blink doesn't stop
        if (ledBlink == 42) ledBlink = 44;
        vTaskDelay(50 / portTICK_PERIOD_MS); // 50ms refresh rate for led
    }
}

void SoundProcessorManager::KillAudioTask() {
    Codec::SetOutputLevels(0, 0);
    // stop audio Task, delete plugins
    runAudioTask = 0;
    while (runAudioTask != 2); // wait for audio task to be dead
    if(nullptr!=sp[0]) delete sp[0];
    if(nullptr!=sp[1]) delete sp[1];
    sp[0] = nullptr;
    sp[1] = nullptr;
    vTaskDelete(ledTaskH);
    ledTaskH = NULL;
    vTaskDelay(100 / portTICK_PERIOD_MS);
    DRIVERS::LedRGB::SetLedRGB(255, 0, 255);
    ESP_LOGI("SPManager", "Audio Task Killed: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
}

void SoundProcessorManager::DisablePluginProcessing() {
    xSemaphoreTake(processMutex, portMAX_DELAY);
    ledBlink = 42;
}

void SoundProcessorManager::EnablePluginProcessing() {
    ledBlink = 5;
    xSemaphoreGive(processMutex);
}

void SoundProcessorManager::RefreshSampleRom() {
    ledBlink = 5;
    ctagSampleRom::RefreshDataStructure();
}

void SoundProcessorManager::SetTrackMachine(const int trackIndex, const string &synthID, float volumeMultiplier) {
    if (sp[0] != nullptr) {
        xSemaphoreTake(processMutex, portMAX_DELAY);
        sp[0]->setTrackMachine(trackIndex, synthID, volumeMultiplier);
        xSemaphoreGive(processMutex);
    }
}

void SoundProcessorManager::SetTrackVolumeMultiplier(const int trackIndex, float volumeMultiplier) {
    // No xSemaphoreTake here — caller (MacroSPManager::RefreshSingleMacro
    // → MacroTranslator::RefreshDefinitionById) already holds processMutex.
    // FreeRTOS mutex is non-recursive; a second take by the same task
    // deadlocks the audio thread on its next Process() block. The forwarded
    // call is a single aligned float write inside the rack — naturally
    // atomic on RISC-V/ARM, no torn reads on the audio side.
    if (sp[0] != nullptr) {
        sp[0]->setTrackVolumeMultiplier(trackIndex, volumeMultiplier);
    }
}

void SoundProcessorManager::SetTrackMute(const int trackIndex, bool muted) {
    // Forwards Pico-side user-mute state into the rack's per-channel mixer so
    // RackChannelMixer's enabled-check gates the sum output regardless of
    // LEVEL. Essential for the Input track (ch16, continuous passthrough with
    // no note-trigger suppression) and cuts synth tails instantly on 1-15.
    // setTrackMute is a no-op virtual on ctagSoundProcessor's base class, so
    // the dispatch is safe when the active processor isn't the PicoSeqRack.
    if (trackIndex < 0 || trackIndex >= 16) return;
    if (sp[0] != nullptr) {
        xSemaphoreTake(processMutex, portMAX_DELAY);
        sp[0]->setTrackMute((uint8_t)trackIndex, muted);
        xSemaphoreGive(processMutex);
    }
}

// ─── Audio health monitoring ─────────────────────────────────────

string SoundProcessorManager::GetAudioHealthJSON() {
    char buf[512];
    snprintf(buf, sizeof(buf),
        "{\"audioLockErrors\":%lu,"
        "\"slowProcessCount\":%lu,"
        "\"freeInternal\":%lu,"
        "\"largestInternal\":%lu,"
        "\"freeSPIRAM\":%lu,"
        "\"largestSPIRAM\":%lu}",
        (unsigned long)audioLockErrors,
        (unsigned long)slowProcessCounter,
        (unsigned long)heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
        (unsigned long)heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    return string(buf);
}

void SoundProcessorManager::ResetAudioHealthCounters() {
    audioLockErrors = 0;
    slowProcessCounter = 0;
}

// ─── Thread-safe JSON copy helpers ───────────────────────────────
// Take processMutex, call the underlying GetCStr* method which writes
// to a shared StringBuffer, copy the result into a SPIRAM-allocated
// buffer, release the mutex, and return the copy.
// Caller MUST free() the returned pointer.

static char *copyToSpiram(const char *src) {
    if (!src) return nullptr;
    size_t len = strlen(src);
    char *copy = (char *)heap_caps_malloc(len + 1, MALLOC_CAP_SPIRAM);
    if (copy) {
        memcpy(copy, src, len + 1);
    } else {
        ESP_LOGE("SPManager", "SPIRAM alloc failed for %u byte JSON copy", (unsigned)len);
    }
    return copy;
}

char *SoundProcessorManager::GetSafeJSONActivePluginParams(const int chan) {
    ledBlink = 1;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    const char *raw = sp[chan] ? sp[chan]->GetCStrJSONParamSpecs() : nullptr;
    char *copy = copyToSpiram(raw);
    xSemaphoreGive(processMutex);
    return copy;
}

char *SoundProcessorManager::GetSafeJSONGetPresets(const int chan) {
    ledBlink = 1;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    const char *raw = sp[chan] ? sp[chan]->GetCStrJSONPresets() : nullptr;
    char *copy = copyToSpiram(raw);
    xSemaphoreGive(processMutex);
    return copy;
}

char *SoundProcessorManager::GetSafeJSONAllPresetData(const int chan) {
    ledBlink = 1;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    const char *raw = sp[chan] ? sp[chan]->GetCStrJSONAllPresetData() : nullptr;
    char *copy = copyToSpiram(raw);
    xSemaphoreGive(processMutex);
    return copy;
}

char *SoundProcessorManager::GetSafeJSONConfiguration() {
    ledBlink = 1;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    const char *raw = model->GetCStrJSONConfiguration();
    char *copy = copyToSpiram(raw);
    xSemaphoreGive(processMutex);
    return copy;
}

char *SoundProcessorManager::GetSafeJSONSoundProcessors() {
    ledBlink = 1;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    const char *raw = model->GetCStrJSONSoundProcessors();
    char *copy = copyToSpiram(raw);
    xSemaphoreGive(processMutex);
    return copy;
}

char *SoundProcessorManager::GetSafeJSONSoundProcessorPresets(const string &id) {
    ledBlink = 1;
    xSemaphoreTake(processMutex, portMAX_DELAY);
    const char *raw = model->GetCStrJSONSoundProcessorPresets(id);
    char *copy = copyToSpiram(raw);
    xSemaphoreGive(processMutex);
    return copy;
}

std::string SoundProcessorManager::GetKitIndexJSON(){
    return ctagSampleRom::GetKitIndexJSON();
}

std::string SoundProcessorManager::GetActiveKitBankIndexJSON(){
    return ctagSampleRom::GetActiveKitBankIndexJSON();
}


