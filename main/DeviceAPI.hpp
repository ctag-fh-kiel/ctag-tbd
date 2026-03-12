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

#pragma once

#include "esp_http_server.h"
#include "esp_err.h"

namespace CTAG {
    namespace REST {
        /**
         * Device API v2 — configuration, IO capabilities, favorites, system.
         *
         * Replaces 6 individual v1 handlers with 2 (GET + POST).
         *
         * GET  /api/v2/device?action=...
         *   getConfig      — device configuration JSON
         *   getIOCaps      — IO capabilities (triggers, CVs, versions)
         *   getFavorites   — all stored favorites
         *   getAll         — bulk: config + ioCaps + favorites
         *
         * POST /api/v2/device?action=...
         *   setConfig          — body: JSON configuration
         *   reboot             — restart the device
         *   storeFavorite&id=N — body: JSON favorite data
         *   recallFavorite&id=N — activate favorite N
         */
        class DeviceAPI final {
        public:
            DeviceAPI() = delete;
            static esp_err_t device_get_handler(httpd_req_t *req);
            static esp_err_t device_post_handler(httpd_req_t *req);
        };
    }
}
