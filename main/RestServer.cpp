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


/* HTTP Restful API Server

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "RestServer.hpp"
#include <string.h>
#include <fcntl.h>
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "cJSON.h"
#include "SPManager.hpp"
#include "Calibration.hpp"


using namespace CTAG;
using namespace CTAG::REST;


static const char *REST_TAG = "esp-rest";

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
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
    return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));
    if (req->uri[strlen(req->uri) - 1] == '/') {
        strlcat(filepath, "/index.html", sizeof(filepath));
    } else {
        strlcat(filepath, req->uri, sizeof(filepath));
    }
    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        ESP_LOGE(REST_TAG, "Failed to open file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    set_content_type_from_file(req, filepath);

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;// = 10240;
    do {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            ESP_LOGE(REST_TAG, "Failed to read file : %s", filepath);
        } else if (read_bytes > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGD(REST_TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t RestServer::get_plugins_get_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, CTAG::AUDIO::SoundProcessorManager::GetCStrJSONSoundProcessors());
    return ESP_OK;
}

esp_err_t RestServer::get_active_plugin_get_handler(httpd_req_t *req){
    size_t qlen = httpd_req_get_url_query_len(req);
    size_t urilen = strlen(req->uri);
    char ch = req->uri[urilen - qlen - 1];
    ch -= 0x30;
    ESP_LOGD(REST_TAG, "Get active plugin for channel %d", ch);
    string res;
    if(ch == 0 || ch == 1){
            res = "{\"id\":\"" + CTAG::AUDIO::SoundProcessorManager::GetStringID(ch) + "\"}";
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, res.c_str());
    return ESP_OK;
}

esp_err_t RestServer::get_params_plugin_get_handler(httpd_req_t *req){
    size_t qlen = httpd_req_get_url_query_len(req);
    size_t urilen = strlen(req->uri);
    char ch = req->uri[urilen - qlen - 1];
    httpd_resp_set_type(req, "application/json");
    ch -= 0x30;
    ESP_LOGD(REST_TAG, "Get plugin params for channel %d", ch);
    if(ch == 0 || ch == 1)
        httpd_resp_sendstr(req, CTAG::AUDIO::SoundProcessorManager::GetCStrJSONActivePluginParams(ch));
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t RestServer::set_active_plugin_get_handler(httpd_req_t *req){
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
    ESP_LOGD(REST_TAG, "Set active plugin for channel %d %s %s", ch, v, s);
    if(ch == 0 || ch == 1)
        CTAG::AUDIO::SoundProcessorManager::SetSoundProcessorChannel(ch, id);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t RestServer::set_plugin_param_get_handler(httpd_req_t *req){
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
    if(strstr(req->uri, "TRIG")){
        httpd_query_key_value(query, "trig", cstrvalue, 128);
        key = "trig";
    }else if(strstr(req->uri, "CV")){
        httpd_query_key_value(query, "cv", cstrvalue, 128);
        key = "cv";
    }else{
        httpd_query_key_value(query, "current", cstrvalue, 128);
        key = "current";
    }
    val = atoi(cstrvalue);
    std::string sid(id);
    ch -= 0x30;
    ESP_LOGD(REST_TAG, "Setting chan %d param %s key %s value %d", ch, id, key.c_str(), val);
    if(ch == 0 || ch == 1)
        CTAG::AUDIO::SoundProcessorManager::SetChannelParamValue(ch, sid, key, val);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t RestServer::get_presets_get_handler(httpd_req_t *req){
    char query[128];
    size_t qlen = httpd_req_get_url_query_len(req);
    size_t urilen = strlen(req->uri);
    httpd_req_get_url_query_str(req, query, 128);
    char ch = req->uri[urilen - qlen - 1];
    httpd_resp_set_type(req, "application/json");
    ch -= 0x30;
    ESP_LOGD(REST_TAG, "Querying presets for channel %d", ch);
    if(ch == 0 || ch == 1)
        httpd_resp_sendstr(req, CTAG::AUDIO::SoundProcessorManager::GetCStrJSONGetPresets(ch));
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t RestServer::save_preset_get_handler(httpd_req_t *req){
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
    ESP_LOGD(REST_TAG, "Store preset for channel %s %d", req->uri, ch);
    if(ch == 0 || ch == 1)
        CTAG::AUDIO::SoundProcessorManager::ChannelSavePreset(ch, string(name), atoi(number));
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t RestServer::load_preset_get_handler(httpd_req_t *req) {
    char query[128];
    char number[16];
    size_t qlen = httpd_req_get_url_query_len(req);
    size_t urilen = strlen(req->uri);
    httpd_req_get_url_query_str(req, query, 128);
    httpd_query_key_value(query, "number", number, 16);
    char ch = req->uri[urilen - qlen - 2];
    ch -= 0x30;
    ESP_LOGD("HTTPD", "Load preset for channel %s %c", req->uri, ch);
    if(ch == 0 || ch == 1)
        CTAG::AUDIO::SoundProcessorManager::ChannelLoadPreset(ch, atoi(number));
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t RestServer::StartRestServer()
{
    const char *base_path = "/spiffs/www\0";
    rest_server_context_t *rest_context = (rest_server_context_t*)calloc(1, sizeof(rest_server_context_t));
    assert(rest_context);
    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.core_id = 0;
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.task_priority = tskIDLE_PRIORITY + 3;
    config.max_uri_handlers = 13;
    config.stack_size = 8192;
    /*
    config.max_open_sockets   = 10;
    config.max_resp_headers   = 10;
    config.recv_wait_timeout   = 20;
    config.send_wait_timeout = 20;
    config.backlog_conn       = 10;
*/
    //config.lru_purge_enable   = false;
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
    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    if(httpd_start(&server, &config) != ESP_OK)
        return ESP_FAIL;
  /*
    httpd_uri_t system_info_get_uri = {
        .uri = "/api/v1/system/info",
        .method = HTTP_GET,
        .handler = system_info_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &system_info_get_uri);

    httpd_uri_t light_brightness_post_uri = {
        .uri = "/api/v1/light/brightness",
        .method = HTTP_POST,
        .handler = light_brightness_post_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &light_brightness_post_uri);
*/
    /* URI handler for getting available sound processors */
    httpd_uri_t get_plugins_get_uri = {
        .uri = "/api/v1/getPlugins",
        .method = HTTP_GET,
        .handler = &RestServer::get_plugins_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &get_plugins_get_uri);

    /* get currently activated plugin */
    httpd_uri_t get_active_plugin_get_uri = {
        .uri = "/api/v1/getActivePlugin*",
        .method = HTTP_GET,
        .handler = &RestServer::get_active_plugin_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &get_active_plugin_get_uri);

    /* get plugin params of active plugin */
    httpd_uri_t get_params_plugin_get_uri = {
        .uri = "/api/v1/getPluginParams*",
        .method = HTTP_GET,
        .handler = &RestServer::get_params_plugin_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &get_params_plugin_get_uri);

    /* set active plugin */
    httpd_uri_t set_active_plugin_get_uri = {
            .uri = "/api/v1/setActivePlugin*",
            .method = HTTP_GET,
            .handler = &RestServer::set_active_plugin_get_handler,
            .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &set_active_plugin_get_uri);

    /* set a plugin param*/
    httpd_uri_t set_plugin_param_get_uri = {
            .uri = "/api/v1/setPluginParam*",
            .method = HTTP_GET,
            .handler = &RestServer::set_plugin_param_get_handler,
            .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &set_plugin_param_get_uri);

    /* get all presets */
    httpd_uri_t get_presets_get_uri = {
            .uri = "/api/v1/getPresets*",
            .method = HTTP_GET,
            .handler = &RestServer::get_presets_get_handler,
            .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &get_presets_get_uri);

    /* get all presets */
    httpd_uri_t get_preset_json_get_uri = {
            .uri = "/api/v1/getAllPresetData*",
            .method = HTTP_GET,
            .handler = &RestServer::get_preset_json_handler,
            .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &get_preset_json_get_uri);

    /* save a preset */
    httpd_uri_t save_preset_get_uri = {
            .uri = "/api/v1/savePreset*",
            .method = HTTP_GET,
            .handler = &RestServer::save_preset_get_handler,
            .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &save_preset_get_uri);

    /* load a preset */
    httpd_uri_t load_preset_get_uri = {
            .uri = "/api/v1/loadPreset*",
            .method = HTTP_GET,
            .handler = &RestServer::load_preset_get_handler,
            .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &load_preset_get_uri);

    /* get configuration*/
    httpd_uri_t get_configuration_get_uri = {
            .uri = "/api/v1/getConfiguration",
            .method = HTTP_GET,
            .handler = &RestServer::get_configuration_get_handler,
            .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &get_configuration_get_uri);

    /* reboot with and without calibration request */
    httpd_uri_t reboot_handler_get_uri = {
            .uri = "/api/v1/reboot",
            .method = HTTP_GET,
            .handler = &RestServer::reboot_handler,
            .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &reboot_handler_get_uri);

    /* set configuration */
    httpd_uri_t set_configuration_post_uri = {
            .uri = "/api/v1/setConfiguration",
            .method = HTTP_POST,
            .handler = &RestServer::set_configuration_post_handler,
            .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &set_configuration_post_uri);

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

