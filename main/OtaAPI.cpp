/***************
TBD-16 — dadamachines WebUI & REST API

(c) 2024-2026 Johannes Elias Lohbihler for dadamachines.

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

Part of the dadamachines additions to the CTAG TBD platform.
See LICENSE in the repository root for full terms.

Provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "OtaAPI.hpp"
#include "SPManager.hpp"
#include <cstring>
#include <cstdio>
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_app_format.h"
#include "esp_heap_caps.h"

using namespace CTAG::REST;

static const char *TAG = "OtaAPI";

/* ── GET /api/v2/ota ─────────────────────────────────────────────── */

esp_err_t OtaAPI::ota_get_handler(httpd_req_t *req) {
    const esp_partition_t *running = esp_ota_get_running_partition();
    const esp_partition_t *next    = esp_ota_get_next_update_partition(NULL);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Connection", "close");

    char buf[256];
    snprintf(buf, sizeof(buf),
             "{\"running\":\"%s\",\"next\":\"%s\",\"maxSize\":%lu}",
             running ? running->label : "unknown",
             next    ? next->label    : "unknown",
             next    ? (unsigned long)next->size : 0UL);
    httpd_resp_sendstr(req, buf);
    return ESP_OK;
}

/* ── POST /api/v2/ota ────────────────────────────────────────────── */

esp_err_t OtaAPI::ota_post_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "OTA firmware upload started, content_len=%d", req->content_len);

    httpd_resp_set_hdr(req, "Connection", "close");

    /* ── Sanity checks ── */
    if (req->content_len <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No firmware data");
        return ESP_FAIL;
    }

    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    if (!update_partition) {
        ESP_LOGE(TAG, "No OTA update partition found");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "No OTA update partition available");
        return ESP_FAIL;
    }

    if ((size_t)req->content_len > update_partition->size) {
        ESP_LOGE(TAG, "Firmware too large: %d > %lu",
                 req->content_len, (unsigned long)update_partition->size);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Firmware image too large");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Writing to partition '%s' @ 0x%lx, size %lu",
             update_partition->label,
             (unsigned long)update_partition->address,
             (unsigned long)update_partition->size);

    /* ── Stop audio processing for a clean flash ── */
    CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();

    /* ── Begin OTA ── */
    esp_ota_handle_t ota_handle = 0;
    esp_err_t err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed: %s", esp_err_to_name(err));
        CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "OTA begin failed");
        return ESP_FAIL;
    }

    /* ── Receive and write firmware in chunks ──
     * Use a large PSRAM buffer (64 KB) to decouple network receive from
     * flash writes.  esp_ota_write() triggers flash sector erases that
     * can stall for 20-100 ms — during which the TCP receive window would
     * fill and cause connection resets on a fast USB-NCM link.  By
     * batching receives into a larger buffer we keep the TCP pipe open
     * while flash is busy.
     */
    static constexpr size_t OTA_BUF_SIZE = 64 * 1024;
    char *buf = (char *)heap_caps_malloc(OTA_BUF_SIZE, MALLOC_CAP_SPIRAM);
    if (!buf) {
        ESP_LOGE(TAG, "Failed to allocate OTA receive buffer from PSRAM");
        esp_ota_abort(ota_handle);
        CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Out of memory for OTA buffer");
        return ESP_FAIL;
    }

    int remaining = req->content_len;
    int received_total = 0;
    bool header_checked = false;

    while (remaining > 0) {
        /* Fill the PSRAM buffer with as much network data as available */
        int buf_filled = 0;
        int buf_target = remaining > (int)OTA_BUF_SIZE ? (int)OTA_BUF_SIZE : remaining;

        while (buf_filled < buf_target) {
            int to_read = buf_target - buf_filled;
            int data_read = httpd_req_recv(req, buf + buf_filled, to_read);

            if (data_read < 0) {
                if (data_read == HTTPD_SOCK_ERR_TIMEOUT) {
                    /* Retry on timeout */
                    continue;
                }
                ESP_LOGE(TAG, "Receive error at byte %d", received_total + buf_filled);
                heap_caps_free(buf);
                esp_ota_abort(ota_handle);
                CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                    "Firmware receive failed");
                return ESP_FAIL;
            }

            if (data_read == 0) {
                ESP_LOGE(TAG, "Connection closed at byte %d/%d",
                         received_total + buf_filled, req->content_len);
                heap_caps_free(buf);
                esp_ota_abort(ota_handle);
                CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                    "Connection closed during upload");
                return ESP_FAIL;
            }

            buf_filled += data_read;
        }

        /* Validate ESP-IDF app header on first chunk */
        if (!header_checked && buf_filled >= (int)sizeof(esp_image_header_t)) {
            esp_image_header_t *hdr = (esp_image_header_t *)buf;
            if (hdr->magic != ESP_IMAGE_HEADER_MAGIC) {
                ESP_LOGE(TAG, "Invalid firmware header magic: 0x%02X (expected 0x%02X)",
                         hdr->magic, ESP_IMAGE_HEADER_MAGIC);
                heap_caps_free(buf);
                esp_ota_abort(ota_handle);
                CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
                httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                                    "Invalid firmware binary (bad header magic)");
                return ESP_FAIL;
            }
            header_checked = true;
        }

        /* Flush the full buffer to flash in one write */
        err = esp_ota_write(ota_handle, buf, buf_filled);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_write failed at byte %d: %s",
                     received_total, esp_err_to_name(err));
            heap_caps_free(buf);
            esp_ota_abort(ota_handle);
            CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Flash write failed");
            return ESP_FAIL;
        }

        received_total += buf_filled;
        remaining -= buf_filled;

        /* Log progress every buffer flush (~64 KB) */
        ESP_LOGI(TAG, "Progress: %d / %d bytes (%.0f%%)",
                 received_total, req->content_len,
                 100.0f * received_total / req->content_len);
    }

    heap_caps_free(buf);

    ESP_LOGI(TAG, "Upload complete: %d bytes received", received_total);

    /* ── Finalize OTA ── */
    err = esp_ota_end(ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed: %s", esp_err_to_name(err));
        CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "OTA finalize failed (image validation error?)");
        return ESP_FAIL;
    }

    /* ── Set boot partition to the newly flashed image ── */
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed: %s", esp_err_to_name(err));
        CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to set boot partition");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "OTA successful! New firmware in '%s'. Awaiting reboot.",
             update_partition->label);

    CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();

    /* ── Respond with success JSON ── */
    httpd_resp_set_type(req, "application/json");
    char resp[128];
    snprintf(resp, sizeof(resp),
             "{\"success\":true,\"partition\":\"%s\",\"size\":%d}",
             update_partition->label, received_total);
    httpd_resp_sendstr(req, resp);

    return ESP_OK;
}
