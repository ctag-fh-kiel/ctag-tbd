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

#include "MacroAPI.hpp"
#include "SPManager.hpp"
#include <cstring>
#include <string>
#include <vector>
#include <inttypes.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "helpers/ctagSampleRom.hpp"
#include "MacroTranslator.hpp"

using namespace CTAG::REST;
using namespace rapidjson;
using namespace CTAG::MACROPRESETS;

static const char *MACRO_TAG = "MacroAPI";
static const char *MACRODEFS_DIR  = "/sdcard/data/macrodefinitions";
static const char *PRESETS_DIR    = "/sdcard/data/macrosoundpresets";

static void set_api_headers(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Connection", "close");
}

/** Send a JSON string as HTTP response */
static esp_err_t send_json(httpd_req_t *req, const char *json) {
    set_api_headers(req);
    httpd_resp_set_type(req, "application/json");
    if (json) httpd_resp_sendstr(req, json);
    else httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

/** Send a simple JSON ok response */
static esp_err_t send_ok(httpd_req_t *req) {
    return send_json(req, "{\"ok\":true}");
}

/**
 * Read all .json files in a directory and return them as a rapidjson Array.
 * Each element is the parsed JSON object from that file.
 * Uses SPIRAM for the read buffer to keep internal heap free.
 */
static void read_all_json_in_dir(const char *dirPath, Value &outArray,
                                  Document::AllocatorType &alloc) {
    DIR *dir = opendir(dirPath);
    if (!dir) {
        ESP_LOGW(MACRO_TAG, "Cannot open dir %s", dirPath);
        return;
    }

    char pathBuf[270];
    char *fileBuf = (char *)heap_caps_malloc(8192, MALLOC_CAP_SPIRAM);
    if (!fileBuf) {
        closedir(dir);
        ESP_LOGE(MACRO_TAG, "SPIRAM alloc failed for file buffer");
        return;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        // Only .json files
        size_t nlen = strlen(ent->d_name);
        if (nlen < 6 || strcasecmp(ent->d_name + nlen - 5, ".json") != 0)
            continue;

        snprintf(pathBuf, sizeof(pathBuf) - 1, "%s/%s", dirPath, ent->d_name);
        pathBuf[sizeof(pathBuf) - 1] = '\0';
        FILE *fp = fopen(pathBuf, "r");
        if (!fp) continue;

        FileReadStream is(fp, fileBuf, 8192);
        Document doc;
        doc.ParseStream(is);
        fclose(fp);

        if (doc.HasParseError() || !doc.IsObject()) {
            ESP_LOGW(MACRO_TAG, "Skip bad JSON: %s", ent->d_name);
            continue;
        }

        Value copy(doc, alloc);
        outArray.PushBack(copy, alloc);
    }

    heap_caps_free(fileBuf);
    closedir(dir);
}


// Forward declarations for handlers defined below
static esp_err_t handle_get_trackdefaults(httpd_req_t *req);
static esp_err_t handle_save_trackdefaults(httpd_req_t *req);

/**
 * GET /api/v2/macros — dispatched by ?action= query parameter.
 *
 *   (no action / default) → track status (original behaviour)
 *   ?action=getall        → bulk: { macroDefs:[], soundPresets:[], tracks:[] }
 *
 * The "getall" action replaces dozens of individual config-file fetches with
 * a single HTTP round-trip, keeping total API calls within the ESP32 limit.
 */
esp_err_t MacroAPI::macroapi_get_handler(httpd_req_t *req) {
    ESP_LOGI(MACRO_TAG, "GET Mem free int %d, SPIRAM %d",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    char query[128] = {0};
    char action[32] = {0};
    httpd_req_get_url_query_str(req, query, sizeof(query));
    httpd_query_key_value(query, "action", action, sizeof(action));

    /* ── action=getall ── bulk fetch all macro data in one response ── */
    if (strcmp(action, "getall") == 0) {
        Document resp(kObjectType);
        auto &alloc = resp.GetAllocator();

        // 1) All macro definitions
        Value defs(kArrayType);
        read_all_json_in_dir(MACRODEFS_DIR, defs, alloc);
        resp.AddMember("macroDefs", defs, alloc);

        // 2) All sound presets
        Value presets(kArrayType);
        read_all_json_in_dir(PRESETS_DIR, presets, alloc);
        resp.AddMember("soundPresets", presets, alloc);

        // 3) Current track state
        Document trackDoc(kObjectType);
        MacroTranslator::instance().SerializeStateInto(trackDoc);
        if (trackDoc.HasMember("tracks"))
            resp.AddMember("tracks", trackDoc["tracks"], alloc);

        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        resp.Accept(writer);
        return send_json(req, sb.GetString());
    }

    /* ── action=get_trackdefaults ── read boot-default presets ── */
    if (strcmp(action, "get_trackdefaults") == 0) {
        return handle_get_trackdefaults(req);
    }

    /* ── default: return current track state ── */
    httpd_resp_set_type(req, "application/json");
    Document resp(kObjectType);
    auto &alloc = resp.GetAllocator();

    Document doc2(kObjectType);
    MacroTranslator::instance().SerializeStateInto(doc2);
    resp.AddMember("tracks", doc2["tracks"], alloc);

    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    resp.Accept(writer);
    return send_json(req, sb.GetString());
}


/* RefreshMacros() parses dozens of JSON files and needs ~12 KB+ of stack.
 * The HTTP task only has 8 KB (internal RAM — cannot increase without
 * fragmenting memory and breaking audio DMA).  We offload the work to a
 * short-lived task whose stack is allocated from SPIRAM (abundant). */
static EventGroupHandle_t reloadEventGroup = nullptr;
static const int RELOAD_DONE_BIT = BIT0;

static void reload_task(void *) {
    CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
    CTAG::AUDIO::SoundProcessorManager::RefreshMacros();
    CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
    xEventGroupSetBits(reloadEventGroup, RELOAD_DONE_BIT);
    vTaskDelete(nullptr);
}

static esp_err_t handle_reload(httpd_req_t *req) {
    if (!reloadEventGroup) {
        reloadEventGroup = xEventGroupCreate();
    }
    xEventGroupClearBits(reloadEventGroup, RELOAD_DONE_BIT);

    TaskHandle_t h = nullptr;
    BaseType_t ok = xTaskCreatePinnedToCoreWithCaps(
        reload_task, "reload_macros", 16384, nullptr,
        tskIDLE_PRIORITY + 3, &h, 0, MALLOC_CAP_SPIRAM);
    if (ok != pdPASS) {
        ESP_LOGE(MACRO_TAG, "Failed to create reload task");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "reload task failed");
        return ESP_FAIL;
    }

    /* Wait synchronously so the WebUI gets 200 only after reload is done. */
    xEventGroupWaitBits(reloadEventGroup, RELOAD_DONE_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
    return send_ok(req);
}

static esp_err_t handle_set_track_macro(httpd_req_t *req) {
    char *content = (char *) heap_caps_malloc(req->content_len + 1, MALLOC_CAP_SPIRAM);
    if (!content) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
        return ESP_FAIL;
    }
    int ret = httpd_req_recv(req, content, req->content_len);
    if (ret <= 0) {
        heap_caps_free(content);
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    content[req->content_len] = 0;

    ESP_LOGI(MACRO_TAG, "set_track_macro data: %s", content);
    CTAG::AUDIO::SoundProcessorManager::SetTrackParametersFromJSON(content);
    heap_caps_free(content);

    return send_ok(req);
}

static const char *TRACKDEFAULTS_PATH = "/sdcard/data/trackdefaults.json";

/**
 * GET  ?action=get_trackdefaults
 * Returns the contents of /sdcard/data/trackdefaults.json (or "{}" if missing).
 */
static esp_err_t handle_get_trackdefaults(httpd_req_t *req) {
    FILE *f = fopen(TRACKDEFAULTS_PATH, "r");
    if (!f) {
        ESP_LOGW(MACRO_TAG, "trackdefaults.json not found, returning {}");
        return send_json(req, "{}");
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0 || sz >= 8192) {
        fclose(f);
        return send_json(req, "{}");
    }
    char *buf = (char *)heap_caps_malloc(sz + 1, MALLOC_CAP_SPIRAM);
    if (!buf) {
        fclose(f);
        return send_json(req, "{}");
    }
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    esp_err_t ret2 = send_json(req, buf);
    heap_caps_free(buf);
    return ret2;
}

/**
 * POST ?action=save_trackdefaults
 * Writes the request body (JSON) to /sdcard/data/trackdefaults.json.
 * If the sampleKit field changed, also updates the active sample bank
 * in sample_rom.json and reloads PSRAM so the device is immediately in sync.
 */
static esp_err_t handle_save_trackdefaults(httpd_req_t *req) {
    if (req->content_len == 0 || req->content_len > 8192) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid body size");
        return ESP_FAIL;
    }
    char *content = (char *)heap_caps_malloc(req->content_len + 1, MALLOC_CAP_SPIRAM);
    if (!content) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
        return ESP_FAIL;
    }
    int ret = httpd_req_recv(req, content, req->content_len);
    if (ret <= 0) {
        heap_caps_free(content);
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) httpd_resp_send_408(req);
        return ESP_FAIL;
    }
    content[req->content_len] = '\0';

    // Resolve the kit filename to an index for PSRAM switching
    int kitIndex = -1;
    {
        Document doc;
        doc.Parse(content);
        if (!doc.HasParseError() && doc.IsObject() &&
            doc.HasMember("kit") && doc["kit"].IsString()) {
            std::string kitFile = doc["kit"].GetString();
            kitIndex = CTAG::SP::HELPERS::ctagSampleRom::GetBankIndexFromFileName(kitFile);
        }
    }

    FILE *f = fopen(TRACKDEFAULTS_PATH, "w");
    if (!f) {
        heap_caps_free(content);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Cannot write file");
        return ESP_FAIL;
    }
    fwrite(content, 1, req->content_len, f);
    fclose(f);
    ESP_LOGI(MACRO_TAG, "Saved trackdefaults.json (%d bytes)", req->content_len);
    heap_caps_free(content);

    // Sync PSRAM with the requested kit so the device is ready at next boot
    if (kitIndex >= 0) {
        ESP_LOGI(MACRO_TAG, "Switching active sample bank to %d and reloading PSRAM", kitIndex);
        CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
        CTAG::SP::HELPERS::ctagSampleRom::SetActiveSampleBank(static_cast<uint8_t>(kitIndex));
        CTAG::SP::HELPERS::ctagSampleRom::RefreshDataStructure();
        CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
    }

    return send_ok(req);
}

