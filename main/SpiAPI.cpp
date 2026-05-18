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
#include "SpiApiProtocol.h"
#include "SharedEngineDefinition.hpp"
#include "SPManager.hpp"
#include "Favorites.hpp"
#include "helpers/ctagSampleRom.hpp"
#include "pico_firmware_update.hpp"

#include "soc/gpio_num.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_image_format.h"
#include "esp_rom_crc.h"
#include "driver/gpio.h"

#include "MacroSoundPresetDataModel.hpp"
#include "SynthDefinitionDataModel.hpp"
#include "MacroDeviceDefinitionDataModel.hpp"
#include "StorageOverlay.hpp"
#include "SynthDefinitionDataModel.hpp"

#include "link.hpp"

#include <set>

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
    std::string SpiAPI::rp2350PicoVersion; // empty = unknown
    bool SpiAPI::rp2350PluginLock = false;
    bool SpiAPI::rp2350RedirectSamples = false;
    SemaphoreHandle_t SpiAPI::rp2350StateMutex = nullptr;
    TaskHandle_t SpiAPI::hTask;
    spi_slave_transaction_t SpiAPI::transaction;
    uint8_t *SpiAPI::send_buffer, *SpiAPI::receive_buffer;

    void SpiAPI::StartSpiAPI(){
        ESP_LOGI("SpiAPI", "Init()");
        rp2350StateMutex = xSemaphoreCreateMutex();
        configASSERT(rp2350StateMutex);
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
            ESP_LOGI("SpiAPI", "resLength %li, totalResponseLength %li, bytes_received %li, bytes_to_be_received %li",
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


    bool SpiAPI::transmitBinary(const RequestType reqType, const uint8_t *data, uint32_t len) {
        // Like transmitCString but takes explicit length — binary-safe (no strlen).
        uint8_t* requestTypeField = send_buffer + 2;
        *requestTypeField = static_cast<uint8_t>(reqType);
        uint32_t* lengthField = (uint32_t*)(send_buffer + 3);

        if (len == 0) {
            // Must still send one frame with length=0 so Pico's receiveBinaryData
            // gets a valid response instead of blocking or reading stale data.
            *lengthField = 0;
            spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY);
            return true;
        }

        uint32_t bytes_to_send = 0;
        uint32_t bytes_sent = 0;
        uint32_t remaining = len;
        while (remaining > 0) {
            *lengthField = remaining;
            bytes_to_send = remaining > 2048 - 7 ? 2048 - 7 : remaining;
            memcpy(send_buffer + 7, data + bytes_sent, bytes_to_send);
            remaining -= bytes_to_send;
            bytes_sent += bytes_to_send;
            spi_slave_transmit(RCV_HOST, &transaction, portMAX_DELAY);
            // fingerprint check
            if (receive_buffer[0] != 0xCA || receive_buffer[1] != 0xFE) {
                return false;
            }
            // check request type acknowledgment
            const uint8_t requestType = receive_buffer[2];
            if (requestType != (uint8_t)reqType) {
                return false;
            }
        }
        return true;
    }

    // Atomic write: write to temp file, then rename to final path.
    // FAT32 (FATFS) rename cannot overwrite — must remove destination first.
    static bool atomicWrite(const std::string &filePath, const void *data, size_t len) {
        std::string tmpPath = filePath + ".tmp";
        FILE *f = fopen(tmpPath.c_str(), "wb");
        if (!f) {
            ESP_LOGE("SpiAPI", "atomicWrite: failed to open %s", tmpPath.c_str());
            return false;
        }
        size_t written = fwrite(data, 1, len, f);
        fclose(f);
        if (written != len) {
            ESP_LOGE("SpiAPI", "atomicWrite: short write (%d/%d)", (int)written, (int)len);
            remove(tmpPath.c_str());
            return false;
        }
        // FAT32 rename does not overwrite — remove destination first
        remove(filePath.c_str());
        if (rename(tmpPath.c_str(), filePath.c_str()) != 0) {
            ESP_LOGE("SpiAPI", "atomicWrite: rename failed %s -> %s", tmpPath.c_str(), filePath.c_str());
            remove(tmpPath.c_str());
            return false;
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
                ESP_LOGE("SpiAPI", "Received transaction length %d, expected 2048 * 8", transaction.trans_len);
                result = true;
                continue;
            }
            if (rcv_data[0] != 0xCA || rcv_data[1] != 0xFE){
                ESP_LOGE("SpiAPI", "Received data %x %x, expected 0xCA 0xFE", rcv_data[0], rcv_data[1]);
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
            const unsigned char* binary_param_3 = (unsigned char*)&rcv_data[9];
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
            // case RequestType::GetPlugins:
            //     cstring = AUDIO::SoundProcessorManager::GetCStrJSONSoundProcessors();
            //     result = transmitCString(requestType, cstring);
            //     break;
            // case RequestType::GetActivePlugin:
            //     cstring = AUDIO::SoundProcessorManager::GetStringID(channel).c_str();
            //     result = transmitCString(requestType, cstring);
            //     break;
            // case RequestType::GetActivePluginParams:
            //     cstring = AUDIO::SoundProcessorManager::GetCStrJSONActivePluginParams(channel);
            //     result = transmitCString(requestType, cstring);
            //     break;
            // case RequestType::SetActivePlugin:
            //     AUDIO::SoundProcessorManager::SetSoundProcessorChannel(channel, string_parameter);
            //     FAV::Favorites::DeactivateFavorite();
            //     result = true;
            //     break;
            // case RequestType::SetPluginParam:
            //     AUDIO::SoundProcessorManager::SetChannelParamValue(channel, string_parameter, "current", param_value);
            //     result = true;
            //     break;
            // case RequestType::SetPluginParamCV:
            //     AUDIO::SoundProcessorManager::SetChannelParamValue(channel, string_parameter, "cv", param_value);
            //     result = true;
            //     break;
            // case RequestType::SetPluginParamTRIG:
            //     AUDIO::SoundProcessorManager::SetChannelParamValue(channel, string_parameter, "trig", param_value);
            //     result = true;
            //     break;
            // case RequestType::GetPresets:
            //     cstring = AUDIO::SoundProcessorManager::GetCStrJSONGetPresets(channel);
            //     result = transmitCString(requestType, cstring);
            //     break;
            // case RequestType::GetPresetData:
            //     cstring = AUDIO::SoundProcessorManager::GetCStrJSONSoundProcessorPresets(string_parameter);
            //     result = transmitCString(requestType, cstring);
            //     break;
            // case RequestType::SetPresetData:{
            //     std::string pluginID = string_parameter;
            //     string_parameter.clear();
            //     result = receiveString(RequestType::SetPresetData, string_parameter);
            //     //ESP_LOGI("SpiAPI", "Result %d, Saving preset %s %s", result, pluginID.c_str(), string_parameter.c_str());
            //     if (true == result) AUDIO::SoundProcessorManager::SetCStrJSONSoundProcessorPreset(pluginID.c_str(), string_parameter.c_str());
            // }
            //     break;
            // case RequestType::SetPluginParamsJSON:{
            //     AUDIO::SoundProcessorManager::SetChannelParamsCstrJSON(channel, string_parameter.c_str());
            // }
            //     break;
            // case RequestType::LoadPreset:
            //     AUDIO::SoundProcessorManager::ChannelLoadPreset(channel, preset_number);
            //     FAV::Favorites::DeactivateFavorite();
            //     result = true;
            //     break;
            // case RequestType::SavePreset:
            //     AUDIO::SoundProcessorManager::ChannelSavePreset(channel, string_parameter, preset_number);
            //     result = true;
            //     break;
            // case RequestType::GetAllFavorites:
            //     cstring = FAV::Favorites::GetAllFavorites().c_str();
            //     result = transmitCString(requestType, cstring);
            //     break;
            // case RequestType::SaveFavorite:
            //     string_parameter.clear();
            //     result = receiveString(RequestType::SaveFavorite, string_parameter);
            //     //ESP_LOGI("SpiAPI", "Result %d, Saving favorite# %d as %s", result, favorite_number, string_parameter.c_str());
            //     if (true == result) FAV::Favorites::StoreFavorite(preset_number, string_parameter);
            //     break;
            // case RequestType::LoadFavorite:
            //     FAV::Favorites::ActivateFavorite(favorite_number);
            //     result = true;
            //     break;
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
            // case RequestType::GetIOCapabilities:
            //     result = transmitCString(requestType, s.c_str());
            //     break;
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
                    xSemaphoreTake(rp2350StateMutex, portMAX_DELAY);
                    rp2350AppId.clear();
                    rp2350PluginLock = false;
                    rp2350RedirectSamples = false;
                    xSemaphoreGive(rp2350StateMutex);
                    break;
                }
                xSemaphoreTake(rp2350StateMutex, portMAX_DELAY);
                rp2350AppId.clear();
                rp2350PluginLock = false;
                rp2350RedirectSamples = false;
                xSemaphoreGive(rp2350StateMutex);
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
                ESP_LOGD("SpiAPI", "GetSampleRomDescriptor");
                {
                    std::string desc = HELPERS::ctagSampleRom::GetSampleRomDescriptorJSON();
                    result = transmitCString(requestType, desc.c_str());
                }
                break;
            case RequestType::SetActiveWaveTableBank:
                ESP_LOGD("SpiAPI", "Setting active wavetable bank to %d", bank_number);
                CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
                HELPERS::ctagSampleRom::SetActiveWaveTableBank(uint8_param_0);
                HELPERS::ctagSampleRom::RefreshDataStructure();
                CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
                break;

            case RequestType::SetActiveSampleKit:
                ESP_LOGD("SpiAPI", "Setting active sample bank to #%d", uint8_param_0);
                CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
                HELPERS::ctagSampleRom::SetActiveSampleBank(uint8_param_0);
                HELPERS::ctagSampleRom::RefreshDataStructure();
                CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
                break;

            case RequestType::GetFirmwareInfo:
                ESP_LOGD("SpiAPI", "GetFirmwareInfo");
                {
                    // Read WebUI version from SD card
                    std::string webui_ver;
                    std::string wvPath = CTAG::RESOURCES::sdcardRoot + "/system/webui-version.json";
                    FILE *wf = fopen(wvPath.c_str(), "r");
                    if (wf) {
                        char buf[256];
                        size_t len = fread(buf, 1, sizeof(buf) - 1, wf);
                        fclose(wf);
                        buf[len] = '\0';
                        const char *v = strstr(buf, "\"version\"");
                        if (v) {
                            v = strchr(v, ':');
                            if (v) {
                                v++;
                                while (*v == ' ' || *v == '"') v++;
                                const char *end = strchr(v, '"');
                                if (end) webui_ver.assign(v, end - v);
                            }
                        }
                    }

                    std::string info("{\"HWV\":\"" + TBD_HW_VERSION
                        + "\",\"FWV\":\"" + TBD_FW_VERSION
                        + "\",\"OTA\":\"" + std::string(esp_get_current_ota_label())
                        + "\",\"WEBUI\":\"" + webui_ver + "\"}");
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
                    ESP_LOGD("SpiAPI", "Getting sample file count (%d)", number);
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
                    ESP_LOGD("SpiAPI", "Getting sample file %d info", file_index);
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
                    ESP_LOGD("SpiAPI", "Getting sample file %d waveform preview, slice %d, size %ld, offset %ld",
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
            case RequestType::GetEngineDefinitionList:
                {
#if CONFIG_TBD_USE_SD_CARD
                    ESP_LOGI("SpiAPI", "Getting engine id list");
                    static struct GetEngineDefinitionIdListResponse listresponse;
                    CTAG::MACROPRESETS::SynthDefinitionDataModel::instance()->WriteListResponse(&listresponse);
                    result = transmitBinary(requestType, (const uint8_t*)&listresponse, sizeof(listresponse));
#endif
                }
                break;
//             case RequestType::GetEngineDefinitionJSON:
//                 {
#if CONFIG_TBD_USE_SD_CARD
//                     std::string synthId = string_parameter;
//                     ESP_LOGD("SpiAPI", "Getting synth definition %s", synthId.c_str());
//                     std::string synthjson = "{}";
//                     auto def = CTAG::MACROPRESETS::SynthDefinitionDataModel::instance()->GetSynthDefinition(synthId);
//                     CTAG::MACROPRESETS::SynthDefinitionUtils::SynthDefinition_SerializeJSON(def, &synthjson);
//                     result = transmitCString(requestType, synthjson.c_str());
#endif
//                 }
//                 break;
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
                    ESP_LOGD("SpiAPI", "Getting macro sound preset list, track %d", trackIndex);
                    CTAG::MACROPRESETS::MacroSoundPresetDataModel::instance().GetPresetIndexJson(trackIndex, &outputjson);
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
                    ESP_LOGD("SpiAPI", "Getting macro sound preset %s", presetId.c_str());
                    std::string outputjson;
                    outputjson = CTAG::AUDIO::SoundProcessorManager::GetMacroSoundPresetJSON(presetId);
                    result = transmitCString(requestType, outputjson.c_str());
                }
#else
                result = transmitCString(requestType, "{}");
#endif
                break;
            case RequestType::GetMacroDefinition:
#if CONFIG_TBD_USE_SD_CARD
                {
                    std::string macroId = string_parameter; // receiveString(RequestType::SaveFavorite, string_parameter);
                    ESP_LOGD("SpiAPI", "Getting macro definition %s", macroId.c_str());
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
                    std::string machineId = string_parameter;
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
                    if (trackIndex < 0 || trackIndex >= MAX_TRACKS) {
                        ESP_LOGE("SpiAPI", "LoadTrackSoundPreset: invalid trackIndex %d", trackIndex);
                        break;
                    }
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
                    // Read track defaults template via overlay and return its contents.
                    // If string_parameter is non-empty, use that as the template name.
                    // Otherwise, read the active template name from user config.
                    // Falls back to "default" if nothing is configured.
                    std::string templateName = string_parameter;
                    if (templateName.empty()) {
                        // Try reading active template from config
                        std::string cfgPath = STORAGE::userPath() + "/" + STORAGE::DIR_CONFIG + "/active-trackdefault.txt";
                        FILE *cf = fopen(cfgPath.c_str(), "r");
                        if (cf) {
                            char nameBuf[64] = {};
                            if (fgets(nameBuf, sizeof(nameBuf), cf)) {
                                // Strip trailing newline
                                size_t len = strlen(nameBuf);
                                while (len > 0 && (nameBuf[len-1] == '\n' || nameBuf[len-1] == '\r')) nameBuf[--len] = '\0';
                                if (len > 0) templateName = nameBuf;
                            }
                            fclose(cf);
                        }
                        if (templateName.empty()) templateName = "default";
                    }

                    std::string filename = templateName + ".json";
                    const std::string path = CTAG::STORAGE::resolveFile(CTAG::STORAGE::DIR_TRACKDEFAULTS, filename.c_str());
                    std::string json = "{}";
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
                        ESP_LOGI("SpiAPI", "GetTrackDefaultPresets: loaded %ld bytes from %s", sz, filename.c_str());
                    } else {
                        ESP_LOGW("SpiAPI", "GetTrackDefaultPresets: %s not found, returning {}", filename.c_str());
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
                    ESP_LOGD("SpiAPI", "Getting track sample bank list: %s", json.c_str());
                    result = transmitCString(requestType, json.c_str());
                }
                break;

            case RequestType::GetSampleBankIndexJSON:
                {
                    std::string json = CTAG::AUDIO::SoundProcessorManager::GetActiveKitBankIndexJSON();
                    ESP_LOGD("SpiAPI", "Getting track sample bank list: %s", json.c_str());
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
                    ESP_LOGD("SpiAPI", "Saving preset json: %s", json.c_str());
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
                    xSemaphoreTake(rp2350StateMutex, portMAX_DELAY);
                    rp2350AppId = string_parameter;
                    rp2350PluginLock = (uint8_param_0 & 0x01) != 0;
                    rp2350RedirectSamples = (uint8_param_0 & 0x02) != 0;
                    xSemaphoreGive(rp2350StateMutex);
                    ESP_LOGI("SpiAPI", "RP2350 announced app: \"%s\" (plugin_lock=%d, redirect_samples=%d)",
                             rp2350AppId.c_str(), rp2350PluginLock ? 1 : 0, rp2350RedirectSamples ? 1 : 0);
                }
                break;

            case RequestType::ReportPicoVersion:
                {
                    xSemaphoreTake(rp2350StateMutex, portMAX_DELAY);
                    rp2350PicoVersion = string_parameter;
                    xSemaphoreGive(rp2350StateMutex);
                    ESP_LOGI("SpiAPI", "RP2350 reported Pico firmware version: \"%s\"", string_parameter.c_str());
                }
                break;

            case RequestType::GetPicoUpdateStatus:
                {
                    bool picoUpdated = DRIVERS::PicoFirmwareUpdate::ConsumePicoUpdateFlag();
                    bool p4Updated = DRIVERS::PicoFirmwareUpdate::ConsumeP4UpdateFlag();

                    std::string status;
                    if (picoUpdated && p4Updated) {
                        status = "both:" + TBD_FW_VERSION;
                    } else if (p4Updated) {
                        status = "p4:" + TBD_FW_VERSION;
                    } else if (picoUpdated) {
                        status = "pico";
                    } else {
                        status = "none";
                    }

                    ESP_LOGI("SpiAPI", "GetPicoUpdateStatus: %s", status.c_str());
                    result = transmitCString(requestType, status.c_str());
                }
                break;

            case RequestType::SaveProjectToP4:
#if CONFIG_TBD_USE_SD_CARD
                {
                    std::string slotName = string_parameter;
                    ESP_LOGI("SpiAPI", "SaveProjectToP4: slot \"%s\"", slotName.c_str());

                    // Receive binary project data from Pico
                    std::string projectData;
                    result = receiveString(RequestType::SaveProjectToP4, projectData);
                    if (!result) {
                        ESP_LOGE("SpiAPI", "SaveProjectToP4: failed to receive data");
                        break;
                    }

                    // Ensure projects directory exists
                    std::string projDir = STORAGE::userPath() + "/" + STORAGE::DIR_PROJECTS;
                    mkdir(projDir.c_str(), 0755);

                    // Atomic write: temp file + rename
                    std::string filePath = projDir + "/project" + slotName + ".bin";
                    STORAGE::lockStorage();
                    result = atomicWrite(filePath, projectData.data(), projectData.size());
                    STORAGE::unlockStorage();
                    if (result) {
                        ESP_LOGI("SpiAPI", "SaveProjectToP4: saved %d bytes to %s", (int)projectData.size(), filePath.c_str());
                    }
                }
#else
                ESP_LOGW("SpiAPI", "SaveProjectToP4: SD card disabled");
                result = true;
#endif
                break;

            case RequestType::LoadProjectFromP4:
#if CONFIG_TBD_USE_SD_CARD
                {
                    std::string slotName = string_parameter;
                    ESP_LOGI("SpiAPI", "LoadProjectFromP4: slot \"%s\"", slotName.c_str());

                    // Try user path first, then factory path
                    std::string filePath = STORAGE::userPath() + "/" + STORAGE::DIR_PROJECTS + "/project" + slotName + ".bin";
                    FILE *f = fopen(filePath.c_str(), "rb");
                    if (!f) {
                        // Fall back to factory projects
                        filePath = STORAGE::factoryPath() + "/" + STORAGE::DIR_PROJECTS + "/project" + slotName + ".bin";
                        f = fopen(filePath.c_str(), "rb");
                    }
                    if (!f) {
                        ESP_LOGW("SpiAPI", "LoadProjectFromP4: project not found for slot \"%s\"", slotName.c_str());
                        // Send empty response (0 bytes) so Pico knows it failed
                        result = transmitBinary(requestType, nullptr, 0);
                        break;
                    }

                    // Get file size
                    fseek(f, 0, SEEK_END);
                    long fileSize = ftell(f);
                    fseek(f, 0, SEEK_SET);

                    if (fileSize <= 0 || fileSize > 65536) {
                        ESP_LOGE("SpiAPI", "LoadProjectFromP4: invalid file size %ld", fileSize);
                        fclose(f);
                        result = transmitBinary(requestType, nullptr, 0);
                        break;
                    }

                    // Read file into buffer
                    uint8_t *buf = (uint8_t*)malloc(fileSize);
                    if (!buf) {
                        ESP_LOGE("SpiAPI", "LoadProjectFromP4: malloc failed (%ld bytes)", fileSize);
                        fclose(f);
                        result = transmitBinary(requestType, nullptr, 0);
                        break;
                    }
                    size_t readBytes = fread(buf, 1, fileSize, f);
                    fclose(f);

                    ESP_LOGI("SpiAPI", "LoadProjectFromP4: transmitting %d bytes from %s", (int)readBytes, filePath.c_str());
                    result = transmitBinary(requestType, buf, (uint32_t)readBytes);
                    free(buf);
                }
#else
                ESP_LOGW("SpiAPI", "LoadProjectFromP4: SD card disabled");
                result = transmitBinary(requestType, nullptr, 0);
#endif
                break;

            case RequestType::ListProjects:
#if CONFIG_TBD_USE_SD_CARD
                {
                    ESP_LOGI("SpiAPI", "ListProjects");
                    // Scan user and factory project dirs, return JSON array of slot names
                    // Format: ["slot00","slot01","slot03"] — only slots that have files
                    std::string json = "[";
                    bool first = true;
                    const char *dirs[] = {
                        (STORAGE::userPath() + "/" + STORAGE::DIR_PROJECTS).c_str(),
                        (STORAGE::factoryPath() + "/" + STORAGE::DIR_PROJECTS).c_str()
                    };
                    // Need stable strings for dir paths
                    std::string userDir = STORAGE::userPath() + "/" + STORAGE::DIR_PROJECTS;
                    std::string factoryDir = STORAGE::factoryPath() + "/" + STORAGE::DIR_PROJECTS;
                    const std::string dirPaths[] = { userDir, factoryDir };

                    // Track which slots we've already seen (user overrides factory)
                    std::set<std::string> seen;
                    for (const auto &dirPath : dirPaths) {
                        DIR *d = opendir(dirPath.c_str());
                        if (!d) continue;
                        struct dirent *ent;
                        while ((ent = readdir(d)) != nullptr) {
                            std::string name = ent->d_name;
                            // Match pattern: project{slotName}.bin
                            if (name.size() > 11 && name.substr(0, 7) == "project" && name.substr(name.size() - 4) == ".bin") {
                                std::string slotName = name.substr(7, name.size() - 11); // strip "project" and ".bin"
                                if (seen.count(slotName)) continue;
                                seen.insert(slotName);
                                if (!first) json += ",";
                                json += "\"" + slotName + "\"";
                                first = false;
                            }
                        }
                        closedir(d);
                    }
                    json += "]";
                    ESP_LOGD("SpiAPI", "ListProjects: %s", json.c_str());
                    result = transmitCString(requestType, json.c_str());
                }
#else
                result = transmitCString(requestType, "[]");
#endif
                break;

            case RequestType::DeleteProject:
#if CONFIG_TBD_USE_SD_CARD
                {
                    std::string slotName = string_parameter;
                    ESP_LOGI("SpiAPI", "DeleteProject: slot \"%s\"", slotName.c_str());
                    // Only delete from user dir — factory projects are immutable
                    std::string filePath = STORAGE::userPath() + "/" + STORAGE::DIR_PROJECTS + "/project" + slotName + ".bin";
                    STORAGE::lockStorage();
                    if (remove(filePath.c_str()) == 0) {
                        ESP_LOGI("SpiAPI", "DeleteProject: deleted %s", filePath.c_str());
                        result = true;
                    } else {
                        ESP_LOGW("SpiAPI", "DeleteProject: file not found %s", filePath.c_str());
                        result = true; // not an error — file already doesn't exist
                    }
                    STORAGE::unlockStorage();
                }
#else
                result = true;
#endif
                break;

            case RequestType::SavePicoConfig:
#if CONFIG_TBD_USE_SD_CARD
                {
                    ESP_LOGI("SpiAPI", "SavePicoConfig");
                    std::string configData;
                    result = receiveString(RequestType::SavePicoConfig, configData);
                    if (!result) {
                        ESP_LOGE("SpiAPI", "SavePicoConfig: failed to receive data");
                        break;
                    }

                    // Ensure config directory exists
                    std::string configDir = STORAGE::userPath() + "/" + STORAGE::DIR_CONFIG;
                    mkdir(configDir.c_str(), 0755);

                    // Atomic write
                    std::string filePath = configDir + "/sequencer.bin";
                    STORAGE::lockStorage();
                    result = atomicWrite(filePath, configData.data(), configData.size());
                    STORAGE::unlockStorage();
                    if (result) {
                        ESP_LOGI("SpiAPI", "SavePicoConfig: saved %d bytes to %s", (int)configData.size(), filePath.c_str());
                    }
                }
#else
                ESP_LOGW("SpiAPI", "SavePicoConfig: SD card disabled");
                result = true;
#endif
                break;

            case RequestType::LoadPicoConfig:
#if CONFIG_TBD_USE_SD_CARD
                {
                    ESP_LOGI("SpiAPI", "LoadPicoConfig");
                    std::string filePath = STORAGE::userPath() + "/" + STORAGE::DIR_CONFIG + "/sequencer.bin";
                    FILE *f = fopen(filePath.c_str(), "rb");
                    if (!f) {
                        ESP_LOGW("SpiAPI", "LoadPicoConfig: file not found %s", filePath.c_str());
                        result = transmitBinary(requestType, nullptr, 0);
                        break;
                    }

                    fseek(f, 0, SEEK_END);
                    long fileSize = ftell(f);
                    fseek(f, 0, SEEK_SET);

                    if (fileSize <= 0 || fileSize > 4096) {
                        ESP_LOGE("SpiAPI", "LoadPicoConfig: invalid file size %ld", fileSize);
                        fclose(f);
                        result = transmitBinary(requestType, nullptr, 0);
                        break;
                    }

                    uint8_t *buf = (uint8_t*)malloc(fileSize);
                    if (!buf) {
                        fclose(f);
                        result = transmitBinary(requestType, nullptr, 0);
                        break;
                    }
                    size_t readBytes = fread(buf, 1, fileSize, f);
                    fclose(f);

                    ESP_LOGI("SpiAPI", "LoadPicoConfig: transmitting %d bytes", (int)readBytes);
                    result = transmitBinary(requestType, buf, (uint32_t)readBytes);
                    free(buf);
                }
#else
                ESP_LOGW("SpiAPI", "LoadPicoConfig: SD card disabled");
                result = transmitBinary(requestType, nullptr, 0);
#endif
                break;

            case RequestType::ListTrackDefaults:
#if CONFIG_TBD_USE_SD_CARD
                {
                    ESP_LOGI("SpiAPI", "ListTrackDefaults");
                    // Scan factory + user trackdefaults dirs, return JSON array with factory flag
                    // Format: [{"n":"default","f":1},{"n":"user1","f":0}]
                    std::string json = "[";
                    bool first = true;
                    std::string factoryDir = STORAGE::factoryPath() + "/" + STORAGE::DIR_TRACKDEFAULTS;
                    std::string userDir = STORAGE::userPath() + "/" + STORAGE::DIR_TRACKDEFAULTS;

                    // First pass: collect factory template names
                    std::set<std::string> factoryNames;
                    std::set<std::string> seen;
                    {
                        DIR *d = opendir(factoryDir.c_str());
                        if (d) {
                            struct dirent *ent;
                            while ((ent = readdir(d)) != nullptr) {
                                std::string name = ent->d_name;
                                if (name.size() > 5 && name.substr(name.size() - 5) == ".json") {
                                    std::string templateName = name.substr(0, name.size() - 5);
                                    factoryNames.insert(templateName);
                                    seen.insert(templateName);
                                    if (!first) json += ",";
                                    json += "{\"n\":\"" + templateName + "\",\"f\":1}";
                                    first = false;
                                }
                            }
                            closedir(d);
                        }
                    }

                    // Second pass: user-only templates
                    {
                        DIR *d = opendir(userDir.c_str());
                        if (d) {
                            struct dirent *ent;
                            while ((ent = readdir(d)) != nullptr) {
                                std::string name = ent->d_name;
                                if (name.size() > 5 && name.substr(name.size() - 5) == ".json") {
                                    std::string templateName = name.substr(0, name.size() - 5);
                                    if (seen.count(templateName)) continue;
                                    seen.insert(templateName);
                                    if (!first) json += ",";
                                    json += "{\"n\":\"" + templateName + "\",\"f\":0}";
                                    first = false;
                                }
                            }
                            closedir(d);
                        }
                    }

                    json += "]";
                    ESP_LOGD("SpiAPI", "ListTrackDefaults: %s", json.c_str());
                    result = transmitCString(requestType, json.c_str());
                }
#else
                result = transmitCString(requestType, "[]");
#endif
                break;

            case RequestType::GetTrackDefault:
#if CONFIG_TBD_USE_SD_CARD
                {
                    std::string templateName = string_parameter;
                    ESP_LOGI("SpiAPI", "GetTrackDefault: \"%s\"", templateName.c_str());

                    // Resolve via overlay (user overrides factory)
                    std::string filename = templateName + ".json";
                    const std::string path = CTAG::STORAGE::resolveFile(CTAG::STORAGE::DIR_TRACKDEFAULTS, filename.c_str());
                    std::string json = "{}";
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
                        ESP_LOGI("SpiAPI", "GetTrackDefault: loaded %ld bytes", sz);
                    } else {
                        ESP_LOGW("SpiAPI", "GetTrackDefault: \"%s\" not found", templateName.c_str());
                    }
                    result = transmitCString(requestType, json.c_str());
                }
#else
                result = transmitCString(requestType, "{}");
#endif
                break;

            case RequestType::SaveTrackDefault:
#if CONFIG_TBD_USE_SD_CARD
                {
                    std::string templateName = string_parameter;
                    ESP_LOGI("SpiAPI", "SaveTrackDefault: \"%s\"", templateName.c_str());

                    // Receive JSON data from Pico
                    std::string jsonData;
                    result = receiveString(RequestType::SaveTrackDefault, jsonData);
                    if (!result) {
                        ESP_LOGE("SpiAPI", "SaveTrackDefault: failed to receive data");
                        break;
                    }

                    // Ensure user trackdefaults dir exists
                    std::string tdDir = STORAGE::userPath() + "/" + STORAGE::DIR_TRACKDEFAULTS;
                    mkdir(tdDir.c_str(), 0755);

                    // Atomic write
                    std::string filePath = tdDir + "/" + templateName + ".json";
                    STORAGE::lockStorage();
                    result = atomicWrite(filePath, jsonData.data(), jsonData.size());
                    STORAGE::unlockStorage();
                    if (result) {
                        ESP_LOGI("SpiAPI", "SaveTrackDefault: saved %d bytes to %s", (int)jsonData.size(), filePath.c_str());
                    }
                }
#else
                ESP_LOGW("SpiAPI", "SaveTrackDefault: SD card disabled");
                result = true;
#endif
                break;

            case RequestType::DeleteTrackDefault:
#if CONFIG_TBD_USE_SD_CARD
                {
                    std::string templateName = string_parameter;
                    ESP_LOGI("SpiAPI", "DeleteTrackDefault: \"%s\"", templateName.c_str());
                    // Only delete from user dir — factory templates are immutable
                    std::string filePath = STORAGE::userPath() + "/" + STORAGE::DIR_TRACKDEFAULTS + "/" + templateName + ".json";
                    STORAGE::lockStorage();
                    if (remove(filePath.c_str()) == 0) {
                        ESP_LOGI("SpiAPI", "DeleteTrackDefault: deleted %s", filePath.c_str());
                    } else {
                        ESP_LOGW("SpiAPI", "DeleteTrackDefault: file not found %s", filePath.c_str());
                    }
                    STORAGE::unlockStorage();
                    result = true;
                }
#else
                result = true;
#endif
                break;

            case RequestType::SetActiveTrackDefault:
#if CONFIG_TBD_USE_SD_CARD
                {
                    std::string templateName = string_parameter;
                    ESP_LOGI("SpiAPI", "SetActiveTrackDefault: \"%s\"", templateName.c_str());
                    std::string cfgDir = STORAGE::userPath() + "/" + STORAGE::DIR_CONFIG;
                    mkdir(cfgDir.c_str(), 0755);
                    std::string cfgPath = cfgDir + "/active-trackdefault.txt";
                    STORAGE::lockStorage();
                    result = atomicWrite(cfgPath, templateName.data(), templateName.size());
                    STORAGE::unlockStorage();
                    if (result) {
                        ESP_LOGI("SpiAPI", "SetActiveTrackDefault: wrote \"%s\" to %s", templateName.c_str(), cfgPath.c_str());
                    }
                }
#else
                result = true;
#endif
                break;

            case RequestType::SetTrackParamValues:
                {
                    int trackIndex = uint8_param_0;
                    int count = uint8_param_1;
                    if (trackIndex < 0 || trackIndex >= MAX_TRACKS) {
                        ESP_LOGE("SpiAPI", "SetTrackParamValues: invalid trackIndex %d", trackIndex);
                        break;
                    }
                    if (count > 16) count = 16;
                    if (count <= 0) break;
                    const int16_t *values = reinterpret_cast<const int16_t*>(string_parameter.data());
                    ESP_LOGI("SpiAPI", "SetTrackParamValues: track %d, %d params", trackIndex, count);
                    for (int i = 0; i < count; i++) {
                        CTAG::AUDIO::SoundProcessorManager::SetTrackParameter(trackIndex, i, (int32_t)values[i]);
                    }
                }
                break;

            case RequestType::SetTrackMute:
                {
                    int trackIndex = uint8_param_0;
                    bool muted = (uint8_param_1 != 0);
                    if (trackIndex < 0 || trackIndex >= 16) {
                        ESP_LOGE("SpiAPI", "SetTrackMute: invalid trackIndex %d", trackIndex);
                        break;
                    }
                    // ESP_LOGI("SpiAPI", "SetTrackMute: track %d = %d", trackIndex, muted ? 1 : 0);
                    CTAG::AUDIO::SoundProcessorManager::SetTrackMute(trackIndex, muted);
                }
                break;

            case RequestType::SaveScreenshot:
#if CONFIG_TBD_USE_SD_CARD
                {
                    ESP_LOGI("SpiAPI", "SaveScreenshot: receiving BMP data");

                    // Receive BMP binary data from Pico
                    std::string screenshotData;
                    result = receiveString(RequestType::SaveScreenshot, screenshotData);
                    if (!result || screenshotData.empty()) {
                        ESP_LOGE("SpiAPI", "SaveScreenshot: failed to receive data");
                        break;
                    }

                    // Ensure screenshots directory exists
                    std::string scrDir = STORAGE::userPath() + "/" + STORAGE::DIR_SCREENSHOTS;
                    mkdir(scrDir.c_str(), 0755);

                    // Find next available filename (scr_0001.bmp .. scr_9999.bmp)
                    char filename[32];
                    int counter = 1;
                    std::string filePath;
                    do {
                        snprintf(filename, sizeof(filename), "scr_%04d.bmp", counter);
                        filePath = scrDir + "/" + filename;
                        counter++;
                    } while (counter < 10000 && STORAGE::fileExists(filePath));

                    if (counter >= 10000) {
                        ESP_LOGE("SpiAPI", "SaveScreenshot: too many screenshots, max 9999");
                        break;
                    }

                    STORAGE::lockStorage();
                    result = atomicWrite(filePath, screenshotData.data(), screenshotData.size());
                    STORAGE::unlockStorage();
                    if (result) {
                        ESP_LOGI("SpiAPI", "SaveScreenshot: saved %d bytes to %s",
                                 (int)screenshotData.size(), filePath.c_str());
                    } else {
                        ESP_LOGE("SpiAPI", "SaveScreenshot: failed to write %s", filePath.c_str());
                    }
                }
#else
                ESP_LOGW("SpiAPI", "SaveScreenshot: SD card disabled");
                result = true;
#endif
                break;
            case RequestType::GetEngineDefinitionsPage:
                {
                    ESP_LOGI("SpiAPI", "GetEngineDefinitionsPage");
                    // for(int j=0; j<64; j++) {
                    //     if (j % 16 == 0) printf("  ");
                    //     printf( "%02X ", binary_param_3[j] );
                    //     if (j % 16 == 15) printf("\n");
                    // }
                    // printf("\n");

                    struct GetEngineDefinitionsPageRequest *pagerequest = (struct GetEngineDefinitionsPageRequest*)binary_param_3;
                    ESP_LOGI("SpiAPI", "GetEngineDefinitionsPage: offset=%d", pagerequest->offset);

                    static struct GetEngineDefinitionsPageResponse pageresponse;
                    CTAG::MACROPRESETS::SynthDefinitionDataModel::instance()->WriteEngineDefinitionPageResponse(
                        pagerequest,
                        &pageresponse);

                    const uint8_t *responseBytes = (const uint8_t*)&pageresponse;
                    // for(int j=0; j<64; j++) {
                    //     if (j % 16 == 0) printf("  ");
                    //     printf( "%02X ", responseBytes[j] );
                    //     if (j % 16 == 15) printf("\n");
                    // }
                    // printf("\n");

                    result = transmitBinary(requestType, responseBytes, sizeof(struct GetEngineDefinitionsPageResponse));

                    // std::string cfgDir = STORAGE::userPath() + "/" + STORAGE::DIR_CONFIG;
                    // mkdir(cfgDir.c_str(), 0755);
                    // std::string cfgPath = cfgDir + "/active-trackdefault.txt";
                    // STORAGE::lockStorage();
                    // result = atomicWrite(cfgPath, templateName.data(), templateName.size());
                    // STORAGE::unlockStorage();
                    // if (result) {
                    //     ESP_LOGI("SpiAPI", "SetActiveTrackDefault: wrote \"%s\" to %s", templateName.c_str(), cfgPath.c_str());
                    // }
                }
                break;
            // case RequestType::GetTrackDefinitionsPage:
            case RequestType::GetMacroDefinitionsPage:
            case RequestType::GetMacroSoundPresetsPage:
            case RequestType::GetKitIndexPage:
            case RequestType::GetSampleBankIndex:
            case RequestType::GetSampleInfoAndPreview:
            case RequestType::UploadMacroSoundPreset:
            case RequestType::ReloadMacroDefinitions:
            case RequestType::ReloadMacroSoundPresets:
            case RequestType::ReloadKitIndes:
            case RequestType::FileQuery:
            case RequestType::FileReadlock:
            case RequestType::FileWriteBlock:
            case RequestType::FileDelete:
            case RequestType::FileListPage:
                ESP_LOGW("SpiAPI", "Unhandled api call");
                break;
            }
        }
    }
}

#endif // CONFIG_TBD_USE_RP2350
