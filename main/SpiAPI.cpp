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

#include "sdkconfig.h"
#if CONFIG_TBD_USE_RP2350

#include "SpiAPI.hpp"
#include "SPManager.hpp"
#include "Favorites.hpp"
#include "helpers/ctagSampleRom.hpp"

#include "soc/gpio_num.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_image_format.h"
#include "esp_rom_crc.h"
#include "driver/gpio.h"

#include "link.hpp"

#define MAX(x, y) ((x)>(y)) ? (x) : (y)

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
    std::string SpiAPI::rp2350AppId;   // empty = unknown/legacy
    bool SpiAPI::rp2350PluginLock = false;
    bool SpiAPI::rp2350RedirectSamples = false;
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


    bool SpiAPI::handle_send_file(){
        // Step 1: Notify slave of incoming file transfer
        // one init package looks as follows:
        // 0xCA, 0xFE: Watermark Byte 0, 1
        // request type: Byte 2
        // file length (uint32_t): Byte 3-6
        // total number of chunks (uint32_t): Byte 7-10
        // file name (cstring): Byte 11-n
        // n is 2048 - 11 = 2037 bytes max for file name

        const uint32_t file_size = *(uint32_t*)&receive_buffer[3];
        const uint32_t total_chunks = *(uint32_t*)&receive_buffer[7];
        const char* fn = (char*)&receive_buffer[11];
        std::string filename_incl_path{fn};

        // Construct full path with /sdcard prefix
        std::string full_path = "/sdcard/" + filename_incl_path;
        ESP_LOGI("SpiAPI", "SendFile: Total file size = %lu bytes", file_size);
        ESP_LOGI("SpiAPI", "SendFile: Total chunks = %lu", total_chunks);
        ESP_LOGI("SpiAPI", "SendFile: Receiving file %s", full_path.c_str());

        // step 1: acknowledge command receipt
        // Send command ACK with file size
        send_buffer[2] = (uint8_t)RequestType::SendFile;
        uint32_t* fileSizeField = (uint32_t*)&send_buffer[3];
        *fileSizeField = file_size;
        spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY);

        // Step 2: receive file data in chunks
        // one sender data package looks as follows:
        // 0xCA, 0xFE: Watermark Byte 0, 1
        // request type: Byte 2
        // chunk number (uint32_t): Byte 3-6
        // chunk data size (uint32_t): Byte 7-10
        // chunk data crc32le (uint32_t): Byte 11-14
        // chunk data: Byte 15-n
        // n is 2048 - 15 = 2033 bytes max for chunk data
        // slave responds in subsequent frame with following acknowledgement
        // 0xCA, 0xFE: Watermark Byte 0, 1
        // request type: Byte 2
        // chunk number (uint32_t): Byte 3-6
        // chunk status (uint8_t): Byte 7 (0 = OK, 1 = CRC error, 2 = other error)
        // on error the sender must restart sending from previous chunk
        // this should only occur on CRC errors, other errors are fatal
        // the slave response is delayed by one transmission
        // except for the first chunk, where general errors are reported immediately
        // retrying should occur only on CRC errors maximum 3 times per chunk
        // we are slave
        ESP_LOGI("SpiAPI", "SendFile: Starting chunk reception...");
        const uint16_t* watermark_field_sender = (uint16_t*)&receive_buffer[0];
        const uint8_t* request_type_field_sender = &receive_buffer[2];
        const uint32_t* chunk_number_field_sender = (uint32_t*)&receive_buffer[3];
        const uint32_t* chunk_data_size_field_sender = (uint32_t*)&receive_buffer[7];
        const uint32_t* chunk_data_crc32le_field_sender = (uint32_t*)&receive_buffer[11];
        const uint8_t* chunk_data_field_sender = &receive_buffer[15];
        uint32_t chunkNumber = 0;
        uint8_t* request_type_field_receiver = &send_buffer[2];
        uint32_t* chunk_number_field_receiver = (uint32_t*)&send_buffer[3];
        uint8_t* chunk_status_field_receiver = &send_buffer[7];

        FILE* f = fopen(full_path.c_str(), "wb");
        if (f == nullptr){
            ESP_LOGE("SpiAPI", "SendFile: Could not open file for writing");
            // report fatal error to master
            *request_type_field_receiver = (uint8_t)RequestType::SendFile;
            *chunk_number_field_receiver = 0;
            *chunk_status_field_receiver = 2; // fatal error
            spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY);
            return false;
        }

        // receive chunks
        *request_type_field_receiver = (uint8_t)RequestType::SendFile;
        *chunk_number_field_receiver = 0;
        *chunk_status_field_receiver = 0; // go for chunk transfer no issues
        while (chunkNumber < total_chunks){
            // wait for next chunk
            spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY);

            // check watermark
            if (*watermark_field_sender != 0xFECA){
                ESP_LOGE("SpiAPI", "SendFile: Chunk %lu: Wrong watermark", chunkNumber);
                // report fatal error to master
                *request_type_field_receiver = (uint8_t)RequestType::SendFile;
                *chunk_number_field_receiver = chunkNumber;
                *chunk_status_field_receiver = 2; // fatal error
                spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY);
                fclose(f);
                goto error;
            }

            // check request type
            if (*request_type_field_sender != (uint8_t)RequestType::SendFile){
                ESP_LOGE("SpiAPI", "SendFile: Chunk %lu: Wrong request type", chunkNumber);
                // report fatal error to master
                *request_type_field_receiver = (uint8_t)RequestType::SendFile;
                *chunk_number_field_receiver = chunkNumber;
                *chunk_status_field_receiver = 2; // fatal error
                spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY);
                fclose(f);
                goto error;
            }

            // check chunk number
            if (*chunk_number_field_sender != chunkNumber){
                ESP_LOGE("SpiAPI", "SendFile: Chunk %lu: Wrong chunk number", chunkNumber);
                // report fatal error to master
                *request_type_field_receiver = (uint8_t)RequestType::SendFile;
                *chunk_number_field_receiver = chunkNumber;
                *chunk_status_field_receiver = 2; // fatal error
                spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY);
                fclose(f);
                goto error;
            }

            // calculate CRC32 of received chunk data and compare
            uint32_t calculated_crc = esp_rom_crc32_le(0, chunk_data_field_sender, *chunk_data_size_field_sender);
            if (calculated_crc != *chunk_data_crc32le_field_sender){
                ESP_LOGE("SpiAPI", "SendFile: Chunk %lu: CRC mismatch, calculated %08X, received %08X",
                         chunkNumber, calculated_crc, *chunk_data_crc32le_field_sender);
                // report CRC error to master
                *request_type_field_receiver = (uint8_t)RequestType::SendFile;
                *chunk_number_field_receiver = chunkNumber;
                *chunk_status_field_receiver = 1; // CRC error
                spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY);
                // retry sending this chunk
                continue;
            }
            // write chunk data to file BEFORE sending ACK
            size_t written = fwrite(chunk_data_field_sender, 1, *chunk_data_size_field_sender, f);
            // report result to master AFTER write completes
            *request_type_field_receiver = (uint8_t)RequestType::SendFile;
            *chunk_number_field_receiver = chunkNumber;
            if (written != *chunk_data_size_field_sender){
                ESP_LOGE("SpiAPI", "SendFile: Chunk %lu: File write error", chunkNumber);
                // report fatal error to master
                *chunk_status_field_receiver = 2; // fatal error
                spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY);
                fclose(f);
                goto error;
            }
            // report successful receipt to master only after successful write
            *chunk_status_field_receiver = 0; // OK
            chunkNumber++;
            // log every 50 chunks
            if (chunkNumber % 50 == 0){
                ESP_LOGI("SpiAPI", "Received chunk %lu / %lu success!", chunkNumber, total_chunks);
            }
        }

        // Send one more transaction to deliver the ACK for the last chunk
        spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY);

        fclose(f);
        ESP_LOGI("SpiAPI", "SendFile: File transfer completed successfully");

        return true;
    error:
        // delete incomplete file
        ESP_LOGE("SpiAPI", "Deleting incomplete file %s", full_path.c_str());
        remove(full_path.c_str());
        return false;
    }


    void SpiAPI::api_task(void *pvParameters){
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
            {
                // Ignore Reboot commands during the first 15s after boot.
                // The RP2350 unconditionally sends spi_api.Reboot() in setup(),
                // which hits us on cold boot when the SPI API initializes before
                // the RP2350's 1s sleep_ms expires. Intentional user-triggered
                // reboots arrive well after the system is stable (~28s boot).
                int64_t uptime_ms = esp_timer_get_time() / 1000;
                if (uptime_ms < 15000) {
                    ESP_LOGW("SpiAPI", "Ignoring Reboot command during boot grace period (%lld ms uptime)", uptime_ms);
                    // Still clear app state — RP2350 is (re)booting and will re-announce
                    rp2350AppId.clear();
                    rp2350PluginLock = false;
                    rp2350RedirectSamples = false;
                    break;
                }
                rp2350AppId.clear();
                rp2350PluginLock = false;
                rp2350RedirectSamples = false;
                ESP_LOGI("SpiAPI", "Rebooting device!");
                esp_restart();
                break;
            }
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

            case RequestType::SetActiveSampleKit:
                ESP_LOGI("SpiAPI", "Setting active sample bank to #%d", uint8_param_0);
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
                ESP_LOGI("SpiAPI", "Rebooting device to OTA %d!", uint8_param_0);
                CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
                boot_into_slot(uint8_param_0); // calls esp_restart() — does not return
                break;
                }
            case RequestType::SendFile:
                result = handle_send_file();
                break;
            case RequestType::GetSampleFileCount:
                {
                    // CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
                    uint8_t number = HELPERS::ctagSampleRom::GetNumberSlices2();
                    ESP_LOGI("SpiAPI", "Getting sample file count (%d)", number);
                    HELPERS::ctagSampleRom srom;
                    int firstnonwt = srom.GetFirstNonWaveTableSlice();
                    uint32_t total = srom.GetNumberSlices() - firstnonwt;
                    // CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
                    char info[100] = { 0, };
                    sprintf(info, "{\"total\":%ld}", total);
                    result = transmitCString(requestType, info);
                }
                break;
            case RequestType::GetSampleFileInfo:
                {
                    int16_t file_index = uint8_param_1 * 256 + uint8_param_0;
                    ESP_LOGI("SpiAPI", "Getting sample file %d info", file_index);
                    HELPERS::ctagSampleRom srom;
                    int firstnonwt = srom.GetFirstNonWaveTableSlice();
                    uint32_t total = srom.GetNumberSlices() - firstnonwt;
                    int16_t slice = firstnonwt + file_index;
                    uint32_t size = srom.GetSliceSize(slice);
                    std::string filename = srom.GetFilenameForSampleSlice(file_index);
                    char info[100] = { 0, };
                    sprintf(info, "{\"index\":\"%d\",\"total\":%ld,\"slice_index\":%d,\"size\":%ld,\"filename\":\"%s\"}",
                        file_index, total, slice, size, filename.c_str());
                    result = transmitCString(requestType, info);
                }
                break;
            case RequestType::GetSampleFileWaveformPreview:
                {
                    int16_t file_index = uint8_param_1 * 256 + uint8_param_0;
                    HELPERS::ctagSampleRom srom;
                    int16_t slice = srom.GetFirstNonWaveTableSlice() + file_index;
                    uint32_t offset = srom.GetSliceOffset(slice);
                    uint32_t size = srom.GetSliceSize(slice);
                    ESP_LOGI("SpiAPI", "Getting sample file %d waveform preview, slice %d, size %ld, offset %ld",
                        file_index, slice, size, offset);
                    char sampledata[520] = { 0, };
                    int16_t slicedata[100] = { 0, };
                    memset(sampledata, 0, sizeof(sampledata));
                    for(int k=0; k<256; k++) {
                        int sliceoffset = (k * floor(size/2) / 256) * 2;
                        srom.ReadSlice((int16_t *)&slicedata, slice, sliceoffset, 20);
                        int16_t amp = 0;
                        for(int j=0; j<20; j++) {
                            amp = MAX(amp, abs(slicedata[j] / 128));
                        }
                        sprintf(sampledata + k * 2, "%02X", (uint8_t)amp);
                        vPortYield();
                    }
                    char info[600] = { 0, };
                    sprintf(info, "{\"index\":%d,\"slice_index\":%d,\"size\":%ld,\"data\":\"%s\"}",
                        file_index, slice, size, sampledata);
                    result = transmitCString(requestType, info);
                }
                break;
            case RequestType::EnableFileTransferMode:
                break;
            case RequestType::DisableFileTransferMode:
                break;
            case RequestType::GetSynthDefinitionsJSON:
                {
                    std::string info = "{\"status\":\"not implemented\"}";
                    result = transmitCString(requestType, info.c_str());
                }
                break;
            case RequestType::GetMacroMachineDefinitionsJSON:
                {
                    std::string info = "{\"status\":\"not implemented\"}";
                    result = transmitCString(requestType, info.c_str());
                }
                break;
            case RequestType::UploadMacroMachineDefinitionJSON:
                {
                    std::string info = "{\"status\":\"not implemented\"}";
                    result = transmitCString(requestType, info.c_str());
                }
                break;
            case RequestType::SetTrackMacroMachine:
                break;
            case RequestType::GetSoundPresetListJSON:
                {
                    std::string info = "{\"status\":\"not implemented\"}";
                    result = transmitCString(requestType, info.c_str());
                }
                break;
            case RequestType::GetSoundPresetJSON:
                {
                    std::string info = "{\"status\":\"not implemented\"}";
                    result = transmitCString(requestType, info.c_str());
                }
                break;
            case RequestType::UploadSoundPresetJSON:
                {
                    std::string info = "{\"status\":\"not implemented\"}";
                    result = transmitCString(requestType, info.c_str());
                }
                break;
            case RequestType::GetMacroSoundPresetList:
