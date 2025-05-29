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

#include "soc/gpio_num.h"
#include "esp_log.h"

namespace CTAG::SPIAPI{
    TaskHandle_t SpiAPI::hTask;
    spi_slave_transaction_t SpiAPI::transaction;
    uint8_t *SpiAPI::send_buffer, *SpiAPI::receive_buffer;
    void SpiAPI::StartSpiAPI(){
        ESP_LOGI("SpiAPI", "Init()");
        //Configuration for the SPI bus
        spi_bus_config_t buscfg = {
            .mosi_io_num = GPIO_NUM_23,
            .miso_io_num = GPIO_NUM_22,
            .sclk_io_num = GPIO_NUM_21,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .data4_io_num = -1,
            .data5_io_num = -1,
            .data6_io_num = -1,
            .data7_io_num = -1,
            .max_transfer_sz = 2048,
            .flags = 0,
            .isr_cpu_id = ESP_INTR_CPU_AFFINITY_0,
            .intr_flags = 0
        };

        //Configuration for the SPI slave interface
        spi_slave_interface_config_t slvcfg = {
            .spics_io_num = GPIO_NUM_20,
            .flags = 0,
            .queue_size = 1,
            .mode = 3,
            .post_setup_cb = 0,
            .post_trans_cb = 0
        };

        send_buffer = (uint8_t*) spi_bus_dma_memory_alloc(SPI3_HOST, 2048, 0);
        send_buffer[0] = 0xCA; send_buffer[1] = 0xFE;
        receive_buffer = (uint8_t*) spi_bus_dma_memory_alloc(SPI3_HOST, 2048, 0);
        transaction.length = 2048 * 8;
        transaction.tx_buffer = send_buffer;
        transaction.rx_buffer = receive_buffer;

        auto ret = spi_slave_initialize(SPI3_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
        assert(ret == ESP_OK);

        xTaskCreatePinnedToCore(api_task, "SpiAPI", 4096*2, nullptr, 10, &hTask, 0);
    }
    static QueueHandle_t dbg_queue;
    static void dbg_task(void *pvParameters){
        while(1){
            uint8_t data = 0;
            xQueueReceive(dbg_queue, &data, portMAX_DELAY);
            ESP_LOGI("SpiAPI", "Received request type: %d", data);
        }
    }
    void SpiAPI::transmitCString(const RequestType reqType, const char* str){
        uint32_t len = strlen(str);
        // fields are: // 0xCA, 0xFE, request type, length (uint32_t), cstring
        uint8_t *requestTypeField = send_buffer + 2;
        *requestTypeField = static_cast<uint8_t>(reqType);
        uint32_t *lengthField = (uint32_t*)(send_buffer + 3);
        uint32_t bytes_to_send = 0;
        uint32_t bytes_sent = 0;
        while (len > 0){
            *lengthField = len;
            bytes_to_send = len > 2048 - 7 ? 2048 - 7 : len; // 7 bytes for header
            const char* ptr_cstring_section = str + bytes_sent;
            memcpy(send_buffer + 7, ptr_cstring_section, bytes_to_send);
            len -= bytes_to_send;
            bytes_sent += bytes_to_send;
            spi_slave_queue_trans(SPI3_HOST, &transaction, portMAX_DELAY);
            spi_slave_transaction_t *rcv_trans;
            spi_slave_get_trans_result(SPI3_HOST, &rcv_trans, portMAX_DELAY);
        }
    }

    void SpiAPI::api_task(void* pvParameters){
#include "IOCapabilities.hpp"
        dbg_queue = xQueueCreate(20, sizeof(uint8_t));
        xTaskCreatePinnedToCore(dbg_task, "SpiAPIDbg", 4096, nullptr, 5, &hTask, 0);
        while(1){
            spi_slave_queue_trans(SPI3_HOST, &transaction, portMAX_DELAY);
            spi_slave_transaction_t *rcv_trans;
            spi_slave_get_trans_result(SPI3_HOST, &rcv_trans, portMAX_DELAY);
            const uint8_t* rcv_data = (uint8_t*) rcv_trans->rx_buffer;

            // check integrity of transaction
            if (rcv_trans->trans_len != 2048 * 8) continue;
            if (rcv_data[0] != 0xCA || rcv_data[1] != 0xFE) continue;

            // parse request
            const RequestType requestType = static_cast<RequestType>(rcv_data[2]);
            xQueueSend(dbg_queue, &requestType, 0);
            const char* cstring = nullptr;

            // params
            const int uint8_param_0 = rcv_data[3]; // first request parameter, e.g. channel, favorite number, ...
            const int uint8_param_1 = rcv_data[4];; // second request parameter, e.g. preset number, ...
            const int int32_param_2 = *(int32_t*)&rcv_data[5]; // third request parameter, e.g. value, ...
            const char* string_param_3 = (char*)&rcv_data[9]; // fourth request parameter, e.g. plugin name, parameter name, ...

            int channel = uint8_param_0; // channel is the first parameter
            if (channel < 0 || channel > 1) {
                channel = 0; // default to channel 0 if out of bounds
            }
            const uint8_t preset_number = uint8_param_1; // bounds check in subsequent class
            const uint8_t favorite_number = uint8_param_0; // bounds check in subsequent class
            const int32_t param_value = int32_param_2; // value is the third parameter, e.g. for setting a parameter value
            const std::string string_parameter {string_param_3};


            // handle request
            switch (requestType) {
            case RequestType::GetPlugins:
                cstring = AUDIO::SoundProcessorManager::GetCStrJSONSoundProcessors();
                transmitCString(requestType, cstring);
                break;
            case RequestType::GetActivePlugin:
                cstring = AUDIO::SoundProcessorManager::GetStringID(channel).c_str();
                transmitCString(requestType, cstring);
                break;
            case RequestType::GetActivePluginParams:
                cstring = AUDIO::SoundProcessorManager::GetCStrJSONActivePluginParams(channel);
                transmitCString(requestType, cstring);
                break;
            case RequestType::SetActivePlugin:
                AUDIO::SoundProcessorManager::SetSoundProcessorChannel(channel, string_parameter);
                FAV::Favorites::DeactivateFavorite();
                break;
            case RequestType::SetPluginParam:
                AUDIO::SoundProcessorManager::SetChannelParamValue(channel, string_parameter, "current", param_value);
                break;
            case RequestType::SetPluginParamCV:
                AUDIO::SoundProcessorManager::SetChannelParamValue(channel, string_parameter, "cv", param_value);
                break;
            case RequestType::SetPluginParamTRIG:
                AUDIO::SoundProcessorManager::SetChannelParamValue(channel, string_parameter, "trig", param_value);
                break;
            case RequestType::GetPresets:
                cstring = AUDIO::SoundProcessorManager::GetCStrJSONGetPresets(channel);
                transmitCString(requestType, cstring);
                break;
            case RequestType::GetPresetData:
                cstring = AUDIO::SoundProcessorManager::GetCStrJSONSoundProcessorPresets(string_parameter);
                transmitCString(requestType, cstring);
                break;
            case RequestType::SetPresetData:
                //AUDIO::SoundProcessorManager::SetCStrJSONSoundProcessorPreset(pluginID, content);
                break;
            case RequestType::LoadPreset:
                AUDIO::SoundProcessorManager::ChannelLoadPreset(channel, preset_number);
                FAV::Favorites::DeactivateFavorite();
                break;
            case RequestType::SavePreset:
                AUDIO::SoundProcessorManager::ChannelSavePreset(channel, string_parameter, preset_number);
                break;
            case RequestType::GetAllFavorites:
                cstring = FAV::Favorites::GetAllFavorites().c_str();
                transmitCString(requestType, cstring);
                break;
            case RequestType::SaveFavorite:
                //FAV::Favorites::StoreFavorite(preset_number, string_parameter);
                break;
            case RequestType::LoadFavorite:
                FAV::Favorites::ActivateFavorite(favorite_number);
                break;
            case RequestType::GetConfiguration:
                cstring = AUDIO::SoundProcessorManager::GetCStrJSONConfiguration();
                transmitCString(requestType, cstring);
                break;
            case RequestType::SetConfiguration:
                //AUDIO::SoundProcessorManager::SetConfigurationFromJSON(string(content));
                break;
            case RequestType::GetIOCapabilities:
                transmitCString(requestType, s.c_str());
                break;
            case RequestType::Reboot:
                ESP_LOGI("SpiAPI", "Rebooting device!");
                esp_restart();
                break;
            }

        }
    }
}