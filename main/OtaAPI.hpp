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
         * OTA Firmware Update API v2.
         *
         * POST /api/v2/ota
         *   Receives a firmware .bin file, writes it to the next OTA
         *   partition (the one NOT currently running), sets it as boot
         *   partition.  The client should reboot the device after a
         *   successful response.
         *
         * GET /api/v2/ota
         *   Returns JSON with current OTA status:
         *   { "running": "ota_0", "next": "ota_1", "maxSize": 5242880 }
         */
        class OtaAPI final {
        public:
            OtaAPI() = delete;
            static esp_err_t ota_get_handler(httpd_req_t *req);
            static esp_err_t ota_post_handler(httpd_req_t *req);
        };
    }
}
