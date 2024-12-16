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
#include <tbd/api/common/rest_api.hpp>
#include <tbd/storage/resources.hpp>

/* HTTP Restful API Server

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <fcntl.h>
#include "esp_system.h"
#include "esp_http_server.h"
#include <tbd/heaps.hpp>
#include "esp_vfs.h"
#include <tbd/sound_manager.hpp>
#include <tbd/favorites.hpp>

// #include "OTAManager.hpp"
#include "../../../tbd_port_desktop/tbd_api_port/include/tbd/api/port/rest_api.hpp"

#include "sdkconfig.h"
#include "esp_flash.h"
#include <tbd/version.hpp>
#include <tbd/logging.hpp>

#if TBD_CALIBRATION
    #include <tbd/calibration.hpp>
#endif

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

namespace {

const char *REST_TAG = "esp-rest";

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath) {
    const char *type = "text/plain";
    auto is_html = false;

    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
        is_html = true;
    } else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    } else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "text/xml";
    }

    if (is_html) {
        httpd_resp_set_hdr(req, "Cache-Control", "max-age: 300, private");
    } else {
        httpd_resp_set_hdr(req, "Cache-Control", "max-age=31536000, immutable");
    }

    return httpd_resp_set_type(req, type);
}

static esp_err_t send_file(const char *filepath, httpd_req_t *req, rest_server_context_t *rest_context) {
    ESP_LOGD(REST_TAG, "Serving file: %s", filepath);
    const int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        TBD_LOGE(REST_TAG, "Failed to open file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;// = 10240;
    do {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            TBD_LOGE(REST_TAG, "Failed to read file : %s", filepath);
        } else if (read_bytes > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                TBD_LOGE(REST_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, nullptr);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    TBD_LOGD(REST_TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, nullptr, 0);
    return ESP_OK;
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req) {
    char filepath[FILE_PATH_MAX];

    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    auto *rest_context = static_cast<rest_server_context_t *>(req->user_ctx);
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));
    strlcat(filepath, req->uri, sizeof(filepath));
    if (req->uri[strlen(req->uri) - 1] == '/') {
        strlcat(filepath, "index.html", sizeof(filepath));
    }
    set_content_type_from_file(req, filepath);
    strlcat(filepath, ".gz", sizeof(filepath));

    return send_file(filepath, req, rest_context);
}

esp_err_t get_plugins_get_handler(httpd_req_t *req) {
    ESP_LOGD("get_plugins_get_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    httpd_resp_set_type(req, "application/json");
    const char* res = tbd::audio::SoundProcessorManager::GetCStrJSONSoundProcessors();
    if(nullptr != res) httpd_resp_sendstr(req, res);
    return ESP_OK;
}

esp_err_t get_active_plugin_get_handler(httpd_req_t *req) {
    TBD_LOGD("get_active_plugin_get_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             tbd_heaps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_free_size(TBD_HEAPS_SPIRAM),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_SPIRAM));
    size_t qlen = httpd_req_get_url_query_len(req);
    size_t urilen = strlen(req->uri);
    char ch = req->uri[urilen - qlen - 1];
    ch -= 0x30;
    TBD_LOGD(REST_TAG, "Get active plugin for channel %d", ch);
    string res;
    if (ch == 0 || ch == 1) {
        res = "{\"id\":\"" + tbd::audio::SoundProcessorManager::GetStringID(ch) + "\"}";
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, res.c_str());
    return ESP_OK;
}

esp_err_t get_params_ui_plugin_get_handler(httpd_req_t *req) {
    ESP_LOGD("get_params_plugin_get_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    size_t qlen = httpd_req_get_url_query_len(req);
    size_t urilen = strlen(req->uri);
    char ch = req->uri[urilen - qlen - 1];
    ch -= 0x30;

    if (ch != 0 && ch != 1) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    httpd_resp_set_type(req, "application/json");
    auto plugin = tbd::audio::SoundProcessorManager::GetStringID(ch);
    ESP_LOGD(REST_TAG, "Get plugin params ui for channel %d and plugin %s", ch, plugin.c_str());

    auto *rest_context = static_cast<rest_server_context_t *>(req->user_ctx);

    auto plugin_config_path = tbd::storage::get_fs_path(std::format("/data/sp/mp-{}.json", plugin));
    if (!plugin_config_path) {
        httpd_resp_send_404(req);
        return ESP_OK;
    }
    return send_file(plugin_config_path->c_str(), req, rest_context);
}

esp_err_t get_params_p_plugin_get_handler(httpd_req_t *req) {
    ESP_LOGD("get_params_plugin_get_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    size_t qlen = httpd_req_get_url_query_len(req);
    size_t urilen = strlen(req->uri);
    char ch = req->uri[urilen - qlen - 1];
    ch -= 0x30;

    if (ch != 0 && ch != 1) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    httpd_resp_set_type(req, "application/json");
    auto plugin = tbd::audio::SoundProcessorManager::GetStringID(ch);
    ESP_LOGD(REST_TAG, "Get plugin params ui for channel %d and plugin %s", ch, plugin.c_str());

    auto *rest_context = static_cast<rest_server_context_t *>(req->user_ctx);
    auto plugin_config_path = tbd::storage::get_fs_path(std::format("/data/sp/mp-{}.json", plugin));
    if (!plugin_config_path) {
        httpd_resp_send_404(req);
        return ESP_OK;
    }
    return send_file(plugin_config_path->c_str(), req, rest_context);
}

esp_err_t get_params_plugin_get_handler(httpd_req_t *req) {
    ESP_LOGD("get_params_plugin_get_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    size_t qlen = httpd_req_get_url_query_len(req);
    size_t urilen = strlen(req->uri);
    char ch = req->uri[urilen - qlen - 1];
    httpd_resp_set_type(req, "application/json");
    ch -= 0x30;
    TBD_LOGD(REST_TAG, "Get plugin params for channel %d", ch);
    if (ch == 0 || ch == 1){
        const char *res = tbd::audio::SoundProcessorManager::GetCStrJSONActivePluginParams(ch);
        if(nullptr != res) httpd_resp_sendstr(req, res);
    }

    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t set_active_plugin_get_handler(httpd_req_t *req) {
    TBD_LOGD("set_active_plugin_get_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             tbd_heaps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_free_size(TBD_HEAPS_SPIRAM),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_SPIRAM));
    char s[128];
    char v[128];
    size_t qlen = httpd_req_get_url_query_len(req);
    size_t urilen = strlen(req->uri);
    char ch = req->uri[urilen - qlen - 2];
    httpd_resp_set_type(req, "application/json");
    httpd_req_get_url_query_str(req, s, 128);
    httpd_query_key_value(s, "id", v, 128);
    std::string id(v);
    ch -= 0x30;
    TBD_LOGD(REST_TAG, "Set active plugin for channel %d %s %s", ch, v, s);
    if (ch == 0 || ch == 1){
        tbd::audio::SoundProcessorManager::SetSoundProcessorChannel(ch, id);
        tbd::Favorites::DeactivateFavorite();
    }

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, 0, 0);
    return ESP_OK;
}

esp_err_t set_plugin_param_get_handler(httpd_req_t *req) {
    TBD_LOGD("set_plugin_param_get_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             tbd_heaps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_free_size(TBD_HEAPS_SPIRAM),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_SPIRAM));
    char query[128];
    char id[128];
    char cstrvalue[128];
    string key("");
    int val = 0;
    size_t qlen = httpd_req_get_url_query_len(req);
    size_t urilen = strlen(req->uri);
    httpd_req_get_url_query_str(req, query, 128);
    httpd_query_key_value(query, "id", id, 128);
    char ch = req->uri[urilen - qlen - 2];
    if (strstr(req->uri, "TRIG")) {
        httpd_query_key_value(query, "trig", cstrvalue, 128);
        key = "trig";
    } else if (strstr(req->uri, "CV")) {
        httpd_query_key_value(query, "cv", cstrvalue, 128);
        key = "cv";
    } else {
        httpd_query_key_value(query, "current", cstrvalue, 128);
        key = "current";
    }
    val = atoi(cstrvalue);
    std::string sid(id);
    ch -= 0x30;
    TBD_LOGD(REST_TAG, "Setting chan %d param %s key %s value %d", ch, id, key.c_str(), val);
    if (ch == 0 || ch == 1)
        tbd::audio::SoundProcessorManager::SetChannelParamValue(ch, sid, key, val);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t get_presets_get_handler(httpd_req_t *req) {
    char query[128];
    size_t qlen = httpd_req_get_url_query_len(req);
    size_t urilen = strlen(req->uri);
    httpd_req_get_url_query_str(req, query, 128);
    char ch = req->uri[urilen - qlen - 1];
    httpd_resp_set_type(req, "application/json");
    ch -= 0x30;
    ESP_LOGD(REST_TAG, "Querying presets for channel %d", ch);
    if (ch == 0){
        const char* res = tbd::audio::SoundProcessorManager::GetCStrJSONGetPresets(ch);
        if(nullptr != res) httpd_resp_sendstr(req, res);
    }
    if (ch == 1 && !tbd::audio::SoundProcessorManager::is_stereo(0)){
        const char* res = tbd::audio::SoundProcessorManager::GetCStrJSONGetPresets(ch);
        if(nullptr != res) httpd_resp_sendstr(req, res);
    }

    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t save_preset_get_handler(httpd_req_t *req) {
    TBD_LOGD("save_preset_get_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             tbd_heaps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_free_size(TBD_HEAPS_SPIRAM),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_SPIRAM));
    char query[128];
    char name[128];
    char number[16];
    size_t qlen = httpd_req_get_url_query_len(req);
    size_t urilen = strlen(req->uri);
    httpd_req_get_url_query_str(req, query, 128);
    httpd_query_key_value(query, "name", name, 128);
    httpd_query_key_value(query, "number", number, 16);
    char ch = req->uri[urilen - qlen - 2];
    ch -= 0x30;
    TBD_LOGD(REST_TAG, "Store preset for channel %s %d", req->uri, ch);
    if (ch == 0 || ch == 1)
        tbd::audio::SoundProcessorManager::ChannelSavePreset(ch, string(name), atoi(number));
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t load_preset_get_handler(httpd_req_t *req) {
    TBD_LOGD("load_preset_get_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             tbd_heaps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_free_size(TBD_HEAPS_SPIRAM),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_SPIRAM));
    char query[128];
    char number[16];
    size_t qlen = httpd_req_get_url_query_len(req);
    size_t urilen = strlen(req->uri);
    httpd_req_get_url_query_str(req, query, 128);
    httpd_query_key_value(query, "number", number, 16);
    char ch = req->uri[urilen - qlen - 2];
    ch -= 0x30;
    TBD_LOGD("HTTPD", "Load preset for channel %s %c", req->uri, ch);
    if (ch == 0 || ch == 1){
        tbd::audio::SoundProcessorManager::ChannelLoadPreset(ch, atoi(number));
        tbd::Favorites::DeactivateFavorite();
    }
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t set_configuration_post_handler(httpd_req_t *req) {
    TBD_LOGD("set_configuration_post_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             tbd_heaps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_free_size(TBD_HEAPS_SPIRAM),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_SPIRAM));
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    char *content = (char *) tbd_heaps_malloc(req->content_len + 1, TBD_HEAPS_SPIRAM);
    int ret = httpd_req_recv(req, content, req->content_len);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }
    content[req->content_len] = 0;
    tbd::audio::SoundProcessorManager::SetConfigurationFromJSON(string(content));
    free(content);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t get_configuration_get_handler(httpd_req_t *req) {
    TBD_LOGD("get_configuration_get_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             tbd_heaps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_free_size(TBD_HEAPS_SPIRAM),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_SPIRAM));
    httpd_resp_set_type(req, "application/json");
    const char *res = tbd::audio::SoundProcessorManager::GetCStrJSONConfiguration();
    if(nullptr != res) httpd_resp_sendstr(req, res);
    return ESP_OK;
}

esp_err_t get_preset_json_handler(httpd_req_t *req) {
    TBD_LOGD("get_configuration_get_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             tbd_heaps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_free_size(TBD_HEAPS_SPIRAM),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_SPIRAM));
    char query[128];
    char pluginID[64];
    httpd_req_get_url_query_str(req, query, 128);
    httpd_resp_set_type(req, "application/json");
    char *pLastSlash = strrchr(req->uri, '/');
    if (pLastSlash) {
        strcpy(pluginID, pLastSlash + 1);
        TBD_LOGD(REST_TAG, "Sending all preset data of plugin %s as JSON", pluginID);
        const char *json = tbd::audio::SoundProcessorManager::GetCStrJSONSoundProcessorPresets(string(pluginID));
        if (nullptr != json)
            httpd_resp_sendstr(req, json);
    }
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t reboot_handler(httpd_req_t *req) {
    char query[128];
    httpd_req_get_url_query_str(req, query, 128);
#if TDB_CALIBRATION
    char calibration[16];
    httpd_query_key_value(query, "calibration", calibration, 16);
    int doCal = atoi(calibration);
    TBD_LOGW(REST_TAG, "Reboot requested with calibration = %d", doCal);
    if (doCal) CTAG::CAL::Calibration::RequestCalibrationOnReboot();
#endif
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, NULL, 0);
    esp_restart();
    return ESP_OK;
}

esp_err_t get_calibration_get_handler(httpd_req_t *req) {
    TBD_LOGD("get_calibration_get_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             tbd_heaps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_free_size(TBD_HEAPS_SPIRAM),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_SPIRAM));
    httpd_resp_set_type(req, "application/json");

#if TDB_CALIBRATION
    const char* res = CTAG::CAL::Calibration::GetCStrJSONCalibration();
    if(nullptr != res) httpd_resp_sendstr(req, res);
#else
    httpd_resp_sendstr(req, "{}");
#endif
    return ESP_OK;
}

// FIXME: make OTA updates available

// esp_err_t ota_handler(httpd_req_t *req) {
//     static int lastOtaRequest = 0;
//     size_t qlen = httpd_req_get_url_query_len(req);
//     size_t urilen = strlen(req->uri);
//     char otaRequest = req->uri[urilen - qlen - 1];
//     otaRequest -= 0x30;
//     esp_err_t err = ESP_ERR_NOT_FOUND;
//     TBD_LOGI("HTTPD", "OTA request type %d, last request was %d, expecting %d", otaRequest, lastOtaRequest,
//              lastOtaRequest + 1);
//     // stage 1, kill audio task and bring into ota update mode
//     if (lastOtaRequest == 0 && otaRequest == 1) {
//         CTAG::OTA::OTAManager::InitiateOTA(req);
//         lastOtaRequest++;
//         err = ESP_OK;
//     }
//     // stage 2, upload SPIFFS image
//     if (lastOtaRequest == 1 && otaRequest == 2) {
//         err = CTAG::OTA::OTAManager::PostHandlerSPIFFS(req);
//         lastOtaRequest++;
//     }
//     // stage 3, upload Flash image
//     if (lastOtaRequest == 2 && otaRequest == 3) {
//         err = CTAG::OTA::OTAManager::PostHandlerApp(req);
//         lastOtaRequest++;
//     }
//     // stage 4, upload Flash image
//     if (lastOtaRequest == 3 && otaRequest == 4) {
//         err = CTAG::OTA::OTAManager::PostHandlerFlashCommit(req);
//         if (err == ESP_OK) {
//             TBD_LOGI("REST", "OTA successful, rebooting!");
//             httpd_resp_set_type(req, "text/html");
//             httpd_resp_send(req, NULL, 0);
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//             esp_restart();
//         }
//     }
//
//     if (err != ESP_OK) {
//         httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error OTA error!");
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//         esp_restart();
//     }
//     httpd_resp_set_type(req, "text/html");
//     httpd_resp_send(req, NULL, 0);
//
//     return ESP_OK;
// }

esp_err_t set_calibration_post_handler(httpd_req_t *req) {
    TBD_LOGD("set_calibration_post_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             tbd_heaps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_free_size(TBD_HEAPS_SPIRAM),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_SPIRAM));
    TBD_LOGI("REST", "Set calibration post handler: content length %d", req->content_len);
    char *content = (char *) tbd_heaps_malloc(req->content_len + 1, TBD_HEAPS_SPIRAM);
    int ret = httpd_req_recv(req, content, req->content_len);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }
    content[req->content_len] = 0;
#if TBD_CALIBRATION
    tbd::Calibration::SetJSONCalibration(string(content));
#endif
    free(content);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t set_preset_json_handler(httpd_req_t *req) {
    char query[128];
    char pluginID[64];
    memset(query, 0, 128);
    memset(pluginID, 0, 64);
    httpd_req_get_url_query_str(req, query, 128);
    char *pLastSlash = strrchr(req->uri, '/');
    if (pLastSlash) {
        strcpy(pluginID, pLastSlash + 1);
        char *content = (char *) tbd_heaps_calloc(1, req->content_len + 1, TBD_HEAPS_SPIRAM);
        TBD_LOGD(REST_TAG, "Storing data for %s as JSON, content length %d", pluginID, req->content_len);
        char *ptrContent = content;
        // get chunked data
        int remaining = req->content_len;
        while(remaining > 0){
            uint32_t size = remaining > 4096 ? 4096 : remaining;
            int data_read = httpd_req_recv(req, ptrContent, size);
            if (data_read <= 0) {  /* 0 return value indicates connection closed */
                /* Check if timeout occurred */
                if (data_read == HTTPD_SOCK_ERR_TIMEOUT) {
                    /* In case of timeout one can choose to retry calling
                     * httpd_req_recv(), but to keep it simple, here we
                     * respond with an HTTP 408 (Request Timeout) error */
                    httpd_resp_send_408(req);
                }else{
                    httpd_resp_send_500(req);
                }
                /* In case of error, returning ESP_FAIL will
                 * ensure that the underlying socket is closed */
                tbd_heaps_free(content);
                return ESP_FAIL;
            }
            ptrContent += data_read;
            remaining -= data_read;
        }
        // terminate c string with \0
        content[req->content_len] = 0;
        TBD_LOGD("REST", "Plugin %s content read %s", pluginID, content);
        // respond to client
        string response = "{\"id\":\"" + string(pluginID) + "\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, response.c_str());
        httpd_resp_send(req, NULL, 0);
        // call up and persist
        tbd::audio::SoundProcessorManager::SetCStrJSONSoundProcessorPreset(pluginID, content);
        // clear up
        tbd_heaps_free(content);
    } else {
        httpd_resp_send_404(req);
    }
    return ESP_OK;
}