#if CONFIG_TBD_USE_SD_CARD
                {
                    // CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
                    std::string outputjson;
                    int trackIndex = uint8_param_0;
                    ESP_LOGI("SpiAPI", "Getting macro sound preset list, track %d", trackIndex);
                    CTAG::AUDIO::SoundProcessorManager::macroSoundDefinitionModel
                        ->GetPresetIndexJson(trackIndex, &outputjson);
                    // CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
                    result = transmitCString(requestType, outputjson.c_str());
                }
#else
                result = transmitCString(requestType, "{}");
#endif
                break;
            case RequestType::GetMacroSoundPreset:
#if CONFIG_TBD_USE_SD_CARD
                {
                    std::string presetId = string_parameter;
                    ESP_LOGI("SpiAPI", "Getting macro sound preset %s", presetId.c_str());
                    std::string outputjson;
                    outputjson = CTAG::AUDIO::SoundProcessorManager::GetMacroSoundPresetJSON(presetId);
                    result = transmitCString(requestType, outputjson.c_str());
                    // CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
                    // CTAG::MACROPRESETS::MacroSoundPreset *preset =
                    //     CTAG::AUDIO::SoundProcessorManager::macroSoundDefinitionModel
                    //         ->GetMacroSoundPreset(presetId);
                    // SerializeJSONInto
                    // GetPresetJson(presetId, &outputjson);
                    // CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
                }
