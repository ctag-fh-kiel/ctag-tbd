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

#include "OTAManager.hpp"
#include <tbd/sound_manager.hpp>
#include "esp_partition.h"
#include <tbd/heaps.hpp>
#include "esp_ota_ops.h"
#include <tbd/logging.hpp>


using namespace CTAG::OTA;

esp_err_t OTAManager::PostHandlerSPIFFS(httpd_req_t *req) {
    cleanup();

    TBD_LOGI("OTA", "Received SPIFFS OTA request");

    int total_len = req->content_len;
    TBD_LOGI("OTA", "Post request size %d", total_len);
    int cur_len = 0;
    largeBuf = (char *) tbd_heaps_malloc(total_len, TBD_HEAPS_SPIRAM);
    if (!largeBuf) {
        TBD_LOGE("OTA", "Could not allocate buffer in SPIRAM");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_ERR_NO_MEM;
    }

    int received = 0;

    while (cur_len < total_len) {
        received = httpd_req_recv(req, largeBuf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            cleanup();
            return ESP_FAIL;
        }
        cur_len += received;
        TBD_LOGI("OTA", "Receiving SPIFFS image %d out of %d bytes", cur_len, total_len);
    }

    spiffsImageSize = total_len;

    TBD_LOGI("OTA", "Successfully received SPIFFS image data.");
    hasMemSPIFFS = true;

    return ESP_OK;
}

esp_err_t OTAManager::PostHandlerApp(httpd_req_t *req) {

    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        TBD_LOGW("OTA",
                 "Configured OTA boot partition at offset 0x%08li, but running from offset 0x%08li",
                 configured->address, running->address);
        TBD_LOGW("OTA",
                 "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    TBD_LOGD("OTA", "Running partition type %i subtype %i (offset 0x%08li)",
             running->type, running->subtype, running->address);

    update_partition = esp_ota_get_next_update_partition(NULL);
    TBD_LOGD("OTA", "Writing to partition subtype %i at offset 0x%li",
             update_partition->subtype, update_partition->address);

    if (update_partition == NULL) {
        TBD_LOGE("OTA", "OTA update partition error!");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error OTA error!");
        cleanup();
        return ESP_ERR_OTA_BASE;
    }

    esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error OTA error!");
        TBD_LOGE("OTA", "esp_ota_begin failed (%s)", esp_err_to_name(err));
        cleanup();
        return err;
    }

    TBD_LOGI("OTA", "Update partition: %s",
             update_partition->label);
    TBD_LOGI("OTA", "Running partition: %s",
             running->label);

    TBD_LOGD("OTA", "Update partition size: %li",
             update_partition->size);
    TBD_LOGD("OTA", "Update partition address: 0x%li",
             update_partition->address);

    int binary_file_length = 0;
    // Deal with all receive packet

    int data_read, data_len_total, remaining = req->content_len;
    bool first_iteration = true;
    size_t counter = 0;
    char fw_buffer[4096] = {};
    while (42) {

        // Read the data for the request
        uint32_t size = remaining > 4096 ? 4096 : remaining;
        data_read = httpd_req_recv(req, fw_buffer, size);

        if (first_iteration) {
            data_len_total = remaining;
            first_iteration = false;
        }

        if (data_read < 0) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error OTA error!");
            TBD_LOGE("OTA", "Failed getting firmware binary");
            cleanup();
            return ESP_ERR_OTA_BASE;
        } else if (data_read > 0) {
            if (counter % 8 == 0) {
                TBD_LOGI("OTA", "Progress: %.1f %%",
                         100.0 * (float) binary_file_length / (float) data_len_total);
            }
            counter++;

            err = esp_ota_write(update_handle, fw_buffer, data_read);

            if (err != ESP_OK) {
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error OTA error!");
                TBD_LOGE("OTA", "Error writing FW %s", esp_err_to_name(err));
                cleanup();
                return err;
            }

            binary_file_length += data_read;
        } else if (data_read == 0) {
            TBD_LOGI("OTA", "Successfully received firmware image (%d bytes)",
                     binary_file_length);
            break;
        }

        remaining -= data_read;
    }

    if ((err = esp_ota_end(update_handle)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error OTA error!");
        TBD_LOGE("OTA", "Error ota end %s", esp_err_to_name(err));
        cleanup();
        return err;
    }

    if (esp_partition_check_identity(esp_ota_get_running_partition(),
                                     update_partition) == true) {
        TBD_LOGE("OTA", "Uploaded FW is same as old!");
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error OTA error!");
        TBD_LOGE("OTA", "Error ota set boot %s", esp_err_to_name(err));
        cleanup();
        return err;
    }

    httpd_resp_set_status(req, HTTPD_200);
    httpd_resp_send(req, NULL, 0);

    hasFlashAppImage = true;

    return ESP_OK;
}

