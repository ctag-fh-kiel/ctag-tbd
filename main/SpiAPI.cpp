/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2025 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "SpiAPI.hpp"
#include "SPManager.hpp"
#include "Favorites.hpp"
#include "helpers/ctagSampleRom.hpp"

#include "soc/gpio_num.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_image_format.h"
#include "driver/gpio.h"

#include "link.hpp"

#define RCV_HOST    SPI3_HOST // SPI2 connects to rp2350 spi1
#define GPIO_HANDSHAKE GPIO_NUM_50 // GPIO50 is used for handshake line, P4_PICO_02 which is GPIO18 on rp2350
#define GPIO_MOSI GPIO_NUM_23
#define GPIO_MISO GPIO_NUM_22
#define GPIO_SCLK GPIO_NUM_21
#define GPIO_CS GPIO_NUM_20


static void boot_into_slot(int slot) { // slot 0 or 1
    esp_partition_subtype_t st = (slot == 0)
        ? ESP_PARTITION_SUBTYPE_APP_OTA_0
        : ESP_PARTITION_SUBTYPE_APP_OTA_1;
    const esp_partition_t *p = esp_partition_find_first(ESP_PARTITION_TYPE_APP, st, NULL);
    if (!p) return;
    printf("Try to boot into %s\n", p->label);
    if (esp_ota_set_boot_partition(p) == ESP_OK) esp_restart();
    printf("Boot into %s\n not successful", p->label);
}

static const char *esp_get_current_ota_label(void) {
    static char label[8] = {0};

    const esp_partition_t *running = esp_ota_get_running_partition();

    if (running->type == ESP_PARTITION_TYPE_APP &&
        running->subtype >= ESP_PARTITION_SUBTYPE_APP_OTA_MIN &&
        running->subtype <= ESP_PARTITION_SUBTYPE_APP_OTA_MAX) {

        int ota_num = running->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN;
        snprintf(label, sizeof(label), "ota%d", ota_num);
        return label;

        } else {
            return "factory";   // or return NULL if you prefer
        }
}

static int count_bootable_ota_partitions(void) {
    int count = 0;
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);

    while (it != NULL) {
        const esp_partition_t *p = esp_partition_get(it);
        if (p->subtype >= ESP_PARTITION_SUBTYPE_APP_OTA_MIN &&
            p->subtype <= ESP_PARTITION_SUBTYPE_APP_OTA_MAX) {
            count++;
            }
        it = esp_partition_next(it);
    }

    esp_partition_iterator_release(it);
    return count;
}


// Called after a transaction is queued and ready for pickup by master. We use this to set the handshake line high.
IRAM_ATTR static void spi_post_setup_cb(spi_slave_transaction_t *trans){
    gpio_set_level(GPIO_HANDSHAKE, 1);
}

// Called after transaction is sent/received. We use this to set the handshake line low.
IRAM_ATTR static void spi_post_trans_cb(spi_slave_transaction_t *trans){
    gpio_set_level(GPIO_HANDSHAKE, 0);
}

namespace CTAG::SPIAPI{
    TaskHandle_t SpiAPI::hTask;
    spi_slave_transaction_t SpiAPI::transaction;
    uint8_t *SpiAPI::send_buffer, *SpiAPI::receive_buffer;

    void SpiAPI::StartSpiAPI(){
        ESP_LOGI("SpiAPI", "Init()");
        //Configuration for the SPI bus
        spi_bus_config_t buscfg = {
            .mosi_io_num = GPIO_MOSI,
            .miso_io_num = GPIO_MISO,
            .sclk_io_num = GPIO_SCLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .data4_io_num = -1,
            .data5_io_num = -1,
            .data6_io_num = -1,
            .data7_io_num = -1,
            .data_io_default_level = false,
            .max_transfer_sz = 2048,
            .flags = 0,
            .isr_cpu_id = ESP_INTR_CPU_AFFINITY_0,
            .intr_flags = ESP_INTR_FLAG_LEVEL3|ESP_INTR_FLAG_IRAM
        };

        //Configuration for the SPI slave interface
        spi_slave_interface_config_t slvcfg = {
            .spics_io_num = GPIO_CS,
            .flags = 0,
            .queue_size = 1,
            .mode = 3,
            .post_setup_cb = spi_post_setup_cb,
            .post_trans_cb = spi_post_trans_cb
        };

        //Configuration for the handshake line
        gpio_config_t io_conf = {
            .pin_bit_mask = BIT64(GPIO_HANDSHAKE),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
            .hys_ctrl_mode = GPIO_HYS_SOFT_DISABLE
        };

        //Configure handshake line as output
        gpio_config(&io_conf);
        gpio_set_level(GPIO_HANDSHAKE, 0);

        //Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
        gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
        gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
        gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);