#else
                result = transmitCString(requestType, "{}");
#endif
                break;
            case RequestType::GetMacroDefinition:
#if CONFIG_TBD_USE_SD_CARD
                {
                    std::string macroId = string_parameter; // receiveString(RequestType::SaveFavorite, string_parameter);
                    ESP_LOGI("SpiAPI", "Getting macro definition %s", macroId.c_str());
                    std::string outputjson;
                    // CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
                    outputjson = CTAG::AUDIO::SoundProcessorManager::GetMacroDefinitionJSON(macroId);
                    result = transmitCString(requestType, outputjson.c_str());
                    // CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
                    // HELPERS::ctagSampleRom::SetActiveSampleBank(uint8_param_0);
                    // HELPERS::ctagSampleRom::RefreshDataStructure();
                    // CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
                }
#else
                result = transmitCString(requestType, "{}");
#endif
                break;
            case RequestType::ActivateTrackMachine:
#if CONFIG_TBD_USE_SD_CARD
                {
                    int trackIndex = uint8_param_0;
                    std::string machineId = string_parameter; // receiveString(RequestType::SaveFavorite, string_parameter);
                    ESP_LOGI("SpiAPI", "Activating track %d machine %s", trackIndex, machineId.c_str());
                    CTAG::AUDIO::SoundProcessorManager::ActivateTrackMachine(trackIndex, machineId);
                    // CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
                    // HELPERS::ctagSampleRom::SetActiveSampleBank(uint8_param_0);
                    // HELPERS::ctagSampleRom::RefreshDataStructure();
                    // CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
                    // result = transmitCString(requestType, cstring);
                }