esp_err_t srom_handler(httpd_req_t *req) {
    char *s = strrchr(req->uri, '/');
    string cmd = ++s;

    TBD_LOGE("REST", "Sample ROM command: %s", cmd.c_str());

    if(cmd.compare("getSize") == 0){
        httpd_resp_set_type(req, "text/plain");
        httpd_resp_sendstr(req, to_string(CONFIG_SAMPLE_ROM_SIZE).c_str());
        return ESP_OK;
    }

    if(cmd.compare("erase") == 0){
        tbd::audio::SoundProcessorManager::DisablePluginProcessing();
        tbd::Favorites::DisableFavoritesUI();
        // erase flash / lengthy operation
        TBD_LOGI("REST", "Erasing flash start %d, size %d!", CONFIG_SAMPLE_ROM_START_ADDRESS, CONFIG_SAMPLE_ROM_SIZE);
        //ESP_ERROR_CHECK(spi_flash_erase_range(CONFIG_SAMPLE_ROM_START_ADDRESS, CONFIG_SAMPLE_ROM_SIZE));
        ESP_ERROR_CHECK(esp_flash_erase_region(NULL, CONFIG_SAMPLE_ROM_START_ADDRESS, CONFIG_SAMPLE_ROM_SIZE));
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, NULL, 0);
        tbd::Favorites::EnableFavoritesUI();
        tbd::audio::SoundProcessorManager::EnablePluginProcessing();
        return ESP_OK;
    }

    if(cmd.compare("upRaw") == 0){
        TBD_LOGI("REST", "Sample ROM flashing!");
        int data_read, remaining = req->content_len, offset = 0;
        char *buffer = (char*)tbd_heaps_malloc(4096, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if(buffer == NULL){
            httpd_resp_send_500(req);
            return ESP_ERR_NO_MEM;
        }
        tbd::audio::SoundProcessorManager::DisablePluginProcessing();
        tbd::Favorites::DisableFavoritesUI();
        int blockCnt = 0;
        while (remaining > 0) {
            // Read the data for the request
            uint32_t size = remaining > 4096 ? 4096 : remaining;
            data_read = httpd_req_recv(req, buffer, size);
            if (data_read < 0) {
                httpd_resp_send_500(req);
                tbd_heaps_free(buffer);
                tbd::Favorites::EnableFavoritesUI();
                tbd::audio::SoundProcessorManager::EnablePluginProcessing();
                tbd_heaps_free(buffer);
                return ESP_ERR_INVALID_ARG;
            } else if (data_read > 0) {
                //spi_flash_write(CONFIG_SAMPLE_ROM_START_ADDRESS + offset, buffer, data_read);
                esp_flash_write(NULL, buffer, CONFIG_SAMPLE_ROM_START_ADDRESS + offset, data_read);
            }
            offset += data_read;
            remaining -= data_read;
            if(blockCnt == 0){
                if(((uint32_t*)buffer)[0] != 0xdeadface){
                    TBD_LOGE("REST", "Not a valid sample rom file!");
                    httpd_resp_send_500(req);
                    tbd_heaps_free(buffer);
                    tbd::Favorites::EnableFavoritesUI();
                    tbd::audio::SoundProcessorManager::EnablePluginProcessing();
                    tbd_heaps_free(buffer);
                    return ESP_ERR_INVALID_ARG;
                }
            }
            blockCnt++;
        }
        tbd_heaps_free(buffer);
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, NULL, 0);
        tbd::audio::SoundProcessorManager::RefreshSampleRom();
        tbd::Favorites::EnableFavoritesUI();
        tbd::audio::SoundProcessorManager::EnablePluginProcessing();
        TBD_LOGI("REST", "Sample ROM flashing completed!");
        return ESP_OK;
    }

    httpd_resp_send_404(req);

    return ESP_OK;
}