esp_err_t RestServer::set_configuration_post_handler(httpd_req_t *req) {
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    char *content = (char*)heap_caps_malloc(req->content_len + 1, MALLOC_CAP_SPIRAM);
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
    CTAG::AUDIO::SoundProcessorManager::SetConfigurationFromJSON(string(content));
    free(content);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t RestServer::get_configuration_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, CTAG::AUDIO::SoundProcessorManager::GetCStrJSONConfiguration());
    return ESP_OK;
}

esp_err_t RestServer::get_preset_json_handler(httpd_req_t *req) {
    char query[128];
    char pluginID[64];
    httpd_req_get_url_query_str(req, query, 128);
    httpd_resp_set_type(req, "application/json");
    char *pLastSlash = strrchr(req->uri, '/');
    if(pLastSlash){
        strcpy(pluginID, pLastSlash + 1);
        ESP_LOGE(REST_TAG, "Sending all preset data of plugin %s as JSON", pluginID);
        httpd_resp_sendstr(req, "moin\0");
        //httpd_resp_sendstr(req, CTAG::AUDIO::SoundProcessorManager::GetCStrJSONAllPresetData(ch));
    }
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t RestServer::reboot_handler(httpd_req_t *req) {
    char query[128];
    char calibration[16];
    httpd_req_get_url_query_str(req, query, 128);
    httpd_query_key_value(query, "calibration", calibration, 16);
    int doCal = atoi(calibration);
    ESP_LOGW(REST_TAG, "Reboot requested with calibration = %d", doCal);
    if(doCal) CTAG::CAL::Calibration::RequestCalibrationOnReboot();
    httpd_resp_send_chunk(req, NULL, 0);
    esp_restart();
    return ESP_OK;
}