#endif
                break;
            case RequestType::LoadTrackSoundPreset:
#if CONFIG_TBD_USE_SD_CARD
                {
                    int trackIndex = uint8_param_0;
                    std::string presetId = string_parameter; // receiveString(RequestType::SaveFavorite, string_parameter);
                    ESP_LOGI("SpiAPI", "Loading track %d macro \"%s\"", trackIndex, presetId.c_str());
                    CTAG::AUDIO::SoundProcessorManager::LoadTrackMacroAndPreset(trackIndex, presetId);

                    // Optional rompler Bank/Slice overrides:
                    //   uint8_param_1 (byte 4) = romBank (0xFF = no override)
                    //   int32_param_2 (bytes 5-8) = sampleSlice (-1 = no override)
                    int romBank = uint8_param_1;
                    int sampleSlice = int32_param_2;
                    if (romBank != 0xFF) {
                        ESP_LOGI("SpiAPI", "  Override track %d Bank=%d", trackIndex, romBank);
                        CTAG::AUDIO::SoundProcessorManager::SetTrackParameter(trackIndex, 0, romBank);
                    }
                    if (sampleSlice >= 0) {
                        ESP_LOGI("SpiAPI", "  Override track %d Slice=%d", trackIndex, sampleSlice);
                        CTAG::AUDIO::SoundProcessorManager::SetTrackParameter(trackIndex, 1, sampleSlice);
                    }
                }