        send_buffer = (uint8_t*)spi_bus_dma_memory_alloc(RCV_HOST, 2048, 0);
        send_buffer[0] = 0xCA;
        send_buffer[1] = 0xFE;
        receive_buffer = (uint8_t*)spi_bus_dma_memory_alloc(RCV_HOST, 2048, 0);
        transaction.length = 2048 * 8;
        transaction.tx_buffer = send_buffer;
        transaction.rx_buffer = receive_buffer;

        auto ret = spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
        assert(ret == ESP_OK);

        xTaskCreatePinnedToCore(api_task, "SpiAPI", 4096 * 2, nullptr, 10, &hTask, 0);
    }

    /*
    static QueueHandle_t dbg_queue;

    static void dbg_task(void* pvParameters){
        while (1){
            uint8_t data = 0;
            xQueueReceive(dbg_queue, &data, portMAX_DELAY);
            ESP_LOGI("SpiAPI", "Received request type: %d", data);
        }
    }
    */

    bool SpiAPI::receiveString(const RequestType reqType, std::string& str){
        send_buffer[2] = (uint8_t) reqType;
        spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY);

        // fingerprint check
        if (receive_buffer[0] != 0xCA || receive_buffer[1] != 0xFE){
            str = "FP wrong: " + std::to_string(receive_buffer[0]) + " " + std::to_string(receive_buffer[1]);
            return false;
        }

        // check request type acknowledgment
        const uint8_t requestType = receive_buffer[2];
        if (requestType != (uint8_t)reqType){
            str = "ACK wrong: " + std::to_string(requestType);
            return false;
        }

        // read the response
        const uint32_t* resLength = (uint32_t*)&receive_buffer[3];
        const uint32_t totalResponseLength = *resLength;
        str.reserve(*resLength); // reserve space for the JSON string
        uint32_t bytes_received = *resLength > 2048 - 7 ? 2048 - 7 : *resLength; // 7 bytes for fingerprint and length
        uint32_t bytes_to_be_received = *resLength - bytes_received;
        str.append((char*)&receive_buffer[7], bytes_received); // skip the first 7 bytes (fingerprint and length)

        while (bytes_to_be_received > 0){
            spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY);


            // fingerprint check
            if (receive_buffer[0] != 0xCA || receive_buffer[1] != 0xFE){
                str = "FP wrong: " + std::to_string(receive_buffer[0]) + " " + std::to_string(receive_buffer[1]);
                return false;
            }

            // check request type acknowledgment
            const uint8_t requestType = receive_buffer[2];
            if (requestType != (uint8_t)reqType){
                str = "ACK wrong: " + std::to_string(requestType);
                return false;
            }

            // append the received data to the json string
            bytes_received = *resLength > 2048 - 7 ? 2048 - 7 : *resLength; // 7 bytes for fingerprint and length
            str.append((char*)&receive_buffer[7], bytes_received);
            bytes_to_be_received -= bytes_received;
            /*
            ESP_LOGI("spiapi", "resLength %li, totalResponseLength %li, bytes_received %li, bytes_to_be_received %li",
                 *resLength, totalResponseLength, bytes_received, bytes_to_be_received);
            */
        }
        if (str.size() != totalResponseLength){
            str = "LEN error: " + std::to_string(totalResponseLength) + ", got " + std::to_string(str.size());
            return false;
        }
        return true;
    }

    bool SpiAPI::transmitCString(const RequestType reqType, const char* str){
        uint32_t len = strlen(str);
        // fields are: // 0xCA, 0xFE, request type, length (uint32_t), cstring
        uint8_t* requestTypeField = send_buffer + 2;
        *requestTypeField = static_cast<uint8_t>(reqType);
        uint32_t* lengthField = (uint32_t*)(send_buffer + 3);
        uint32_t bytes_to_send = 0;
        uint32_t bytes_sent = 0;
        while (len > 0){
            *lengthField = len;
            bytes_to_send = len > 2048 - 7 ? 2048 - 7 : len; // 7 bytes for header
            const char* ptr_cstring_section = str + bytes_sent;
            memcpy(send_buffer + 7, ptr_cstring_section, bytes_to_send);
            len -= bytes_to_send;
            bytes_sent += bytes_to_send;
            spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY);
            // fingerprint check
            if (receive_buffer[0] != 0xCA || receive_buffer[1] != 0xFE){
                return false;
            }
            // check request type acknowledgment
            const uint8_t requestType = receive_buffer[2];
            if (requestType != (uint8_t)reqType){
                return false;
            }
        }
        return true;
    }

    void SpiAPI::api_task(void* pvParameters){
#include "IOCapabilities.hpp"
        //dbg_queue = xQueueCreate(20, sizeof(uint8_t));
        //xTaskCreatePinnedToCore(dbg_task, "SpiAPIDbg", 4096, nullptr, 5, &hTask, 0);
        bool result = true;
        while (1){
            if (result) spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY); // recycle last transaction, if previous was not successful, sometimes data gets stuck
            const uint8_t* rcv_data = (uint8_t*)transaction.rx_buffer;

            // check integrity of transaction
            if (transaction.trans_len != 2048 * 8){
                ESP_LOGE("spiapi", "Received transaction length %d, expected 2048 * 8", transaction.trans_len);
                result = true;
                continue;
            }
            if (rcv_data[0] != 0xCA || rcv_data[1] != 0xFE){
                ESP_LOGE("spiapi", "Received data %x %x, expected 0xCA 0xFE", rcv_data[0], rcv_data[1]);
                result = true;
                continue;
            }

            // parse request
            const RequestType requestType = static_cast<RequestType>(rcv_data[2]);
            //xQueueSend(dbg_queue, &requestType, 0);
            const char* cstring = nullptr;

            // params
            const int uint8_param_0 = rcv_data[3]; // first request parameter, e.g. channel, favorite number, ...
            const int uint8_param_1 = rcv_data[4];; // second request parameter, e.g. preset number, ...
            const int int32_param_2 = *(int32_t*)&rcv_data[5]; // third request parameter, e.g. value, ...
            const char* string_param_3 = (char*)&rcv_data[9];
            const float float_param_0 = *(float*)&rcv_data[3];
            // fourth request parameter, e.g. plugin name, parameter name, ...

            int channel = uint8_param_0; // channel is the first parameter
            if (channel < 0 || channel > 1){
                channel = 0; // default to channel 0 if out of bounds
            }
            const uint8_t preset_number = uint8_param_1; // bounds check in subsequent class
            const uint8_t favorite_number = uint8_param_0; // bounds check in subsequent class
            const uint8_t bank_number = uint8_param_0; // bounds check in subsequent class
            const int32_t param_value = int32_param_2;
            // value is the third parameter, e.g. for setting a parameter value
            std::string string_parameter{string_param_3};

            // handle request
            switch (requestType){
            case RequestType::GetPlugins:
                cstring = AUDIO::SoundProcessorManager::GetCStrJSONSoundProcessors();
                result = transmitCString(requestType, cstring);
                break;
            case RequestType::GetActivePlugin:
                cstring = AUDIO::SoundProcessorManager::GetStringID(channel).c_str();
                result = transmitCString(requestType, cstring);
                break;
            case RequestType::GetActivePluginParams:
                cstring = AUDIO::SoundProcessorManager::GetCStrJSONActivePluginParams(channel);
                result = transmitCString(requestType, cstring);
                break;
            case RequestType::SetActivePlugin:
                AUDIO::SoundProcessorManager::SetSoundProcessorChannel(channel, string_parameter);
                FAV::Favorites::DeactivateFavorite();
                result = true;
                break;
            case RequestType::SetPluginParam:
                AUDIO::SoundProcessorManager::SetChannelParamValue(channel, string_parameter, "current", param_value);
                result = true;
                break;
            case RequestType::SetPluginParamCV:
                AUDIO::SoundProcessorManager::SetChannelParamValue(channel, string_parameter, "cv", param_value);
                result = true;
                break;
            case RequestType::SetPluginParamTRIG:
                AUDIO::SoundProcessorManager::SetChannelParamValue(channel, string_parameter, "trig", param_value);
                result = true;
                break;
            case RequestType::GetPresets:
                cstring = AUDIO::SoundProcessorManager::GetCStrJSONGetPresets(channel);
                result = transmitCString(requestType, cstring);
                break;
            case RequestType::GetPresetData:
                cstring = AUDIO::SoundProcessorManager::GetCStrJSONSoundProcessorPresets(string_parameter);
                result = transmitCString(requestType, cstring);
                break;
            case RequestType::SetPresetData:{
                std::string pluginID = string_parameter;
                string_parameter.clear();
                result = receiveString(RequestType::SetPresetData, string_parameter);
                //ESP_LOGI("SpiAPI", "Result %d, Saving preset %s %s", result, pluginID.c_str(), string_parameter.c_str());
                if (true == result) AUDIO::SoundProcessorManager::SetCStrJSONSoundProcessorPreset(pluginID.c_str(), string_parameter.c_str());
            }
                break;
            case RequestType::SetPluginParamsJSON:{
                AUDIO::SoundProcessorManager::SetChannelParamsCstrJSON(channel, string_parameter.c_str());
            }
                break;
            case RequestType::LoadPreset:
                AUDIO::SoundProcessorManager::ChannelLoadPreset(channel, preset_number);
                FAV::Favorites::DeactivateFavorite();
                result = true;
                break;
            case RequestType::SavePreset:
                AUDIO::SoundProcessorManager::ChannelSavePreset(channel, string_parameter, preset_number);
                result = true;
                break;
            case RequestType::GetAllFavorites:
                cstring = FAV::Favorites::GetAllFavorites().c_str();
                result = transmitCString(requestType, cstring);
                break;
            case RequestType::SaveFavorite:
                string_parameter.clear();
                result = receiveString(RequestType::SaveFavorite, string_parameter);
                //ESP_LOGI("SpiAPI", "Result %d, Saving favorite# %d as %s", result, favorite_number, string_parameter.c_str());
                if (true == result) FAV::Favorites::StoreFavorite(preset_number, string_parameter);
                break;
            case RequestType::LoadFavorite:
                FAV::Favorites::ActivateFavorite(favorite_number);
                result = true;
                break;
            case RequestType::GetConfiguration:
                cstring = AUDIO::SoundProcessorManager::GetCStrJSONConfiguration();
                result = transmitCString(requestType, cstring);
                break;
            case RequestType::SetConfiguration:
                string_parameter.clear();
                result = receiveString(RequestType::SetConfiguration, string_parameter);
                if (true == result) AUDIO::SoundProcessorManager::SetConfigurationFromJSON(string_parameter);
                //ESP_LOGI("SpiAPI", "Result %d, Saving config %s", result, string_parameter.c_str());
                break;
            case RequestType::GetIOCapabilities:
                result = transmitCString(requestType, s.c_str());
                break;
            case RequestType::Reboot:
                ESP_LOGI("SpiAPI", "Rebooting device!");
                esp_restart();
                break;
            case RequestType::RebootToOTA1:
                ESP_LOGI("SpiAPI", "Rebooting device to OTA1!");
                CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
                boot_into_slot(1);
                break;
            case RequestType::GetSampleRomDescriptor:
                ESP_LOGI("SpiAPI", "GetSampleRomDescriptor");
                {
                    std::string desc = HELPERS::ctagSampleRom::GetSampleRomDescriptorJSON();
                    result = transmitCString(requestType, desc.c_str());
                }
                break;
            case RequestType::SetActiveWaveTableBank:
                ESP_LOGI("SpiAPI", "Setting active wavetable bank to %d", bank_number);
                CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
                HELPERS::ctagSampleRom::SetActiveWaveTableBank(uint8_param_0);
                HELPERS::ctagSampleRom::RefreshDataStructure();
                CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
                break;
            case RequestType::SetActiveSampleRomBank:
                ESP_LOGI("SpiAPI", "Setting active sample bank to %d", bank_number);
                CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
                HELPERS::ctagSampleRom::SetActiveSampleBank(uint8_param_0);
                HELPERS::ctagSampleRom::RefreshDataStructure();
                CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
                break;
            case RequestType::GetFirmwareInfo:
                ESP_LOGI("SpiAPI", "GetFirmwareInfo");
                {
                    std::string info("{\"HWV\":\"" + TBD_HW_VERSION + "\",\"FWV\":\"" + TBD_FW_VERSION + "\",\"OTA\":\"" + std::string(esp_get_current_ota_label()) + "\"}");
                    result = transmitCString(requestType, info.c_str());
                }
                break;
            case RequestType::SetAbletonLinkTempo:
                {
                    ESP_LOGI("SpiAPI", "Set Ableton Link bpm: %3.2f", float_param_0);
                    CTAG::LINK::link::SetLinkTempo(float_param_0);
                }
                break;
            case RequestType::SetAbletonLinkStartStop:
                {
                    ESP_LOGI("SpiAPI", "Set Ableton Link Start/Stop: %d", uint8_param_0);
                    CTAG::LINK::link::SetLinkStartStop(uint8_param_0 != 0);
                }
                break;
            case RequestType::RebootToOTAX:
                {
                int num_ota = count_bootable_ota_partitions();
                if (uint8_param_0 >= num_ota){
                    ESP_LOGE("SpiAPI", "Requested OTA %d but only %d OTAs available!", uint8_param_0, num_ota);
                    break;
                }
                boot_into_slot(uint8_param_0);
                ESP_LOGI("SpiAPI", "Rebooting device to OTA %d!", uint8_param_0);
                CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
                break;
                }
            }
        }
    }
}
