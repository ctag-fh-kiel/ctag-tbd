/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020-2026 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

/*
 * HTTP REST Server — v2 API
 *
 * All domain logic is in dedicated modules:
 *   PluginAPI  (GET/POST /api/v2/plugins)   — sound processor management
 *   DeviceAPI  (GET/POST /api/v2/device)    — config, IO caps, favorites
 *   StorageAPI (GET/POST /api/v2/samples)   — storage & sample management
 *   StorageAPI (GET/POST /api/v2/storage)   — generic storage REST API
 *   MacroAPI   (GET/POST /api/v2/macros)    — macro presets
 *   OtaAPI     (GET/POST /api/v2/ota)       — firmware OTA update
 *
 * This file handles:
 *   - Server configuration (max_uri_handlers = 20, using 13 of 20)
 *   - Static file serving (gzipped assets from SD card)
 *   - URI handler registration for all 4 API domains
 */

#include "PluginAPI.hpp"
#include "DeviceAPI.hpp"
#include "StorageAPI.hpp"
#if CONFIG_TBD_USE_SD_CARD
#include "MacroAPI.hpp"
#endif
#include "OtaAPI.hpp"
#include "RestServer.hpp"
#include <string.h>
#include <fcntl.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "sdkconfig.h"

using namespace CTAG;
using namespace CTAG::REST;

static const char *REST_TAG = "esp-rest";

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

#define CHECK_FILE_EXTENSION(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/* ── Static file serving ──────────────────────────────────────────── */

/**
 * Set HTTP response content type and cache headers by file extension.
 * Static assets get Connection:close to free the socket immediately.
 */
static esp_err_t set_content_type_from_file(httpd_req_t *req,
                                            const char *filepath) {
    const char *type = "text/plain";
    bool is_static_asset = false;

    httpd_resp_set_hdr(req, "Connection", "close");

    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
        httpd_resp_set_hdr(req, "Cache-Control",
            "no-cache, no-store, must-revalidate");
        httpd_resp_set_hdr(req, "Pragma", "no-cache");
        httpd_resp_set_hdr(req, "Expires", "0");
    } else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
        is_static_asset = true;
    } else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
        is_static_asset = true;
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
        is_static_asset = true;
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
        is_static_asset = true;
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "image/svg+xml";
        is_static_asset = true;
    }

    if (is_static_asset) {
        httpd_resp_set_hdr(req, "Cache-Control",
            "public, max-age=2592000, immutable");
    }

    return httpd_resp_set_type(req, type);
}

/**
 * Serve gzipped static files from the SD card.
 * Must be the LAST registered GET handler (wildcard catch-all).
 */