#endif
                break;

            case RequestType::GetTrackDefaultPresets:
                {
                    // Read /sdcard/data/trackdefaults.json and return its contents.
                    // If the file does not exist, return empty object "{}".
                    // The file maps track indices to preset IDs, e.g.:
                    // { "tracks": [ {"index":0,"preset":"db-all-def"}, ... ] }
                    std::string json = "{}";
                    std::string path = std::string(CTAG::RESOURCES::sdcardRoot) + "/data/trackdefaults.json";
                    FILE *f = fopen(path.c_str(), "r");
                    if (f) {
                        fseek(f, 0, SEEK_END);
                        long sz = ftell(f);
                        fseek(f, 0, SEEK_SET);
                        if (sz > 0 && sz < 8192) {
                            char *buf = (char*)malloc(sz + 1);
                            if (buf) {
                                fread(buf, 1, sz, f);
                                buf[sz] = '\0';
                                json = buf;
                                free(buf);
                            }
                        }
                        fclose(f);
                        ESP_LOGI("SpiAPI", "GetTrackDefaultPresets: loaded %ld bytes from trackdefaults.json", sz);
                    } else {
                        ESP_LOGW("SpiAPI", "GetTrackDefaultPresets: trackdefaults.json not found, returning {}");
                    }
                    result = transmitCString(requestType, json.c_str());
                }
                break;


            // case RequestType::SetTrackSampleBank:
            //     {
            //         int trackIndex = uint8_param_0;
            //         std::string bankName = string_parameter;
            //         ESP_LOGI("SpiAPI", "Setting track %d sample bank to \"%s\"", trackIndex, bankName.c_str());
            //         CTAG::AUDIO::SoundProcessorManager::SetTrackSampleBank(trackIndex, bankName);
            //     }
            //     break;

            case RequestType::GetKitIndexJSON:
                {
                    std::string json = CTAG::AUDIO::SoundProcessorManager::GetKitIndexJSON();
                    ESP_LOGI("SpiAPI", "Getting track sample bank list: %s", json.c_str());
                    result = transmitCString(requestType, json.c_str());
                }
                break;

            case RequestType::GetSampleBankIndexJSON:
                {
                    std::string json = CTAG::AUDIO::SoundProcessorManager::GetActiveKitBankIndexJSON();
                    ESP_LOGI("SpiAPI", "Getting track sample bank list: %s", json.c_str());
                    result = transmitCString(requestType, json.c_str());
                }
                break;

            // case RequestType::GetSynthUpdates:
            //     {
            //         std::string info = "{"
            //             "\"presetupdates\":0," // a preset was updated
            //             "\"macroupdates\":0," // a macro was updated
            //             "\"bankupdates\":0", // a samplebank was changed
            //             "\"trackupdates\":0" // any track changed preset etc.
            //         "}";
            //         result = transmitCString(requestType, info.c_str());
            //     }
            //     break;

            case RequestType::PutSamplePresetJSON:
