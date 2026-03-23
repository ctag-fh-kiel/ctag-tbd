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

#include "PluginAPI.hpp"
#include "SPManager.hpp"
#include "Favorites.hpp"
#include "sdkconfig.h"
#if CONFIG_TBD_USE_RP2350
#include "SpiAPI.hpp"
#endif
#include <cstring>
#include <string>
#include "esp_log.h"
#include "esp_heap_caps.h"

using namespace CTAG::REST;
using namespace std;

static const char *TAG = "PluginAPI";

/* ── Helpers ──────────────────────────────────────────────────────── */

static void set_api_headers(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Connection", "close");
}

static esp_err_t send_json(httpd_req_t *req, const char *json) {
    set_api_headers(req);
    httpd_resp_set_type(req, "application/json");
    if (json) httpd_resp_sendstr(req, json);
    else httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

/** Send a SPIRAM-allocated JSON string, then free it */
static esp_err_t send_safe_json(httpd_req_t *req, char *json) {
    set_api_headers(req);
    httpd_resp_set_type(req, "application/json");
    if (json) {
        httpd_resp_sendstr(req, json);
        free(json);
    } else {
        httpd_resp_send(req, NULL, 0);
    }
    return ESP_OK;
}

static esp_err_t send_ok(httpd_req_t *req) {
    return send_json(req, "{\"ok\":true}");
}

/** Parse ch=0|1 from query string.  Returns -1 on error. */
static int get_channel(const char *query) {
    char ch[4] = {0};
    if (httpd_query_key_value(query, "ch", ch, sizeof(ch)) == ESP_OK) {
        int c = atoi(ch);
        if (c == 0 || c == 1) return c;
    }
    return -1;
}

/* ── GET action handlers ──────────────────────────────────────────── */

/** action=list — all available sound processors */
static esp_err_t handle_list(httpd_req_t *req) {
    return send_safe_json(req,
        CTAG::AUDIO::SoundProcessorManager::GetSafeJSONSoundProcessors());
}

/** action=getActive&ch=N — active plugin ID for channel */
static esp_err_t handle_get_active(httpd_req_t *req, const char *query) {
    int ch = get_channel(query);
    if (ch < 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "need ch=0|1");
        return ESP_FAIL;
    }
    string res = "{\"id\":\"" +
        CTAG::AUDIO::SoundProcessorManager::GetStringID(ch) + "\"}";
    return send_json(req, res.c_str());
}

/** action=getParams&ch=N — parameter specs for active plugin */
static esp_err_t handle_get_params(httpd_req_t *req, const char *query) {
    int ch = get_channel(query);
    if (ch < 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "need ch=0|1");
        return ESP_FAIL;
    }
    return send_safe_json(req,
        CTAG::AUDIO::SoundProcessorManager::GetSafeJSONActivePluginParams(ch));
}

/** action=getPresets&ch=N — preset names for channel */
static esp_err_t handle_get_presets(httpd_req_t *req, const char *query) {
    int ch = get_channel(query);
    if (ch < 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "need ch=0|1");
        return ESP_FAIL;
    }
    return send_safe_json(req,
        CTAG::AUDIO::SoundProcessorManager::GetSafeJSONGetPresets(ch));
}

/** action=getPresetData&id=X — all preset JSON for a sound processor */
static esp_err_t handle_get_preset_data(httpd_req_t *req, const char *query) {
    char id[64] = {0};
    if (httpd_query_key_value(query, "id", id, sizeof(id)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "need id=");
        return ESP_FAIL;
    }
    return send_safe_json(req,
        CTAG::AUDIO::SoundProcessorManager::GetSafeJSONSoundProcessorPresets(
            string(id)));
}

/**
 * action=getAll — bulk response with everything the plugin UI needs at boot.
 * Uses chunked transfer to avoid building the entire response in memory.
 *
 * Response shape:
 * {
 *   "plugins": [...],
 *   "active":  { "0": "id0", "1": "id1" },
 *   "params":  { "0": {...}, "1": {...} },
 *   "presets": { "0": [...], "1": [...] }
 * }
 */