static esp_err_t rest_common_get_handler(httpd_req_t *req) {
    char filepath[FILE_PATH_MAX];

    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    rest_server_context_t *rest_context =
        (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));

    /* Strip query string from URI before constructing file path */
    char uri_path[128];
    strlcpy(uri_path, req->uri, sizeof(uri_path));
    char *query = strchr(uri_path, '?');
    if (query) *query = '\0';

    if (uri_path[strlen(uri_path) - 1] == '/') {
        strlcat(filepath, "/index.html", sizeof(filepath));
    } else {
        strlcat(filepath, uri_path, sizeof(filepath));
    }
    set_content_type_from_file(req, filepath);
    strlcat(filepath, ".gz", sizeof(filepath));

    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        ESP_LOGE(REST_TAG, "Failed to open file: %s", filepath);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
            "Failed to read existing file");
        return ESP_FAIL;
    }

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;
    do {
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            ESP_LOGE(REST_TAG, "Failed to read file: %s", filepath);
        } else if (read_bytes > 0) {
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                httpd_resp_sendstr_chunk(req, NULL);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                    "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);

    close(fd);
    ESP_LOGD(REST_TAG, "File sending complete");
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

/* ── Server startup & handler registration ────────────────────────── */

esp_err_t RestServer::StartRestServer() {
    const char *base_path = "/sdcard/www";
    rest_server_context_t *rest_context =
        (rest_server_context_t *)calloc(1, sizeof(rest_server_context_t));
    assert(rest_context);
    strlcpy(rest_context->base_path, base_path,
        sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.core_id            = 0;
    config.uri_match_fn       = httpd_uri_match_wildcard;
    config.task_priority      = tskIDLE_PRIORITY + 4;
    config.max_uri_handlers   = 20;   // upstream p4_main limit; using 13 of 20
    config.stack_size         = 8192;
    config.max_req_hdr_len    = 1024; // default 512 too small for modern browser headers (causes 431)
    config.recv_wait_timeout  = 10;   // upstream p4_main value — lru_purge handles socket pressure
    config.send_wait_timeout  = 10;
    config.lru_purge_enable   = true;  // Auto-close least-recently-used connections when out of sockets
    // All API responses now use Connection:close to free sockets immediately.
    // Static files also use Connection:close.

    ESP_LOGI(REST_TAG, "Starting HTTP Server (v2 API — 13 of 20 handler slots)");
    if (httpd_start(&server, &config) != ESP_OK)
        return ESP_FAIL;

    /* ── 1. Plugin API  (2 handlers) ── */
    httpd_uri_t plugins_get = {
        .uri      = "/api/v2/plugins",
        .method   = HTTP_GET,
        .handler  = &PluginAPI::plugins_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &plugins_get);

    httpd_uri_t plugins_post = {
        .uri      = "/api/v2/plugins",
        .method   = HTTP_POST,
        .handler  = &PluginAPI::plugins_post_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &plugins_post);

    /* ── 2. Device API  (2 handlers) ── */
    httpd_uri_t device_get = {
        .uri      = "/api/v2/device",
        .method   = HTTP_GET,
        .handler  = &DeviceAPI::device_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &device_get);

    httpd_uri_t device_post = {
        .uri      = "/api/v2/device",
        .method   = HTTP_POST,
        .handler  = &DeviceAPI::device_post_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &device_post);

    /* ── 3. Sample API  (2 handlers) ── */
    httpd_uri_t samples_get = {
        .uri      = "/api/v2/samples*",
        .method   = HTTP_GET,
        .handler  = &StorageAPI::samples_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &samples_get);

    httpd_uri_t samples_post = {
        .uri      = "/api/v2/samples*",
        .method   = HTTP_POST,
        .handler  = &StorageAPI::samples_post_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &samples_post);

    /* ── 4. Macro API  (2 handlers) ── */
#if CONFIG_TBD_USE_SD_CARD
    httpd_uri_t macros_get = {
        .uri      = "/api/v2/macros",
        .method   = HTTP_GET,
        .handler  = &MacroAPI::macroapi_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &macros_get);

    httpd_uri_t macros_post = {
        .uri      = "/api/v2/macros",
        .method   = HTTP_POST,
        .handler  = &MacroAPI::macroapi_post_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &macros_post);
#endif // CONFIG_TBD_USE_SD_CARD

    /* ── 5. OTA API  (2 handlers) ── */
    httpd_uri_t ota_get = {
        .uri      = "/api/v2/ota",
        .method   = HTTP_GET,
        .handler  = &OtaAPI::ota_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &ota_get);

    httpd_uri_t ota_post = {
        .uri      = "/api/v2/ota",
        .method   = HTTP_POST,
        .handler  = &OtaAPI::ota_post_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &ota_post);

    /* ── 6. Generic Storage API  (2 handlers) ── */
    httpd_uri_t storage_get = {
        .uri      = "/api/v2/storage*",
        .method   = HTTP_GET,
        .handler  = &StorageAPI::storage_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &storage_get);

    httpd_uri_t storage_post = {
        .uri      = "/api/v2/storage*",
        .method   = HTTP_POST,
        .handler  = &StorageAPI::storage_post_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &storage_post);

    /* ── 7. Static files — must be LAST (wildcard catch-all) ── */
    httpd_uri_t static_get = {
        .uri      = "/*",
        .method   = HTTP_GET,
        .handler  = rest_common_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &static_get);

    ESP_LOGI(REST_TAG, "All 13 URI handlers registered successfully");
    return ESP_OK;
}