esp_err_t MacroAPI::macroapi_post_handler(httpd_req_t *req) {
    ESP_LOGI(MACRO_TAG, "POST Mem free int %d, SPIRAM %d",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    size_t qlen = httpd_req_get_url_query_len(req);
    char action[32] = {0};

    char defId[64] = {0};

    if (qlen > 0) {
        char *query = (char *)malloc(qlen + 1);
        httpd_req_get_url_query_str(req, query, qlen + 1);
        httpd_query_key_value(query, "action", action, sizeof(action));
        httpd_query_key_value(query, "id", defId, sizeof(defId));
        free(query);
    }

    if (strcmp(action, "reload") == 0) {
        if (defId[0] != '\0') {
            /* Targeted single-defion reload — lightweight, runs inline */
            ESP_LOGI(MACRO_TAG, "Targeted reload for id=%s", defId);
            CTAG::AUDIO::SoundProcessorManager::RefreshSingleMacro(std::string(defId));
            return send_ok(req);
        }
        return handle_reload(req);
    }
    else if (strcmp(action, "set_track_parameters") == 0) {
        return handle_set_track_macro(req);
    }
    else if (strcmp(action, "update_track") == 0) {
        return handle_set_track_macro(req);
    }
    else if (strcmp(action, "save_trackdefaults") == 0) {
        return handle_save_trackdefaults(req);
    }

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}