// transmit io capabilities
esp_err_t get_iocaps_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, tbd::sysinfo::device_capabilities.c_str());
    return ESP_OK;
}

esp_err_t favorite_post_handler(httpd_req_t *req) {
    TBD_LOGD("favorite_post_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             tbd_heaps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             tbd_heaps_get_free_size(TBD_HEAPS_SPIRAM),
             tbd_heaps_get_largest_free_block(TBD_HEAPS_SPIRAM));
    string cmd{req->uri};

    TBD_LOGD("REST", "Favorite handler cmd: %s", cmd.c_str());

    if(cmd.rfind("getAll") != string::npos){
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, tbd::Favorites::GetAllFavorites().c_str());
        return ESP_OK;
    }

    if(cmd.rfind("store") != string::npos){
        char *content = (char *) tbd_heaps_malloc(req->content_len + 1, TBD_HEAPS_SPIRAM);
        int ret = httpd_req_recv(req, content, req->content_len);
        if (ret <= 0) {  /* 0 return value indicates connection closed */
            /* Check if timeout occurred */
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* In case of timeout one can choose to retry calling
                 * httpd_req_recv(), but to keep it simple, here we
                 * respond with an HTTP 408 (Request Timeout) error */
                httpd_resp_send_408(req);
            }
            /* In case of error, returning ESP_FAIL will
             * ensure that the underlying socket is closed */
            tbd_heaps_free(content);
            return ESP_FAIL;
        }
        content[req->content_len] = 0;
        // call upstream API here
        int id = stoi((cmd.substr(cmd.length() - 1, 1)));
        tbd::Favorites::StoreFavorite(id, string(content));
        free(content);
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, NULL, 0);
        return ESP_OK;
    }

    if(cmd.rfind("recall") != string::npos){
        // call upstream API here
        int id = stoi((cmd.substr(cmd.length() - 1, 1)));
        tbd::Favorites::ActivateFavorite(id);
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, NULL, 0);
        return ESP_OK;
    }

    httpd_resp_send_404(req);

    return ESP_OK;
}