static esp_err_t handle_get_all(httpd_req_t *req) {
    set_api_headers(req);
    httpd_resp_set_type(req, "application/json");

    #define CHUNK(s) httpd_resp_send_chunk(req, (s), strlen(s))

    CHUNK("{\"plugins\":");
    char *plugins =
        CTAG::AUDIO::SoundProcessorManager::GetSafeJSONSoundProcessors();
    CHUNK(plugins ? plugins : "[]");
    free(plugins);

    CHUNK(",\"active\":{\"0\":\"");
    string id0 = CTAG::AUDIO::SoundProcessorManager::GetStringID(0);
    CHUNK(id0.c_str());
    CHUNK("\",\"1\":\"");
    string id1 = CTAG::AUDIO::SoundProcessorManager::GetStringID(1);
    CHUNK(id1.c_str());
    CHUNK("\"}");

    CHUNK(",\"params\":{\"0\":");
    char *p0 =
        CTAG::AUDIO::SoundProcessorManager::GetSafeJSONActivePluginParams(0);
    CHUNK(p0 ? p0 : "null");
    free(p0);
    CHUNK(",\"1\":");
    char *p1 =
        CTAG::AUDIO::SoundProcessorManager::GetSafeJSONActivePluginParams(1);
    CHUNK(p1 ? p1 : "null");
    free(p1);
    CHUNK("}");

    CHUNK(",\"presets\":{\"0\":");
    char *pr0 =
        CTAG::AUDIO::SoundProcessorManager::GetSafeJSONGetPresets(0);
    CHUNK(pr0 ? pr0 : "[]");
    free(pr0);
    CHUNK(",\"1\":");
    char *pr1 =
        CTAG::AUDIO::SoundProcessorManager::GetSafeJSONGetPresets(1);
    CHUNK(pr1 ? pr1 : "[]");
    free(pr1);
    CHUNK("}}");

    httpd_resp_send_chunk(req, NULL, 0);  // terminate

    #undef CHUNK
    return ESP_OK;
}

/* ── POST action handlers ─────────────────────────────────────────── */

/** action=setActive&ch=N&id=X — activate a sound processor on a channel */
static esp_err_t handle_set_active(httpd_req_t *req, const char *query) {
    int ch = get_channel(query);
    char id[128] = {0};
    httpd_query_key_value(query, "id", id, sizeof(id));
    if (ch < 0 || id[0] == '\0') {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "need ch=0|1 and id=");
        return ESP_FAIL;
    }

    // Block ALL HTTP plugin switching when RP2350 app has locked plugins.
    // SPI-originated switches (from RP2350 itself) are unaffected.
#if CONFIG_TBD_USE_RP2350
    if (CTAG::SPIAPI::SpiAPI::IsPluginLocked()) {
        ESP_LOGW(TAG, "Plugin switch blocked — RP2350 app '%s' has locked plugins (ch=%d id=%s)",
                 CTAG::SPIAPI::SpiAPI::GetRP2350AppId().c_str(), ch, id);
        set_api_headers(req);
        httpd_resp_set_status(req, "423 Locked");
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, "{\"error\":\"plugin_locked\",\"reason\":\"RP2350 app has locked plugin switching\"}");
        return ESP_OK;
    }
#endif

    ESP_LOGI(TAG, "setActive ch=%d id=%s", ch, id);
    CTAG::AUDIO::SoundProcessorManager::SetSoundProcessorChannel(ch, string(id));
    CTAG::FAV::Favorites::DeactivateFavorite();
    return send_ok(req);
}

/**
 * action=setParam&ch=N&id=X&key=K&val=V — set a single parameter value.
 *
 * key must be one of: "current", "trig", "cv"
 */
static esp_err_t handle_set_param(httpd_req_t *req, const char *query) {
    int ch = get_channel(query);
    char id[128] = {0};
    char key[16] = {0};
    char val[16] = {0};
    httpd_query_key_value(query, "id", id, sizeof(id));
    httpd_query_key_value(query, "key", key, sizeof(key));
    httpd_query_key_value(query, "val", val, sizeof(val));

    if (ch < 0 || id[0] == '\0' || key[0] == '\0') {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "need ch, id, key, val");
        return ESP_FAIL;
    }

    CTAG::AUDIO::SoundProcessorManager::SetChannelParamValue(
        ch, string(id), string(key), atoi(val));

    /* Minimal response — this is called for every slider drag */
    set_api_headers(req);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