esp_err_t OTAManager::PostHandlerFlashCommit(httpd_req_t *req) {
    if (!hasFlashAppImage || !hasMemSPIFFS) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error OTA flash!");
        return ESP_ERR_NOT_SUPPORTED;
    }
    if (flashSPIFFS() != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error OTA SPIFFS flash!");
        return ESP_ERR_NOT_SUPPORTED;
    }
    return ESP_OK;
}

void OTAManager::cleanup() {
    if (largeBuf) tbd_heaps_free(largeBuf);
    if (smallBuf) tbd_heaps_free(smallBuf);
}

esp_err_t OTAManager::flashSPIFFS() {
    TBD_LOGI("OTA", "Flashing SPIFFS");
    esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS,
                                                     "storage");
    if (pi == NULL) {
        TBD_LOGE("OTA", "Could not get partition iterator!");
        cleanup();
        return ESP_ERR_NOT_FOUND;
    }

    const esp_partition_t *p = esp_partition_get(pi);
    TBD_LOGI("OTA", "Found partition %s, size %08li, address %08li", p->label, p->size, p->address);

    if (p->size != spiffsImageSize) {
        TBD_LOGE("OTA", "SPIFFS image is incorrect size!");
        cleanup();
        esp_partition_iterator_release(pi);
        return ESP_ERR_FLASH_BASE;
    }

    TBD_LOGI("OTA", "Erasing...");
    if (esp_partition_erase_range(p, 0, p->size) != ESP_OK) {
        TBD_LOGE("OTA", "Error erasing SPIFFS flash partition!");
        cleanup();
        esp_partition_iterator_release(pi);
        return ESP_ERR_FLASH_BASE;
    }

    TBD_LOGI("OTA", "Flashing...");
    smallBuf = (char *) tbd_heaps_malloc(64 * 1024, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    if (smallBuf == NULL) {
        TBD_LOGE("OTA", "Could not allocate internal buffer");
        cleanup();
        esp_partition_iterator_release(pi);
        return ESP_ERR_NO_MEM;
    }

    uint32_t offset = 0;
    while (offset != spiffsImageSize) {
        int size = (spiffsImageSize - offset) > 64 * 1024 ? 64 * 1024 : spiffsImageSize - offset;
        // copy to internal mem as flash write and SPIRAM access are prohibited
        memcpy(smallBuf, largeBuf + offset, size);
        if (esp_partition_write(p, offset, smallBuf, size) != ESP_OK) {
            TBD_LOGE("OTA", "Error writing SPIFFS flash partition!");
            cleanup();
            esp_partition_iterator_release(pi);
            return ESP_ERR_FLASH_BASE;
        }
        offset += size;
        TBD_LOGI("OTA", "Flash writing, left %li", spiffsImageSize - offset);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    TBD_LOGI("OTA", "SPIFFS successfully written!");

    esp_partition_iterator_release(pi);
    cleanup();

    return ESP_OK;
}

esp_err_t OTAManager::InitiateOTA(httpd_req_t *req) {
    // stop audio task
    TBD_LOGI("OTA", "Initiating OTA, stopping audio task.");
    tbd::audio::SoundProcessorManager::KillAudioTask();
    return ESP_OK;
}

char *OTAManager::largeBuf = nullptr;
char *OTAManager::smallBuf = nullptr;
bool OTAManager::hasMemSPIFFS = false;
bool OTAManager::hasFlashAppImage = false;
uint32_t OTAManager::spiffsImageSize = 0;