esp_err_t register_handlers() {
    const char *base_path = "/spiffs/www\0";
    rest_server_context_t *rest_context = (rest_server_context_t *) calloc(1, sizeof(rest_server_context_t));
    assert(rest_context);
    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.core_id = 0;
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.task_priority = tskIDLE_PRIORITY + 4;
    config.max_uri_handlers = 22;
    config.stack_size = 8192;
    config.recv_wait_timeout   = 20;
    config.send_wait_timeout = 20;
    /*
    config.max_open_sockets   = 10;
    config.max_resp_headers   = 10;

    config.backlog_conn       = 10;
    config.lru_purge_enable   = false;
     */
    /*
    #define HTTPD_DEFAULT_CONFIG() {                        \
            .task_priority      = tskIDLE_PRIORITY+5,       \
            .stack_size         = 4096,                     \
            .core_id            = tskNO_AFFINITY,           \
            .server_port        = 80,                       \
            .ctrl_port          = 32768,                    \
            .max_open_sockets   = 7,                        \
            .max_uri_handlers   = 8,                        \
            .max_resp_headers   = 8,                        \
            .backlog_conn       = 5,                        \
            .lru_purge_enable   = false,                    \
            .recv_wait_timeout  = 5,                        \
            .send_wait_timeout  = 5,                        \
            .global_user_ctx = NULL,                        \
            .global_user_ctx_free_fn = NULL,                \
            .global_transport_ctx = NULL,                   \
            .global_transport_ctx_free_fn = NULL,           \
            .open_fn = NULL,                                \
            .close_fn = NULL,                               \
            .uri_match_fn = NULL                            \
    }
    */
    TBD_LOGI(REST_TAG, "Starting HTTP Server");
    if (httpd_start(&server, &config) != ESP_OK)
        return ESP_FAIL;

    /* URI handler for getting available sound processors */
    httpd_uri_t get_plugins_get_uri = {
        .uri = "/api/v1/getPlugins",
        .method = HTTP_GET,
        .handler = &get_plugins_get_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &get_plugins_get_uri);

    /* get currently activated plugin */
    httpd_uri_t get_active_plugin_get_uri = {
        .uri = "/api/v1/getActivePlugin*",
        .method = HTTP_GET,
        .handler = &get_active_plugin_get_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &get_active_plugin_get_uri);

    /* get plugin params of active plugin */
    httpd_uri_t get_params_ui_plugin_get_uri = {
        .uri = "/api/v1/getPluginParamsUI/*",
        .method = HTTP_GET,
        .handler = &get_params_plugin_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &get_params_ui_plugin_get_uri);

    /* get plugin params of active plugin */
    httpd_uri_t get_params_p_plugin_get_uri = {
        .uri = "/api/v1/getPluginParamsP/*",
        .method = HTTP_GET,
        .handler = &get_params_plugin_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &get_params_p_plugin_get_uri);

    /* get plugin params of active plugin */
    httpd_uri_t get_params_plugin_get_uri = {
        .uri = "/api/v1/getPluginParams*",
        .method = HTTP_GET,
        .handler = &get_params_plugin_get_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &get_params_plugin_get_uri);

    /* set active plugin */
    httpd_uri_t set_active_plugin_get_uri = {
        .uri = "/api/v1/setActivePlugin*",
        .method = HTTP_GET,
        .handler = &set_active_plugin_get_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &set_active_plugin_get_uri);

    /* set a plugin param*/
    httpd_uri_t set_plugin_param_get_uri = {
        .uri = "/api/v1/setPluginParam*",
        .method = HTTP_GET,
        .handler = &set_plugin_param_get_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &set_plugin_param_get_uri);

    /* get all presets */
    httpd_uri_t get_presets_get_uri = {
        .uri = "/api/v1/getPresets*",
        .method = HTTP_GET,
        .handler = &get_presets_get_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &get_presets_get_uri);

    /* get all presets */
    httpd_uri_t get_preset_json_get_uri = {
        .uri = "/api/v1/getPresetData*",
        .method = HTTP_GET,
        .handler = &get_preset_json_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &get_preset_json_get_uri);

    /* save a preset */
    httpd_uri_t save_preset_get_uri = {
        .uri = "/api/v1/savePreset*",
        .method = HTTP_GET,
        .handler = &save_preset_get_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &save_preset_get_uri);

    /* load a preset */
    httpd_uri_t load_preset_get_uri = {
        .uri = "/api/v1/loadPreset*",
        .method = HTTP_GET,
        .handler = &load_preset_get_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &load_preset_get_uri);

    /* get configuration*/
    httpd_uri_t get_configuration_get_uri = {
        .uri = "/api/v1/getConfiguration",
        .method = HTTP_GET,
        .handler = &get_configuration_get_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &get_configuration_get_uri);

    /* get configuration*/
    httpd_uri_t get_calibration_get_uri = {
        .uri = "/api/v1/getCalibration",
        .method = HTTP_GET,
        .handler = &get_calibration_get_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &get_calibration_get_uri);

    /* reboot with and without calibration request */
    httpd_uri_t reboot_handler_get_uri = {
        .uri = "/api/v1/reboot",
        .method = HTTP_GET,
        .handler = &reboot_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &reboot_handler_get_uri);

    /* get io caps */
    httpd_uri_t io_caps_handler_get_uri = {
        .uri = "/api/v1/getIOCaps",
        .method = HTTP_GET,
        .handler = &get_iocaps_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &io_caps_handler_get_uri);

    /* set configuration */
    httpd_uri_t set_configuration_post_uri = {
        .uri = "/api/v1/setConfiguration",
        .method = HTTP_POST,
        .handler = &set_configuration_post_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &set_configuration_post_uri);


    /* set calibration */
    httpd_uri_t set_calibration_post_uri = {
        .uri = "/api/v1/setCalibration*",
        .method = HTTP_POST,
        .handler = &set_calibration_post_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &set_calibration_post_uri);


    httpd_uri_t set_preset_data_post_uri = {
        .uri = "/api/v1/setPresetData*",
        .method = HTTP_POST,
        .handler = &set_preset_json_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &set_preset_data_post_uri);

    // FIXME: ota api
    // /* set spiffs upload */
    // httpd_uri_t ota_post_uri = {
    //     .uri = "/api/v1/otaAPI*",
    //     .method = HTTP_POST,
    //     .handler = &ota_handler,
    //     .user_ctx = rest_context
    // };

    // httpd_register_uri_handler(server, &ota_post_uri);

    /* favorite handler*/
    httpd_uri_t favorite_get_uri = {
        .uri = "/api/v1/favorite*",
        .method = HTTP_POST,
        .handler = &favorite_post_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &favorite_get_uri);

    /* set sample upload */
    httpd_uri_t srom_post_uri = {
        .uri = "/api/v1/srom*",
        .method = HTTP_POST,
        .handler = &srom_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &srom_post_uri);

    /* URI handler for getting web server files */
    httpd_uri_t common_get_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = rest_common_get_handler,
        .user_ctx = rest_context
};
    httpd_register_uri_handler(server, &common_get_uri);

    return ESP_OK;
}

}

namespace tbd::api {

void RestApi::begin(const RestApiParams&) {
    register_handlers();
}

void RestApi::end() {

}



}