/** action=savePreset&ch=N&name=X&number=Y */
static esp_err_t handle_save_preset(httpd_req_t *req, const char *query) {
    int ch = get_channel(query);
    char name[128] = {0};
    char number[16] = {0};
    httpd_query_key_value(query, "name", name, sizeof(name));
    httpd_query_key_value(query, "number", number, sizeof(number));
    if (ch < 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "need ch=0|1");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "savePreset ch=%d name=%s number=%s", ch, name, number);
    CTAG::AUDIO::SoundProcessorManager::ChannelSavePreset(
        ch, string(name), atoi(number));
    return send_ok(req);
}

/** action=loadPreset&ch=N&number=Y */
static esp_err_t handle_load_preset(httpd_req_t *req, const char *query) {
    int ch = get_channel(query);
    char number[16] = {0};
    httpd_query_key_value(query, "number", number, sizeof(number));
    if (ch < 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "need ch=0|1");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "loadPreset ch=%d number=%s", ch, number);
    CTAG::AUDIO::SoundProcessorManager::ChannelLoadPreset(ch, atoi(number));
    CTAG::FAV::Favorites::DeactivateFavorite();
    return send_ok(req);
}

/** action=setPresetData&id=X  (body = JSON preset blob) */
static esp_err_t handle_set_preset_data(httpd_req_t *req, const char *query) {
    char id[64] = {0};
    httpd_query_key_value(query, "id", id, sizeof(id));
    if (id[0] == '\0') {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "need id=");
        return ESP_FAIL;
    }

    char *content = (char *)heap_caps_calloc(
        1, req->content_len + 1, MALLOC_CAP_SPIRAM);
    if (!content) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "alloc");
        return ESP_FAIL;
    }

    char *ptr = content;
    int remaining = req->content_len;
    while (remaining > 0) {
        uint32_t chunk = remaining > 4096 ? 4096 : remaining;
        int recv = httpd_req_recv(req, ptr, chunk);
        if (recv <= 0) {
            if (recv == HTTPD_SOCK_ERR_TIMEOUT) {
                httpd_resp_send_408(req);
            } else {
                httpd_resp_send_500(req);
            }
            heap_caps_free(content);
            return ESP_FAIL;
        }
        ptr += recv;
        remaining -= recv;
    }
    content[req->content_len] = 0;
    ESP_LOGI(TAG, "setPresetData id=%s len=%d", id, req->content_len);

    /* Respond before the potentially slow disk write */
    string response = "{\"id\":\"" + string(id) + "\"}";
    send_json(req, response.c_str());

    CTAG::AUDIO::SoundProcessorManager::SetCStrJSONSoundProcessorPreset(
        id, content);
    heap_caps_free(content);
    return ESP_OK;
}

/* ══════════════════════════════════════════════════════════════════════
 *  Main dispatch entry points
 * ══════════════════════════════════════════════════════════════════════ */

esp_err_t PluginAPI::plugins_get_handler(httpd_req_t *req) {
    ESP_LOGD(TAG, "GET %s  int=%d spiram=%d", req->uri,
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    char query[256] = {0};
    char action[32] = {0};
    httpd_req_get_url_query_str(req, query, sizeof(query));
    httpd_query_key_value(query, "action", action, sizeof(action));

    if (strcmp(action, "list") == 0)          return handle_list(req);
    if (strcmp(action, "getActive") == 0)     return handle_get_active(req, query);
    if (strcmp(action, "getParams") == 0)     return handle_get_params(req, query);
    if (strcmp(action, "getPresets") == 0)    return handle_get_presets(req, query);
    if (strcmp(action, "getPresetData") == 0) return handle_get_preset_data(req, query);
    if (strcmp(action, "getAll") == 0)        return handle_get_all(req);

    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "unknown action");
    return ESP_FAIL;
}

esp_err_t PluginAPI::plugins_post_handler(httpd_req_t *req) {
    ESP_LOGD(TAG, "POST %s  int=%d spiram=%d", req->uri,
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    char query[256] = {0};
    char action[32] = {0};
    httpd_req_get_url_query_str(req, query, sizeof(query));
    httpd_query_key_value(query, "action", action, sizeof(action));

    if (strcmp(action, "setActive") == 0)      return handle_set_active(req, query);
    if (strcmp(action, "setParam") == 0)       return handle_set_param(req, query);
    if (strcmp(action, "savePreset") == 0)     return handle_save_preset(req, query);
    if (strcmp(action, "loadPreset") == 0)     return handle_load_preset(req, query);
    if (strcmp(action, "setPresetData") == 0)  return handle_set_preset_data(req, query);

    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "unknown action");
    return ESP_FAIL;
}
