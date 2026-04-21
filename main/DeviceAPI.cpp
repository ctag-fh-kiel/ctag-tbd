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

#include "DeviceAPI.hpp"
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

static const char *TAG = "DeviceAPI";

/*
 * IOCapabilities.hpp declares `string const s(...)` at whatever scope
 * it's included in.  Include once at file scope so both handle_get_iocaps
 * and handle_get_all can reference it.
 */
#include "IOCapabilities.hpp"

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

/**
 * send_safe_json — send a SPIRAM-allocated JSON string, then free it.
 * Use with GetSafeJSON*() which returns heap_caps_malloc'd copies.
 */
static esp_err_t send_safe_json(httpd_req_t *req, char *json) {
    esp_err_t r = send_json(req, json);
    free(json);   // safe even if json==NULL
    return r;
}

static esp_err_t send_ok(httpd_req_t *req) {
    return send_json(req, "{\"ok\":true}");
}

/* ── GET action handlers ──────────────────────────────────────────── */

/** action=getConfig — device configuration */
static esp_err_t handle_get_config(httpd_req_t *req) {
    return send_safe_json(req,
        CTAG::AUDIO::SoundProcessorManager::GetSafeJSONConfiguration());
}

/** action=getIOCaps — IO capabilities (triggers, CVs, versions) */
static esp_err_t handle_get_iocaps(httpd_req_t *req) {
    set_api_headers(req);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, s.c_str());
    return ESP_OK;
}

/** action=getFavorites — all stored favorites */
static esp_err_t handle_get_favorites(httpd_req_t *req) {
    string favs = CTAG::FAV::Favorites::GetAllFavorites();
    return send_json(req, favs.c_str());
}

/** action=getAudioHealth — lock errors, CPU overruns, memory stats */
static esp_err_t handle_get_audio_health(httpd_req_t *req) {
    string health = CTAG::AUDIO::SoundProcessorManager::GetAudioHealthJSON();
    return send_json(req, health.c_str());
}

/** action=getAppInfo — active RP2350 app identity + capability flags */
static esp_err_t handle_get_app_info(httpd_req_t *req) {
#if CONFIG_TBD_USE_RP2350
    const string &appId = CTAG::SPIAPI::SpiAPI::GetRP2350AppId();
    bool locked = CTAG::SPIAPI::SpiAPI::IsPluginLocked();
    bool redirectSamples = CTAG::SPIAPI::SpiAPI::ShouldRedirectSamples();
    const string &picoVer = CTAG::SPIAPI::SpiAPI::GetPicoVersion();
#else
    const string appId = "none";
    bool locked = false;
    bool redirectSamples = false;
    const string picoVer = "";
#endif
    string json = "{\"rp2350_app\":\"" + appId
        + "\",\"plugin_lock\":" + (locked ? "true" : "false")
        + ",\"redirect_samples\":" + (redirectSamples ? "true" : "false")
        + ",\"pico_version\":\"" + picoVer + "\""
        + "}";
    return send_json(req, json.c_str());
}

/** action=resetAudioHealth — zero the lock error & slow process counters */
static esp_err_t handle_reset_audio_health(httpd_req_t *req) {
    CTAG::AUDIO::SoundProcessorManager::ResetAudioHealthCounters();
    return send_ok(req);
}

/** action=triggerScreenshot — request a screenshot capture on the Pico */
static esp_err_t handle_trigger_screenshot(httpd_req_t *req) {
#if CONFIG_TBD_USE_RP2350
    CTAG::AUDIO::SoundProcessorManager::screenshotRequestCounter++;
    return send_ok(req);
#else
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "no RP2350");
    return ESP_FAIL;
#endif
}

/** action=injectButton&button=XX&event=YY — inject a button press on the Pico */
static esp_err_t handle_inject_button(httpd_req_t *req, const char *query) {
#if CONFIG_TBD_USE_RP2350
    char btnStr[8] = {0};
    char evtStr[8] = {0};
    httpd_query_key_value(query, "button", btnStr, sizeof(btnStr));
    httpd_query_key_value(query, "event", evtStr, sizeof(evtStr));
    uint8_t button = (uint8_t)atoi(btnStr);
    uint8_t event = (uint8_t)atoi(evtStr);
    if (button == 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "invalid button");
        return ESP_FAIL;
    }
    CTAG::AUDIO::SoundProcessorManager::injectedButton.store(button);
    CTAG::AUDIO::SoundProcessorManager::injectedButtonEvent.store(event);
    return send_ok(req);
#else
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "no RP2350");
    return ESP_FAIL;
#endif
}

/**
 * action=getAll — bulk response: config + ioCaps + favorites.
 * Uses chunked transfer to avoid a huge allocation.
 *
 * Response shape:
 * {
 *   "config":    {...},
 *   "ioCaps":    {...},
 *   "favorites": [...]
 * }
 */
static esp_err_t handle_get_all(httpd_req_t *req) {
    set_api_headers(req);
    httpd_resp_set_type(req, "application/json");

    #define CHUNK(s) httpd_resp_send_chunk(req, (s), strlen(s))

    CHUNK("{\"config\":");
    char *config =
        CTAG::AUDIO::SoundProcessorManager::GetSafeJSONConfiguration();
    CHUNK(config ? config : "{}");
    free(config);

    CHUNK(",\"ioCaps\":");
    CHUNK(s.c_str());

    CHUNK(",\"favorites\":");
    string favs = CTAG::FAV::Favorites::GetAllFavorites();
    CHUNK(favs.c_str());

    CHUNK("}");
    httpd_resp_send_chunk(req, NULL, 0);  // terminate

    #undef CHUNK
    return ESP_OK;
}