#if CONFIG_TBD_USE_SD_CARD
                {
                    std::string json = string_parameter;
                    ESP_LOGI("SpiAPI", "Saving preset json: %s", json.c_str());
                    CTAG::AUDIO::SoundProcessorManager::PutSamplePresetJSON(json);
                    CTAG::AUDIO::SoundProcessorManager::RefreshSoundPresets();
                }
#endif
                break;

            case RequestType::LoadTrackMacroDefinition:
#if CONFIG_TBD_USE_SD_CARD
                {
                    int trackIndex = uint8_param_0;
                    std::string macroId = string_parameter; // receiveString(RequestType::SaveFavorite, string_parameter);
                    ESP_LOGI("SpiAPI", "Activating track %d macro %s", trackIndex, macroId.c_str());
                    CTAG::AUDIO::SoundProcessorManager::LoadTrackMacro(trackIndex, macroId);
                }
#endif
                break;

            case RequestType::AnnounceApp:
                {
                    // Generic app announcement from RP2350.
                    // uint8_param_0 bit 0 = plugin_lock (block HTTP plugin switching)
                    // uint8_param_0 bit 1 = redirect_samples (WebUI defaults to Samples view)
                    // string_parameter  = app display name (e.g. "Groovebox")
                    rp2350AppId = string_parameter;
                    rp2350PluginLock = (uint8_param_0 & 0x01) != 0;
                    rp2350RedirectSamples = (uint8_param_0 & 0x02) != 0;
                    ESP_LOGI("SpiAPI", "RP2350 announced app: \"%s\" (plugin_lock=%d, redirect_samples=%d)",
                             rp2350AppId.c_str(), rp2350PluginLock ? 1 : 0, rp2350RedirectSamples ? 1 : 0);
                }
                break;
            }
        }
    }
}

#endif // CONFIG_TBD_USE_RP2350