/* ── POST action handlers ─────────────────────────────────────────── */

/** action=setConfig  (body = JSON configuration) */
static esp_err_t handle_set_config(httpd_req_t *req) {
    char *content = (char *)heap_caps_malloc(
        req->content_len + 1, MALLOC_CAP_SPIRAM);
    if (!content) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "alloc");
        return ESP_FAIL;
    }
    int ret = httpd_req_recv(req, content, req->content_len);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) httpd_resp_send_408(req);
        heap_caps_free(content);
        return ESP_FAIL;
    }
    content[req->content_len] = 0;
    CTAG::AUDIO::SoundProcessorManager::SetConfigurationFromJSON(string(content));
    heap_caps_free(content);
    return send_ok(req);
}

/** action=reboot — restart the device */
static esp_err_t handle_reboot(httpd_req_t *req) {
    ESP_LOGW(TAG, "Reboot requested");
    send_ok(req);
    esp_restart();
    return ESP_OK;  // unreachable
}

/** action=storeFavorite&id=N  (body = JSON favorite data) */
static esp_err_t handle_store_favorite(httpd_req_t *req, const char *query) {
    char idStr[4] = {0};
    httpd_query_key_value(query, "id", idStr, sizeof(idStr));
    int id = atoi(idStr);

    char *content = (char *)heap_caps_malloc(
        req->content_len + 1, MALLOC_CAP_SPIRAM);
    if (!content) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "alloc");
        return ESP_FAIL;
    }
    int ret = httpd_req_recv(req, content, req->content_len);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) httpd_resp_send_408(req);
        heap_caps_free(content);
        return ESP_FAIL;
    }
    content[req->content_len] = 0;
    ESP_LOGI(TAG, "storeFavorite id=%d", id);
    CTAG::FAV::Favorites::StoreFavorite(id, string(content));
    heap_caps_free(content);
    return send_ok(req);
}

/** action=recallFavorite&id=N */
static esp_err_t handle_recall_favorite(httpd_req_t *req, const char *query) {
    // Block favorite recall when RP2350 app has locked plugin switching.
#if CONFIG_TBD_USE_RP2350
    if (CTAG::SPIAPI::SpiAPI::IsPluginLocked()) {
        ESP_LOGW(TAG, "Favorite recall blocked — RP2350 app '%s' has locked plugins",
                 CTAG::SPIAPI::SpiAPI::GetRP2350AppId().c_str());
        set_api_headers(req);
        httpd_resp_set_status(req, "423 Locked");
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, "{\"error\":\"plugin_locked\",\"reason\":\"RP2350 app has locked plugin switching\"}");
        return ESP_OK;
    }
#endif

    char idStr[4] = {0};
    httpd_query_key_value(query, "id", idStr, sizeof(idStr));
    int id = atoi(idStr);
    ESP_LOGI(TAG, "recallFavorite id=%d", id);
    CTAG::FAV::Favorites::ActivateFavorite(id);
    return send_ok(req);
}

/* ══════════════════════════════════════════════════════════════════════
 *  Main dispatch entry points
 * ══════════════════════════════════════════════════════════════════════ */

esp_err_t DeviceAPI::device_get_handler(httpd_req_t *req) {
    ESP_LOGD(TAG, "GET %s", req->uri);

    char query[128] = {0};
    char action[32] = {0};
    httpd_req_get_url_query_str(req, query, sizeof(query));
    httpd_query_key_value(query, "action", action, sizeof(action));

    if (strcmp(action, "getConfig") == 0)      return handle_get_config(req);
    if (strcmp(action, "getIOCaps") == 0)      return handle_get_iocaps(req);
    if (strcmp(action, "getFavorites") == 0)   return handle_get_favorites(req);
    if (strcmp(action, "getAll") == 0)         return handle_get_all(req);
    if (strcmp(action, "getAudioHealth") == 0) return handle_get_audio_health(req);
    if (strcmp(action, "getAppInfo") == 0)     return handle_get_app_info(req);
    if (strcmp(action, "triggerScreenshot") == 0) return handle_trigger_screenshot(req);
    if (strcmp(action, "injectButton") == 0)   return handle_inject_button(req, query);

    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "unknown action");
    return ESP_FAIL;
}

esp_err_t DeviceAPI::device_post_handler(httpd_req_t *req) {
    ESP_LOGD(TAG, "POST %s", req->uri);

    char query[128] = {0};
    char action[32] = {0};
    httpd_req_get_url_query_str(req, query, sizeof(query));
    httpd_query_key_value(query, "action", action, sizeof(action));

    if (strcmp(action, "setConfig") == 0)        return handle_set_config(req);
    if (strcmp(action, "reboot") == 0)            return handle_reboot(req);
    if (strcmp(action, "storeFavorite") == 0)     return handle_store_favorite(req, query);
    if (strcmp(action, "recallFavorite") == 0)    return handle_recall_favorite(req, query);
    if (strcmp(action, "resetAudioHealth") == 0)  return handle_reset_audio_health(req);

    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "unknown action");
    return ESP_FAIL;
}
